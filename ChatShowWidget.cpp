#include "ChatShowWidget.h"
#include <QIcon>
#include <QSizePolicy>
#include <QTimer>
#include <QStackedWidget>
#include <QDebug>
#include <QWheelEvent>
#include <QScrollBar>
#include <QRegion>

namespace
{
	void applyCircularMask(QPushButton* button)
	{
		if (!button)
		{
			return;
		}
		const QSize size = button->size();
		if (!size.isValid())
		{
			return;
		}
		const int diameter = qMin(size.width(), size.height());
		const int offsetX = (size.width() - diameter) / 2;
		const int offsetY = (size.height() - diameter) / 2;
		const QRect circleRect(offsetX, offsetY, diameter, diameter);
		button->setMask(QRegion(circleRect, QRegion::Ellipse));
	}
}
ChatShowWidget::ChatShowWidget(QWidget *parent)
	: QWidget(parent)
	, mainLayout(nullptr)
	, headerWidget(nullptr)
	, headerLayout(nullptr)
	, chatFrame(nullptr)
	, frameLayout(nullptr)
	, toggleButton(nullptr)
	, chatTitle(nullptr)
	, listWgChatFrame(nullptr)
	, stackedWidget(nullptr)
	, emptyStateWidget(nullptr)
	, emptyIconLabel(nullptr)
	, emptyTextLabel(nullptr)
{
	setupUI();
	applyStyles();
	QTimer::singleShot(0, this, [this]()
	{
		connectSignals();
	});
}

ChatShowWidget::~ChatShowWidget()
{
}

void ChatShowWidget::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	if (chatFrame && UpButton && DownButton)
	{
		// 计算按钮位置 - 右下角
		int frameWidth = chatFrame->width();
		int frameHeight = chatFrame->height();
		int buttonWidth = 30;
		int buttonHeight = 30;
		int margin = 15; // 距离边缘的距离
		 // Up按钮位置
		UpButton->move(frameWidth - buttonWidth - margin,
			frameHeight - (buttonHeight * 2 + 8) - margin);
		// Down按钮位置
		DownButton->move(frameWidth - buttonWidth - margin,
			frameHeight - buttonHeight - margin);
		// 确保按钮在最上层
		UpButton->raise();
		DownButton->raise();
		listWgChatFrame->setSpacing(2);
		onScrollPositionChanged();
		applyCircularMask(UpButton);
		applyCircularMask(DownButton);
	}
}

void ChatShowWidget::setupUI()
{
	// 创建主布局
	mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(10);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	// 创建头部组件
	headerWidget = new QWidget(this);
	headerWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	headerWidget->setMinimumHeight(60);
	headerWidget->setObjectName("chatHeader");

	headerLayout = new QHBoxLayout(headerWidget);
	headerLayout->setContentsMargins(20, 15, 20, 15);

	// 切换按钮
	toggleButton = new QPushButton(headerWidget);
	toggleButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	toggleButton->setFixedSize(20, 20);
	toggleButton->setObjectName("toggleButton");
	toggleButton->setText("");
	setToggleIcon(QIcon(":/QtWidgetsApp/ICONs/icon_open.png"));

	// 标题
	chatTitle = new QLabel(headerWidget);
	chatTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	chatTitle->setText(tr("GKG AI Assit"));
	chatTitle->setObjectName("chatTitle");
	QFont titleFont;
	titleFont.setFamily("Microsoft YaHei UI");
	titleFont.setPointSize(16);
	titleFont.setBold(true);
	chatTitle->setFont(titleFont);

	// 添加到头部布局
	headerLayout->addWidget(toggleButton);
	headerLayout->addWidget(chatTitle);

	// 创建聊天框架
	chatFrame = new QFrame(this);
	chatFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	chatFrame->setFrameShape(QFrame::StyledPanel);
	chatFrame->setFrameShadow(QFrame::Raised);
	chatFrame->setObjectName("frame");

	// 恢复原来的垂直布局
	frameLayout = new QVBoxLayout(chatFrame);
	frameLayout->setSpacing(0);
	frameLayout->setContentsMargins(8, 8, 8, 8);

	// 创建堆叠widget来切换显示聊天列表和空状态
	stackedWidget = new QStackedWidget(chatFrame);
	stackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// 聊天消息列表
	listWgChatFrame = new QListWidget();
	listWgChatFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	listWgChatFrame->setObjectName("listWgChatFrame");
	listWgChatFrame->setSpacing(2);

	// 优化聊天框架设置
	listWgChatFrame->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	listWgChatFrame->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	listWgChatFrame->setResizeMode(QListView::Adjust);
	// 增加滚动步长以提升滚动速度（特别是对于包含代码的消息）
	listWgChatFrame->verticalScrollBar()->setSingleStep(40);
	// 增加页面步长以提升滚动速度
	listWgChatFrame->verticalScrollBar()->setPageStep(200);
	// 优化视图设置
	listWgChatFrame->setViewMode(QListView::ListMode);
	listWgChatFrame->setUniformItemSizes(false); // 由于消息高度不同，不能使用uniform
	// 为列表安装事件过滤器以优化滚轮滚动
	listWgChatFrame->installEventFilter(this);
	// 设置空状态组件
	setupEmptyStateWidget();
	// 将聊天列表和空状态添加到堆叠widget
	stackedWidget->addWidget(listWgChatFrame);  // index 0
	stackedWidget->addWidget(emptyStateWidget); // index 1
	// 默认显示空状态
	stackedWidget->setCurrentIndex(1);
	frameLayout->addWidget(stackedWidget);
	// 创建Up/Down按钮 - 作为chatFrame的子组件，使用绝对定位
	UpButton = new QPushButton(chatFrame);
	UpButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	UpButton->setMinimumSize(30, 30);
	UpButton->setMaximumSize(30, 30);
	UpButton->setToolTip("Up");
	UpButton->setObjectName("UpButton");
	UpButton->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_up.png"));

	DownButton = new QPushButton(chatFrame);
	DownButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	DownButton->setMinimumSize(30, 30);
	DownButton->setMaximumSize(30, 30);
	DownButton->setToolTip("Down");
	DownButton->setObjectName("DownButton");
	DownButton->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_down.png"));
	applyCircularMask(UpButton);
	applyCircularMask(DownButton);

	// 添加到主布局
	mainLayout->addWidget(headerWidget);
	mainLayout->addWidget(chatFrame);
}

void ChatShowWidget::setupEmptyStateWidget()
{
	// 创建空状态容器
	emptyStateWidget = new QWidget();
	emptyStateWidget->setObjectName("emptyStateWidget");

	// 创建垂直布局
	QVBoxLayout* emptyLayout = new QVBoxLayout(emptyStateWidget);
	emptyLayout->setAlignment(Qt::AlignCenter);
	emptyLayout->setSpacing(15);
	emptyLayout->setContentsMargins(40, 40, 40, 40);

	// 创建图标容器，确保居中
	QWidget* iconContainer = new QWidget();
	iconContainer->setFixedSize(100, 100);
	QHBoxLayout* iconLayout = new QHBoxLayout(iconContainer);
	iconLayout->setContentsMargins(0, 0, 0, 0);
	iconLayout->setAlignment(Qt::AlignCenter);

	// 创建图标标签
	emptyIconLabel = new QLabel();
	emptyIconLabel->setObjectName("emptyIconLabel");
	emptyIconLabel->setAlignment(Qt::AlignCenter);
	emptyIconLabel->setFixedSize(80, 80);

	// 设置图标 - 创建现代化的机器人图标
	QPixmap iconPixmap(80, 80);
	iconPixmap.fill(Qt::transparent);
	QPainter painter(&iconPixmap);
	painter.setRenderHint(QPainter::Antialiasing);

	// 绘制机器人头部（向右移动5像素）
	painter.setBrush(QBrush(QColor(75, 164, 242))); // 蓝色
	painter.setPen(QPen(QColor(60, 140, 220), 2));
	painter.drawRoundedRect(20, 20, 50, 45, 12, 12);

	// 绘制眼睛（向右移动5像素）
	painter.setBrush(QBrush(Qt::white));
	painter.setPen(Qt::NoPen);
	painter.drawEllipse(30, 32, 8, 8);
	painter.drawEllipse(52, 32, 8, 8);

	// 绘制眼珠（向右移动5像素）
	painter.setBrush(QBrush(QColor(60, 140, 220)));
	painter.drawEllipse(32, 34, 4, 4);
	painter.drawEllipse(54, 34, 4, 4);

	// 绘制向上的嘴巴（微笑，向右移动5像素）
	painter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
	painter.drawArc(37, 48, 16, 10, 0, -180 * 16);

	// 绘制触角（向右移动5像素）
	painter.setPen(QPen(QColor(60, 140, 220), 2, Qt::SolidLine, Qt::RoundCap));
	painter.drawLine(40, 20, 40, 12);
	painter.drawLine(50, 20, 50, 12);
	painter.setBrush(QBrush(QColor(255, 165, 0))); // 橙色小球
	painter.setPen(Qt::NoPen);
	painter.drawEllipse(38, 10, 4, 4);
	painter.drawEllipse(48, 10, 4, 4);

	emptyIconLabel->setPixmap(iconPixmap);

	// 将图标添加到容器
	iconLayout->addWidget(emptyIconLabel);

	// 创建文本标签
	emptyTextLabel = new QLabel();
	emptyTextLabel->setText(tr("Let me help you!"));
	emptyTextLabel->setObjectName("emptyTextLabel");
	emptyTextLabel->setAlignment(Qt::AlignCenter);
	emptyTextLabel->setWordWrap(true);

	QFont emptyFont;
	emptyFont.setFamily("Microsoft YaHei UI");
	emptyFont.setPointSize(18);
	emptyFont.setWeight(QFont::Medium);
	emptyTextLabel->setFont(emptyFont);

	// 添加到布局
	emptyLayout->addStretch(2);
	emptyLayout->addWidget(iconContainer);
	emptyLayout->addWidget(emptyTextLabel);
	emptyLayout->addStretch(3);
}

void ChatShowWidget::updateEmptyState()
{
	if (!listWgChatFrame || !stackedWidget) return;
	if (listWgChatFrame->count() == 0) 
	{
		showEmptyState();
	}
	else {
		hideEmptyState();
	}
}

void ChatShowWidget::showEmptyState()
{
	if (stackedWidget)
	{
		stackedWidget->setCurrentIndex(1); // 显示空状态
		// 隐藏上下按钮
		if (UpButton) UpButton->hide();
		if (DownButton) DownButton->hide();
	}
}

void ChatShowWidget::hideEmptyState()
{
	if (stackedWidget)
	{
		stackedWidget->setCurrentIndex(0); // 显示聊天列表	
		// 显示上下按钮（根据滚动状态）
		onScrollPositionChanged();
	}
}

void ChatShowWidget::connectSignals()
{
	if (toggleButton)
	{
		connect(toggleButton, &QPushButton::clicked,
			this, &ChatShowWidget::onToggleButtonClicked,
			Qt::UniqueConnection);
	}

	connect(UpButton, &QPushButton::clicked, this, &ChatShowWidget::onUpButtonClicked, Qt::UniqueConnection);
	connect(DownButton, &QPushButton::clicked, this, &ChatShowWidget::onDownButtonClicked, Qt::UniqueConnection);
	if (listWgChatFrame && listWgChatFrame->verticalScrollBar()) 
	{
		connect(listWgChatFrame->verticalScrollBar(), &QScrollBar::valueChanged,
			this, &ChatShowWidget::onScrollPositionChanged, Qt::UniqueConnection);
	}
}

void ChatShowWidget::setChatTitle(const QString& title)
{
	if (chatTitle)
		chatTitle->setText(title);
}

void ChatShowWidget::setToggleIcon(const QIcon& icon)
{
	if (toggleButton)
	{
		toggleButton->setIcon(icon);
		QSize buttonSize = toggleButton->size();
		QSize iconSize(buttonSize.width(), buttonSize.height());
		toggleButton->setIconSize(iconSize);
	}
}

void ChatShowWidget::onToggleButtonClicked()
{
	emit toggleButtonClicked();
}


void ChatShowWidget::onUpButtonClicked()
{
	listWgChatFrame->scrollToTop();
}

void ChatShowWidget::onDownButtonClicked()
{
	listWgChatFrame->scrollToBottom();
}

void ChatShowWidget::onScrollPositionChanged()
{
	if (!listWgChatFrame || !UpButton || !DownButton) 
	{
		return;
	}

	// 如果当前显示的是空状态，不显示上下按钮
	if (stackedWidget && stackedWidget->currentIndex() == 1)
	{
		UpButton->setVisible(false);
		DownButton->setVisible(false);
		return;
	}

	QScrollBar* scrollBar = listWgChatFrame->verticalScrollBar();
	if (!scrollBar) 
	{
		return;
	}

	int currentValue = scrollBar->value();
	int maxValue = scrollBar->maximum();
	int minValue = scrollBar->minimum();

	// 判断滚动位置并控制按钮显示
	if (currentValue >= maxValue - 5)
	{
		// 在最底部，只显示Up按钮
		UpButton->setVisible(true);
		DownButton->setVisible(false);
	}
	else if (currentValue <= minValue + 5) 
	{
		// 在最顶部，只显示Down按钮
		UpButton->setVisible(false);
		DownButton->setVisible(true);
	}
	else 
	{
		// 在中间位置，显示两个按钮
		UpButton->setVisible(true);
		DownButton->setVisible(true);
	}
}

bool ChatShowWidget::eventFilter(QObject* obj, QEvent* event)
{
	// 优化滚轮滚动性能
	if (obj == listWgChatFrame && event->type() == QEvent::Wheel)
	{
		QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
		QScrollBar* scrollBar = listWgChatFrame->verticalScrollBar();
		
		if (scrollBar && scrollBar->isVisible())
		{
			// 计算加速滚动：对于包含代码的消息，使用更大的滚动步长
			int delta = wheelEvent->angleDelta().y();
			int step = qAbs(delta) / 8; // 标准步长
			
			// 如果滚动幅度较大，增加滚动速度（加速滚动）
			if (qAbs(delta) > 120)
			{
				step = step * 2; // 加速2倍
			}
			
			// 应用滚动
			int currentValue = scrollBar->value();
			int newValue = currentValue - (delta > 0 ? step : -step);
			scrollBar->setValue(newValue);
			
			// 阻止默认滚轮事件处理，使用我们的优化版本
			return true;
		}
	}
	
	return QWidget::eventFilter(obj, event);
}

void ChatShowWidget::applyStyles()
{
	const QString style = QStringLiteral(R"(/* =================== 头部区域样式 =================== */
QWidget#chatHeader {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(255, 255, 255, 0.96), stop:1 rgba(241, 245, 249, 0.94));
    border: none;
    border-radius: 14px 14px 0 0;
    border-bottom: 1px solid rgba(226, 232, 240, 0.75);
    padding: 0;
}

QLabel#chatTitle {
    color: #0f172a;
    font-weight: 700;
    font-size: 19px;
    padding: 0 18px;
    background: transparent;
}

QPushButton#toggleButton {
    background-color: rgba(253, 253, 253, 0.1);
    border: 1px solid rgba(37, 99, 235, 0.35);
    border-radius: 10px;
    padding: 4px;
    min-width: 20px;
    min-height: 20px;
    max-width: 20px;
    max-height: 20px;
    color: #2563eb;
}

QPushButton#toggleButton:hover {
    background-color: rgba(37, 99, 235, 0.18);
    border-color: rgba(37, 99, 235, 0.55);
}

QPushButton#toggleButton:pressed {
    background-color: rgba(37, 99, 235, 0.28);
    border-color: rgba(37, 99, 235, 0.65);
}

QFrame#frame {
    background: rgba(255, 255, 255, 0.92);
    border-radius: 0 0 14px 14px;
    border: 1px solid rgba(203, 213, 225, 0.6);
}

QWidget#emptyStateWidget,
QLabel#emptyIconLabel {
    background: transparent;
    border: none;
}

QLabel#emptyTextLabel {
    color: #475569;
    background: transparent;
    border: none;
    font-size: 20px;
    font-weight: 600;
}

QPushButton#UpButton,
QPushButton#DownButton {
    background-color: rgba(253, 253, 253, 0.98);
    border: none;
    border-radius: 999px;
    margin: 3px;
    min-width: 30px;
    min-height: 30px;
    max-width: 30px;
    max-height: 30px;
    color: #1d4ed8;
}

QPushButton#UpButton:hover,
QPushButton#DownButton:hover {
    background-color: rgba(37, 99, 235, 0.24);
}

QPushButton#UpButton:pressed,
QPushButton#DownButton:pressed {
    background-color: rgba(37, 99, 235, 0.35);
}

QPushButton#UpButton:focus,
QPushButton#DownButton:focus {
    outline: none;
})");

	setStyleSheet(style);
}