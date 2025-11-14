#include "ChatInputWidget.h"
#include <QApplication>
#include <QDebug>
#include <QIcon>

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
	, attachMenu(nullptr)
	, addFileAction(nullptr)
	, selectKnowledgeBaseAction(nullptr)
	, promptLibraryAction(nullptr)
	, knowledgeBaseMenu(nullptr)
	, m_promptLibrary(nullptr)
	, m_promptDialog(nullptr)
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
	textEdit->setPlaceholderText(tr("Input your message...Ctrl+Enter"));
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
	attachButton->setToolTip(tr("Add Data"));
	attachButton->setFixedSize(15, 15);
	attachButton->setCursor(Qt::PointingHandCursor);

	// 创建知识库子菜单
	attachMenu = new QMenu(this);
	addFileAction = new QAction(tr("Add File"), this);
	selectKnowledgeBaseAction = new QAction(tr("Select KnowledgeBase"), this);
	addFileAction->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_text_file.png"));
	selectKnowledgeBaseAction->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_konwledge.png"));
	knowledgeBaseMenu = new QMenu(this);
	knowledgeBaseMenu->setTitle(tr("Select KnowledgeBase"));
	selectKnowledgeBaseAction->setMenu(knowledgeBaseMenu);
	attachMenu->addAction(addFileAction);
	attachMenu->addAction(selectKnowledgeBaseAction);
	attachMenu->addSeparator();
	promptLibraryAction = new QAction(tr("Prompt Library"), this);
	attachMenu->addAction(promptLibraryAction);

	optionsComboBox = new QComboBox(bottomWidget);
	optionsComboBox->setToolTip(tr("Select AI Model"));
	optionsComboBox->setFixedHeight(40);
	optionsComboBox->setMinimumWidth(150);

	sendButton = new QPushButton(tr("Send"), bottomWidget);
	sendButton->setObjectName("sendButton");
	sendButton->setFixedSize(80, 40);
	sendButton->setCursor(Qt::PointingHandCursor);
	sendButton->setEnabled(true);

	imageContainer = new QWidget(this);
	imageLayout = new QHBoxLayout(imageContainer);
	imageLayout->setSpacing(10);
	imageLayout->setContentsMargins(10, 10, 10, 10);
	imageLayout->setAlignment(Qt::AlignLeft);

	imageScrollArea = new QScrollArea(bottomWidget);
	imageScrollArea->setWidget(imageContainer);
	imageScrollArea->setWidgetResizable(true);
	imageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	imageScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	imageScrollArea->setMaximumHeight(50);
	imageScrollArea->setMinimumHeight(0);
	imageScrollArea->hide();

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

	horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	mainGridLayout = new QGridLayout(this);
	mainGridLayout->setSpacing(15);
	mainGridLayout->setContentsMargins(20, 20, 20, 20);
	mainGridLayout->addWidget(textEdit, 0, 0);
	mainGridLayout->addWidget(bottomWidget, 1, 0);
	mainGridLayout->setAlignment(Qt::AlignCenter);

 
	bottomGridLayout = new QHBoxLayout(bottomWidget);
	bottomGridLayout->setSpacing(12);
	bottomGridLayout->setContentsMargins(0, 0, 0, 0);

	bottomGridLayout->addWidget(attachButton);
	bottomGridLayout->addWidget(optionsComboBox);
	bottomGridLayout->addItem(horizontalSpacer);
	bottomGridLayout->addWidget(imageScrollArea);
	bottomGridLayout->addWidget(sendButton);
	textEdit->setFocus();
}

void ChatInputWidget::setConnect()
{
	connect(attachButton, &QPushButton::clicked, this, &ChatInputWidget::onAttachButtonClicked);
	connect(sendButton, &QPushButton::clicked, this, &ChatInputWidget::onSendButtonClicked);
	connect(optionsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ChatInputWidget::onOptionChanged);
	connect(addFileAction, &QAction::triggered, this, &ChatInputWidget::onAddFileClicked);
	connect(promptLibraryAction, &QAction::triggered, this, &ChatInputWidget::onPromptLibraryClicked);
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
	QStringList textSuffixes = { "txt", "md", "log", "json", "xml", "csv", "py", "cpp", "h", "js", "html", "css" ,"bat","iss" };

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
		QMessageBox::warning(this, tr("Error"), errorMsg);
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
		if (!file.open(QIODevice::ReadOnly))
			return;
		QByteArray fileData = file.readAll();
		file.close();
		ImageBase64.append(fileData.toBase64());
	}

	attachedFiles.append(file);
	updateFileDisplay();
	InitFileFinished(filePath);
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
			RemoveFileSignal(filePath);
			break;
		}
	}
	updateFileDisplay();
}

void ChatInputWidget::clearAllFiles()
{
	//QStringList fileList;
	//for (auto file : attachedFiles)
	//{
	//	QString filePath = file.filePath;
	//	fileList.append(filePath);
	//}
	//RemoveAllFilesSignal(fileList);
	attachedFiles.clear();
	ImageBase64.clear();
	attachedTextContents.clear();
	updateFileDisplay();
}

void ChatInputWidget::updateFileDisplay()
{
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

	imageScrollArea->show();
	imageScrollArea->setMinimumWidth(380);
	imageScrollArea->setMinimumHeight(60);

	for (const AttachedFile &file : attachedFiles)
	{
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

		QHBoxLayout *headerLayout = new QHBoxLayout();
		headerLayout->setSpacing(4);

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

		QString tooltip;
		if (file.fileType == IMAGE_FILE)
		{
			QImageReader reader(file.filePath);
			QSize size = reader.size();
			tooltip = QStringLiteral("图片: %1\n尺寸: %2x%3")
				.arg(QFileInfo(file.filePath).fileName())
				.arg(size.width())
				.arg(size.height());
		}
		else
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

		connect(deleteBtn, &QPushButton::clicked, [this, file]() {
			removeFile(file.filePath);
		});

		headerLayout->addWidget(fileLabel);
		headerLayout->addWidget(nameLabel, 1);
		headerLayout->addWidget(deleteBtn);

		layout->addLayout(headerLayout);

		imageLayout->addWidget(fileWidget);
	}

	imageLayout->addStretch();
}

void ChatInputWidget::onAttachButtonClicked()
{
	QPoint pos = attachButton->mapToGlobal(QPoint(0, attachButton->height()));
	attachMenu->exec(pos);
	addButtonSignal();
}

void ChatInputWidget::onAddFileClicked()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr("Select File"),
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
	QString selectedOption = optionsComboBox->itemText(index);
	ModelSelect(index);
}

bool ChatInputWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == textEdit && event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

		if ((keyEvent->modifiers() & Qt::ControlModifier) &&
			(keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter))
		{

			if (sendButton->isEnabled() && !textEdit->toPlainText().trimmed().isEmpty())
			{
				onSendButtonClicked();
			}
			return true;
		}

		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
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
				QString tempErrorMsg;
				if (validateImageFile(filePath, tempErrorMsg))
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

                QString tempErrorMsg;
				if (validateImageFile(filePath, tempErrorMsg))
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
		"ChatInputWidget {"
		"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
		"stop:0 #ffffff, stop:1 #f8f9fb);"
		"border-radius: 16px;"
		"border: 1px solid #e1e8ed;"
		"}"

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

		"QPushButton {"
		"border: none;"
		"border-radius: 20px;"
		"font-weight: 600;"
		"font-size: 13px;"
		"font-family: 'Microsoft YaHei', 'Segoe UI', sans-serif;"
		"}"

		"QPushButton#sendButton {"
		"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
		"stop:0 #3498db, stop:1 #2980b9);"
		"color: white;"
		"font-weight: bold;"
		"}"

		"QPushButton#sendButton:hover:enabled {"
		"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
		"stop:0 #5dade2, stop:1 #3498db);"
		"}"

		"QPushButton#sendButton:pressed {"
		"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
		"stop:0 #2980b9, stop:1 #1f618d);"
		"}"

		"QPushButton#sendButton:disabled {"
		"background: #bdc3c7;"
		"color: #7f8c8d;"
		"}"

		"QPushButton#attachButton {"
		"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
		"stop:0 #95a5a6, stop:1 #7f8c8d);"
		"color: white;"
		"font-size: 13px;"
		"font-weight: bold;"
		"}"

		"QPushButton#attachButton:hover {"
		"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
		"stop:0 #a9b9ba, stop:1 #95a5a6);"
		"}"
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

		"QMenu {"
		"background-color: white;"
		"border: 1px solid #e1e8ed;"
		"border-radius: 8px;"
		"padding: 4px 0px;"
		"min-width: 120px;"
		"}"

		"QMenu::item {"
		"background-color: transparent;"
		"padding: 8px 16px;"
		"margin: 0px 4px;"
		"border-radius: 4px;"
		"color: #2c3e50;"
		"font-size: 13px;"
		"font-family: 'Microsoft YaHei', 'Segoe UI', sans-serif;"
		"}"

		"QMenu::item:selected {"
		"background-color: #3498db;"
		"color: white;"
		"}"

		"QMenu::item:pressed {"
		"background-color: #2980b9;"
		"}"
	);
}

// 新增：更新知识库菜单的方法
void ChatInputWidget::updateKnowledgeBaseMenu()
{
	// 清除现有的动作
	for (QAction* action : knowledgeBaseActions) {
		knowledgeBaseMenu->removeAction(action);
		action->deleteLater();
	}
	knowledgeBaseActions.clear();

	// 如果知识库列表为空，显示"暂无知识库"
	if (knowledgeBaseList.size()<1) {
		QAction* emptyAction = new QAction(tr("NO KnowledgeBase"), this);
		emptyAction->setEnabled(false);
		knowledgeBaseMenu->addAction(emptyAction);
		knowledgeBaseActions.append(emptyAction);
		return;
	}
	
	// 添加知识库项
	for (const auto& pair : knowledgeBaseList)
	{
		QAction* kbAction = new QAction(pair.second.first, this);
		kbAction->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_konwledge.png"));
		kbAction->setToolTip(pair.first);

		// 连接信号槽
		connect(kbAction, &QAction::triggered, this, &ChatInputWidget::onKnowledgeBaseItemClicked);

		knowledgeBaseMenu->addAction(kbAction);
		knowledgeBaseActions.append(kbAction);
	}

	// 添加分隔线和管理选项
	knowledgeBaseMenu->addSeparator();
	QAction* manageAction = new QAction(tr("Manage Knowledge..."), this);
	manageAction->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_settings.png")); // 如果有设置图标的话
	connect(manageAction, &QAction::triggered, [this]() {
		emit KnowledgeBaseSelect(QStringLiteral("__MANAGE__")); // 特殊标识用于管理
	});
	knowledgeBaseMenu->addAction(manageAction);
	knowledgeBaseActions.append(manageAction);
}

// 新增：知识库子项点击槽函数
void ChatInputWidget::onKnowledgeBaseItemClicked()
{
	QAction* senderAction = qobject_cast<QAction*>(sender());
	if (senderAction) {
		QString knowledgeID = senderAction->toolTip();
		emit KnowledgeBaseSelect(knowledgeID);
	}
}

void ChatInputWidget::setPromptLibrary(PromptLibrary* library)
{
	m_promptLibrary = library;
}

void ChatInputWidget::onPromptLibraryClicked()
{
	if (!m_promptLibrary)
	{
		// 如果没有设置提示词库，创建一个
		m_promptLibrary = new PromptLibrary(this);
	}

	if (!m_promptDialog)
	{
		m_promptDialog = new PromptLibraryDialog(m_promptLibrary, this);
		connect(m_promptDialog, &PromptLibraryDialog::promptSelected,
			this, &ChatInputWidget::onPromptSelected);
	}

	m_promptDialog->exec();
}

void ChatInputWidget::onPromptSelected(const QString& content)
{
	if (content.isEmpty() || !textEdit)
	{
		return;
	}

	// 获取当前光标位置
	QTextCursor cursor = textEdit->textCursor();
	QString currentText = textEdit->toPlainText();
	
	// 如果当前有选中文本，替换它；否则在光标位置插入
	if (cursor.hasSelection())
	{
		cursor.removeSelectedText();
	}
	
	// 插入提示词内容
	cursor.insertText(content);
	textEdit->setTextCursor(cursor);
	textEdit->setFocus();
}

void ChatInputWidget::onSelectKnowledgeBaseClicked(std::map<QString, std::pair<QString, QString>> knowBase)
{
	knowledgeBaseList = knowBase;
	updateKnowledgeBaseMenu();
}
