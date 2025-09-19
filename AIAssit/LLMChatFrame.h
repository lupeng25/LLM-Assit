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
#include <QMenu>
#include <QUuid>
#include "..\include\cmark\cmark.h"
#include "SyntaxHighlighter.h"
class QPaintEvent;
class QPainter;
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
		User_System,//系统
		User_Owner,    //自己
		User_Customer,   //用户
		User_Time,  //时间
	};
	// UI Layout Constants
	struct LayoutConstants 
	{
		static constexpr int MIN_HEIGHT = 30;
		static constexpr int ICON_SIZE = 40;
		static constexpr int ICON_SPACING = 20;
		static constexpr int ICON_BORDER_WIDTH = 5;
		static constexpr int ICON_MARGIN = 10;
		static constexpr int TRIANGLE_WIDTH = 6;
		static constexpr int FRAME_MARGIN = 20;
		static constexpr int TEXT_PADDING = 12;
		static constexpr int BUTTON_HEIGHT = 24;
		static constexpr int BUTTON_SPACING = 8;
		static constexpr int EXTRA_HEIGHT = 80;
		static constexpr int BORDER_RADIUS = 4;
		static constexpr int Icon_Loading = 16;
		static constexpr int TIME_HEIGHT = 20;  
		static constexpr int TIME_MARGIN = 5;  
		static constexpr int ExButton_Height = 15;
		static constexpr int ExButton_Spacing = 10;
	};

	// Color scheme
	struct ColorScheme 
	{
		static const QColor REASONING_BACKGROUND;
		static const QColor REASONING_BORDER;
		static const QColor ANSWER_BACKGROUND;
		static const QColor ANSWER_BORDER;
		static const QColor USER_BACKGROUND;
		static const QColor TIME_TEXT;
	};
	//消息设置接口
	void setText(QString text, QString time, QSize allSize, User_Type userType);
	void setTextWithReason(const QString& reasoning, const QString& answer, QString time, QSize allSize, User_Type userType);
	void setTextSuccess();
	void setUserType(User_Type auth);
	//框计算
	QSize fontRect(QString str);
	QSize fontRect(const QString& reasoning, const QString& answer);
	//文本处理接口
	void appendText(const QString& delta);
	void appendReasonText(const QString& delta);
	void resetText(const QString& delta);
	void updateTextLayout();
	QString markdownToHtml(const QString &markdown);
	//数据获取
	inline QString text() { return m_Msg; }
	inline QString RawText() const { return m_RawMsg; }
	inline QString reasoningText() const { return m_ReasoningText; }
	inline QString reasonRawText() const { return m_RawReasoningMsg; }
	inline QString time() { return m_Time; }
	inline QSize size() { return m_AllSize; }
	inline User_Type userType() { return m_UserType; }
	inline QString BubbleID() const { return m_UniqueID; };
	//状态改变
	inline void ChangeStream() { m_IsStreamEnd = true; }
	inline void ChangeAccpetStatus() { m_IsReasoning = false; }
	inline bool isSending() const { return m_IsSending; }
	void setBubbleID(QString id);

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

private slots:
	void onCopyThinkingClicked();
	void onCopyAnswerClicked();
	void onRegenerateClicked();

private:
	void initTalkPic();
	void initButtons();
	void updateButtonsPosition();
	void updateButtonsVisibility();
	void calculateLayout();
	void drawTimeLabel(QPainter& painter);
	QSize getRealString(QString src);
	QSize getRealString(const QString& reasoning, const QString& answer);
	QString m_Msg;
	QString m_RawMsg;
	QString m_ReasoningText;
	QString m_RawReasoningMsg;
	QString m_Time;
	QString m_CurTime;
	QSize m_AllSize;
	User_Type m_UserType = User_System;
	QString m_UniqueID;
	int m_FrameWidth;
	int m_textWidth;
	int m_spaceWidth;
	int m_lineHeight;
	QRect m_IconLeftRect;
	QRect m_IconRightRect;
	QRect m_TriangleLeftRect;
	QRect m_TriangleRightRect;
	QRect m_FrameLeftRect;
	QRect m_FrameRightRect;
	QRect m_TextLeftRect;
	QRect m_TextRightRect;
	QRect m_FrameLeftReasonRect;
	QRect m_TextLeftReasonRect;
	QRect m_TimeRect;
	QPixmap m_LeftPixmap;
	QPixmap m_RightPixmap;
	QLabel* m_Loading = Q_NULLPTR;
	QMovie* m_LoadingMovie = Q_NULLPTR;
	bool m_IsSending = false;
	bool m_IsStreamEnd = false;
	bool m_IsReasoning = true;

	// 添加按钮成员变量
	QPushButton* m_CopyThinkBtn = Q_NULLPTR;
	QPushButton* m_CopyAnswerBtn = Q_NULLPTR;
	QPushButton* m_ReGenerateBtn = Q_NULLPTR;
	//代码高亮块
	SyntaxHighlighter* m_syntaxHighlighter;
	int m_savedScrollPosition = -1;
};
