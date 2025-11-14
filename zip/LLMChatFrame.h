#pragma once
#include <QAbstractTextDocumentLayout>
#include <QRegularExpression>
#include <QTextDocument>
#include <QApplication>
#include <QPushButton> 
#include <QListWidget>
#include <QClipboard>
#include <QScrollBar>
#include <QWidget>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QUuid>
#include <memory>
#include "include\cmark\cmark.h"
#include "SyntaxHighlighter.h"
class QPaintEvent;
class QPainter;
class QEvent;
class QLabel;
class QMovie;
class LLMChatFrame : public QWidget
{
	Q_OBJECT
public:
	explicit LLMChatFrame(QWidget *parent = nullptr);
	~LLMChatFrame();
	// 用户类型枚举
	enum User_Type
	{
		User_System,    // 系统消息
		User_Owner,     // AI消息
		User_Customer,  // 用户消息
		User_Time,      // 时间标签
	};
	// 布局常量结构
	struct LayoutConstants
	{
		static constexpr int MIN_HEIGHT = 36;              // 最小高度
		static constexpr int ICON_SIZE = 44;               // 图标大小
		static constexpr int ICON_SPACING = 18;            // 图标间距
		static constexpr int ICON_BORDER_WIDTH = 0;        // 图标边框宽度
		static constexpr int ICON_MARGIN = 2;              // 图标边距
		static constexpr int ICON_RING_MARGIN = 6;         // 图标环边距
		static constexpr int TRIANGLE_WIDTH = 10;          // 三角形宽度
		static constexpr int FRAME_MARGIN = 24;            // 框架边距
		static constexpr int TEXT_PADDING = 16;            // 文本内边距
		static constexpr int BUTTON_HEIGHT = 30;           // 按钮高度
		static constexpr int BUTTON_SPACING = 12;          // 按钮间距
		static constexpr int EXTRA_HEIGHT = 80;            // 额外高度
		static constexpr int BORDER_RADIUS = 18;           // 边框圆角
		static constexpr int SHADOW_OFFSET = 4;            // 阴影偏移
		static constexpr int Icon_Loading = 16;            // 加载图标大小
		static constexpr int TIME_HEIGHT = 28;             // 时间标签高度
		static constexpr int TIME_MARGIN = 6;              // 时间标签边距
		static constexpr int TIME_HORIZONTAL_PADDING = 12; // 时间标签水平内边距
		static constexpr int TIME_VERTICAL_PADDING = 4;    // 时间标签垂直内边距
		static constexpr int ExButton_Height = 15;         // 扩展按钮高度
		static constexpr int ExButton_Spacing = 10;        // 扩展按钮间距
	};
	// 配色方案结构
	struct ColorScheme
	{
		static const QColor REASONING_BACKGROUND; // 推理背景色
		static const QColor REASONING_BORDER;     // 推理边框色
		static const QColor ANSWER_BACKGROUND;    // 回答背景色
		static const QColor ANSWER_BORDER;        // 回答边框色
		static const QColor USER_BACKGROUND;      // 用户消息背景色
		static const QColor TIME_TEXT;            // 时间文本颜色
		static const QColor SHADOW_COLOR;         // 阴影颜色
		static const QColor ICON_RING_OUTER;      // 图标外环颜色
		static const QColor ICON_RING_INNER;      // 图标内环颜色
		static const QColor TIME_BACKGROUND;      // 时间背景色
	};
	//消息设置接口
	// 设置消息文本
	void setText(QString text, QString time, QSize allSize, User_Type userType);
	// 设置带推理的消息文本
	void setTextWithReason(const QString& reasoning, const QString& answer, QString time, QSize allSize, User_Type userType);
	// 设置消息发送成功状态
	void setTextSuccess();
	// 设置用户类型
	void setUserType(User_Type auth) { this->m_UserType = auth; };
	// 设置重要性标记
	void setImportant(bool important);
	// 获取重要性标记
	bool isImportant() const { return m_messageData.isImportant; }
	// 设置用户笔记
	void setUserNote(const QString& note);
	// 获取用户笔记
	const QString& userNote() const { return m_messageData.userNote; }
	//框计算
	// 计算单条消息的尺寸
	QSize fontRect(const QString& str);
	// 计算带推理消息的尺寸
	QSize fontRect(const QString& reasoning, const QString& answer);
	//文本处理接口
	// 追加文本内容
	void appendText(const QString& delta);
	// 将Markdown转换为HTML
	QString markdownToHtml(const QString &markdown);
	//数据获取
	// 获取消息HTML文本
	const QString& getText() const { return m_messageData.msg; }
	// 获取原始消息文本
	const QString& getRawText() const { return m_messageData.rawMsg; }
	// 获取推理文本
	const QString& getReasoningText() const { return m_messageData.reasoningText; }
	// 获取原始推理文本
	const QString& getReasonRawText() const { return m_messageData.rawReasoningMsg; }
	// 获取时间
	const QString& getTime() const { return m_messageData.time; }
	// 获取当前时间
	const QString& getCurTime() const { return m_messageData.curTime; }
	// 获取气泡ID
	const QString& getBubbleID() const { return m_messageData.uniqueID; }
	// 获取尺寸
	const QSize& getSize() const { return m_layoutData.allSize; }
	// 获取用户类型
	const User_Type& getUserType() const { return m_UserType; }
	//状态改变
	// 标记流式传输结束
	void ChangeStream() { m_state.isStreamEnd = true; }
	// 改变接受状态
	void ChangeAccpetStatus() { m_state.isReasoning = false; m_state.answerHeaderInserted = false; }
	// 是否正在发送
	bool isSending() const { return m_state.isSending; }
	// 设置气泡ID
	void setBubbleID(QString id) { m_messageData.uniqueID = id; };
	// 重置以便重用
	void resetForReuse();
	// 准备删除
	void prepareForDeletion();
	// 构建纯文本导出内容
	QString buildPlainExport() const;
	// 构建Markdown格式导出内容
	QString buildMarkdownExport() const;
	// 构建HTML格式导出内容
	QString buildHtmlExport() const;
	// 将HTML转换为纯文本
	QString htmlToPlainText(const QString& html) const;
	// 导出文本到文件
	bool exportTextToFile(const QString& content, const QString& dialogTitle, const QString& filter, const QString& suffix);
	// 将气泡导出为图片
	bool exportBubbleAsImage();
	// 构建默认文件名
	QString buildDefaultFileName(const QString& suffix) const;
	// 复制纯文本到剪贴板
	void copyToClipboardPlain(bool reasoningSection);
	// 复制Markdown格式到剪贴板
	void copyToClipboardMarkdown(bool reasoningSection);
	// 复制HTML格式到剪贴板
	void copyToClipboardHtml(bool reasoningSection);
signals:
	// 复制思考内容点击信号
	void copyThinkingClicked();
	// 复制回答内容点击信号
	void copyAnswerClicked();
	// 重新生成点击信号
	void regenerateClicked(QString msg);
	// 气泡笔记改变信号
	void bubbleNoteChanged(const QString& bubbleId, const QString& note);
	// 气泡重要性切换信号
	void bubbleImportantToggled(const QString& bubbleId, bool isImportant);
protected:
	// 事件处理
	bool event(QEvent* event) override;
	// 绘制事件
	void paintEvent(QPaintEvent *event) override;
	// 大小改变事件
	void resizeEvent(QResizeEvent *event) override;
	// 鼠标按下事件
	void mousePressEvent(QMouseEvent *event) override;
	// 鼠标释放事件
	void mouseReleaseEvent(QMouseEvent *event) override;
	// 上下文菜单事件
	void contextMenuEvent(QContextMenuEvent *event) override;
	// 鼠标进入事件
	void enterEvent(QEvent* event) override;
	// 鼠标离开事件
	void leaveEvent(QEvent* event) override;
	// 事件过滤器
	bool eventFilter(QObject* watched, QEvent* event) override;
	private slots:
	// 复制思考内容按钮点击处理
	void onCopyThinkingClicked();
	// 复制回答内容按钮点击处理
	void onCopyAnswerClicked();
	// 重新生成按钮点击处理
	void onRegenerateClicked();
private:
	// 初始化头像图片
	void initTalkPic();
	// 初始化按钮
	void initButtons();
	// 初始化资源
	void initRescource();
	// 计算布局
	void calculateLayout();
	// 更新按钮位置
	void updateButtonsPosition();
	// 更新按钮可见性
	void updateButtonsVisibility();
	// 绘制时间标签
	void drawTimeLabel(QPainter& painter);
	// 绘制用户消息
	void drawCustomerMessage(QPainter& painter);
	// 绘制AI消息
	void drawOwnerMessage(QPainter& painter);
	// 绘制时间消息
	void drawTimeMessage(QPainter& painter);
	// 获取实际字符串尺寸（单条消息）
	QSize getRealString(QString src);
	// 获取实际字符串尺寸（带推理消息）
	QSize getRealString(const QString& reasoning, const QString& answer);
	// 设置文本文档
	void setTextDocs(QTextDocument& docText, const QString& src, const int& iWidth, const QString& defaultColor = QString());
	// 更新按钮悬停状态
	void updateButtonHoverState(QPushButton* button, bool hovered);
	// 内容改变后刷新布局
	void refreshLayoutAfterContentChange();
	// 绘制重要性徽章
	void drawImportanceBadge(QPainter& painter, const QRect& bubbleRect);
	// 绘制笔记徽章
	void drawNoteBadge(QPainter& painter, const QRect& bubbleRect);
	// 应用笔记工具提示
	void applyNoteToolTip();
	// 处理笔记请求
	void handleNoteRequested();
	// 切换重要性
	void toggleImportant();
	// 计算时间额外高度
	int computeTimeExtraHeight() const;
	// 布局单条消息
	void layoutSingleMessage(const QSize& contentSize, int timeExtraHeight);
	// 布局带推理消息
	void layoutReasoningMessage(const QSize& reasoningSize, const QSize& answerSize, int timeExtraHeight);
	// 计算额外高度
	int computeAdditionalHeight() const;
	struct MessageData
	{
		QString msg;
		QString rawMsg;
		QString reasoningText;
		QString rawReasoningMsg;
		QString time;
		QString curTime;
		QString uniqueID;
		bool isImportant = false;
		QString userNote;
	} m_messageData;
	struct LayoutData
	{
		QSize allSize;
		int iFrameWidth = 0;
		int iTextWidth = 0;
		int iSpaceWidth = 0;
		int iLineHeight = 0;
		struct Rects
		{
			QRect iconLeft;
			QRect iconRight;
			QRect triangleLeft;
			QRect triangleRight;
			QRect frameLeft;
			QRect frameRight;
			QRect textLeft;
			QRect textRight;
			QRect frameLeftReason;
			QRect textLeftReason;
			QRect timeRect;
		} rects;
	} m_layoutData;
	struct StateFlags
	{
		bool isSending = false;
		bool isStreamEnd = false;
		bool isReasoning = true;
		int savedScrollPosition = -1;
		bool answerHeaderInserted = false;
	} m_state;
	struct LayoutCache
	{
		QSize cachedSize;
		QString cachedText;
		int cachedWidth = -1;
		bool isValid = false;
	} m_layoutCache;

	// UI性能优化：减少不必要的重绘
	bool m_needsUpdate = false;
	bool m_layoutDirty = false;
	bool m_isHovered = false;
	bool m_allowDeferredDelete = false;
	QGraphicsDropShadowEffect* m_shadowEffect = nullptr;
	struct UIComponents
	{
		struct Loading
		{
			std::unique_ptr<QLabel> label;
			std::unique_ptr<QMovie> movie;
		} loading;
		struct Icon
		{
			std::unique_ptr<QPixmap> leftPix;
			std::unique_ptr<QPixmap> rightPix;
		}icons;
		struct Buttons
		{
			std::unique_ptr<QPushButton> copyThinking;
			std::unique_ptr<QPushButton> copyAnswer;
			std::unique_ptr<QPushButton> regenerate;
			void hideAll()
			{
				if (copyThinking) copyThinking->hide();
				if (copyAnswer) copyAnswer->hide();
				if (regenerate) regenerate->hide();
			}
		} buttons;
	} m_ui;
	//代码高亮块
	std::unique_ptr<SyntaxHighlighter> m_syntaxHighlighter;
	std::vector<QPushButton> m_SuggestButton;
	User_Type m_UserType = User_System;
};