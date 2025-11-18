#include "LLMChatFrame.h"
#include <QFontMetrics>
#include <QPaintEvent>
#include <QDateTime>
#include <QPainter>
#include <QMovie>
#include <QLabel>
#include <QColor>
#include <QPainterPath>
#include <QLinearGradient>
#include <QEvent>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QtGlobal>
#include <QHash>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QFileInfo>
#include <QMimeData>
#include <QInputDialog>
#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtMath>
#include <cmath>

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

void LLMChatFrame::copyToClipboardPlain(bool reasoningSection)
{
	const QString html = (reasoningSection && !m_messageData.reasoningText.isEmpty())
		? m_messageData.reasoningText
		: m_messageData.msg;
	const QString plainText = htmlToPlainText(html);
	if (plainText.isEmpty())
	{
		return;
	}
	if (QClipboard* clipboard = QApplication::clipboard())
	{
		clipboard->setText(plainText);
	}
}

void LLMChatFrame::copyToClipboardMarkdown(bool reasoningSection)
{
	const QString titleReasoning = tr("### Thinking");
	const QString titleAnswer = tr("### Answer");
	const QString html = (reasoningSection && !m_messageData.reasoningText.isEmpty())
		? m_messageData.reasoningText
		: m_messageData.msg;
	const QString plainText = htmlToPlainText(html).trimmed();
	if (plainText.isEmpty())
	{
		return;
	}
	const QString heading = (reasoningSection && !m_messageData.reasoningText.isEmpty()) ? titleReasoning : titleAnswer;
	const QString markdown = QStringLiteral("%1\n\n%2").arg(heading, plainText);
	if (QClipboard* clipboard = QApplication::clipboard())
	{
		clipboard->setText(markdown);
	}
}

void LLMChatFrame::copyToClipboardHtml(bool reasoningSection)
{
	const QString html = (reasoningSection && !m_messageData.reasoningText.isEmpty())
		? m_messageData.reasoningText
		: m_messageData.msg;
	if (html.isEmpty())
	{
		return;
	}
	if (QClipboard* clipboard = QApplication::clipboard())
	{
		auto* mimeData = new QMimeData();
		mimeData->setHtml(html);
		mimeData->setText(htmlToPlainText(html));
		clipboard->setMimeData(mimeData);
	}
}

QString LLMChatFrame::buildPlainExport() const
{
	QStringList sections;
	if (!m_messageData.reasoningText.isEmpty())
	{
		const QString reasoning = htmlToPlainText(m_messageData.reasoningText).trimmed();
		if (!reasoning.isEmpty())
		{
			sections << tr("### Thinking") << reasoning;
		}
	}
	const QString answer = htmlToPlainText(m_messageData.msg).trimmed();
	if (!answer.isEmpty())
	{
		sections << tr("### Answer") << answer;
	}
	return sections.join(QStringLiteral("\n\n")).trimmed();
}

QString LLMChatFrame::buildMarkdownExport() const
{
	QStringList lines;
	if (!m_messageData.reasoningText.isEmpty())
	{
		const QString reasoning = htmlToPlainText(m_messageData.reasoningText).trimmed();
		if (!reasoning.isEmpty())
		{
			lines << tr("### Thinking") << QString() << reasoning << QString();
		}
	}
	const QString answer = htmlToPlainText(m_messageData.msg).trimmed();
	if (!answer.isEmpty())
	{
		lines << tr("### Answer") << QString() << answer;
	}
	return lines.join(QStringLiteral("\n")).trimmed();
}

QString LLMChatFrame::buildHtmlExport() const
{
	const QString roleClass = (m_UserType == User_Customer)
		? QStringLiteral("assistant")
		: (m_UserType == User_Owner ? QStringLiteral("user") : QStringLiteral("system"));
	QString reasoningSection;
	if (!m_messageData.reasoningText.isEmpty())
	{
		reasoningSection = QStringLiteral(
			"<section class=\"reasoning\">"
			"<h3>%1</h3>%2"
			"</section>").arg(tr("Thinking"), m_messageData.reasoningText);
	}
	QString answerSection = QStringLiteral(
		"<section class=\"answer\">"
		"<h3>%1</h3>%2"
		"</section>").arg(tr("Answer"), m_messageData.msg);

static const QString kTemplate = QStringLiteral(
	R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<title>%1</title>
<style>
body { font-family: "Helvetica Neue", "Microsoft YaHei", sans-serif; background-color: #f9fafb; padding: 40px; color: #0f172a; }
.chat-bubble { max-width: 680px; margin: 0 auto; background: #ffffff; border-radius: 20px; box-shadow: 0 24px 48px rgba(15, 23, 42, 0.08); padding: 32px 40px; line-height: 1.75; }
.chat-bubble.assistant { border-top: 6px solid #3b82f6; }
.chat-bubble.user { border-top: 6px solid #10b981; }
.chat-bubble.system { border-top: 6px solid #6366f1; }
.chat-bubble h3 { margin-top: 0; font-size: 20px; color: #111827; }
.chat-bubble section { margin-bottom: 28px; }
.chat-bubble section:last-child { margin-bottom: 0; }
pre { background: #0f172a; color: #e2e8f0; padding: 12px 16px; border-radius: 12px; overflow-x: auto; font-family: "JetBrains Mono", "Courier New", monospace; }
code { background: rgba(15, 23, 42, 0.08); padding: 2px 6px; border-radius: 6px; font-size: 90%; }
</style>
</head>
<body>
<article class="chat-bubble %2">
%3
%4
</article>
</body>
</html>)");

	return kTemplate.arg(tr("Export Chat"), roleClass, reasoningSection, answerSection);
}

QString LLMChatFrame::htmlToPlainText(const QString& html) const
{
	if (html.isEmpty())
	{
		return QString();
	}
	QTextDocument doc;
	doc.setHtml(html);
	return doc.toPlainText();
}

void LLMChatFrame::setImportant(bool important)
{
	if (m_messageData.isImportant == important)
	{
		return;
	}
	m_messageData.isImportant = important;
	update();
}

void LLMChatFrame::setUserNote(const QString& note)
{
	QString trimmed = note;
	m_messageData.userNote = trimmed;
	applyNoteToolTip();
	update();
}

void LLMChatFrame::refreshLayoutAfterContentChange()
{
	m_layoutCache.isValid = false;
	m_docCache.invalidate(); // 使文档缓存失效
	m_layoutDirty = true;
	if (m_UserType == User_Customer)
	{
		fontRect(m_messageData.reasoningText, m_messageData.msg);
	}
	else
	{
		fontRect(m_messageData.msg);
	}
	updateButtonsVisibility();
	update();
}

void LLMChatFrame::drawImportanceBadge(QPainter& painter, const QRect& bubbleRect)
{
	if (!bubbleRect.isValid())
	{
		return;
	}
	const int badgeSize = 18;
	const QPointF center = (m_UserType == User_Customer)
		? QPointF(bubbleRect.left() + badgeSize * 0.5 + 8, bubbleRect.top() + badgeSize * 0.5 + 6)
		: QPointF(bubbleRect.right() - badgeSize * 0.5 - 8, bubbleRect.top() + badgeSize * 0.5 + 6);
	const qreal outerRadius = badgeSize * 0.5;
	const qreal innerRadius = outerRadius * 0.5;

	QPolygonF star;
	for (int i = 0; i < 10; ++i)
	{
		const qreal angle = qDegreesToRadians(36.0 * i - 90.0);
		const qreal radius = (i % 2 == 0) ? outerRadius : innerRadius;
		star << QPointF(center.x() + radius * std::cos(angle),
			center.y() + radius * std::sin(angle));
	}

	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setPen(QPen(QColor(217, 119, 6), 1.2));
	painter.setBrush(QColor(252, 211, 77));
	painter.drawPolygon(star);
	painter.restore();
}

void LLMChatFrame::drawNoteBadge(QPainter& painter, const QRect& bubbleRect)
{
	if (m_messageData.userNote.trimmed().isEmpty())
	{
		return;
	}
	if (!bubbleRect.isValid())
	{
		return;
	}
	const QString badgeText = tr("NOTE");
	QFont badgeFont = font();
	badgeFont.setPointSize(8);
	badgeFont.setBold(true);

	QFontMetrics metrics(badgeFont);
	const int paddingH = 6;
	const int paddingV = 2;
	const int textWidth = metrics.horizontalAdvance(badgeText);
	const int textHeight = metrics.height();
	const QSize badgeSize(textWidth + paddingH * 2, textHeight + paddingV * 2);

	const QPoint badgePos = (m_UserType == User_Customer)
		? QPoint(bubbleRect.right() - badgeSize.width() - 12, bubbleRect.bottom() - badgeSize.height() - 8)
		: QPoint(bubbleRect.left() + 12, bubbleRect.bottom() - badgeSize.height() - 8);
	const QRect badgeRect(badgePos, badgeSize);

	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setBrush(QColor(14, 165, 233, 200));
	painter.setPen(Qt::NoPen);
	painter.drawRoundedRect(badgeRect, badgeRect.height() / 2.0, badgeRect.height() / 2.0);
	painter.setPen(Qt::white);
	painter.setFont(badgeFont);
	painter.drawText(badgeRect, Qt::AlignCenter, badgeText);
	painter.restore();
}

void LLMChatFrame::applyNoteToolTip()
{
	const QString trimmed = m_messageData.userNote.trimmed();
	if (trimmed.isEmpty())
	{
		setToolTip(QString());
	}
	else
	{
		setToolTip(tr("Note: %1").arg(trimmed));
	}
}

void LLMChatFrame::handleNoteRequested()
{
	QDialog dialog(this);
	dialog.setWindowTitle(tr("Edit Note"));
	dialog.setMinimumSize(500, 300);
	dialog.resize(500, 300);
	dialog.setModal(true);
	dialog.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	// 应用与软件整体风格一致的样式
	dialog.setStyleSheet(
		"QDialog {"
		"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
		"        stop:0 #eef2ff, stop:1 #e0f2fe);"
		"    border-radius: 12px;"
		"    font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;"
		"}"
		"QLabel {"
		"    color: #1e293b;"
		"    font-size: 14px;"
		"    font-weight: 600;"
		"    padding: 8px 0px;"
		"    background: transparent;"
		"}"
		"QTextEdit {"
		"    border: 1px solid rgba(203, 213, 225, 0.8);"
		"    border-radius: 8px;"
		"    padding: 12px;"
		"    background: rgba(248, 250, 252, 0.95);"
		"    selection-background-color: #3b82f6;"
		"    selection-color: white;"
		"    font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;"
		"    font-size: 14px;"
		"    color: #1e293b;"
		"    min-height: 150px;"
		"}"
		"QTextEdit:focus {"
		"    border-color: #3b82f6;"
		"    background: rgba(255, 255, 255, 0.98);"
		"}"
		"QPushButton {"
		"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
		"        stop:0 #3b82f6, stop:1 #1d4ed8);"
		"    border: none;"
		"    color: white;"
		"    padding: 10px 24px;"
		"    border-radius: 8px;"
		"    font-weight: 600;"
		"    font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;"
		"    font-size: 14px;"
		"    min-width: 90px;"
		"    min-height: 36px;"
		"}"
		"QPushButton:hover {"
		"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
		"        stop:0 #60a5fa, stop:1 #3b82f6);"
		"}"
		"QPushButton:pressed {"
		"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
		"        stop:0 #1d4ed8, stop:1 #1e40af);"
		"}"
		"QPushButton:disabled {"
		"    background: #cbd5e1;"
		"    color: #94a3b8;"
		"}"
	);

	QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
	mainLayout->setSpacing(16);
	mainLayout->setContentsMargins(24, 24, 24, 24);

	QLabel* label = new QLabel(tr("Note:"), &dialog);
	mainLayout->addWidget(label);

	QTextEdit* textEdit = new QTextEdit(&dialog);
	textEdit->setPlainText(m_messageData.userNote);
	textEdit->setPlaceholderText(tr("Enter your note here..."));
	mainLayout->addWidget(textEdit);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->setSpacing(12);
	buttonLayout->addStretch();

	QPushButton* cancelButton = new QPushButton(tr("Cancel"), &dialog);
	QPushButton* okButton = new QPushButton(tr("OK"), &dialog);
	okButton->setDefault(true);

	buttonLayout->addWidget(cancelButton);
	buttonLayout->addWidget(okButton);
	mainLayout->addLayout(buttonLayout);

	QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
	QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
	QObject::connect(textEdit, &QTextEdit::textChanged, [okButton, textEdit]() {
		// 可以在这里添加验证逻辑
	});

	// 居中显示对话框
	dialog.adjustSize();
	if (QWidget* parentWindow = window())
	{
		QPoint parentCenter = parentWindow->geometry().center();
		QRect dialogRect = dialog.geometry();
		dialog.move(parentCenter.x() - dialogRect.width() / 2, parentCenter.y() - dialogRect.height() / 2);
	}

	// 设置焦点到文本编辑框
	textEdit->setFocus();
	textEdit->selectAll();

	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	const QString input = textEdit->toPlainText();
	const QString trimmed = input.trimmed();
	if (trimmed == m_messageData.userNote)
	{
		return;
	}
	m_messageData.userNote = trimmed;
	applyNoteToolTip();
	update();
	if (!m_messageData.uniqueID.isEmpty())
	{
		emit bubbleNoteChanged(m_messageData.uniqueID, m_messageData.userNote);
	}
}

void LLMChatFrame::toggleImportant()
{
	m_messageData.isImportant = !m_messageData.isImportant;
	update();
	if (!m_messageData.uniqueID.isEmpty())
	{
		emit bubbleImportantToggled(m_messageData.uniqueID, m_messageData.isImportant);
	}
}

bool LLMChatFrame::exportTextToFile(const QString& content, const QString& dialogTitle, const QString& filter, const QString& suffix)
{
	if (content.isEmpty())
	{
		QMessageBox::information(this, tr("No Export Content"), tr("The current bubble has no exportable text."));
		return false;
	}

	QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
	if (defaultDir.isEmpty())
	{
		defaultDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
	}
	if (defaultDir.isEmpty())
	{
		defaultDir = QStringLiteral(".");
	}
	QString suggestedName = buildDefaultFileName(suffix);
	QString defaultPath = QDir(defaultDir).filePath(suggestedName);

	QString filePath = QFileDialog::getSaveFileName(this, dialogTitle, defaultPath, filter);
	if (filePath.isEmpty())
	{
		return false;
	}

	QFileInfo info(filePath);
	if (info.suffix().isEmpty() && !suffix.isEmpty())
	{
		filePath += QStringLiteral(".") + suffix;
	}

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, tr("Export Failed"), tr("Cannot write to file: %1").arg(file.errorString()));
		return false;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	out << content;
	file.close();
	return true;
}

bool LLMChatFrame::exportBubbleAsImage()
{
	QPixmap pixmap = this->grab();
	if (pixmap.isNull())
	{
		QMessageBox::warning(this, tr("Export Failed"), tr("Unable to capture the current bubble."));
		return false;
	}

	QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
	if (defaultDir.isEmpty())
	{
		defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
	}
	if (defaultDir.isEmpty())
	{
		defaultDir = QStringLiteral(".");
	}
	QString defaultPath = QDir(defaultDir).filePath(buildDefaultFileName(QStringLiteral("png")));

	QString filePath = QFileDialog::getSaveFileName(
		this,
		tr("Export As Image"),
		defaultPath,
		tr("PNG Images (*.png);;JPEG Images (*.jpg *.jpeg);;WebP Images (*.webp)")
	);
	if (filePath.isEmpty())
	{
		return false;
	}

	QFileInfo info(filePath);
	QString chosenSuffix = info.suffix().toLower();
	if (chosenSuffix.isEmpty())
	{
		chosenSuffix = QStringLiteral("png");
		filePath += QStringLiteral(".png");
	}

	if (!pixmap.save(filePath, chosenSuffix.toUpper().toUtf8().constData()))
	{
		QMessageBox::warning(this, tr("Export Failed"), tr("Unable to save the image file. Please check write permissions."));
		return false;
	}
	return true;
}

QString LLMChatFrame::buildDefaultFileName(const QString& suffix) const
{
	QDateTime timestamp;
	bool ok = false;
	const qint64 epoch = m_messageData.time.toLongLong(&ok);
	if (ok)
	{
		timestamp = QDateTime::fromSecsSinceEpoch(epoch);
	}
	if (!timestamp.isValid())
	{
		timestamp = QDateTime::currentDateTime();
	}
	const QString role = (m_UserType == User_Customer)
		? QStringLiteral("assistant")
		: (m_UserType == User_Owner ? QStringLiteral("user") : QStringLiteral("system"));
	const QString baseName = QStringLiteral("%1_%2")
		.arg(role, timestamp.toString(QStringLiteral("yyyyMMdd_HHmmss")));
	return suffix.isEmpty() ? baseName : QStringLiteral("%1.%2").arg(baseName, suffix);
}
LLMChatFrame::LLMChatFrame(QWidget *parent)
	: QWidget(parent)
{
	QFont font("Microsoft YaHei", 12);
	setFont(font);
	setAttribute(Qt::WA_Hover, true);
	initRescource();
	initTalkPic();
	initButtons();
	m_shadowEffect = new QGraphicsDropShadowEffect(this);
	m_shadowEffect->setBlurRadius(28);
	m_shadowEffect->setOffset(0, 12);
	m_shadowEffect->setColor(QColor(15, 23, 42, 70));
	m_shadowEffect->setEnabled(false);
	setGraphicsEffect(m_shadowEffect);
	setMouseTracking(true);
	m_allowDeferredDelete = false;
}
LLMChatFrame::~LLMChatFrame()
{
}

bool LLMChatFrame::event(QEvent* event)
{
	if (event->type() == QEvent::DeferredDelete)
	{
		if (!m_allowDeferredDelete)
		{
			event->setAccepted(false);
			return true;
		}
	}
	return QWidget::event(event);
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
		btn->installEventFilter(this);
		auto* effect = new QGraphicsDropShadowEffect(btn);
		effect->setBlurRadius(18);
		effect->setOffset(0, 6);
		effect->setColor(QColor(37, 99, 235, 120));
		btn->setGraphicsEffect(effect);
	}
	// 设置图标和工具提示
	m_ui.buttons.copyThinking->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_CopyThink.png"));
	m_ui.buttons.copyThinking->setToolTip(tr("Copy Reasoning"));
	m_ui.buttons.copyAnswer->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_CopyAnswer.png"));
	m_ui.buttons.copyAnswer->setToolTip(tr("Copy Answer"));
	m_ui.buttons.regenerate->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_Regenerate.png"));
	m_ui.buttons.regenerate->setToolTip(tr("Regenerate Response"));
	// 连接信号槽
	connect(m_ui.buttons.copyThinking.get(), &QPushButton::clicked, this, &LLMChatFrame::onCopyThinkingClicked);
	connect(m_ui.buttons.copyAnswer.get(), &QPushButton::clicked, this, &LLMChatFrame::onCopyAnswerClicked);
	connect(m_ui.buttons.regenerate.get(), &QPushButton::clicked, this, &LLMChatFrame::onRegenerateClicked);
	m_ui.buttons.collapseToggle->setToolTip(tr("Collapse/Expand Message"));
	m_ui.buttons.collapseToggle->setText("-");
	m_ui.buttons.collapseToggle->setFixedSize(28, 28);
	m_ui.buttons.collapseToggle->setStyleSheet(
		"QPushButton {"
		"    background-color: rgba(255, 255, 255, 0.8);"
		"    border: none;"
		"    color: #64748b;"
		"    font-size: 16px;"
		"    font-weight: bold;"
		"    border-radius: 4px;"
		"}"
		"QPushButton:hover {"
		"    background-color: rgba(100, 116, 139, 0.15);"
		"}"
		"QPushButton:pressed {"
		"    background-color: rgba(100, 116, 139, 0.25);"
		"}"
	);
	connect(m_ui.buttons.collapseToggle.get(), &QPushButton::clicked, this, &LLMChatFrame::onCollapseToggleClicked);
	
	m_ui.buttons.reasoningCollapseToggle->setToolTip(tr("Collapse/Expand Reasoning"));
	m_ui.buttons.reasoningCollapseToggle->setText("-");
	m_ui.buttons.reasoningCollapseToggle->setFixedSize(28, 28);
	m_ui.buttons.reasoningCollapseToggle->setStyleSheet(
		"QPushButton {"
		"    background-color: rgba(255, 255, 255, 0.8);"
		"    border: none;"
		"    color: #64748b;"
		"    font-size: 16px;"
		"    font-weight: bold;"
		"    border-radius: 4px;"
		"}"
		"QPushButton:hover {"
		"    background-color: rgba(100, 116, 139, 0.15);"
		"}"
		"QPushButton:pressed {"
		"    background-color: rgba(100, 116, 139, 0.25);"
		"}"
	);
	connect(m_ui.buttons.reasoningCollapseToggle.get(), &QPushButton::clicked, this, &LLMChatFrame::onReasoningCollapseToggleClicked);
	
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
	m_ui.buttons.collapseToggle = std::make_unique<QPushButton>(this);
	m_ui.buttons.reasoningCollapseToggle = std::make_unique<QPushButton>(this);
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
	
	// 设置推理框折叠/展开按钮
	if (m_layoutData.rects.frameLeftReason.isValid())
	{
		int collapseButtonX = m_layoutData.rects.frameLeftReason.right() - 30;
		int collapseButtonY = m_layoutData.rects.frameLeftReason.top() + 5;
		m_ui.buttons.reasoningCollapseToggle->setGeometry(collapseButtonX, collapseButtonY, 28, 28);
		m_ui.buttons.reasoningCollapseToggle->show();
		m_ui.buttons.reasoningCollapseToggle->setText(m_messageData.isReasoningCollapsed ? "+" : "-");
	}
	else
	{
		m_ui.buttons.reasoningCollapseToggle->hide();
	}
	
	// 设置回答框折叠/展开按钮
	if (m_layoutData.rects.frameLeft.isValid())
	{
		int collapseButtonX = m_layoutData.rects.frameLeft.right() - 30;
		int collapseButtonY = m_layoutData.rects.frameLeft.top() + 5;
		m_ui.buttons.collapseToggle->setGeometry(collapseButtonX, collapseButtonY, 28, 28);
		m_ui.buttons.collapseToggle->show();
		m_ui.buttons.collapseToggle->setText(m_messageData.isCollapsed ? "+" : "-");
	}
	else
	{
		m_ui.buttons.collapseToggle->hide();
	}
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
		// 折叠按钮始终显示（如果消息不是时间类型）
		if (m_UserType != User_Time && m_UserType != User_System)
		{
			// 设置推理框折叠按钮
			if (m_UserType == User_Customer && m_layoutData.rects.frameLeftReason.isValid())
			{
				int collapseButtonX = m_layoutData.rects.frameLeftReason.right() - 30;
				int collapseButtonY = m_layoutData.rects.frameLeftReason.top() + 5;
				m_ui.buttons.reasoningCollapseToggle->setGeometry(collapseButtonX, collapseButtonY, 28, 28);
				m_ui.buttons.reasoningCollapseToggle->show();
				m_ui.buttons.reasoningCollapseToggle->setText(m_messageData.isReasoningCollapsed ? "+" : "-");
			}
			else
			{
				m_ui.buttons.reasoningCollapseToggle->hide();
			}
			
			// 设置回答框折叠按钮
			const QRect& answerBubbleRect = (m_UserType == User_Customer)
				? m_layoutData.rects.frameLeft
				: m_layoutData.rects.frameRight;
			if (answerBubbleRect.isValid())
			{
				int collapseButtonX = answerBubbleRect.right() - 30;
				int collapseButtonY = answerBubbleRect.top() + 5;
				m_ui.buttons.collapseToggle->setGeometry(collapseButtonX, collapseButtonY, 28, 28);
				m_ui.buttons.collapseToggle->show();
				m_ui.buttons.collapseToggle->setText(m_messageData.isCollapsed ? "+" : "-");
			}
			else
			{
				m_ui.buttons.collapseToggle->hide();
			}
		}
		else
		{
			m_ui.buttons.collapseToggle->hide();
			m_ui.buttons.reasoningCollapseToggle->hide();
		}
		// 隐藏其他按钮
		if (m_ui.buttons.copyThinking) m_ui.buttons.copyThinking->hide();
		if (m_ui.buttons.copyAnswer) m_ui.buttons.copyAnswer->hide();
		if (m_ui.buttons.regenerate) m_ui.buttons.regenerate->hide();
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

void LLMChatFrame::setCollapsed(bool collapsed)
{
	if (m_messageData.isCollapsed == collapsed)
	{
		return;
	}
	m_messageData.isCollapsed = collapsed;
	m_docCache.invalidate(); // 折叠状态改变，使文档缓存失效
	refreshLayoutAfterContentChange();
	if (!m_messageData.uniqueID.isEmpty())
	{
		emit bubbleCollapsedToggled(m_messageData.uniqueID, m_messageData.isCollapsed);
	}
}

void LLMChatFrame::toggleCollapsed()
{
	setCollapsed(!m_messageData.isCollapsed);
}

void LLMChatFrame::onCollapseToggleClicked()
{
	toggleCollapsed();
}

void LLMChatFrame::onReasoningCollapseToggleClicked()
{
	toggleReasoningCollapsed();
}

QString LLMChatFrame::getCollapsedSummary() const
{
	QString content = m_messageData.rawMsg.isEmpty() ? m_messageData.msg : m_messageData.rawMsg;
	QTextDocument doc;
	doc.setHtml(content);
	QString plainText = doc.toPlainText();
	
	// 获取前100个字符作为摘要
	const int maxLength = 100;
	if (plainText.length() > maxLength)
	{
		return plainText.left(maxLength) + "...";
	}
	return plainText;
}

QString LLMChatFrame::getReasoningCollapsedSummary() const
{
	QString content = m_messageData.rawReasoningMsg.isEmpty() ? m_messageData.reasoningText : m_messageData.rawReasoningMsg;
	QTextDocument doc;
	doc.setHtml(content);
	QString plainText = doc.toPlainText();
	
	// 获取前100个字符作为摘要
	const int maxLength = 100;
	if (plainText.length() > maxLength)
	{
		return plainText.left(maxLength) + "...";
	}
	return plainText;
}

void LLMChatFrame::setReasoningCollapsed(bool collapsed)
{
	if (m_messageData.isReasoningCollapsed == collapsed)
	{
		return;
	}
	m_messageData.isReasoningCollapsed = collapsed;
	m_docCache.invalidate(); // 折叠状态改变，使文档缓存失效
	refreshLayoutAfterContentChange();
	if (!m_messageData.uniqueID.isEmpty())
	{
		emit bubbleCollapsedToggled(m_messageData.uniqueID, m_messageData.isReasoningCollapsed);
	}
}

void LLMChatFrame::toggleReasoningCollapsed()
{
	setReasoningCollapsed(!m_messageData.isReasoningCollapsed);
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
		// 从高亮后的HTML中提取代码块ID并保存代码内容
		static const QRegularExpression idRegex("codeblock://copy\\?id=([^\"]+)");
		QRegularExpressionMatch idMatch = idRegex.match(highlightedHtml);
		if (idMatch.hasMatch())
		{
			QString codeBlockId = idMatch.captured(1);
			m_codeBlockMap[codeBlockId] = code;
		}
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
	const QString baseStyle = QStringLiteral(
		"font-family:'Source Sans Pro','Microsoft YaHei','Segoe UI',sans-serif;"
		"font-size:14.5px; line-height:1.7;"
		);
	if (!defaultColor.isEmpty())
	{
		html = QStringLiteral("<div style=\"color:%1;%2\">%3</div>")
			.arg(defaultColor, baseStyle, src);
	}
	else
	{
		html = QStringLiteral("<div style=\"%1\">%2</div>")
			.arg(baseStyle, src);
	}
	docText.setHtml(html);
	QFont font("MicrosoftYaHei", 12);
	docText.setDefaultFont(font);
	QTextOption option(Qt::AlignLeft | Qt::AlignVCenter);
	option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	docText.setDefaultTextOption(option);
	docText.setTextWidth(iWidth);
	
	// 性能优化：禁用不必要的功能以提升渲染速度
	docText.setUndoRedoEnabled(false);
	// 使用更快的渲染模式（如果可用）
	docText.setUseDesignMetrics(false);
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
	const int bubbleBottomY = bubbleTopY + bubbleHeight;
	const auto clampTriangleTop = [&](int iconCenterY)
	{
		int desiredTop = iconCenterY - LayoutConstants::TRIANGLE_HEIGHT / 2;
		int minTop = bubbleTopY + LayoutConstants::ICON_MARGIN;
		int maxTop = bubbleBottomY - LayoutConstants::TRIANGLE_HEIGHT - LayoutConstants::ICON_MARGIN;
		if (maxTop < minTop)
		{
			maxTop = minTop;
		}
		return qBound(minTop, desiredTop, maxTop);
	};
	int leftTriangleTop = clampTriangleTop(m_layoutData.rects.iconLeft.center().y());
	int rightTriangleTop = clampTriangleTop(m_layoutData.rects.iconRight.center().y());
	m_layoutData.rects.triangleLeft = QRect(
		LayoutConstants::ICON_SIZE + LayoutConstants::ICON_SPACING + LayoutConstants::ICON_BORDER_WIDTH,
		leftTriangleTop,
		LayoutConstants::TRIANGLE_WIDTH,
		LayoutConstants::TRIANGLE_HEIGHT);
	m_layoutData.rects.triangleRight = QRect(
		width() - LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::ICON_SIZE - LayoutConstants::ICON_SPACING - LayoutConstants::TRIANGLE_WIDTH,
		rightTriangleTop,
		LayoutConstants::TRIANGLE_WIDTH,
		LayoutConstants::TRIANGLE_HEIGHT);
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
	const int reasoningBottomY = bubbleTopY + reasoningHeight;
	const auto clampTriangleTopReasoning = [&](int iconCenterY)
	{
		int desiredTop = iconCenterY - LayoutConstants::TRIANGLE_HEIGHT / 2;
		int minTop = bubbleTopY + LayoutConstants::ICON_MARGIN;
		int maxTop = reasoningBottomY - LayoutConstants::TRIANGLE_HEIGHT - LayoutConstants::ICON_MARGIN;
		if (maxTop < minTop)
		{
			maxTop = minTop;
		}
		return qBound(minTop, desiredTop, maxTop);
	};
	int leftTriangleTop = clampTriangleTopReasoning(m_layoutData.rects.iconLeft.center().y());
	int rightTriangleTop = clampTriangleTopReasoning(m_layoutData.rects.iconRight.center().y());
	m_layoutData.rects.triangleLeft = QRect(
		LayoutConstants::ICON_SIZE + LayoutConstants::ICON_SPACING + LayoutConstants::ICON_BORDER_WIDTH,
		leftTriangleTop,
		LayoutConstants::TRIANGLE_WIDTH,
		LayoutConstants::TRIANGLE_HEIGHT);
	m_layoutData.rects.triangleRight = QRect(
		width() - LayoutConstants::ICON_BORDER_WIDTH - LayoutConstants::ICON_SIZE - LayoutConstants::ICON_SPACING - LayoutConstants::TRIANGLE_WIDTH,
		rightTriangleTop,
		LayoutConstants::TRIANGLE_WIDTH,
		LayoutConstants::TRIANGLE_HEIGHT);
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
	m_messageData.rawMsg = AnswerHtml;
	m_messageData.rawReasoningMsg.clear();
	m_messageData.reasoningText.clear();
	m_messageData.isImportant = false;
	m_messageData.userNote.clear();
	applyNoteToolTip();
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
	m_messageData.isImportant = false;
	m_messageData.userNote.clear();
	applyNoteToolTip();
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
		m_messageData.rawReasoningMsg.clear();
		m_messageData.reasoningText.clear();
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
void LLMChatFrame::resetForReuse()
{
	// 清理代码块映射
	m_codeBlockMap.clear();
	// 恢复基础数据
	m_messageData = MessageData();
	m_layoutData = LayoutData();
	m_state = StateFlags();
	m_layoutCache = LayoutCache();
	m_docCache.invalidate(); // 清除文档缓存
	m_UserType = User_System;
	m_allowDeferredDelete = false;

	// 标记状态
	m_needsUpdate = false;
	m_layoutDirty = true;
	m_isHovered = false;

	// 停止动画并隐藏加载指示
	if (m_ui.loading.movie)
	{
		m_ui.loading.movie->stop();
	}
	if (m_ui.loading.label)
	{
		m_ui.loading.label->hide();
	}
	m_messageData.isImportant = false;
	m_messageData.userNote.clear();
	m_messageData.isCollapsed = false;
	m_messageData.isReasoningCollapsed = false;
	applyNoteToolTip();

	// 重置按钮状态
	if (m_ui.buttons.copyThinking)
	{
		m_ui.buttons.copyThinking->setEnabled(true);
	}
	if (m_ui.buttons.copyAnswer)
	{
		m_ui.buttons.copyAnswer->setEnabled(true);
	}
	if (m_ui.buttons.regenerate)
	{
		m_ui.buttons.regenerate->setEnabled(true);
	}
	m_ui.buttons.hideAll();

	// 清空辅助状态
	m_SuggestButton.clear();

	// 关掉阴影效果，等待重新布局时启用
	if (m_shadowEffect)
	{
		m_shadowEffect->setEnabled(false);
	}

	setMinimumSize(QSize(0, 0));
	update();
}

void LLMChatFrame::prepareForDeletion()
{
	m_allowDeferredDelete = true;
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
	// 如果折叠，使用摘要文本计算高度
	QString contentForLayout = m_messageData.isCollapsed ? getCollapsedSummary() : m_messageData.msg;
	QSize size = getRealString(contentForLayout);
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
	// 如果折叠，使用摘要文本计算高度
	QString answerForLayout = m_messageData.isCollapsed ? getCollapsedSummary() : m_messageData.msg;
	QString reasoningForLayout = m_messageData.isReasoningCollapsed ? getReasoningCollapsedSummary() : m_messageData.reasoningText;
	QSize answerSize = getRealString(answerForLayout);
	QSize reasoningSize = getRealString(reasoningForLayout);
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
		const QString trimmedDelta = delta.trimmed();
		if (!trimmedDelta.isEmpty() &&
			!m_messageData.rawMsg.contains(QStringLiteral("### 回答")) &&
			!trimmedDelta.startsWith(QStringLiteral("### 回答")))
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
	QColor reasoningBackground = ColorScheme::REASONING_BACKGROUND;
	QColor reasoningBorder = ColorScheme::REASONING_BORDER;
	QColor answerBackground = ColorScheme::ANSWER_BACKGROUND;
	QColor answerBorder = ColorScheme::ANSWER_BORDER;
	QColor reasoningShadow = ColorScheme::SHADOW_COLOR;
	QColor answerShadow = ColorScheme::SHADOW_COLOR;
	if (m_isHovered)
	{
		reasoningBackground = reasoningBackground.lighter(108);
		reasoningBorder = reasoningBorder.lighter(115);
		answerBackground = answerBackground.lighter(107);
		answerBorder = answerBorder.lighter(112);
		reasoningShadow.setAlpha(qMin(255, reasoningShadow.alpha() + 40));
		answerShadow.setAlpha(qMin(255, answerShadow.alpha() + 50));
	}
	else
	{
		reasoningShadow.setAlpha(qRound(reasoningShadow.alpha() * 0.7));
	}
	drawBubble(painter, m_layoutData.rects.frameLeftReason, QRect(), true,
		reasoningBackground, reasoningBorder,
		reasoningShadow, LayoutConstants::BORDER_RADIUS, LayoutConstants::SHADOW_OFFSET / 2);
	drawBubble(painter, m_layoutData.rects.frameLeft, m_layoutData.rects.triangleLeft, true,
		answerBackground, answerBorder,
		answerShadow, LayoutConstants::BORDER_RADIUS, LayoutConstants::SHADOW_OFFSET);
	const QString primaryTextColor = QStringLiteral("#0F172A");
	const bool hasReasoningBubble = m_layoutData.rects.frameLeftReason.isValid();
	
	// 获取或创建缓存的文档（优化性能）
	// 注意：需要传入折叠状态以确保缓存正确性
	auto getOrCreateDoc = [this, &primaryTextColor](std::unique_ptr<QTextDocument>& doc, 
		const QString& html, int width, QString& cachedHtml, int& cachedWidth, 
		bool isCollapsed, bool& cachedCollapsed) -> QTextDocument* {
		// 检查缓存是否有效（包括折叠状态）
		if (doc && cachedHtml == html && cachedWidth == width && cachedCollapsed == isCollapsed)
		{
			return doc.get();
		}
		
		// 创建新文档或重用现有文档
		if (!doc)
		{
			doc = std::make_unique<QTextDocument>();
			// 优化文档设置以提升渲染性能
			doc->setUndoRedoEnabled(false);
		}
		
		setTextDocs(*doc, html, width, primaryTextColor);
		cachedHtml = html;
		cachedWidth = width;
		cachedCollapsed = isCollapsed;
		return doc.get();
	};
	
	if (m_state.isStreamEnd)
	{
		// 流式输出结束后，需要考虑折叠状态
		QString answerHtml = m_messageData.isCollapsed ? getCollapsedSummary() : markdownToHtml(m_messageData.rawMsg);
		QTextDocument* docAnswer = getOrCreateDoc(m_docCache.docAnswer, answerHtml, 
			m_layoutData.rects.textLeft.width(), 
			m_docCache.cachedAnswerHtml, m_docCache.cachedAnswerWidth,
			m_messageData.isCollapsed, m_docCache.answerCollapsed);
		
		painter.save();
		painter.translate(m_layoutData.rects.textLeft.topLeft());
		docAnswer->documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
		painter.restore();
		
		if (hasReasoningBubble)
		{
			QString reasoningHtml = m_messageData.isReasoningCollapsed ? 
				getReasoningCollapsedSummary() : markdownToHtml(m_messageData.rawReasoningMsg);
			QTextDocument* docReasoning = getOrCreateDoc(m_docCache.docReasoning, reasoningHtml, 
				m_layoutData.rects.textLeftReason.width(), 
				m_docCache.cachedReasoningHtml, m_docCache.cachedReasoningWidth,
				m_messageData.isReasoningCollapsed, m_docCache.reasoningCollapsed);
			
			painter.save();
			painter.translate(m_layoutData.rects.textLeftReason.topLeft());
			docReasoning->documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
			painter.restore();
		}
	}
	else
	{
		if (hasReasoningBubble)
		{
			QString reasoningHtml = m_messageData.isReasoningCollapsed ? 
				getReasoningCollapsedSummary() : m_messageData.reasoningText;
			QTextDocument* docReasoning = getOrCreateDoc(m_docCache.docReasoning, reasoningHtml, 
				m_layoutData.rects.textLeftReason.width(), 
				m_docCache.cachedReasoningHtml, m_docCache.cachedReasoningWidth,
				m_messageData.isReasoningCollapsed, m_docCache.reasoningCollapsed);
			
			painter.save();
			painter.translate(m_layoutData.rects.textLeftReason.topLeft());
			docReasoning->documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
			painter.restore();
		}
		
		QString answerHtml = m_messageData.isCollapsed ? getCollapsedSummary() : m_messageData.msg;
		QTextDocument* docAnswer = getOrCreateDoc(m_docCache.docAnswer, answerHtml, 
			m_layoutData.rects.textLeft.width(), 
			m_docCache.cachedAnswerHtml, m_docCache.cachedAnswerWidth,
			m_messageData.isCollapsed, m_docCache.answerCollapsed);
		
		painter.save();
		painter.translate(m_layoutData.rects.textLeft.topLeft());
		docAnswer->documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
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
	if (m_isHovered)
	{
		ringTop = ringTop.lighter(118);
		ringBottom = ringBottom.lighter(122);
	}
	ringGradient.setColorAt(0.0, ringTop);
	ringGradient.setColorAt(1.0, ringBottom);
	painter.setPen(Qt::NoPen);
	painter.setBrush(ringGradient);
	painter.drawEllipse(ringRect);
	painter.drawPixmap(iconRect, *m_ui.icons.rightPix);
	painter.restore();
	QColor ownerBackground = ColorScheme::USER_BACKGROUND;
	QColor ownerBorder = ColorScheme::USER_BACKGROUND.darker(115);
	QColor ownerShadow = ColorScheme::SHADOW_COLOR;
	if (m_isHovered)
	{
		ownerBackground = ownerBackground.lighter(110);
		ownerBorder = ownerBorder.lighter(118);
		ownerShadow.setAlpha(qMin(255, ownerShadow.alpha() + 50));
	}
	drawBubble(painter, m_layoutData.rects.frameRight, m_layoutData.rects.triangleRight, false,
		ownerBackground, ownerBorder,
		ownerShadow, LayoutConstants::BORDER_RADIUS, LayoutConstants::SHADOW_OFFSET);
	
	// 使用缓存的文档（优化性能）
	const QString ownerTextColor = QStringLiteral("#F8FAFC");
	QString answerHtml = m_messageData.isCollapsed ? getCollapsedSummary() : m_messageData.msg;
	int textWidth = m_layoutData.rects.textRight.width();
	
	// 检查缓存是否有效
	if (!m_docCache.docOwner || m_docCache.cachedOwnerHtml != answerHtml || 
		m_docCache.cachedOwnerWidth != textWidth || m_docCache.answerCollapsed != m_messageData.isCollapsed)
	{
		if (!m_docCache.docOwner)
		{
			m_docCache.docOwner = std::make_unique<QTextDocument>();
			m_docCache.docOwner->setUndoRedoEnabled(false);
		}
		setTextDocs(*m_docCache.docOwner, answerHtml, textWidth, ownerTextColor);
		m_docCache.cachedOwnerHtml = answerHtml;
		m_docCache.cachedOwnerWidth = textWidth;
		m_docCache.answerCollapsed = m_messageData.isCollapsed;
	}
	
	painter.save();
	painter.translate(m_layoutData.rects.textRight.topLeft());
	m_docCache.docOwner->documentLayout()->draw(&painter, QAbstractTextDocumentLayout::PaintContext());
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

	// 折叠按钮由 QPushButton 绘制，不需要在这里绘制

	if (m_UserType == User_Customer || m_UserType == User_Owner)
	{
		const QRect& bubbleRect = (m_UserType == User_Customer)
			? m_layoutData.rects.frameLeft
			: m_layoutData.rects.frameRight;
		if (m_messageData.isImportant)
		{
			drawImportanceBadge(painter, bubbleRect);
		}
		if (!m_messageData.userNote.trimmed().isEmpty())
		{
			drawNoteBadge(painter, bubbleRect);
		}
	}

	// 清除更新标志
	m_needsUpdate = false;
}

bool LLMChatFrame::eventFilter(QObject* watched, QEvent* event)
{
	const auto buttons = { m_ui.buttons.copyThinking.get(),
		m_ui.buttons.copyAnswer.get(),
		m_ui.buttons.regenerate.get() };
	for (auto* btn : buttons)
	{
		if (watched == btn && btn)
		{
			switch (event->type())
			{
			case QEvent::Enter:
				updateButtonHoverState(btn, true);
				return false;
			case QEvent::Leave:
				updateButtonHoverState(btn, false);
				return false;
			case QEvent::MouseButtonPress:
				if (auto* effect = qobject_cast<QGraphicsDropShadowEffect*>(btn->graphicsEffect()))
				{
					effect->setBlurRadius(14);
					effect->setOffset(0, 3);
				}
				return false;
			case QEvent::MouseButtonRelease:
				updateButtonHoverState(btn, btn->underMouse());
				return false;
			default:
				break;
			}
		}
	}
	return QWidget::eventFilter(watched, event);
}

void LLMChatFrame::enterEvent(QEvent* event)
{
	m_isHovered = true;
	if (m_shadowEffect)
	{
		m_shadowEffect->setEnabled(true);
	}
	update();
	QWidget::enterEvent(event);
}

void LLMChatFrame::leaveEvent(QEvent* event)
{
	m_isHovered = false;
	if (m_shadowEffect)
	{
		m_shadowEffect->setEnabled(false);
	}
	update();
	QWidget::leaveEvent(event);
}

void LLMChatFrame::updateButtonHoverState(QPushButton* button, bool hovered)
{
	if (!button)
	{
		return;
	}
	if (auto* effect = qobject_cast<QGraphicsDropShadowEffect*>(button->graphicsEffect()))
	{
		effect->setBlurRadius(hovered ? 24 : 18);
		effect->setOffset(0, hovered ? 9 : 6);
		effect->setColor(hovered ? QColor(37, 99, 235, 160) : QColor(37, 99, 235, 120));
	}
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
		m_docCache.invalidate(); // 使文档缓存失效（宽度改变需要重新渲染）
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
	// 检测代码块复制按钮点击
	QPoint localPos = event->pos();
	QString anchor;
	
	// 检查点击位置是否在文本区域内
	if (m_UserType == User_Owner && m_layoutData.rects.textRight.contains(localPos))
	{
		QTextDocument doc;
		setTextDocs(doc, m_messageData.msg, m_layoutData.rects.textRight.width(), QStringLiteral("#F8FAFC"));
		QPoint textPos = localPos - m_layoutData.rects.textRight.topLeft();
		anchor = doc.documentLayout()->anchorAt(textPos);
	}
	else if (m_UserType == User_Customer)
	{
		if (m_layoutData.rects.textLeft.contains(localPos))
		{
			QTextDocument doc;
			setTextDocs(doc, m_messageData.msg, m_layoutData.rects.textLeft.width(), QStringLiteral("#0F172A"));
			QPoint textPos = localPos - m_layoutData.rects.textLeft.topLeft();
			anchor = doc.documentLayout()->anchorAt(textPos);
		}
		else if (m_layoutData.rects.textLeftReason.isValid() && m_layoutData.rects.textLeftReason.contains(localPos))
		{
			QTextDocument doc;
			setTextDocs(doc, m_messageData.reasoningText, m_layoutData.rects.textLeftReason.width(), QStringLiteral("#0F172A"));
			QPoint textPos = localPos - m_layoutData.rects.textLeftReason.topLeft();
			anchor = doc.documentLayout()->anchorAt(textPos);
		}
	}
	
	// 如果点击的是代码块复制链接
	if (anchor.startsWith("codeblock://copy?id="))
	{
		QString codeBlockId = anchor.mid(20); // 跳过 "codeblock://copy?id="
		handleCodeBlockCopy(codeBlockId);
		event->accept();
		return;
	}
	
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
	const QPoint localPos = mapFromGlobal(event->globalPos());
	const bool hasReasoning = !m_messageData.reasoningText.isEmpty();
	const bool reasoningSection = hasReasoning
		&& m_layoutData.rects.frameLeftReason.contains(localPos)
		&& m_UserType == User_Customer;

	QMenu menu(this);
	QAction* editNoteAction = menu.addAction(tr("Edit Note..."));
	QAction* toggleImportantAction = menu.addAction(m_messageData.isImportant ? tr("Unmark Important") : tr("Mark As Important"));
	menu.addSeparator();
	QMenu* copyMenu = menu.addMenu(tr("Copy"));
	QAction* copyPlainAction = copyMenu->addAction(tr("Copy Text"));
	QAction* copyMarkdownAction = copyMenu->addAction(tr("Copy Markdown"));
	QAction* copyHtmlAction = copyMenu->addAction(tr("Copy HTML"));
	QMenu* exportMenu = menu.addMenu(tr("Export To File"));
	QAction* exportImageAction = exportMenu->addAction(tr("Export As Image..."));
	QAction* exportMarkdownAction = exportMenu->addAction(tr("Export As Markdown..."));
	QAction* exportHtmlAction = exportMenu->addAction(tr("Export As HTML..."));
	QAction* exportTextAction = exportMenu->addAction(tr("Export As Text..."));

	QAction* selectedAction = menu.exec(event->globalPos());
	if (!selectedAction)
	{
		return;
	}

	if (selectedAction == editNoteAction)
	{
		handleNoteRequested();
	}
	else if (selectedAction == toggleImportantAction)
	{
		toggleImportant();
	}
	else if (selectedAction == copyPlainAction)
	{
		copyToClipboardPlain(reasoningSection);
	}
	else if (selectedAction == copyMarkdownAction)
	{
		copyToClipboardMarkdown(reasoningSection);
	}
	else if (selectedAction == copyHtmlAction)
	{
		copyToClipboardHtml(reasoningSection);
	}
	else if (selectedAction == exportImageAction)
	{
		exportBubbleAsImage();
	}
	else if (selectedAction == exportMarkdownAction)
	{
		exportTextToFile(buildMarkdownExport(),
			tr("Export As Markdown"),
			tr("Markdown Files (*.md)"),
			QStringLiteral("md"));
	}
	else if (selectedAction == exportHtmlAction)
	{
		exportTextToFile(buildHtmlExport(),
			tr("Export As HTML"),
			tr("HTML Files (*.html *.htm)"),
			QStringLiteral("html"));
	}
	else if (selectedAction == exportTextAction)
	{
		exportTextToFile(buildPlainExport(),
			tr("Export As Text"),
			tr("Text Files (*.txt)"),
			QStringLiteral("txt"));
	}
}

void LLMChatFrame::handleCodeBlockCopy(const QString& codeBlockId)
{
	if (m_codeBlockMap.contains(codeBlockId))
	{
		QString code = m_codeBlockMap[codeBlockId];
		QClipboard* clipboard = QApplication::clipboard();
		if (clipboard)
		{
			clipboard->setText(code);
			// 可选：显示复制成功的提示
			// 可以使用 QToolTip 或状态栏提示
		}
	}
}

void LLMChatFrame::drawCollapseButton(QPainter& painter, const QRect& bubbleRect, bool isReasoning)
{
	// 按钮由 QPushButton 绘制，这里不需要绘制
	// 保留函数以保持接口一致性
	Q_UNUSED(painter);
	Q_UNUSED(bubbleRect);
	Q_UNUSED(isReasoning);
}
