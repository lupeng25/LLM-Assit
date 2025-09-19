#include "LLMChatFrame.h"
#include <QFontMetrics>
#include <QPaintEvent>
#include <QDateTime>
#include <QPainter>
#include <QMovie>
#include <QLabel>
#include <QDebug>
#include <QColor>
// Color scheme definitions
const QColor LLMChatFrame::ColorScheme::REASONING_BACKGROUND(153, 177, 200);
const QColor LLMChatFrame::ColorScheme::REASONING_BORDER(153, 145, 234);
const QColor LLMChatFrame::ColorScheme::ANSWER_BACKGROUND(123, 229, 63);
const QColor LLMChatFrame::ColorScheme::ANSWER_BORDER(200, 145, 234);
const QColor LLMChatFrame::ColorScheme::USER_BACKGROUND(75, 164, 242);
const QColor LLMChatFrame::ColorScheme::TIME_TEXT(153, 153, 153);

LLMChatFrame::LLMChatFrame(QWidget *parent)
	: QWidget(parent)
{
	QFont font("Microsoft YaHei", 12);
	setFont(font);
	initTalkPic();
	initButtons();
	setMouseTracking(true);
	m_syntaxHighlighter = new SyntaxHighlighter();
}

LLMChatFrame::~LLMChatFrame()
{
	delete m_syntaxHighlighter;
}

void LLMChatFrame::initTalkPic()
{
	m_LeftPixmap = QPixmap(":/QtWidgetsApp/ICONs/Customer Copy.png");
	m_RightPixmap = QPixmap(":/QtWidgetsApp/ICONs/CustomerService.png");

	if (m_LeftPixmap.isNull()) {
		m_LeftPixmap = QPixmap(LayoutConstants::ICON_SIZE, LayoutConstants::ICON_SIZE);
		m_LeftPixmap.fill(ColorScheme::USER_BACKGROUND);
	}

	if (m_RightPixmap.isNull()) {
		m_RightPixmap = QPixmap(LayoutConstants::ICON_SIZE, LayoutConstants::ICON_SIZE);
		m_RightPixmap.fill(ColorScheme::ANSWER_BACKGROUND);
	}

	m_LoadingMovie = new QMovie(this);
	m_LoadingMovie->setFileName(":/QtWidgetsApp/ICONs/loading4.gif");

	m_Loading = new QLabel(this);
	m_Loading->setMovie(m_LoadingMovie);
	m_Loading->resize(LayoutConstants::Icon_Loading, LayoutConstants::Icon_Loading);
	m_Loading->setAttribute(Qt::WA_TranslucentBackground, true);
	m_Loading->setAutoFillBackground(false);
}

void LLMChatFrame::initButtons()
{
	// 创建按钮
	m_CopyThinkBtn = new QPushButton(this);
	m_CopyAnswerBtn = new QPushButton(this);
	m_ReGenerateBtn = new QPushButton(this);

	// 设置按钮样式
	QString buttonStyle =
		"QPushButton {"
		"    background-color: #FFFFFF;"
		"    border: 1px solid #FFFFFF;"
		"    border-radius: 4px;"
		"    padding: 4px 8px;"
		"    font-size: 11px;"
		"    color: #FFFFFF;"
		"}"
		"QPushButton:hover {"
		"    background-color: #e8e8e8;"
		"    border-color: #b0b0b0;"
		"}"
		"QPushButton:pressed {"
		"    background-color: #d0d0d0;"
		"}";

	m_CopyThinkBtn->setStyleSheet(buttonStyle);
	m_CopyAnswerBtn->setStyleSheet(buttonStyle);
	m_ReGenerateBtn->setStyleSheet(buttonStyle);

	// 设置按钮大小
	m_CopyThinkBtn->setFixedHeight(LayoutConstants::ExButton_Height);
	m_CopyAnswerBtn->setFixedHeight(LayoutConstants::ExButton_Height);
	m_ReGenerateBtn->setFixedHeight(LayoutConstants::ExButton_Height);

	m_CopyThinkBtn->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_CopyThink.png"));
	m_CopyAnswerBtn->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_CopyAnswer.png"));
	m_ReGenerateBtn->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_Regenerate.png"));
	m_CopyThinkBtn->setToolTip(QStringLiteral("复制推理"));
	m_CopyAnswerBtn->setToolTip(QStringLiteral("复制回答"));
	m_ReGenerateBtn->setToolTip(QStringLiteral("继续生成"));

	// 连接信号槽
	connect(m_CopyThinkBtn, &QPushButton::clicked, this, &LLMChatFrame::onCopyThinkingClicked);
	connect(m_CopyAnswerBtn, &QPushButton::clicked, this, &LLMChatFrame::onCopyAnswerClicked);
	connect(m_ReGenerateBtn, &QPushButton::clicked, this, &LLMChatFrame::onRegenerateClicked);

	// 初始时隐藏按钮
	m_CopyThinkBtn->hide();
	m_CopyAnswerBtn->hide();
	m_ReGenerateBtn->hide();
}

// 更新按钮位置
void LLMChatFrame::updateButtonsPosition()
{
	if (m_UserType != User_Customer) 
	{
		m_CopyThinkBtn->hide();
		m_CopyAnswerBtn->hide();
		m_ReGenerateBtn->hide();
		return;
	}

	// 计算按钮位置 - 放在聊天框下方
	int buttonY = qMax(m_FrameLeftReasonRect.bottom(), m_FrameLeftRect.bottom()) + 5;
	int buttonStartX = m_FrameLeftRect.x();

	const int buttonWidth = 70;
	int currentX = buttonStartX;

	// 设置"复制思考"按钮
	if (!m_ReasoningText.isEmpty())
	{
		m_CopyThinkBtn->setGeometry(currentX, buttonY, buttonWidth, LayoutConstants::BUTTON_HEIGHT);
		m_CopyThinkBtn->show();
		currentX += buttonWidth + LayoutConstants::BUTTON_SPACING;
	}
	else {
		m_CopyThinkBtn->hide();
	}

	// 设置"复制回答"按钮
	m_CopyAnswerBtn->setGeometry(currentX, buttonY, buttonWidth, LayoutConstants::BUTTON_HEIGHT);
	m_CopyAnswerBtn->show();
	currentX += buttonWidth + LayoutConstants::BUTTON_SPACING;

	// 设置"重新生成"按钮
	m_ReGenerateBtn->setGeometry(currentX, buttonY, buttonWidth, LayoutConstants::BUTTON_HEIGHT);
	m_ReGenerateBtn->show();
}

// 更新按钮可见性
void LLMChatFrame::updateButtonsVisibility()
{
	// 只有AI回复（User_Customer）且发送完成时才显示按钮
	bool shouldShow = (m_UserType == User_Customer) && m_IsSending;

	if (shouldShow) 
	{
		updateButtonsPosition();
	}
	else 
	{
		m_CopyThinkBtn->hide();
		m_CopyAnswerBtn->hide();
		m_ReGenerateBtn->hide();
	}
}

// 按钮点击事件处理
void LLMChatFrame::onCopyThinkingClicked()
{
	QClipboard *clipboard = QApplication::clipboard();
	QTextDocument doc;
	doc.setHtml(m_ReasoningText);
	clipboard->setText(doc.toPlainText());
	emit copyThinkingClicked();
}

void LLMChatFrame::onCopyAnswerClicked()
{
	QClipboard *clipboard = QApplication::clipboard();
	QTextDocument doc;
	doc.setHtml(m_Msg);
	clipboard->setText(doc.toPlainText());
	emit copyAnswerClicked();
}

void LLMChatFrame::onRegenerateClicked()
{
	emit regenerateClicked(m_UniqueID);
}

QString LLMChatFrame::markdownToHtml(const QString &markdown)
{
	QByteArray markdownData = markdown.toUtf8();
	char *html = cmark_markdown_to_html(markdownData.data(), markdownData.size(), CMARK_OPT_DEFAULT);
	QString htmlString = QString::fromUtf8(html);
	//free(html);

	// 使用 SyntaxHighlighter 处理代码块
	QRegularExpression codeBlockRegex("<pre><code(?:\\s+class=\"language-([^\"]*)\")?(.*?)>(.*?)</code></pre>",
		QRegularExpression::DotMatchesEverythingOption);
	QRegularExpressionMatchIterator iterator = codeBlockRegex.globalMatch(htmlString);
	QString result = htmlString;

	// 从后往前替换，避免位置偏移
	QList<QRegularExpressionMatch> matches;
	while (iterator.hasNext())
	{
		matches.prepend(iterator.next());
	}

	for (const auto& match : matches) 
	{
		QString language = match.captured(1).toLower(); // 语言标识
		QString code = match.captured(3);               // 代码内容

														// HTML解码
		code = code.replace("&lt;", "<").replace("&gt;", ">").replace("&amp;", "&");

		// 使用语法高亮器生成高亮后的HTML
		QString highlightedHtml = m_syntaxHighlighter->highlightCodeBlock(code, language);

		result.replace(match.captured(0), highlightedHtml);
	}

	// 处理行内代码
	QRegularExpression inlineCodeRegex("<code>(.*?)</code>");
	QRegularExpressionMatchIterator inlineIterator = inlineCodeRegex.globalMatch(result);

	QList<QRegularExpressionMatch> inlineMatches;
	while (inlineIterator.hasNext()) 
	{
		inlineMatches.prepend(inlineIterator.next());
	}

	for (const auto& match : inlineMatches)
	{
		QString code = match.captured(1);
		QString highlightedInline = m_syntaxHighlighter->highlightInlineCode(code);
		result.replace(match.captured(0), highlightedInline);
	}

	return result;
}
void LLMChatFrame::setBubbleID(QString id)
{
	m_UniqueID = id;
}
//设置思考过程
void LLMChatFrame::setText(QString text, QString time, QSize allSize, LLMChatFrame::User_Type userType)
{
	QString AnswerHtml = markdownToHtml(text);
	m_Msg = AnswerHtml;
	m_UserType = userType;
	m_Time = time;
	m_CurTime = QDateTime::fromTime_t(time.toInt()).toString("hh:mm");
	m_AllSize = allSize;

	if (userType == User_Customer && !m_IsSending)
	{
		m_Loading->move(m_FrameLeftRect.x() - m_Loading->width() - 10,
			m_FrameLeftRect.y() + m_FrameLeftRect.height() / 2 - m_Loading->height() / 2);
		m_Loading->show();
		m_LoadingMovie->start();
	}
	else
	{
		m_Loading->hide();
	}
	this->update();
}

void LLMChatFrame::setTextWithReason(const QString& reasoning, const QString& answer, QString time, QSize allSize, User_Type userType)
{
	QString AnswerHtml = markdownToHtml(answer);
	m_Msg = AnswerHtml;
	m_RawMsg = AnswerHtml;
	m_UserType = userType;
	m_Time = time;
	m_CurTime = QDateTime::fromTime_t(time.toInt()).toString("hh:mm");
	m_AllSize = allSize;

	if (userType == User_Customer) 
	{
		QString ReasoningHtml = markdownToHtml(reasoning);
		m_RawReasoningMsg = ReasoningHtml;
		m_ReasoningText = ReasoningHtml;//只有回答者才区分
		if (!m_IsSending)
		{
			m_Loading->move(m_FrameLeftRect.x() - m_Loading->width() - 10,
				m_FrameLeftRect.y() + m_FrameLeftRect.height() / 2 - m_Loading->height() / 2);
			m_Loading->show();
			m_LoadingMovie->start();
		}
	}
	else 
	{
		m_Loading->hide();
	}

	this->update();
}

//思考过程结束
void LLMChatFrame::setTextSuccess()
{
	m_Loading->hide();
	m_LoadingMovie->stop();
	m_IsSending = true;
	updateButtonsVisibility();
}

//设置气泡框的大小
QSize LLMChatFrame::fontRect(QString str)
{
	if (str.isEmpty()) return QSize();

	QString html = str.contains("</p>") ? str : markdownToHtml(str);
	m_Msg = html;
	calculateLayout();
	QSize size = getRealString(m_Msg);// 获取文本的实际大小

	 // 为时间显示预留额外高度
	int timeExtraHeight = (m_UserType != User_Time) ?
		LayoutConstants::TIME_HEIGHT + LayoutConstants::TIME_MARGIN : 0;

	int hei = std::max(size.height(), LayoutConstants::MIN_HEIGHT);

	// 调整所有矩形的Y坐标，为时间显示预留空间
	int bubbleTopY = timeExtraHeight + m_lineHeight / 4 * 3;

	// 设置气泡框的三角形区域（左侧和右侧）
	m_TriangleLeftRect = QRect(LayoutConstants::ICON_SIZE + LayoutConstants::ICON_SPACING + LayoutConstants::ICON_BORDER_WIDTH,
		timeExtraHeight + m_lineHeight / 2, LayoutConstants::TRIANGLE_WIDTH, hei - m_lineHeight);
	m_TriangleRightRect = QRect(this->width() - LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::ICON_SIZE - LayoutConstants::ICON_SPACING - LayoutConstants::TRIANGLE_WIDTH,
		timeExtraHeight + m_lineHeight / 2, LayoutConstants::TRIANGLE_WIDTH, hei - m_lineHeight);

	// 根据文本大小调整聊天框的宽度和高度
	if (size.width() < (m_textWidth + m_spaceWidth))
	{
		m_FrameLeftRect.setRect(m_TriangleLeftRect.x() + m_TriangleLeftRect.width(), bubbleTopY,
			size.width() - m_spaceWidth + 2 * LayoutConstants::TEXT_PADDING, hei - m_lineHeight);
		m_FrameRightRect.setRect(this->width() - size.width() + m_spaceWidth - 2 * LayoutConstants::TEXT_PADDING - LayoutConstants::ICON_SIZE - LayoutConstants::ICON_SPACING - LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::TRIANGLE_WIDTH,
			bubbleTopY, size.width() - m_spaceWidth + 2 * LayoutConstants::TEXT_PADDING, hei - m_lineHeight);
	}
	else 
	{
		m_FrameLeftRect.setRect(m_TriangleLeftRect.x() + m_TriangleLeftRect.width(), bubbleTopY,
			m_FrameWidth, hei - m_lineHeight);
		m_FrameRightRect.setRect(LayoutConstants::ICON_SIZE + LayoutConstants::FRAME_MARGIN + LayoutConstants::ICON_SPACING + LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::TRIANGLE_WIDTH,
			bubbleTopY, m_FrameWidth, hei - m_lineHeight);
	}

	m_TextLeftRect.setRect(m_FrameLeftRect.x() + LayoutConstants::TEXT_PADDING, m_FrameLeftRect.y() + LayoutConstants::ICON_MARGIN,
		m_FrameLeftRect.width() - 2 * LayoutConstants::TEXT_PADDING, m_FrameLeftRect.height() - 2 * LayoutConstants::ICON_MARGIN);
	m_TextRightRect.setRect(m_FrameRightRect.x() + LayoutConstants::TEXT_PADDING, m_FrameRightRect.y() + LayoutConstants::ICON_MARGIN,
		m_FrameRightRect.width() - 2 * LayoutConstants::TEXT_PADDING, m_FrameRightRect.height() - 2 * LayoutConstants::ICON_MARGIN);

	this->update();
	m_AllSize = QSize(size.width(), hei + timeExtraHeight);
	return QSize(size.width(), hei + timeExtraHeight);
}

QSize LLMChatFrame::fontRect(const QString& reasoning, const QString& answer)
{
	if (reasoning.isEmpty() && answer.isEmpty()) return QSize();

	m_ReasoningText = reasoning.contains("</p>") ? reasoning : markdownToHtml(reasoning);
	m_Msg = answer.contains("</p>") ? answer : markdownToHtml(answer);
	calculateLayout();

	QSize AnswerSize = getRealString(m_Msg);// 获取文本的实际大小
	QSize ReasoningSize = getRealString(m_ReasoningText);

	// 为时间显示预留额外高度
	int timeExtraHeight = (m_UserType != User_Time) ?
		LayoutConstants::TIME_HEIGHT + LayoutConstants::TIME_MARGIN : 0;

	int AnswerHeight = std::max(AnswerSize.height(), LayoutConstants::MIN_HEIGHT);
	int ReasoningHeight = std::max(ReasoningSize.height(), LayoutConstants::MIN_HEIGHT);

	// 调整所有矩形的Y坐标，为时间显示预留空间
	int bubbleTopY = timeExtraHeight + m_lineHeight / 4 * 3;

	m_TriangleLeftRect = QRect(LayoutConstants::ICON_SIZE + LayoutConstants::ICON_SPACING + LayoutConstants::ICON_BORDER_WIDTH,
		timeExtraHeight + m_lineHeight / 2, LayoutConstants::TRIANGLE_WIDTH, ReasoningHeight - m_lineHeight);
	m_TriangleRightRect = QRect(this->width() - LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::ICON_SIZE - LayoutConstants::ICON_SPACING - LayoutConstants::TRIANGLE_WIDTH,
		timeExtraHeight + m_lineHeight / 2, LayoutConstants::TRIANGLE_WIDTH, ReasoningHeight - m_lineHeight);

	// 根据文本大小调整聊天框的宽度和高度
	if (AnswerSize.width() < (m_textWidth + m_spaceWidth)) {
		m_FrameLeftReasonRect.setRect(m_TriangleLeftRect.x() + m_TriangleLeftRect.width(), bubbleTopY,
			ReasoningSize.width() - m_spaceWidth + 2 * LayoutConstants::TEXT_PADDING, ReasoningHeight - m_lineHeight);
		m_FrameRightRect.setRect(this->width() - AnswerSize.width() + m_spaceWidth - 2 * LayoutConstants::TEXT_PADDING - LayoutConstants::ICON_SIZE - LayoutConstants::ICON_SPACING - LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::TRIANGLE_WIDTH,
			bubbleTopY, AnswerSize.width() - m_spaceWidth + 2 * LayoutConstants::TEXT_PADDING, AnswerHeight - m_lineHeight);
		m_FrameLeftRect.setRect(m_TriangleLeftRect.x() + m_TriangleLeftRect.width(),
			ReasoningHeight - m_lineHeight / 4 + timeExtraHeight, AnswerSize.width() - m_spaceWidth + 2 * LayoutConstants::TEXT_PADDING, AnswerHeight - m_lineHeight);
	}
	else {
		m_FrameLeftReasonRect.setRect(m_TriangleLeftRect.x() + m_TriangleLeftRect.width(), bubbleTopY,
			m_FrameWidth, ReasoningHeight - m_lineHeight);
		m_FrameRightRect.setRect(LayoutConstants::ICON_SIZE + LayoutConstants::FRAME_MARGIN + LayoutConstants::ICON_SPACING + LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::TRIANGLE_WIDTH,
			bubbleTopY, m_FrameWidth, AnswerHeight - m_lineHeight);
		m_FrameLeftRect.setRect(m_TriangleLeftRect.x() + m_TriangleLeftRect.width(),
			ReasoningHeight - m_lineHeight / 4 + timeExtraHeight, AnswerSize.width() - m_spaceWidth + 2 * LayoutConstants::TEXT_PADDING, AnswerHeight - m_lineHeight);
	}

	m_TextLeftReasonRect.setRect(m_FrameLeftReasonRect.x() + LayoutConstants::TEXT_PADDING, m_FrameLeftReasonRect.y() + LayoutConstants::ICON_MARGIN,
		m_FrameLeftReasonRect.width() - 2 * LayoutConstants::TEXT_PADDING, m_FrameLeftReasonRect.height() - 2 * LayoutConstants::ICON_MARGIN);
	m_TextRightRect.setRect(m_FrameRightRect.x() + LayoutConstants::TEXT_PADDING, m_FrameRightRect.y() + LayoutConstants::ICON_MARGIN,
		m_FrameRightRect.width() - 2 * LayoutConstants::TEXT_PADDING, m_FrameRightRect.height() - 2 * LayoutConstants::ICON_MARGIN);
	m_TextLeftRect.setRect(m_FrameLeftRect.x() + LayoutConstants::TEXT_PADDING, m_FrameLeftRect.y(),
		m_FrameLeftRect.width() - 2 * LayoutConstants::TEXT_PADDING, m_FrameLeftRect.height() - 2 * LayoutConstants::ICON_MARGIN);

	// 更新按钮位置
	updateButtonsVisibility();
	this->update();

	// 如果是AI回复且已完成发送，需要为按钮预留空间
	int extraHeight = 0;
	if (m_UserType == User_Customer && m_IsSending) {
		extraHeight = LayoutConstants::ExButton_Height + 10; // 按钮高度 + 间距
	}

	if (this->userType() == User_Owner)
		m_AllSize = QSize(AnswerSize.width(), AnswerHeight + timeExtraHeight);
	else
		m_AllSize = QSize(ReasoningSize.width(), AnswerHeight + ReasoningHeight + timeExtraHeight);

	return m_AllSize;
}

//获取文字大小
QSize LLMChatFrame::getRealString(QString src)
{
	QFontMetricsF fm(this->font());
	m_lineHeight = fm.lineSpacing();
	QRectF textRect = fm.boundingRect(QRectF(0, 0, m_textWidth, 9999),
		Qt::TextWordWrap, src);
	int w = textRect.width();
	int h = textRect.height();
	// 加一点额外垂直间距，避免被切顶
	const int extra = 80;
	QTextDocument doc;
	doc.setHtml(src);  // 设置 HTML 格式的消息内容
	QFont font("MicrosoftYaHei", 12);//字体
	doc.setDefaultFont(font);
	QTextOption option(Qt::AlignLeft | Qt::AlignVCenter);//左对齐+垂直居中
	option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); // 设置自动换行
	doc.setDefaultTextOption(option);
	doc.setTextWidth(w);
	QSize docSize = doc.size().toSize();
	return QSize(w + m_spaceWidth, docSize.height() + extra);
}

QSize LLMChatFrame::getRealString(const QString& reasoning, const QString& answer)
{
	QFontMetricsF fm(this->font());
	m_lineHeight = fm.lineSpacing();

	QSize reasoningSize(0, 0);
	QSize answerSize(0, 0);

	if (!reasoning.isEmpty()) {
		QRectF reasoningRect = fm.boundingRect(QRectF(0, 0, m_textWidth, 9999), Qt::TextWordWrap, reasoning);
		reasoningSize = QSize(reasoningRect.width(), reasoningRect.height());
	}

	if (!answer.isEmpty()) {
		QRectF answerRect = fm.boundingRect(QRectF(0, 0, m_textWidth, 9999), Qt::TextWordWrap, answer);
		answerSize = QSize(answerRect.width(), answerRect.height());
	}

	const int extra = 80; // 额外垂直间距

	int w = qMax(reasoningSize.width(), answerSize.width()) + m_spaceWidth;
	int h = reasoningSize.height() + answerSize.height() + extra;

	return QSize(w, h);
}

//绘制时间标签的方法
void LLMChatFrame::drawTimeLabel(QPainter& painter)
{
	if (m_UserType == User_Time || m_CurTime.isEmpty()) return;

	QPen penText;
	penText.setColor(ColorScheme::TIME_TEXT);
	painter.setPen(penText);

	QFont timeFont = this->font();
	timeFont.setFamily("MicrosoftYaHei");
	timeFont.setPointSize(9);
	painter.setFont(timeFont);

	//  让时间与图标位置对齐
	QRect timeRect;
	QTextOption option;

	if (m_UserType == User_Customer)
	{
		// AI回复时间与左侧图标对齐
		timeRect = QRect(m_IconLeftRect.x(), 0, width() - m_IconLeftRect.x(), LayoutConstants::TIME_HEIGHT);
		option.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else 
	{
		// 用户消息时间与右侧图标对齐
		timeRect = QRect(0, 0, m_IconRightRect.right(), LayoutConstants::TIME_HEIGHT);
		option.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	}

	painter.drawText(timeRect, m_CurTime, option);
}

void LLMChatFrame::calculateLayout()
{
	m_FrameWidth = width() - LayoutConstants::FRAME_MARGIN - 2 * (LayoutConstants::ICON_SIZE + LayoutConstants::ICON_SPACING + LayoutConstants::ICON_BORDER_WIDTH);
	m_textWidth = m_FrameWidth - 2 * LayoutConstants::TEXT_PADDING;
	m_spaceWidth = width() - m_textWidth;

	// 计算时间显示区域 - 在顶部
	if (m_UserType != User_Time) 
	{
		m_TimeRect = QRect(0, 0, width(), LayoutConstants::TIME_HEIGHT);
	}

	// 调整图标位置，为时间显示预留空间
	int iconTopMargin = (m_UserType != User_Time) ?
		LayoutConstants::TIME_HEIGHT + LayoutConstants::TIME_MARGIN + LayoutConstants::ICON_MARGIN :
		LayoutConstants::ICON_MARGIN;

	m_IconLeftRect = QRect(LayoutConstants::ICON_SPACING, iconTopMargin,
		LayoutConstants::ICON_SIZE, LayoutConstants::ICON_SIZE);
	m_IconRightRect = QRect(width() - LayoutConstants::ICON_SPACING - LayoutConstants::ICON_SIZE, iconTopMargin,
		LayoutConstants::ICON_SIZE, LayoutConstants::ICON_SIZE);
}

void LLMChatFrame::updateTextLayout()
{
	QString html = markdownToHtml(m_Msg);
	m_Msg = html;
	QSize size = getRealString(m_Msg);
	fontRect(m_Msg);  // 重算矩形区域
	m_AllSize = size;
}

//添加流式
void LLMChatFrame::appendText(const QString& delta)
{
	if (!m_IsReasoning&&m_RawReasoningMsg == QStringLiteral("\n ### 推理 \n\n\n"))
	{
		m_RawReasoningMsg.append(QStringLiteral("推理未开启"));
	}

	if (m_IsReasoning)
	{
		m_ReasoningText += delta;
		m_RawReasoningMsg += delta;
		m_RawReasoningMsg.replace(QRegularExpression("<think>"), QStringLiteral("\n ### 推理 \n"));
		m_RawReasoningMsg.replace(QRegularExpression("</think>"), QStringLiteral("\n ### 回答 "));
	}
	else
	{
		m_Msg += delta;
		m_RawMsg += delta;
		m_RawMsg.replace(QRegularExpression("<think>"), QStringLiteral("\n ### 推理 \n"));
		m_RawMsg.replace(QRegularExpression("</think>"), QStringLiteral("\n ### 回答 "));
	}
}

void LLMChatFrame::appendReasonText(const QString& delta)
{
	m_ReasoningText += delta;
	m_RawReasoningMsg += delta;
	m_RawReasoningMsg.replace(QRegularExpression("<think>"), QStringLiteral("\n ### 推理 \n"));
	m_RawReasoningMsg.replace(QRegularExpression("</think>"), QStringLiteral("\n ### 回答 \n"));
}

//重置文字
void LLMChatFrame::resetText(const QString& delta)
{
	m_Msg = delta;
}

void LLMChatFrame::setUserType(User_Type type)
{
	this->m_UserType = type;
}

void LLMChatFrame::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);//消锯齿
	painter.setPen(Qt::NoPen);
	painter.setBrush(QBrush(Qt::gray));

	// 绘制时间标签（除了User_Time类型）
	drawTimeLabel(painter);

	if (m_UserType == User_Type::User_Customer)
	{
		// Icon
		painter.drawPixmap(m_IconLeftRect, m_LeftPixmap);

		//回答框
		painter.setBrush(QBrush(ColorScheme::REASONING_BORDER));
		painter.drawRoundedRect(m_FrameLeftReasonRect.x() - 1, m_FrameLeftReasonRect.y() - 1, m_FrameLeftReasonRect.width() + 2, m_FrameLeftReasonRect.height() + 2, LayoutConstants::BORDER_RADIUS, LayoutConstants::BORDER_RADIUS);
		painter.setBrush(QBrush(ColorScheme::REASONING_BACKGROUND));
		painter.drawRoundedRect(m_FrameLeftReasonRect, LayoutConstants::BORDER_RADIUS, LayoutConstants::BORDER_RADIUS);

		painter.setBrush(QBrush(ColorScheme::ANSWER_BORDER));
		painter.drawRoundedRect(m_FrameLeftRect.x() - 1, m_FrameLeftRect.y() - 1, m_FrameLeftRect.width() + 2, m_FrameLeftRect.height() + 2, LayoutConstants::BORDER_RADIUS, LayoutConstants::BORDER_RADIUS);
		painter.setBrush(QBrush(ColorScheme::ANSWER_BACKGROUND));
		painter.drawRoundedRect(m_FrameLeftRect, LayoutConstants::BORDER_RADIUS, LayoutConstants::BORDER_RADIUS);

		//三角
		QPointF points[3] = {
			QPointF(m_TriangleLeftRect.x(), m_TriangleLeftRect.y() + 10),
			QPointF(m_TriangleLeftRect.x() + m_TriangleLeftRect.width(), m_TriangleLeftRect.y() + 5),
			QPointF(m_TriangleLeftRect.x() + m_TriangleLeftRect.width(), m_TriangleLeftRect.y() + 15),
		};
		QPen pen;
		pen.setColor(ColorScheme::REASONING_BORDER);
		painter.setPen(pen);
		painter.drawPolygon(points, 3);
		//三角加边
		QPen TriangleEdge;
		TriangleEdge.setColor(ColorScheme::ANSWER_BACKGROUND);
		painter.setPen(TriangleEdge);
		painter.drawLine(QPointF(m_TriangleLeftRect.x() - 1, m_TriangleLeftRect.y() + 10), QPointF(m_TriangleLeftRect.x() + m_TriangleLeftRect.width(), m_TriangleLeftRect.y() + 4));
		painter.drawLine(QPointF(m_TriangleLeftRect.x() - 1, m_TriangleLeftRect.y() + 10), QPointF(m_TriangleLeftRect.x() + m_TriangleLeftRect.width(), m_TriangleLeftRect.y() + 16));

		//字体设置
		QFont font("MicrosoftYaHei", 12);//字体
		QTextOption option(Qt::AlignLeft | Qt::AlignVCenter);//左对齐+垂直居中
		option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); // 设置自动换行
		if (m_IsStreamEnd)
		{
			QString html = markdownToHtml(m_RawMsg);
			QTextDocument doc;
			doc.setHtml(html);  // 设置 HTML 格式的消息内容
			doc.setDefaultFont(font);
			doc.setDefaultTextOption(option);
			doc.setTextWidth(m_TextLeftRect.width());
			painter.save();
			painter.translate(m_TextLeftRect.topLeft());
			doc.documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
			painter.restore();

			QString html2 = markdownToHtml(m_RawReasoningMsg);
			QTextDocument doc2;
			doc2.setHtml(html2);  // 设置 HTML 格式的消息内容
			doc2.setDefaultFont(font);
			doc2.setDefaultTextOption(option);
			doc2.setTextWidth(m_TextLeftReasonRect.width());
			QSize docSize2 = doc2.size().toSize();
			painter.save();
			painter.translate(m_TextLeftReasonRect.topLeft());
			doc2.documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
			painter.restore();
		}
		else
		{
			QTextDocument doc;
			doc.setHtml(m_ReasoningText);
			doc.setDefaultFont(font);
			doc.setDefaultTextOption(option);
			doc.setTextWidth(m_TextLeftReasonRect.width());
			QSize docSize = doc.size().toSize();
			painter.save();
			painter.translate(m_TextLeftReasonRect.topLeft());
			doc.documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
			painter.restore();

			QTextDocument doc2;
			doc2.setHtml(m_Msg);
			doc2.setDefaultFont(font);
			doc2.setDefaultTextOption(option);
			doc2.setTextWidth(m_TextLeftRect.width());
			painter.save();
			painter.translate(m_TextLeftRect.topLeft());
			doc2.documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
			painter.restore();
		}

	}
	else if (m_UserType == User_Type::User_Owner)
	{ // 自己
		painter.drawPixmap(m_IconRightRect, m_RightPixmap);

		//框
		painter.setBrush(ColorScheme::USER_BACKGROUND);
		painter.drawRoundedRect(m_FrameRightRect, LayoutConstants::BORDER_RADIUS, LayoutConstants::BORDER_RADIUS);

		//三角
		QPointF points[3] = {
			QPointF(m_TriangleRightRect.x() + m_TriangleRightRect.width(), m_TriangleRightRect.y() + 10),
			QPointF(m_TriangleRightRect.x(), m_TriangleRightRect.y() + 5),
			QPointF(m_TriangleRightRect.x(), m_TriangleRightRect.y() + 15),
		};
		QPen pen;
		pen.setColor(ColorScheme::USER_BACKGROUND);
		painter.setPen(pen);
		painter.drawPolygon(points, 3);

		QTextDocument doc;
		doc.setHtml(m_Msg);  // 设置 HTML 格式的消息内容
		QFont font("MicrosoftYaHei", 12);//字体
		doc.setDefaultFont(font);
		QTextOption option(Qt::AlignLeft | Qt::AlignVCenter);
		option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); // 设置自动换行
		doc.setDefaultTextOption(option);
		doc.setTextWidth(m_TextRightRect.width());
		painter.translate(m_TextRightRect.topLeft());
		doc.documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
	}
	else if (m_UserType == User_Type::User_Time)
	{ // 时间
		QPen penText;
		penText.setColor(ColorScheme::TIME_TEXT);
		painter.setPen(penText);
		QTextOption option(Qt::AlignCenter);
		option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
		QFont te_font = this->font();
		te_font.setFamily("MicrosoftYaHei");
		te_font.setPointSize(10);
		painter.setFont(te_font);
		painter.drawText(this->rect(), m_CurTime, option);
	}
}

void LLMChatFrame::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	// 当尺寸变化时重新计算布局
	if (m_UserType != User_Time) {
		if (!m_ReasoningText.isEmpty() && m_UserType == User_Customer) 
		{
			// 有推理文本的情况
			fontRect(m_ReasoningText, m_Msg);
		}
		else 
		{
			// 普通消息
			fontRect(m_Msg);
		}
	}
}

void LLMChatFrame::mousePressEvent(QMouseEvent *event)
{
	QListWidget* listWidget = qobject_cast<QListWidget*>(parent()->parent());
	if (listWidget)
	{
		QScrollBar* scrollBar = listWidget->verticalScrollBar();
		if (scrollBar)
		{
			m_savedScrollPosition = scrollBar->value();
		}
	}
	event->accept();
}

void LLMChatFrame::mouseReleaseEvent(QMouseEvent *event)
{
	QListWidget* listWidget = qobject_cast<QListWidget*>(parent()->parent());
	if (listWidget) 
	{
		QScrollBar* scrollBar = listWidget->verticalScrollBar();
		if (scrollBar && m_savedScrollPosition >= 0) 
		{
			scrollBar->setValue(m_savedScrollPosition);
			m_savedScrollPosition = -1;
		}
	}
	event->accept();
}

//右键菜单
void LLMChatFrame::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	QAction* copyAction = menu.addAction(QStringLiteral("复制"));

	QAction* selectedAction = menu.exec(event->globalPos());
	if (selectedAction == copyAction)
	{
		QClipboard *clipboard = QApplication::clipboard();
		QTextDocument doc;
		QPoint SelectPoint = this->mapFromGlobal(event->globalPos());

		if (m_FrameLeftReasonRect.contains(SelectPoint))
		{
			doc.setHtml(m_ReasoningText);
		}
		else
		{
			doc.setHtml(m_Msg);
		}

		QString plainText = doc.toPlainText();
		clipboard->setText(plainText);
	}
}
