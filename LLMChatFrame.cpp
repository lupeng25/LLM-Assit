#include "LLMChatFrame.h"
#include <QFontMetrics>
#include <QPaintEvent>
#include <QDateTime>
#include <QPainter>
#include <QMovie>
#include <QLabel>
#include <QDebug>
#include <QColor>
#include <QPainterPath>
#include <QLinearGradient>
#include <QHash>
const QColor LLMChatFrame::ColorScheme::REASONING_BACKGROUND(238, 243, 255);
const QColor LLMChatFrame::ColorScheme::REASONING_BORDER(199, 210, 254);
const QColor LLMChatFrame::ColorScheme::ANSWER_BACKGROUND(225, 239, 255);
const QColor LLMChatFrame::ColorScheme::ANSWER_BORDER(186, 210, 254);
const QColor LLMChatFrame::ColorScheme::USER_BACKGROUND(59, 130, 246);
const QColor LLMChatFrame::ColorScheme::TIME_TEXT(71, 85, 105);
const QColor LLMChatFrame::ColorScheme::SHADOW_COLOR(15, 23, 42, 36);
const QColor LLMChatFrame::ColorScheme::ICON_RING_OUTER(59, 130, 246, 96);
const QColor LLMChatFrame::ColorScheme::ICON_RING_INNER(129, 140, 248, 55);
const QColor LLMChatFrame::ColorScheme::TIME_BACKGROUND(148, 163, 184, 60);
namespace
{
	QPainterPath createBubblePath(const QRect& bubbleRect, const QRect& triangleRect, bool alignLeft, int radius)
	{
		QPainterPath path;
		if (!bubbleRect.isValid())
		{
			return path;
		}
		QRectF rect = bubbleRect;
		path.addRoundedRect(rect, radius, radius);
		if (triangleRect.isValid())
		{
			QPainterPath tail;
			double topOffset = triangleRect.height() * 0.25;
			double bottomOffset = triangleRect.height() * 0.25;
			if (alignLeft)
			{
				tail.moveTo(triangleRect.right(), triangleRect.top() + topOffset);
				tail.lineTo(triangleRect.left(), triangleRect.center().y());
				tail.lineTo(triangleRect.right(), triangleRect.bottom() - bottomOffset);
			}
			else
			{
				tail.moveTo(triangleRect.left(), triangleRect.top() + topOffset);
				tail.lineTo(triangleRect.right(), triangleRect.center().y());
				tail.lineTo(triangleRect.left(), triangleRect.bottom() - bottomOffset);
			}
			tail.closeSubpath();
			path = path.united(tail);
		}
		return path;
	}
	void drawBubble(QPainter& painter, const QRect& rect, const QRect& triangleRect, bool alignLeft,
		const QColor& fillColor, const QColor& borderColor, const QColor& shadowColor,
		int radius, int shadowOffset)
	{
		if (!rect.isValid())
		{
			return;
		}
		QPainterPath path = createBubblePath(rect, triangleRect, alignLeft, radius);
		if (path.isEmpty())
		{
			return;
		}
		if (shadowOffset > 0 && shadowColor.alpha() > 0)
		{
			painter.save();
			painter.setRenderHint(QPainter::Antialiasing, true);
			painter.setPen(Qt::NoPen);
			painter.setBrush(shadowColor);
			painter.drawPath(path.translated(shadowOffset, shadowOffset));
			painter.restore();
		}
		painter.save();
		painter.setRenderHint(QPainter::Antialiasing, true);
		QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
		gradient.setColorAt(0.0, fillColor.lighter(108));
		gradient.setColorAt(1.0, fillColor);
		painter.setBrush(gradient);
		painter.setPen(QPen(borderColor, 1.2));
		painter.drawPath(path);
		painter.restore();
	}
	QPixmap createAvatarPixmap(const QPixmap& source)
	{
		if (source.isNull())
		{
			return QPixmap();
		}
		QPixmap scaled = source.scaled(LLMChatFrame::LayoutConstants::ICON_SIZE,
			LLMChatFrame::LayoutConstants::ICON_SIZE,
			Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
		QPixmap result(LLMChatFrame::LayoutConstants::ICON_SIZE,
			LLMChatFrame::LayoutConstants::ICON_SIZE);
		result.fill(Qt::transparent);
		QPainter painter(&result);
		painter.setRenderHint(QPainter::Antialiasing, true);
		QPainterPath clipPath;
		clipPath.addEllipse(result.rect());
		painter.setClipPath(clipPath);
		painter.drawPixmap(result.rect(), scaled);
		painter.end();
		return result;
	}
}
LLMChatFrame::LLMChatFrame(QWidget *parent)
	: QWidget(parent)
{
	QFont font("Microsoft YaHei", 12);
	setFont(font);
	initRescource();
	initTalkPic();
	initButtons();
	setMouseTracking(true);
}
LLMChatFrame::~LLMChatFrame()
{
}
void LLMChatFrame::initTalkPic()
{
	*m_ui.icons.leftPix = QPixmap(":/QtWidgetsApp/ICONs/Customer Copy.png");
	*m_ui.icons.rightPix = QPixmap(":/QtWidgetsApp/ICONs/CustomerService.png");
	auto prepareAvatar = [](std::unique_ptr<QPixmap>& pix, const QColor& fallbackColor)
	{
		if (pix->isNull())
		{
			QPixmap fallback(LayoutConstants::ICON_SIZE, LayoutConstants::ICON_SIZE);
			fallback.fill(Qt::transparent);
			QPainter painter(&fallback);
			painter.setRenderHint(QPainter::Antialiasing, true);
			painter.setPen(Qt::NoPen);
			painter.setBrush(fallbackColor);
			painter.drawEllipse(fallback.rect());
			painter.end();
			*pix = fallback;
		}
		*pix = createAvatarPixmap(*pix);
	};
	prepareAvatar(m_ui.icons.leftPix, ColorScheme::ANSWER_BORDER);
	prepareAvatar(m_ui.icons.rightPix, ColorScheme::USER_BACKGROUND);
	m_ui.loading.movie->setFileName(":/QtWidgetsApp/ICONs/loading4.gif");
	m_ui.loading.label->setMovie(m_ui.loading.movie.get());
	m_ui.loading.label->resize(LayoutConstants::Icon_Loading, LayoutConstants::Icon_Loading);
	m_ui.loading.label->setAttribute(Qt::WA_TranslucentBackground, true);
	m_ui.loading.label->setAutoFillBackground(false);
}
void LLMChatFrame::initButtons()
{
	// 设置按钮样式
	QString buttonStyle =
		"QPushButton {"
		"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
		"        stop:0 #2563eb, stop:1 #1d4ed8);"
		"    border: none;"
		"    border-radius: 14px;"
		"    padding: 6px 14px;"
		"    font-size: 12px;"
		"    font-weight: 600;"
		"    color: #f8fafc;"
		"}"
		"QPushButton:hover {"
		"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
		"        stop:0 #3b82f6, stop:1 #2563eb);"
		"}"
		"QPushButton:pressed {"
		"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
		"        stop:0 #1d4ed8, stop:1 #1e40af);"
		"}";
	// 应用样式到所有按钮
	for (auto* btn : { m_ui.buttons.copyThinking.get(),
		m_ui.buttons.copyAnswer.get(),
		m_ui.buttons.regenerate.get() })
	{
		btn->setStyleSheet(buttonStyle);
		btn->setFixedHeight(LayoutConstants::BUTTON_HEIGHT);
		btn->setMinimumWidth(88);
		btn->setCursor(Qt::PointingHandCursor);
		btn->setFocusPolicy(Qt::NoFocus);
	}
	// 设置图标和工具提示
	m_ui.buttons.copyThinking->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_CopyThink.png"));
	m_ui.buttons.copyThinking->setToolTip(tr("Copy the reasoning"));
	m_ui.buttons.copyAnswer->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_CopyAnswer.png"));
	m_ui.buttons.copyAnswer->setToolTip(tr("Copy the answer"));
	m_ui.buttons.regenerate->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_Regenerate.png"));
	m_ui.buttons.regenerate->setToolTip(tr("Regengrate"));
	// 连接信号槽
	connect(m_ui.buttons.copyThinking.get(), &QPushButton::clicked, this, &LLMChatFrame::onCopyThinkingClicked);
	connect(m_ui.buttons.copyAnswer.get(), &QPushButton::clicked, this, &LLMChatFrame::onCopyAnswerClicked);
	connect(m_ui.buttons.regenerate.get(), &QPushButton::clicked, this, &LLMChatFrame::onRegenerateClicked);
	// 初始时隐藏按钮
	m_ui.buttons.hideAll();
}
void LLMChatFrame::initRescource()
{
	m_syntaxHighlighter = std::make_unique<SyntaxHighlighter>();
	m_ui.icons.leftPix = std::make_unique<QPixmap>();
	m_ui.icons.rightPix = std::make_unique<QPixmap>();
	m_ui.loading.label = std::make_unique<QLabel>(this);
	m_ui.loading.movie = std::make_unique<QMovie>(this);
	m_ui.buttons.copyThinking = std::make_unique<QPushButton>(this);
	m_ui.buttons.copyAnswer = std::make_unique<QPushButton>(this);
	m_ui.buttons.regenerate = std::make_unique<QPushButton>(this);
}
void LLMChatFrame::updateButtonsPosition()
{
	if (m_UserType != User_Customer)
	{
		m_ui.buttons.hideAll();
		return;
	}
	// 计算按钮位置 - 放在聊天框下方
	int frameBottom = qMax(m_layoutData.rects.frameLeftReason.bottom(), m_layoutData.rects.frameLeft.bottom());
	int buttonY = frameBottom + LayoutConstants::SHADOW_OFFSET + LayoutConstants::BUTTON_SPACING;
	int buttonStartX = m_layoutData.rects.frameLeft.x();
	const int buttonWidth = 96;
	const int buttonHeight = LayoutConstants::BUTTON_HEIGHT;
	int currentX = buttonStartX;
	// 设置"复制思考"按钮
	if (!m_messageData.reasoningText.isEmpty())
	{
		m_ui.buttons.copyThinking->setGeometry(currentX, buttonY, buttonWidth, buttonHeight);
		m_ui.buttons.copyThinking->show();
		currentX += buttonWidth + LayoutConstants::BUTTON_SPACING;
	}
	else
	{
		m_ui.buttons.copyThinking->hide();
	}
	// 设置"复制回答"按钮
	m_ui.buttons.copyAnswer->setGeometry(currentX, buttonY, buttonWidth, buttonHeight);
	m_ui.buttons.copyAnswer->show();
	currentX += buttonWidth + LayoutConstants::BUTTON_SPACING;
	// 设置"重新生成"按钮
	m_ui.buttons.regenerate->setGeometry(currentX, buttonY, buttonWidth, buttonHeight);
	m_ui.buttons.regenerate->show();
}
void LLMChatFrame::updateButtonsVisibility()
{
	// 只有AI回复（User_Customer）且发送完成时才显示按钮
	bool shouldShow = (m_UserType == User_Customer) && m_state.isSending;
	if (shouldShow)
	{
		updateButtonsPosition();
	}
	else
	{
		m_ui.buttons.hideAll();
	}
}
void LLMChatFrame::onCopyThinkingClicked()
{
	QClipboard *clipboard = QApplication::clipboard();
	QTextDocument doc;
	doc.setHtml(m_messageData.reasoningText);
	clipboard->setText(doc.toPlainText());
	emit copyThinkingClicked();
}
void LLMChatFrame::onCopyAnswerClicked()
{
	QClipboard *clipboard = QApplication::clipboard();
	QTextDocument doc;
	doc.setHtml(m_messageData.msg);
	clipboard->setText(doc.toPlainText());
	emit copyAnswerClicked();
}
void LLMChatFrame::onRegenerateClicked()
{
	emit regenerateClicked(m_messageData.uniqueID);
}
QString LLMChatFrame::markdownToHtml(const QString &markdown)
{
	// 添加Markdown转换缓存以提高性能
	static QHash<QString, QString> mdCache;
	static const int MAX_CACHE_SIZE = 100;

	// 检查缓存
	auto it = mdCache.find(markdown);
	if (it != mdCache.end()) {
		return *it;
	}

	// 如果缓存过大，清除一半
	if (mdCache.size() >= MAX_CACHE_SIZE) {
		// 清除较旧的缓存项（简单的策略：清除一半）
		QHash<QString, QString> newCache;
		int count = 0;
		for (auto it = mdCache.begin(); it != mdCache.end(); ++it) {
			if (count++ % 2 == 0) {
				newCache.insert(it.key(), it.value());
			}
		}
		mdCache = newCache;
	}

	QByteArray markdownData = markdown.toUtf8();
	char *html = cmark_markdown_to_html(markdownData.data(), markdownData.size(), CMARK_OPT_DEFAULT);
	QString htmlString = QString::fromUtf8(html);
//	free(html);  // 释放cmark分配的内存

				 // 使用 SyntaxHighlighter 处理代码块
	static const QRegularExpression codeBlockRegex(
		"<pre><code(?:\\s+class=\"language-([^\"]*)\")?(.*?)>(.*?)</code></pre>",
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
	static const QRegularExpression inlineCodeRegex("<code>(.*?)</code>");
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

	// 缓存结果
	mdCache.insert(markdown, result);

	return result;
}
void LLMChatFrame::setTextDocs(QTextDocument& docText, const QString& src, const int& iWidth, const QString& defaultColor)
{
	QString html = src;
	if (!defaultColor.isEmpty())
	{
		html = QStringLiteral("<div style=\"color:%1;\">").arg(defaultColor) + src + QStringLiteral("</div>");
	}
	docText.setHtml(html);
	QFont font("MicrosoftYaHei", 12);
	docText.setDefaultFont(font);
	QTextOption option(Qt::AlignLeft | Qt::AlignVCenter);
	option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	docText.setDefaultTextOption(option);
	docText.setTextWidth(iWidth);
}
int LLMChatFrame::computeTimeExtraHeight() const
{
	return (m_UserType != User_Time) ?
		LayoutConstants::TIME_HEIGHT + LayoutConstants::TIME_MARGIN : 0;
}
int LLMChatFrame::computeAdditionalHeight() const
{
	int extra = LayoutConstants::SHADOW_OFFSET;
	if (m_UserType == User_Customer)
	{
		extra += LayoutConstants::BUTTON_HEIGHT + LayoutConstants::BUTTON_SPACING * 2;
	}
	return extra;
}
static int computeFrameWidth(int contentWidth, int textWidth, int spaceWidth)
{
	int frameWidth = contentWidth - spaceWidth + 2 * LLMChatFrame::LayoutConstants::TEXT_PADDING;
	return std::max(frameWidth, 0);
}
void LLMChatFrame::layoutSingleMessage(const QSize& contentSize, int timeExtraHeight)
{
	int bubbleHeight = std::max(contentSize.height(), LayoutConstants::MIN_HEIGHT);
	int bubbleTopY = timeExtraHeight + static_cast<int>(m_layoutData.iLineHeight * 0.75);
	m_layoutData.rects.triangleLeft = QRect(LayoutConstants::ICON_SIZE + LayoutConstants::ICON_SPACING + LayoutConstants::ICON_BORDER_WIDTH,
		timeExtraHeight + m_layoutData.iLineHeight / 2, LayoutConstants::TRIANGLE_WIDTH,
		bubbleHeight - m_layoutData.iLineHeight);
	m_layoutData.rects.triangleRight = QRect(width() - LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::ICON_SIZE - LayoutConstants::ICON_SPACING - LayoutConstants::TRIANGLE_WIDTH,
		timeExtraHeight + m_layoutData.iLineHeight / 2, LayoutConstants::TRIANGLE_WIDTH,
		bubbleHeight - m_layoutData.iLineHeight);
	bool useContentWidth = contentSize.width() < (m_layoutData.iTextWidth + m_layoutData.iSpaceWidth);
	int leftFrameWidth = useContentWidth
		? computeFrameWidth(contentSize.width(), m_layoutData.iTextWidth, m_layoutData.iSpaceWidth)
		: m_layoutData.iFrameWidth;
	int rightFrameWidth = leftFrameWidth;
	m_layoutData.rects.frameLeft = QRect(m_layoutData.rects.triangleLeft.x() + m_layoutData.rects.triangleLeft.width(), bubbleTopY,
		leftFrameWidth, bubbleHeight - m_layoutData.iLineHeight);
	int rightX = useContentWidth
		? width() - contentSize.width() + m_layoutData.iSpaceWidth - 2 * LayoutConstants::TEXT_PADDING - LayoutConstants::ICON_SIZE - LayoutConstants::ICON_SPACING - LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::TRIANGLE_WIDTH
		: LayoutConstants::ICON_SIZE + LayoutConstants::FRAME_MARGIN + LayoutConstants::ICON_SPACING + LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::TRIANGLE_WIDTH;
	m_layoutData.rects.frameRight = QRect(rightX, bubbleTopY,
		rightFrameWidth, bubbleHeight - m_layoutData.iLineHeight);
	m_layoutData.rects.frameLeft.adjust(0, 0, 0, LayoutConstants::ICON_MARGIN);
	m_layoutData.rects.frameRight.adjust(0, 0, 0, LayoutConstants::ICON_MARGIN);
	m_layoutData.rects.frameLeftReason = QRect();
	m_layoutData.rects.textLeft = QRect(m_layoutData.rects.frameLeft.x() + LayoutConstants::TEXT_PADDING,
		m_layoutData.rects.frameLeft.y() + LayoutConstants::ICON_MARGIN,
		m_layoutData.rects.frameLeft.width() - 2 * LayoutConstants::TEXT_PADDING,
		m_layoutData.rects.frameLeft.height() - LayoutConstants::ICON_MARGIN - LayoutConstants::TEXT_PADDING);
	m_layoutData.rects.textRight = QRect(m_layoutData.rects.frameRight.x() + LayoutConstants::TEXT_PADDING,
		m_layoutData.rects.frameRight.y() + LayoutConstants::ICON_MARGIN,
		m_layoutData.rects.frameRight.width() - 2 * LayoutConstants::TEXT_PADDING,
		m_layoutData.rects.frameRight.height() - LayoutConstants::ICON_MARGIN - LayoutConstants::TEXT_PADDING);
	m_layoutData.rects.textLeftReason = QRect();
}
void LLMChatFrame::layoutReasoningMessage(const QSize& reasoningSize, const QSize& answerSize, int timeExtraHeight)
{
	int reasoningHeight = std::max(reasoningSize.height(), LayoutConstants::MIN_HEIGHT);
	int answerHeight = std::max(answerSize.height(), LayoutConstants::MIN_HEIGHT);
	int bubbleTopY = timeExtraHeight + static_cast<int>(m_layoutData.iLineHeight * 0.75);
	m_layoutData.rects.triangleLeft = QRect(LayoutConstants::ICON_SIZE + LayoutConstants::ICON_SPACING + LayoutConstants::ICON_BORDER_WIDTH,
		timeExtraHeight + m_layoutData.iLineHeight / 2, LayoutConstants::TRIANGLE_WIDTH,
		reasoningHeight - m_layoutData.iLineHeight);
	m_layoutData.rects.triangleRight = QRect(width() - LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::ICON_SIZE - LayoutConstants::ICON_SPACING - LayoutConstants::TRIANGLE_WIDTH,
		timeExtraHeight + m_layoutData.iLineHeight / 2, LayoutConstants::TRIANGLE_WIDTH,
		reasoningHeight - m_layoutData.iLineHeight);
	bool useContentWidth = answerSize.width() < (m_layoutData.iTextWidth + m_layoutData.iSpaceWidth);
	int reasoningFrameWidth = useContentWidth
		? computeFrameWidth(reasoningSize.width(), m_layoutData.iTextWidth, m_layoutData.iSpaceWidth)
		: m_layoutData.iFrameWidth;
	int answerFrameWidth = useContentWidth
		? computeFrameWidth(answerSize.width(), m_layoutData.iTextWidth, m_layoutData.iSpaceWidth)
		: m_layoutData.iFrameWidth;
	m_layoutData.rects.frameLeftReason = QRect(m_layoutData.rects.triangleLeft.x() + m_layoutData.rects.triangleLeft.width(), bubbleTopY,
		reasoningFrameWidth, reasoningHeight - m_layoutData.iLineHeight);
	m_layoutData.rects.frameLeft = QRect(m_layoutData.rects.triangleLeft.x() + m_layoutData.rects.triangleLeft.width(),
		reasoningHeight - m_layoutData.iLineHeight / 4 + timeExtraHeight,
		answerFrameWidth, answerHeight - m_layoutData.iLineHeight);
	int rightX = useContentWidth
		? width() - answerSize.width() + m_layoutData.iSpaceWidth - 2 * LayoutConstants::TEXT_PADDING - LayoutConstants::ICON_SIZE - LayoutConstants::ICON_SPACING - LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::TRIANGLE_WIDTH
		: LayoutConstants::ICON_SIZE + LayoutConstants::FRAME_MARGIN + LayoutConstants::ICON_SPACING + LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::TRIANGLE_WIDTH;
	m_layoutData.rects.frameRight = QRect(rightX, bubbleTopY,
		answerFrameWidth, answerHeight - m_layoutData.iLineHeight);
	m_layoutData.rects.frameLeftReason.adjust(0, 0, 0, LayoutConstants::ICON_MARGIN);
	m_layoutData.rects.frameLeft.adjust(0, 0, 0, LayoutConstants::ICON_MARGIN);
	m_layoutData.rects.frameRight.adjust(0, 0, 0, LayoutConstants::ICON_MARGIN);
	m_layoutData.rects.textLeftReason = QRect(m_layoutData.rects.frameLeftReason.x() + LayoutConstants::TEXT_PADDING,
		m_layoutData.rects.frameLeftReason.y() + LayoutConstants::ICON_MARGIN,
		m_layoutData.rects.frameLeftReason.width() - 2 * LayoutConstants::TEXT_PADDING,
		m_layoutData.rects.frameLeftReason.height() - LayoutConstants::ICON_MARGIN - LayoutConstants::TEXT_PADDING);
	m_layoutData.rects.textLeft = QRect(m_layoutData.rects.frameLeft.x() + LayoutConstants::TEXT_PADDING,
		m_layoutData.rects.frameLeft.y() + LayoutConstants::ICON_MARGIN,
		m_layoutData.rects.frameLeft.width() - 2 * LayoutConstants::TEXT_PADDING,
		m_layoutData.rects.frameLeft.height() - LayoutConstants::ICON_MARGIN - LayoutConstants::TEXT_PADDING);
	m_layoutData.rects.textRight = QRect(m_layoutData.rects.frameRight.x() + LayoutConstants::TEXT_PADDING,
		m_layoutData.rects.frameRight.y() + LayoutConstants::ICON_MARGIN,
		m_layoutData.rects.frameRight.width() - 2 * LayoutConstants::TEXT_PADDING,
		m_layoutData.rects.frameRight.height() - LayoutConstants::ICON_MARGIN - LayoutConstants::TEXT_PADDING);
}
void LLMChatFrame::setText(QString text, QString time, QSize allSize, LLMChatFrame::User_Type userType)
{
	QString AnswerHtml = markdownToHtml(text);
	m_messageData.msg = AnswerHtml;
	m_UserType = userType;
	m_messageData.time = time;
	m_messageData.curTime = QDateTime::fromTime_t(time.toInt()).toString("hh:mm");
	m_layoutData.allSize = m_layoutData.allSize;
	if (userType == User_Customer && !m_state.isSending)
	{
		m_ui.loading.label->move(m_layoutData.rects.frameLeft.x() - m_ui.loading.label->width() - 10,
			m_layoutData.rects.frameLeft.y() + m_layoutData.rects.frameLeft.height() / 2 - m_ui.loading.label->height() / 2);
		m_ui.loading.label->show();
		m_ui.loading.movie->start();
	}
	else
	{
		m_ui.loading.label->hide();
	}
	this->update();
}
void LLMChatFrame::setTextWithReason(const QString& reasoning, const QString& answer, QString time, QSize allSize, User_Type userType)
{
	QString AnswerHtml = markdownToHtml(answer);
	m_messageData.msg = AnswerHtml;
	m_messageData.rawMsg = AnswerHtml;
	m_UserType = userType;
	m_messageData.time = time;
	m_messageData.curTime = QDateTime::fromTime_t(time.toInt()).toString("hh:mm");
	m_layoutData.allSize = allSize;
	if (userType == User_Customer)
	{
		QString ReasoningHtml = markdownToHtml(reasoning);
		m_messageData.rawReasoningMsg = ReasoningHtml;
		m_messageData.reasoningText = ReasoningHtml;//只有回答者才区分
		if (!m_state.isSending)
		{
			m_ui.loading.label->move(m_layoutData.rects.frameLeft.x() - m_ui.loading.label->width() - 10,
				m_layoutData.rects.frameLeft.y() + m_layoutData.rects.frameLeft.height() / 2 - m_ui.loading.label->height() / 2);
			m_ui.loading.label->show();
			m_ui.loading.movie->start();
		}
	}
	else
	{
		m_ui.loading.label->hide();
	}
	this->update();
}
void LLMChatFrame::setTextSuccess()
{
	m_ui.loading.label->hide();
	m_ui.loading.movie->stop();
	m_state.isSending = true;
	// 清除布局缓存，因为内容已更新
	m_layoutCache.isValid = false;
	m_layoutDirty = true;
	if (m_UserType == User_Customer)
	{
		if (!m_messageData.reasoningText.isEmpty())
		{
			fontRect(m_messageData.reasoningText, m_messageData.msg);
		}
		else
		{
			fontRect(m_messageData.msg);
		}
	}
	updateButtonsVisibility();
	// 注意：fontRect()内部已经调用了update()，所以这里不需要再次调用
}
QSize LLMChatFrame::fontRect(const QString& str)
{
	if (str.isEmpty()) return QSize();

	// 检查布局缓存（如果文本、宽度未改变，且缓存有效）
	int currentWidth = width();
	if (m_layoutCache.isValid &&
		m_layoutCache.cachedText == str &&
		m_layoutCache.cachedSize.isValid() &&
		m_layoutCache.cachedWidth == currentWidth &&
		m_layoutCache.cachedWidth > 0 &&
		!m_layoutDirty) {  // UI性能优化：如果布局标记为脏，即使缓存有效也要重新计算
						   // 缓存有效，布局是正确的，清除脏标志
		m_layoutDirty = false;
		return m_layoutCache.cachedSize;
	}

	QString html = str.contains("</p>") ? str : markdownToHtml(str);
	m_messageData.msg = html;
	calculateLayout();
	QSize size = getRealString(m_messageData.msg);
	int timeExtraHeight = computeTimeExtraHeight();
	int extraHeight = computeAdditionalHeight();
	layoutSingleMessage(size, timeExtraHeight);
	int bubbleHeight = std::max(size.height(), LayoutConstants::MIN_HEIGHT);
	m_layoutData.allSize = QSize(size.width(), bubbleHeight + timeExtraHeight + extraHeight);

	// 更新缓存
	m_layoutCache.cachedSize = m_layoutData.allSize;
	m_layoutCache.cachedText = str;
	m_layoutCache.cachedWidth = currentWidth;
	m_layoutCache.isValid = true;

	// UI性能优化：标记需要更新
	m_needsUpdate = true;
	// 确保UI会更新（调用者可能不会显式调用update）
	update();

	return m_layoutData.allSize;
}
QSize LLMChatFrame::fontRect(const QString& reasoning, const QString& answer)
{
	if (reasoning.isEmpty() && answer.isEmpty()) return QSize();
	m_messageData.reasoningText = reasoning.contains("</p>") ? reasoning : markdownToHtml(reasoning);
	m_messageData.msg = answer.contains("</p>") ? answer : markdownToHtml(answer);
	calculateLayout();
	QSize answerSize = getRealString(m_messageData.msg);
	QSize reasoningSize = getRealString(m_messageData.reasoningText);
	int timeExtraHeight = computeTimeExtraHeight();
	int extraHeight = computeAdditionalHeight();
	layoutReasoningMessage(reasoningSize, answerSize, timeExtraHeight);
	updateButtonsVisibility();
	// UI性能优化：标记需要更新
	m_needsUpdate = true;
	// 确保UI会更新（调用者可能不会显式调用update）
	update();
	if (this->getUserType() == User_Owner)
	{
		int answerHeight = std::max(answerSize.height(), LayoutConstants::MIN_HEIGHT);
		m_layoutData.allSize = QSize(answerSize.width(), answerHeight + timeExtraHeight + extraHeight);
	}
	else
	{
		int totalHeight = std::max(answerSize.height(), LayoutConstants::MIN_HEIGHT)
			+ std::max(reasoningSize.height(), LayoutConstants::MIN_HEIGHT);
		m_layoutData.allSize = QSize(reasoningSize.width(), totalHeight + timeExtraHeight + extraHeight);
	}
	return m_layoutData.allSize;
}
QSize LLMChatFrame::getRealString(QString src)
{
	QFontMetricsF fm(this->font());
	m_layoutData.iLineHeight = fm.lineSpacing();
	QRectF textRect = fm.boundingRect(QRectF(0, 0, m_layoutData.iTextWidth, 9999),
		Qt::TextWordWrap, src);
	int w = textRect.width();
	int h = textRect.height();
	// 加一点额外垂直间距，避免被切顶
	const int extra = 80;
	QTextDocument doc;
	setTextDocs(doc, src, w);
	QSize docSize = doc.size().toSize();
	return QSize(w + m_layoutData.iSpaceWidth, docSize.height() + extra);
}
QSize LLMChatFrame::getRealString(const QString& reasoning, const QString& answer)
{
	QFontMetricsF fm(this->font());
	m_layoutData.iLineHeight = fm.lineSpacing();
	QSize reasoningSize(0, 0);
	QSize answerSize(0, 0);
	if (!reasoning.isEmpty()) {
		QRectF reasoningRect = fm.boundingRect(QRectF(0, 0, m_layoutData.iTextWidth, 9999), Qt::TextWordWrap, reasoning);
		reasoningSize = QSize(reasoningRect.width(), reasoningRect.height());
	}
	if (!answer.isEmpty()) {
		QRectF answerRect = fm.boundingRect(QRectF(0, 0, m_layoutData.iTextWidth, 9999), Qt::TextWordWrap, answer);
		answerSize = QSize(answerRect.width(), answerRect.height());
	}
	const int extra = 80; // 额外垂直间距
	int w = qMax(reasoningSize.width(), answerSize.width()) + m_layoutData.iSpaceWidth;
	int h = reasoningSize.height() + answerSize.height() + extra;
	return QSize(w, h);
}
void LLMChatFrame::calculateLayout()
{
	int currentWidth = width();
	// UI性能优化：如果宽度没变且布局未标记为脏，跳过重新计算
	if (!m_layoutDirty && m_layoutCache.cachedWidth == currentWidth && m_layoutCache.cachedWidth > 0) {
		return;
	}

	m_layoutData.iFrameWidth = currentWidth - LayoutConstants::FRAME_MARGIN - 2 * (LayoutConstants::ICON_SIZE + LayoutConstants::ICON_SPACING + LayoutConstants::ICON_BORDER_WIDTH);
	m_layoutData.iTextWidth = m_layoutData.iFrameWidth - 2 * LayoutConstants::TEXT_PADDING;
	m_layoutData.iSpaceWidth = currentWidth - m_layoutData.iTextWidth;
	// 计算时间显示区域 - 在顶部
	if (m_UserType != User_Time)
	{
		m_layoutData.rects.timeRect = QRect(0, 0, currentWidth, LayoutConstants::TIME_HEIGHT);
	}
	// 调整图标位置，为时间显示预留空间
	int iconTopMargin = (m_UserType != User_Time) ?
		LayoutConstants::TIME_HEIGHT + LayoutConstants::TIME_MARGIN + LayoutConstants::ICON_MARGIN :
		LayoutConstants::ICON_MARGIN;
	m_layoutData.rects.iconLeft = QRect(LayoutConstants::ICON_SPACING, iconTopMargin,
		LayoutConstants::ICON_SIZE, LayoutConstants::ICON_SIZE);
	m_layoutData.rects.iconRight = QRect(currentWidth - LayoutConstants::ICON_SPACING - LayoutConstants::ICON_SIZE, iconTopMargin,
		LayoutConstants::ICON_SIZE, LayoutConstants::ICON_SIZE);

	// 更新缓存的宽度
	m_layoutCache.cachedWidth = currentWidth;
	m_layoutDirty = false;
}

void LLMChatFrame::appendText(const QString& delta)
{
	static const QString REASONING_EMPTY = QStringLiteral("\n ### 推理 \n\n\n");
	static const QString REASONING_HEADER = QStringLiteral("\n ### 推理 \n");
	static const QString ANSWER_HEADER = QStringLiteral("\n ### 回答 ");
	// 使用正确的标签：<think> 和 </think>
	static const QRegularExpression regexReasoningStart(QStringLiteral("<think>"));
	static const QRegularExpression regexReasoningEnd(QStringLiteral("</think>"));

	if (!m_state.isReasoning && m_messageData.rawReasoningMsg == REASONING_EMPTY)
	{
		m_messageData.rawReasoningMsg.append(tr("Reasoning not enabled"));
	}
	if (m_state.isReasoning)
	{
		m_messageData.reasoningText += delta;
		m_messageData.rawReasoningMsg += delta;
		m_messageData.rawReasoningMsg.replace(regexReasoningStart, REASONING_HEADER);
		m_messageData.rawReasoningMsg.replace(regexReasoningEnd, QString());
	}
	else
	{
		if (!delta.trimmed().isEmpty() &&
			!m_messageData.rawMsg.contains(QStringLiteral("### 回答")))
		{
			const QString header = QStringLiteral("\n ### 回答 \n\n");
			m_messageData.msg += header;
			m_messageData.rawMsg += header;
		}
		m_messageData.msg += delta;
		m_messageData.rawMsg += delta;
		m_messageData.rawMsg.replace(regexReasoningStart, REASONING_HEADER);
		m_messageData.rawMsg.replace(regexReasoningEnd, QString());
	}
}

void LLMChatFrame::drawTimeLabel(QPainter& painter)
{
	if (m_UserType == User_Time || m_messageData.curTime.isEmpty())
		return;
	QFont timeFont = this->font();
	timeFont.setFamily("Microsoft YaHei");
	timeFont.setPointSize(9);
	painter.setFont(timeFont);
	QFontMetrics metrics(timeFont);
	QString timeText = m_messageData.curTime;
	int textWidth = metrics.horizontalAdvance(timeText);
	int textHeight = metrics.height();
	int chipWidth = textWidth + LayoutConstants::TIME_HORIZONTAL_PADDING * 2;
	int chipHeight = textHeight + LayoutConstants::TIME_VERTICAL_PADDING * 2;
	int chipY = LayoutConstants::TIME_MARGIN;
	int chipX = (m_UserType == User_Customer)
		? m_layoutData.rects.iconLeft.x()
		: m_layoutData.rects.iconRight.right() - chipWidth;
	QRect chipRect(chipX, chipY, chipWidth, chipHeight);
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, true);
	QLinearGradient chipGradient(chipRect.topLeft(), chipRect.bottomLeft());
	QColor top = ColorScheme::TIME_BACKGROUND;
	QColor bottom = ColorScheme::TIME_BACKGROUND;
	bottom.setAlpha(qRound(bottom.alpha() * 0.6));
	chipGradient.setColorAt(0.0, top);
	chipGradient.setColorAt(1.0, bottom);
	painter.setPen(Qt::NoPen);
	painter.setBrush(chipGradient);
	painter.drawRoundedRect(chipRect, chipHeight / 2.0, chipHeight / 2.0);
	painter.restore();
	painter.save();
	painter.setPen(ColorScheme::TIME_TEXT);
	painter.drawText(chipRect, Qt::AlignCenter, timeText);
	painter.restore();
}
void LLMChatFrame::drawCustomerMessage(QPainter& painter)
{
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, true);
	QRect iconRect = m_layoutData.rects.iconLeft;
	QRect ringRect = iconRect.adjusted(-LayoutConstants::ICON_RING_MARGIN, -LayoutConstants::ICON_RING_MARGIN,
		LayoutConstants::ICON_RING_MARGIN, LayoutConstants::ICON_RING_MARGIN);
	QLinearGradient ringGradient(ringRect.topLeft(), ringRect.bottomLeft());
	ringGradient.setColorAt(0.0, ColorScheme::ICON_RING_OUTER);
	ringGradient.setColorAt(1.0, ColorScheme::ICON_RING_INNER);
	painter.setPen(Qt::NoPen);
	painter.setBrush(ringGradient);
	painter.drawEllipse(ringRect);
	painter.drawPixmap(iconRect, *m_ui.icons.leftPix);
	painter.restore();
	QColor reasoningShadow = ColorScheme::SHADOW_COLOR;
	reasoningShadow.setAlpha(qRound(reasoningShadow.alpha() * 0.7));
	drawBubble(painter, m_layoutData.rects.frameLeftReason, QRect(), true,
		ColorScheme::REASONING_BACKGROUND, ColorScheme::REASONING_BORDER,
		reasoningShadow, LayoutConstants::BORDER_RADIUS, LayoutConstants::SHADOW_OFFSET / 2);
	drawBubble(painter, m_layoutData.rects.frameLeft, m_layoutData.rects.triangleLeft, true,
		ColorScheme::ANSWER_BACKGROUND, ColorScheme::ANSWER_BORDER,
		ColorScheme::SHADOW_COLOR, LayoutConstants::BORDER_RADIUS, LayoutConstants::SHADOW_OFFSET);
	const QString primaryTextColor = QStringLiteral("#0F172A");
	const bool hasReasoningBubble = m_layoutData.rects.frameLeftReason.isValid();
	if (m_state.isStreamEnd)
	{
		QTextDocument docAnswer;
		setTextDocs(docAnswer, markdownToHtml(m_messageData.rawMsg), m_layoutData.rects.textLeft.width(), primaryTextColor);
		painter.save();
		painter.translate(m_layoutData.rects.textLeft.topLeft());
		docAnswer.documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
		painter.restore();
		if (hasReasoningBubble)
		{
			QTextDocument docReasoning;
			setTextDocs(docReasoning, markdownToHtml(m_messageData.rawReasoningMsg), m_layoutData.rects.textLeftReason.width(), primaryTextColor);
			painter.save();
			painter.translate(m_layoutData.rects.textLeftReason.topLeft());
			docReasoning.documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
			painter.restore();
		}
	}
	else
	{
		if (hasReasoningBubble)
		{
			QTextDocument docReasoning;
			setTextDocs(docReasoning, m_messageData.reasoningText, m_layoutData.rects.textLeftReason.width(), primaryTextColor);
			painter.save();
			painter.translate(m_layoutData.rects.textLeftReason.topLeft());
			docReasoning.documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
			painter.restore();
		}
		QTextDocument docAnswer;
		setTextDocs(docAnswer, m_messageData.msg, m_layoutData.rects.textLeft.width(), primaryTextColor);
		painter.save();
		painter.translate(m_layoutData.rects.textLeft.topLeft());
		docAnswer.documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
		painter.restore();
	}
}
void LLMChatFrame::drawOwnerMessage(QPainter& painter)
{
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, true);
	QRect iconRect = m_layoutData.rects.iconRight;
	QRect ringRect = iconRect.adjusted(-LayoutConstants::ICON_RING_MARGIN, -LayoutConstants::ICON_RING_MARGIN,
		LayoutConstants::ICON_RING_MARGIN, LayoutConstants::ICON_RING_MARGIN);
	QLinearGradient ringGradient(ringRect.topLeft(), ringRect.bottomLeft());
	QColor ringTop = ColorScheme::USER_BACKGROUND;
	ringTop.setAlpha(110);
	QColor ringBottom = ColorScheme::USER_BACKGROUND;
	ringBottom.setAlpha(60);
	ringGradient.setColorAt(0.0, ringTop);
	ringGradient.setColorAt(1.0, ringBottom);
	painter.setPen(Qt::NoPen);
	painter.setBrush(ringGradient);
	painter.drawEllipse(ringRect);
	painter.drawPixmap(iconRect, *m_ui.icons.rightPix);
	painter.restore();
	QColor ownerBorder = ColorScheme::USER_BACKGROUND.darker(115);
	drawBubble(painter, m_layoutData.rects.frameRight, m_layoutData.rects.triangleRight, false,
		ColorScheme::USER_BACKGROUND, ownerBorder,
		ColorScheme::SHADOW_COLOR, LayoutConstants::BORDER_RADIUS, LayoutConstants::SHADOW_OFFSET);
	QTextDocument doc;
	setTextDocs(doc, m_messageData.msg, m_layoutData.rects.textRight.width(), QStringLiteral("#F8FAFC"));
	painter.save();
	painter.translate(m_layoutData.rects.textRight.topLeft());
	doc.documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
	painter.restore();
}
void LLMChatFrame::drawTimeMessage(QPainter& painter)
{
	painter.setPen(ColorScheme::TIME_TEXT);
	QFont timeFont = font();
	timeFont.setFamily("MicrosoftYaHei");
	timeFont.setPointSize(10);
	painter.setFont(timeFont);
	QTextOption option(Qt::AlignCenter);
	option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	painter.drawText(rect(), m_messageData.curTime, option);
}
void LLMChatFrame::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	painter.setPen(Qt::NoPen);
	painter.setBrush(QBrush(Qt::gray));

	// UI性能优化：只绘制需要更新的区域（如果有）
	if (!event->region().isEmpty()) {
		painter.setClipRegion(event->region());
	}

	drawTimeLabel(painter);
	if (m_UserType == User_Type::User_Customer)
	{
		drawCustomerMessage(painter);
	}
	else if (m_UserType == User_Type::User_Owner)
	{
		drawOwnerMessage(painter);
	}
	else if (m_UserType == User_Type::User_Time)
	{
		drawTimeMessage(painter);
	}

	// 清除更新标志
	m_needsUpdate = false;
}
void LLMChatFrame::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	// UI性能优化：只有在宽度真正改变时才重新计算
	int oldWidth = m_layoutCache.cachedWidth;
	int newWidth = width();

	if (oldWidth != newWidth) {
		// 清除布局缓存，因为尺寸已经改变
		m_layoutCache.isValid = false;
		m_layoutDirty = true;

		// 当尺寸变化时重新计算布局
		// 注意：fontRect()内部已经调用了update()，所以这里不需要再次调用
		if (m_UserType != User_Time)
		{
			if (!m_messageData.reasoningText.isEmpty() && m_UserType == User_Customer)
				fontRect(m_messageData.reasoningText, m_messageData.msg);
			else
				fontRect(m_messageData.msg);
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
			m_state.savedScrollPosition = scrollBar->value();
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
		if (scrollBar && m_state.savedScrollPosition >= 0)
		{
			scrollBar->setValue(m_state.savedScrollPosition);
			m_state.savedScrollPosition = -1;
		}
	}
	event->accept();
}
void LLMChatFrame::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	QAction* copyAction = menu.addAction(tr("Copy"));
	QAction* selectedAction = menu.exec(event->globalPos());
	if (selectedAction == copyAction)
	{
		QClipboard *clipboard = QApplication::clipboard();
		QTextDocument doc;
		QPoint SelectPoint = this->mapFromGlobal(event->globalPos());
		if (m_layoutData.rects.frameLeftReason.contains(SelectPoint))
		{
			doc.setHtml(m_messageData.reasoningText);
		}
		else
		{
			doc.setHtml(m_messageData.msg);
		}
		QString plainText = doc.toPlainText();
		clipboard->setText(plainText);
	}
}