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
#include"include/cmark/cmark.h"
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
	enum User_Type
	{
		User_System,
		User_Owner,
		User_Customer,
		User_Time,
	};
	struct LayoutConstants
	{
		static constexpr int MIN_HEIGHT = 36;
		static constexpr int ICON_SIZE = 44;
		static constexpr int ICON_SPACING = 18;
		static constexpr int ICON_BORDER_WIDTH = 0;
		static constexpr int ICON_MARGIN = 2;
		static constexpr int ICON_RING_MARGIN = 6;
		static constexpr int TRIANGLE_WIDTH = 10;
		static constexpr int FRAME_MARGIN = 24;
		static constexpr int TEXT_PADDING = 16;
		static constexpr int BUTTON_HEIGHT = 30;
		static constexpr int BUTTON_SPACING = 12;
		static constexpr int EXTRA_HEIGHT = 80;
		static constexpr int BORDER_RADIUS = 18;
		static constexpr int SHADOW_OFFSET = 4;
		static constexpr int Icon_Loading = 16;
		static constexpr int TIME_HEIGHT = 28;
		static constexpr int TIME_MARGIN = 6;
		static constexpr int TIME_HORIZONTAL_PADDING = 12;
		static constexpr int TIME_VERTICAL_PADDING = 4;
		static constexpr int ExButton_Height = 15;
		static constexpr int ExButton_Spacing = 10;
	};
	struct ColorScheme
	{
		static const QColor REASONING_BACKGROUND;
		static const QColor REASONING_BORDER;
		static const QColor ANSWER_BACKGROUND;
		static const QColor ANSWER_BORDER;
		static const QColor USER_BACKGROUND;
		static const QColor TIME_TEXT;
		static const QColor SHADOW_COLOR;
		static const QColor ICON_RING_OUTER;
		static const QColor ICON_RING_INNER;
		static const QColor TIME_BACKGROUND;
	};
	//消息设置接口
	void setText(QString text, QString time, QSize allSize, User_Type userType);
	void setTextWithReason(const QString& reasoning, const QString& answer, QString time, QSize allSize, User_Type userType);
	void setTextSuccess();
	void setUserType(User_Type auth) { this->m_UserType = auth; };
	//框计算
	QSize fontRect(const QString& str);
	QSize fontRect(const QString& reasoning, const QString& answer);
	//文本处理接口
	void appendText(const QString& delta);
	QString markdownToHtml(const QString &markdown);
	//数据获取
	const QString& getText() const { return m_messageData.msg; }
	const QString& getRawText() const { return m_messageData.rawMsg; }
	const QString& getReasoningText() const { return m_messageData.reasoningText; }
	const QString& getReasonRawText() const { return m_messageData.rawReasoningMsg; }
	const QString& getTime() const { return m_messageData.time; }
	const QString& getCurTime() const { return m_messageData.curTime; }
	const QString& getBubbleID() const { return m_messageData.uniqueID; }
	const QSize& getSize() const { return m_layoutData.allSize; }
	const User_Type& getUserType() const { return m_UserType; }
	//状态改变
	void ChangeStream() { m_state.isStreamEnd = true; }
	void ChangeAccpetStatus() { m_state.isReasoning = false; m_state.answerHeaderInserted = false; }
	bool isSending() const { return m_state.isSending; }
	void setBubbleID(QString id) { m_messageData.uniqueID = id; };
	void resetForReuse();
signals:
	void copyThinkingClicked();
	void copyAnswerClicked();
	void regenerateClicked(QString msg);
protected:
	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void contextMenuEvent(QContextMenuEvent *event) override;
	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;
	bool eventFilter(QObject* watched, QEvent* event) override;
	private slots:
	void onCopyThinkingClicked();
	void onCopyAnswerClicked();
	void onRegenerateClicked();
private:
	void initTalkPic();
	void initButtons();
	void initRescource();
	void calculateLayout();
	void updateButtonsPosition();
	void updateButtonsVisibility();
	void drawTimeLabel(QPainter& painter);
	void drawCustomerMessage(QPainter& painter);
	void drawOwnerMessage(QPainter& painter);
	void drawTimeMessage(QPainter& painter);
	QSize getRealString(QString src);
	QSize getRealString(const QString& reasoning, const QString& answer);
	void setTextDocs(QTextDocument& docText, const QString& src, const int& iWidth, const QString& defaultColor = QString());
	void updateButtonHoverState(QPushButton* button, bool hovered);
	int computeTimeExtraHeight() const;
	void layoutSingleMessage(const QSize& contentSize, int timeExtraHeight);
	void layoutReasoningMessage(const QSize& reasoningSize, const QSize& answerSize, int timeExtraHeight);
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
