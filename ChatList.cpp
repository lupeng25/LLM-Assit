#include "ChatList.h"
#include <QPropertyAnimation>
#include <QTimer>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QStyleOptionViewItem>

namespace
{
	class ChatListDelegate : public QStyledItemDelegate
	{
	public:
		explicit ChatListDelegate(QObject* parent = nullptr)
			: QStyledItemDelegate(parent)
		{
		}

		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
		{
			Q_UNUSED(index);
			const int baseHeight = 60;
			return QSize(option.rect.width(), baseHeight);
		}

		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
		{
			if (!painter)
			{
				return;
			}

			painter->save();

			QStyleOptionViewItem opt(option);
			initStyleOption(&opt, index);

			const QRectF cardRect = opt.rect.adjusted(6, 4, -6, -4);
			const bool isSelected = opt.state & QStyle::State_Selected;
			const bool isHovered = opt.state & QStyle::State_MouseOver;

			QColor baseColor(248, 250, 252);
			QColor borderColor(226, 232, 240, 180);
			QColor shadowColor(15, 23, 42, 24);

			if (isSelected)
			{
				baseColor = QColor(37, 99, 235);
				borderColor = QColor(30, 64, 175, 220);
				shadowColor = QColor(37, 99, 235, 55);
			}
			else if (isHovered)
			{
				baseColor = QColor(241, 245, 249);
				borderColor = QColor(191, 219, 254, 200);
				shadowColor = QColor(59, 130, 246, 40);
			}

			// 绘制阴影
			if (shadowColor.alpha() > 0)
			{
				QPainterPath shadowPath;
				QRectF shadowRect = cardRect.adjusted(0, 2, 0, 8);
				shadowPath.addRoundedRect(shadowRect, 18, 18);
				painter->setPen(Qt::NoPen);
				painter->setBrush(shadowColor);
				painter->drawPath(shadowPath);
			}

			// 绘制卡片背景
			QPainterPath cardPath;
			cardPath.addRoundedRect(cardRect, 18, 18);
			painter->setPen(Qt::NoPen);
			painter->setBrush(baseColor);
			painter->drawPath(cardPath);

			// 绘制描边
			painter->setPen(QPen(borderColor, 1));
			painter->setBrush(Qt::NoBrush);
			painter->drawPath(cardPath);

			// 文本内容
			const QString title = index.data(Qt::DisplayRole).toString();
			const QString timestamp = index.data(ChatList::TimestampRole).toString();
			const bool hasTimestamp = !timestamp.trimmed().isEmpty();

			const QColor titleColor = isSelected ? QColor(248, 250, 252)
				: QColor(15, 23, 42);
			const QColor timeColor = isSelected ? QColor(226, 232, 240, 240)
				: QColor(100, 116, 139);

			QRectF textRect = cardRect.adjusted(18, 14, -18, -14);
			const int timeWidth = hasTimestamp ? opt.fontMetrics.horizontalAdvance(timestamp) + 6 : 0;
			const QRectF timeRect = hasTimestamp
				? QRectF(textRect.right() - timeWidth, textRect.top(), timeWidth, 20)
				: QRectF();

			QFont titleFont = opt.font;
			titleFont.setPointSizeF(titleFont.pointSizeF() + 0.4);
			titleFont.setWeight(QFont::DemiBold);

			if (hasTimestamp)
			{
				painter->setPen(timeColor);
				painter->setFont(opt.font);
				painter->drawText(timeRect, Qt::AlignRight | Qt::AlignVCenter, timestamp);
			}

			QRectF titleRect = textRect;
			if (hasTimestamp)
			{
				titleRect.setRight(timeRect.left() - 12);
			}

			painter->setPen(titleColor);
			painter->setFont(titleFont);
			painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, title);

			painter->restore();
		}
	};
}

ChatList::ChatList(QWidget *parent)
	: QWidget(parent)
	, mainLayout(nullptr)
	, btnNewConversation(nullptr)
	, m_conversationList(nullptr)
{
	setupUI();
	connectSignals();
}

ChatList::~ChatList()
{
	// 清理资源
	m_conversationList->clear();
}

void ChatList::setupUI()
{
	// 设置固定宽度
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	setMinimumWidth(220);
	setMaximumWidth(320);

	// 创建主布局
	mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(12);
	mainLayout->setContentsMargins(16, 16, 16, 16);

	// 创建新对话按钮
	btnNewConversation = new QPushButton(this);
	btnNewConversation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	btnNewConversation->setMinimumHeight(64);
	btnNewConversation->setText(tr("New Conversation"));
	btnNewConversation->setObjectName("btnNewConversation");
	btnNewConversation->setCursor(Qt::PointingHandCursor);
	btnNewConversation->setStyleSheet(R"(
		QPushButton {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
				stop:0 #3b82f6, stop:1 #2563eb);
			border-radius: 18px;
			border: none;
			padding: 14px 18px;
			color: #F8FAFC;
			font-size: 15px;
			font-weight: 600;
			letter-spacing: 0.5px;
		}
		QPushButton:hover {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
				stop:0 #60a5fa, stop:1 #3b82f6);
		}
		QPushButton:pressed {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
				stop:0 #2563eb, stop:1 #1d4ed8);
		}
	)");

	// 创建对话列表
	m_conversationList = new QListWidget(this);
	m_conversationList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	m_conversationList->setObjectName("m_conversationList");
	m_conversationList->setContextMenuPolicy(Qt::CustomContextMenu);
	m_conversationList->setMouseTracking(true);
	m_conversationList->setSpacing(14);
	m_conversationList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	m_conversationList->setFrameShape(QFrame::NoFrame);
	m_conversationList->setSelectionMode(QAbstractItemView::SingleSelection);
	m_conversationList->setStyleSheet(R"(
		QListWidget {
			background: transparent;
			outline: none;
		}
		QListWidget::item {
			margin: 0px;
		}
		QListWidget::indicator {
			width: 0px;
			height: 0px;
		}
	)");
	m_conversationList->setItemDelegate(new ChatListDelegate(m_conversationList));

	// 添加到布局
	mainLayout->addWidget(btnNewConversation);
	mainLayout->addWidget(m_conversationList);
}

void ChatList::connectSignals()
{
	// 连接新对话按钮点击信号
	connect(btnNewConversation, &QPushButton::clicked,
		this, &ChatList::onNewConversationClicked);

	// 连接列表选择改变信号
	connect(m_conversationList, &QListWidget::currentItemChanged,
		this, &ChatList::onConversationSelectionChanged);

	// 连接右键菜单信号
	connect(m_conversationList, &QListWidget::customContextMenuRequested,
		this, &ChatList::showContextMenu);
}

void ChatList::addConversationItem(const QString& text, const QString& id)
{
	QListWidgetItem* item = new QListWidgetItem();
	item->setText(text);
	item->setData(IdRole, id);
	item->setFlags(item->flags() | Qt::ItemIsSelectable);
	m_conversationList->addItem(item);
}

void ChatList::insertConversationItem(int index, const QString& text, const QString& id)
{
	QListWidgetItem* item = new QListWidgetItem();
	item->setText(text);
	item->setData(IdRole, id);
	item->setFlags(item->flags() | Qt::ItemIsSelectable);
	m_conversationList->insertItem(index, item);

	// 添加创建动画效果
	QTimer::singleShot(10, [this, item]() {
		QPropertyAnimation* animation = new QPropertyAnimation();
		animation->setTargetObject(m_conversationList);
		animation->setPropertyName("geometry");
		animation->setDuration(200);
		animation->setEasingCurve(QEasingCurve::OutCubic);
		animation->start(QAbstractAnimation::DeleteWhenStopped);
	});
}

void ChatList::removeConversationItem(const QString& id)
{
	for (int i = 0; i < m_conversationList->count(); ++i)
	{
		QListWidgetItem* item = m_conversationList->item(i);
		if (item && item->data(IdRole).toString() == id)
		{
			delete m_conversationList->takeItem(i);
			break;
		}
	}
}

void ChatList::clearConversations()
{
	m_conversationList->clear();
}

void ChatList::setCurrentConversation(const QString& id)
{
	for (int i = 0; i < m_conversationList->count(); ++i)
	{
		QListWidgetItem* item = m_conversationList->item(i);
		if (item && item->data(IdRole).toString() == id)
		{
			m_conversationList->setCurrentItem(item);
			break;
		}
	}
}

QString ChatList::getCurrentConversationId() const
{
	QListWidgetItem* current = m_conversationList->currentItem();
	if (current)
	{
		return current->data(IdRole).toString();
	}
	return QString();
}

QListWidgetItem* ChatList::getCurrentItem() const
{
	return m_conversationList->currentItem();
}

void ChatList::setCurrentItemText(const QString& text)
{
	QListWidgetItem* current = m_conversationList->currentItem();
	if (current)
	{
		current->setText(text);
	}
}

void ChatList::onNewConversationClicked()
{
	emit newConversationRequested();
}

void ChatList::onConversationSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	emit conversationChanged(current, previous);

	if (current)
	{
		QString conversationId = current->data(IdRole).toString();
		emit conversationSelected(conversationId);
	}
}

void ChatList::showContextMenu(const QPoint& pos)
{
	QListWidgetItem* item = m_conversationList->itemAt(pos);
	if (!item) return;

	QMenu contextMenu(this);
	contextMenu.setStyleSheet(R"(
        QMenu {
            background: rgba(255, 255, 255, 0.95);
            border: 1px solid rgba(102, 126, 234, 0.3);
            border-radius: 8px;
            padding: 4px;
        }
        QMenu::item {
            padding: 8px 16px;
            border-radius: 4px;
            color: #555;
        }
        QMenu::item:selected {
            background: rgba(102, 126, 234, 0.15);
            color: #667eea;
        }
    )");

	QAction* renameAction = contextMenu.addAction(tr("Rename"));
	QAction* deleteAction = contextMenu.addAction(tr("Delete Chat"));

	connect(renameAction, &QAction::triggered, this, &ChatList::renameRequested);
	connect(deleteAction, &QAction::triggered, this, &ChatList::deleteRequested);

	contextMenu.exec(m_conversationList->mapToGlobal(pos));

	emit contextMenuRequested(pos);
}

QListWidgetItem* ChatList::findItemById(const QString& id) const
{
	for (int i = 0; i < m_conversationList->count(); ++i)
	{
		QListWidgetItem* item = m_conversationList->item(i);
		if (item && item->data(IdRole).toString() == id)
		{
			return item;
		}
	}
	return nullptr;
}

void ChatList::setConversationTimestamp(const QString& id, const QString& timestamp)
{
	if (auto* item = findItemById(id))
	{
		item->setData(TimestampRole, timestamp);
		m_conversationList->update(m_conversationList->visualItemRect(item));
	}
}
 