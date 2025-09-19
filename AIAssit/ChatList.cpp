#include "ChatList.h"
#include <QPropertyAnimation>
#include <QTimer>

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
	setMinimumWidth(200);
	setMaximumWidth(280);

	// 创建主布局
	mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(8);
	mainLayout->setContentsMargins(12, 12, 12, 12);

	// 创建新对话按钮
	btnNewConversation = new QPushButton(this);
	btnNewConversation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	btnNewConversation->setMinimumHeight(84);
	btnNewConversation->setText(QStringLiteral("新对话"));
	btnNewConversation->setObjectName("btnNewConversation");

	// 创建对话列表
	m_conversationList = new QListWidget(this);
	m_conversationList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	m_conversationList->setObjectName("m_conversationList");
	m_conversationList->setContextMenuPolicy(Qt::CustomContextMenu);

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
	item->setData(Qt::UserRole, id);
	item->setFlags(item->flags() | Qt::ItemIsSelectable);
	m_conversationList->addItem(item);
}

void ChatList::insertConversationItem(int index, const QString& text, const QString& id)
{
	QListWidgetItem* item = new QListWidgetItem();
	item->setText(text);
	item->setData(Qt::UserRole, id);
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
		if (item && item->data(Qt::UserRole).toString() == id) 
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
		if (item && item->data(Qt::UserRole).toString() == id) 
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
		return current->data(Qt::UserRole).toString();
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
		QString conversationId = current->data(Qt::UserRole).toString();
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

	QAction* renameAction = contextMenu.addAction(QStringLiteral("重命名"));
	QAction* deleteAction = contextMenu.addAction(QStringLiteral("删除对话"));

	connect(renameAction, &QAction::triggered, this, &ChatList::renameRequested);
	connect(deleteAction, &QAction::triggered, this, &ChatList::deleteRequested);

	contextMenu.exec(m_conversationList->mapToGlobal(pos));

	emit contextMenuRequested(pos);
}