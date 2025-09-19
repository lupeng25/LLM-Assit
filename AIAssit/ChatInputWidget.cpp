#include "ChatInputWidget.h"
#include <QApplication>
#include <QDebug>

ChatInputWidget::ChatInputWidget(QWidget *parent)
	: QWidget(parent)
	, textEdit(nullptr)
	, attachButton(nullptr)
	, optionsComboBox(nullptr)
	, sendButton(nullptr)
	, mainGridLayout(nullptr)
	, bottomGridLayout(nullptr)
	, buttonGridLayout(nullptr)
	, bottomWidget(nullptr)
	, horizontalSpacer(nullptr)
	, imageScrollArea(nullptr)
	, imageContainer(nullptr)
	, imageLayout(nullptr)
{
	setupUI();
	setupStyles();
	setConnect();
	setAcceptDrops(true);
}

ChatInputWidget::~ChatInputWidget()
{
}

void ChatInputWidget::setupUI()
{
	setWindowTitle("Modern Chat Input");
	// 添加整体阴影效果
	QGraphicsDropShadowEffect *windowShadow = new QGraphicsDropShadowEffect;
	windowShadow->setBlurRadius(20);
	windowShadow->setColor(QColor(0, 0, 0, 30));
	windowShadow->setOffset(0, 4);
	setGraphicsEffect(windowShadow);

	// 创建主文本编辑器
	textEdit = new QTextEdit(this);
	textEdit->setPlaceholderText(QStringLiteral("输入您的消息...Ctrl+Enter"));
	textEdit->setMaximumHeight(120);
	textEdit->setMinimumHeight(80);
	textEdit->installEventFilter(this);

	// 添加文本框阴影
	QGraphicsDropShadowEffect *textShadow = new QGraphicsDropShadowEffect;
	textShadow->setBlurRadius(8);
	textShadow->setColor(QColor(0, 0, 0, 15));
	textShadow->setOffset(0, 2);
	textEdit->setGraphicsEffect(textShadow);

	// 创建底部控件容器
	bottomWidget = new QWidget(this);
	bottomWidget->setFixedHeight(70);

	// 创建附件按钮
	attachButton = new QPushButton(QStringLiteral("+"), bottomWidget);
	attachButton->setObjectName("attachButton");
	attachButton->setToolTip(QStringLiteral("添加附件 (支持拖拽)"));
	attachButton->setFixedSize(20, 20);
	attachButton->setCursor(Qt::PointingHandCursor);

	// 创建选项下拉框
	optionsComboBox = new QComboBox(bottomWidget);
	optionsComboBox->setToolTip(QStringLiteral("选择AI模型"));
	optionsComboBox->setFixedHeight(40);
	optionsComboBox->setMinimumWidth(150);


	// 创建发送按钮
	sendButton = new QPushButton(QStringLiteral("发送"), bottomWidget);
	sendButton->setObjectName("sendButton");
	sendButton->setFixedSize(80, 40);
	sendButton->setCursor(Qt::PointingHandCursor);
	sendButton->setEnabled(true);

	// 创建图片容器
	imageContainer = new QWidget(this);
	imageLayout = new QHBoxLayout(imageContainer);
	imageLayout->setSpacing(10);
	imageLayout->setContentsMargins(10, 10, 10, 10);
	imageLayout->setAlignment(Qt::AlignLeft);

	// 创建滚动区域
	imageScrollArea = new QScrollArea(bottomWidget);
	imageScrollArea->setWidget(imageContainer);
	imageScrollArea->setWidgetResizable(true);
	imageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	imageScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	imageScrollArea->setMaximumHeight(50);
	imageScrollArea->setMinimumHeight(0);
	imageScrollArea->hide(); // 初始隐藏

	// 设置样式
	imageScrollArea->setStyleSheet(
		"QScrollArea {"
		"background-color: #f8f9fa;"
		"border: 1px solid #e1e8ed;"
		"border-radius: 8px;"
		"}"
		"QScrollArea QWidget {"
		"background-color: transparent;"
		"}"
	);
	

	// 创建水平间距器
	horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	// 设置主布局
	mainGridLayout = new QGridLayout(this);
	mainGridLayout->setSpacing(15);
	mainGridLayout->setContentsMargins(20, 20, 20, 20);
	mainGridLayout->addWidget(textEdit, 0, 0);
	mainGridLayout->addWidget(bottomWidget, 1, 0);
	mainGridLayout->setAlignment(Qt::AlignCenter);

	// 设置底部控件的布局
	bottomGridLayout = new QHBoxLayout(bottomWidget);
	bottomGridLayout->setSpacing(12);
	bottomGridLayout->setContentsMargins(0, 0, 0, 0);

	bottomGridLayout->addWidget(attachButton);
	bottomGridLayout->addWidget(optionsComboBox);
	bottomGridLayout->addItem(horizontalSpacer);
	bottomGridLayout->addWidget(imageScrollArea);
	bottomGridLayout->addWidget(sendButton);

	// 设置初始焦点
	textEdit->setFocus();
}

void ChatInputWidget::setConnect()
{
	connect(attachButton, &QPushButton::clicked, this, &ChatInputWidget::onAttachButtonClicked);
	connect(sendButton, &QPushButton::clicked, this, &ChatInputWidget::onSendButtonClicked);
	connect(optionsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ChatInputWidget::onOptionChanged);
}

void ChatInputWidget::onSendButtonClicked()
{
	QString UserMessage = textEdit->toPlainText();
	ChatSendMessage msgBody;
	msgBody.SendText = UserMessage;
	if (UserMessage.isEmpty()) 
		return;
	msgBody.Image64 = ImageBase64;
	msgBody.fileContext = attachedTextContents;
	sendButton->setEnabled(false);
	textEdit->clear();
	clearAllFiles();
	MessageUp(msgBody);
}

void ChatInputWidget::SetButtonEnable(bool bEnable)
{
	sendButton->setEnabled(bEnable);
}

void ChatInputWidget::setModelCurrIndex(int iModel)
{
	optionsComboBox->setCurrentIndex(iModel);
}

ChatInputWidget::FileType ChatInputWidget::getFileType(const QString &filePath)
{
	QFileInfo fileInfo(filePath);
	QString suffix = fileInfo.suffix().toLower();

	QStringList imageSuffixes = { "png", "jpg", "jpeg", "bmp", "gif", "webp" };
	QStringList textSuffixes = { "txt", "md", "log", "json", "xml", "csv", "py", "cpp", "h", "js", "html", "css" ,"bat","iss"};

	if (imageSuffixes.contains(suffix)) {
		return IMAGE_FILE;
	}
	else if (textSuffixes.contains(suffix)) {
		return TEXT_FILE;
	}

	return TEXT_FILE;
}

QString ChatInputWidget::readTextFile(const QString &filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
		return QString();
	QTextStream in(&file);
	in.setCodec("UTF-8");
	QString content = in.readAll();
	file.close();

	return content;
}

bool ChatInputWidget::validateImageFile(const QString &filePath, QString &errorMsg)
{
	QFileInfo fileInfo(filePath);
	// 检查文件是否存在
	if (!fileInfo.exists()) 
	{
		errorMsg = QStringLiteral("文件不存在");
		return false;
	}
	// 检查文件大小
	if (fileInfo.size() > MAX_FILE_SIZE) 
	{
		errorMsg = QStringLiteral("文件大小超过10MB限制");
		return false;
	}
	// 检查是否已添加
	for (const auto &file : attachedFiles) 
	{
		if (file.filePath == filePath) 
		{
			errorMsg = QStringLiteral("该文件已经添加");
			return false;
		}
	}
	if (attachedFiles.size() >= MAX_IMAGES)
	{
		errorMsg = QStringLiteral("最多只能添加%1个文件").arg(MAX_IMAGES);
		return false;
	}
	return true;
}

void ChatInputWidget::addFile(const QString &filePath, FileType type)
{
	QString errorMsg;
	if (!validateImageFile(filePath, errorMsg))
	{
		QMessageBox::warning(this, QStringLiteral("错误"), errorMsg);
		return;
	}

	AttachedFile file;
	file.filePath = filePath;
	file.fileType = type;
	file.displayName = QFileInfo(filePath).baseName();

	if (type == FileType::TEXT_FILE)
	{
		QString textContent = readTextFile(filePath);
		attachedTextContents.append(textContent);
	}
	else if (type == FileType::IMAGE_FILE)
	{
		QFile file(filePath);
		QByteArray fileData = file.readAll();
		ImageBase64.append(fileData.toBase64());
	}
	
	attachedFiles.append(file);
	updateFileDisplay();
}

void ChatInputWidget::removeFile(const QString &filePath)
{
	for (int i = 0; i < attachedFiles.size(); ++i) 
	{
		if (attachedFiles[i].filePath == filePath) 
		{
			if (attachedFiles[i].fileType == TEXT_FILE)
				attachedTextContents.removeAt(i);
			else if (attachedFiles[i].fileType == IMAGE_FILE)
				ImageBase64.removeAt(i);
			attachedFiles.removeAt(i);
			break;
		}
	}
	updateFileDisplay();
}

void ChatInputWidget::clearAllFiles()
{
	attachedFiles.clear();
	ImageBase64.clear();
	attachedTextContents.clear();
	updateFileDisplay();
}

void ChatInputWidget::updateFileDisplay()
{
	// 清除现有的控件
	while (QLayoutItem *item = imageLayout->takeAt(0)) 
	{
		if (QWidget *widget = item->widget())
			widget->deleteLater();
		delete item;
	}

	if (attachedFiles.isEmpty())
	{
		imageScrollArea->hide();
		imageScrollArea->setMinimumHeight(0);
		this->update();
		return;
	}

	// 显示文件区域
	imageScrollArea->show();
	imageScrollArea->setMinimumWidth(380);
	imageScrollArea->setMinimumHeight(60);

	// 为每个文件创建预览控件
	for (const AttachedFile &file : attachedFiles)
	{
		// 创建文件预览容器
		QWidget *fileWidget = new QWidget();
		fileWidget->setFixedSize(80, 50);
		fileWidget->setStyleSheet(
			"QWidget {"
			"background-color: #f8f9fa;"
			"border: 1px solid #e1e8ed;"
			"border-radius: 6px;"
			"}"
			"QWidget:hover {"
			"background-color: #e9ecef;"
			"border-color: #3498db;"
			"}"
		);

		QVBoxLayout *layout = new QVBoxLayout(fileWidget);
		layout->setContentsMargins(3, 3, 3, 3);
		layout->setSpacing(2);

		// 创建水平布局用于图标和文件名
		QHBoxLayout *headerLayout = new QHBoxLayout();
		headerLayout->setSpacing(4);

		// 文件预览/图标
		QLabel *fileLabel = new QLabel();
		fileLabel->setFixedSize(32, 32);
		fileLabel->setAlignment(Qt::AlignCenter);
		fileLabel->setStyleSheet(
			"QLabel {"
			"border: none;"
			"background-color: transparent;"
			"padding: 2px;"
			"}"
		);

		if (file.fileType == IMAGE_FILE) 
		{
			QPixmap pixmap(file.filePath);
			if (!pixmap.isNull()) 
			{
				pixmap = pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				fileLabel->setPixmap(pixmap);
			}
		}
		else 
		{
			QPixmap pixmap(":/QtWidgetsApp/ICONs/icon_text_file.png");
			fileLabel->setPixmap(pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}

		// 文件名标签
		QLabel *nameLabel = new QLabel(QFileInfo(file.filePath).fileName());
		nameLabel->setStyleSheet(
			"QLabel {"
			"color: #2c3e50;"
			"font-size: 11px;"
			"background: transparent;"
			"border: none;"
			"padding: 0;"
			"qproperty-wordWrap: true;"
			"}"
		);
		nameLabel->setMaximumWidth(70);
		nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

		// 删除按钮
		QPushButton *deleteBtn = new QPushButton("X");
		deleteBtn->setFixedSize(16, 16);
		deleteBtn->setStyleSheet(
			"QPushButton {"
			"background-color: rgba(231, 76, 60, 0.8);"
			"border: none;"
			"border-radius: 8px;"
			"padding: 0;"
			"margin: 0;"
			"position: absolute;"
			"top: 2px;"
			"right: 2px;"
			"}"
			"QPushButton:hover {"
			"background-color: #e74c3c;"
			"}"
			"QPushButton:pressed {"
			"background-color: #c0392b;"
			"}"
		);

		// 设置工具提示
		QString tooltip;
		if (file.fileType == IMAGE_FILE)
		{
			QImageReader reader(file.filePath);
			QSize size = reader.size();
			tooltip = QStringLiteral("图片: %1\n尺寸: %2x%3")
				.arg(QFileInfo(file.filePath).fileName())
				.arg(size.width())
				.arg(size.height());
		} else 
		{
			tooltip = QStringLiteral("文件: %1\n类型: 文本文件").arg(QFileInfo(file.filePath).fileName());
			int index = attachedFiles.indexOf(file);
			if (index >= 0 && index < attachedTextContents.size())
			{
				QString content = attachedTextContents[index];
				if (content.length() > 100) 
				{
					content = content.left(100) + "...";
				}
				tooltip += QStringLiteral("\n内容预览:\n%1").arg(content);
			}
		}
		fileWidget->setToolTip(tooltip);

		// 连接删除按钮
		connect(deleteBtn, &QPushButton::clicked, [this, file]() {
			removeFile(file.filePath);
		});

		// 组装布局
		headerLayout->addWidget(fileLabel);
		headerLayout->addWidget(nameLabel, 1);
		headerLayout->addWidget(deleteBtn);
		
		layout->addLayout(headerLayout);

		imageLayout->addWidget(fileWidget);
	}

	// 添加弹性空间
	imageLayout->addStretch();
}

void ChatInputWidget::onAttachButtonClicked()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		QStringLiteral("选择文件"),
		"",
		QStringLiteral("支持的文件 (*.png *.jpg *.jpeg *.bmp *.gif *.webp *.txt *.md *.log *.json *.xml *.csv *.cpp *.h *.py *.js *.html *.css *.bat *.iss);;")
		+ QStringLiteral("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.webp);;")
		+ QStringLiteral("文本文件 (*.txt *.md *.log *.json *.xml *.csv *.cpp *.h *.py *.js *.html *.css *.bat *.iss)")
	);
	if (!fileName.isEmpty())
	{
		FileType type = getFileType(fileName);
		addFile(fileName, type);
	}
}

void ChatInputWidget::onOptionChanged(int index)
{
	// 使用索引来判断选中了哪个选项
	QString selectedOption = optionsComboBox->itemText(index);
	ModelSelect(index);
}

bool ChatInputWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == textEdit && event->type() == QEvent::KeyPress) 
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

		// 检查 Ctrl+Enter 组合键
		if ((keyEvent->modifiers() & Qt::ControlModifier) &&
			(keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)) 
		{

			// 如果发送按钮启用且文本不为空，则发送消息
			if (sendButton->isEnabled() && !textEdit->toPlainText().trimmed().isEmpty())
			{
				onSendButtonClicked();
			}
			return true;
		}

		// 普通的Enter键只换行，不发送
		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
			// 让默认行为处理（换行）
			return false;
		}
	}

	return QWidget::eventFilter(obj, event);
}

void ChatInputWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasUrls()) 
	{
		bool hasValidFiles = false;
		for (const QUrl &url : event->mimeData()->urls())
		{
			if (url.isLocalFile()) 
			{
				QString filePath = url.toLocalFile();
				if (validateImageFile(filePath, QString{}))
				{
					hasValidFiles = true;
					break;
				}
			}
		}
		if (hasValidFiles) 
		{
			event->acceptProposedAction();
			return;
		}
	}
	event->ignore();
}

void ChatInputWidget::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();
	if (mimeData->hasUrls()) 
	{
		for (const QUrl &url : mimeData->urls()) 
		{
			if (url.isLocalFile())
			{
				QString filePath = url.toLocalFile();
				FileType type = getFileType(filePath);

				if (validateImageFile(filePath, QString{}))
				{
					addFile(filePath, type);
				}
				
			}
		}
		event->acceptProposedAction();
	}
}

void ChatInputWidget::UpdateModelList(bool success, QStringList modelList, const QString& errorMessage)
{
	this->optionsComboBox->clear();
	for (auto it = modelList.constBegin(); it != modelList.constEnd(); ++it)
	{
		this->optionsComboBox->addItem(*it);
	}
}

void ChatInputWidget::setupStyles()
{
	this->setStyleSheet(
		// 主容器样式
		"ChatInputWidget {"
		"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
		"stop:0 #ffffff, stop:1 #f8f9fb);"
		"border-radius: 16px;"
		"border: 1px solid #e1e8ed;"
		"}"

		// 文本编辑器样式
		"QTextEdit {"
		"background-color: #ffffff;"
		"border: 2px solid #e1e8ed;"
		"border-radius: 12px;"
		"padding: 16px;"
		"font-size: 15px;"
		"line-height: 1.5;"
		"color: #2c3e50;"
		"font-family: 'Microsoft YaHei', 'Segoe UI', sans-serif;"
		"selection-background-color: #3498db;"
		"selection-color: white;"
		"}"

		"QTextEdit:focus {"
		"border-color: #3498db;"
		"background-color: #fefefe;"
		"}"

		// 通用按钮样式
		"QPushButton {"
		"border: none;"
		"border-radius: 20px;"
		"font-weight: 600;"
		"font-size: 13px;"
		"font-family: 'Microsoft YaHei', 'Segoe UI', sans-serif;"
		"transition: all 0.3s ease;"
		"}"

		// 发送按钮样式
		"QPushButton#sendButton {"
		"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
		"stop:0 #3498db, stop:1 #2980b9);"
		"color: white;"
		"font-weight: bold;"
		"}"

		"QPushButton#sendButton:hover:enabled {"
		"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
		"stop:0 #5dade2, stop:1 #3498db);"
		"transform: translateY(-1px);"
		"}"

		"QPushButton#sendButton:pressed {"
		"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
		"stop:0 #2980b9, stop:1 #1f618d);"
		"}"

		"QPushButton#sendButton:disabled {"
		"background: #bdc3c7;"
		"color: #7f8c8d;"
		"}"

		// 附件按钮样式
		"QPushButton#attachButton {"
		"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
		"stop:0 #95a5a6, stop:1 #7f8c8d);"
		"color: white;"
		"font-size: 16px;"
		"}"

		"QPushButton#attachButton:hover {"
		"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
		"stop:0 #a9b9ba, stop:1 #95a5a6);"
		"transform: scale(1.05);"
		"}"

		// 下拉框样式
		"QComboBox {"
		"background-color: #ffffff;"
		"border: 2px solid #e1e8ed;"
		"border-radius: 20px;"
		"padding: 8px 15px;"
		"font-size: 13px;"
		"color: #2c3e50;"
		"font-weight: 500;"
		"}"

		"QComboBox:hover {"
		"border-color: #3498db;"
		"background-color: #f8f9fa;"
		"}"

		"QComboBox:focus {"
		"border-color: #3498db;"
		"}"

		"QComboBox::drop-down {"
		"border: none;"
		"width: 25px;"
		"}"

		"QComboBox::down-arrow {"
		"image: none;"
		"border-left: 6px solid transparent;"
		"border-right: 6px solid transparent;"
		"border-top: 6px solid #7f8c8d;"
		"margin: 0 8px;"
		"}"

		"QComboBox QAbstractItemView {"
		"background-color: white;"
		"border: 2px solid #e1e8ed;"
		"border-radius: 8px;"
		"selection-background-color: #3498db;"
		"selection-color: white;"
		"outline: none;"
		"}"
	);
}