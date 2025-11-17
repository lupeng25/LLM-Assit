#include "Frm_AIAssit.h"
#include <QResizeEvent>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>
#include <QSignalBlocker>
#include <QTextDocument>
#include <QFileDialog>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>
#include <QFileInfo>
#include <QApplication>
#include <algorithm>
#include "include\cmark\cmark.h"

namespace
{
	constexpr int kSidebarExpandWidth = 280;
	constexpr int kSidebarCompactWidth = 248;
	constexpr int kSidebarTightWidth = 220;
	constexpr int kCompactBreakpoint = 1320;
	constexpr int kTightBreakpoint = 1080;
	constexpr int kCollapseBreakpoint = 900;
	constexpr int kResponsiveAnimationDelayMs = 60;

	QString buildDialogTitleSource(const QString& reasoningHtml, const QString& answerHtml)
	{
		QTextDocument answerDoc;
		answerDoc.setHtml(answerHtml);
		const QString answerPlain = answerDoc.toPlainText().trimmed();

		QTextDocument reasoningDoc;
		reasoningDoc.setHtml(reasoningHtml);
		const QString reasoningPlain = reasoningDoc.toPlainText().trimmed();

		if (!reasoningPlain.isEmpty())
		{
			return QStringLiteral("\n ### 推理 \n%1\n ### 回答 \n%2").arg(reasoningPlain, answerPlain);
		}
		return QStringLiteral("\n ### 回答 \n\n%1").arg(answerPlain);
	}
}
Frm_AIAssit::Frm_AIAssit(QWidget *parent)
	: QWidget(parent)
	, m_configRepository(std::make_unique<AppConfigRepository>())
	, m_chatSessionService(std::make_unique<ChatSessionService>(this))
	, m_clientManager(std::make_unique<LLMClientManager>(this)) {
	m_enableBubblePool = true;
	m_bubblePoolHost = new QWidget(this);
	m_bubblePoolHost->setObjectName("BubblePoolHost");
	m_bubblePoolHost->setAttribute(Qt::WA_DontShowOnScreen, true);
	m_bubblePoolHost->hide();
	ui.setupUi(this);

	connect(m_clientManager.get(), &LLMClientManager::clientChanged, this, &Frm_AIAssit::attachToClient);
	connect(m_clientManager.get(), &LLMClientManager::connectionCheckFailed, this, &Frm_AIAssit::onConnectionCheckFailed);
	connect(m_clientManager.get(), &LLMClientManager::connectionCheckSucceeded, this, &Frm_AIAssit::onConnectionCheckSucceeded);
	connect(m_clientManager.get(), &LLMClientManager::modelsFetchFailed, this, &Frm_AIAssit::onModelsFetchFailed);
	connect(m_clientManager.get(), &LLMClientManager::clientError, this, &Frm_AIAssit::onClientManagerError);

	initParams();
	setupSignals();
	initUI();
}
Frm_AIAssit::~Frm_AIAssit()
{
	if (m_chatSessionService) {
		m_chatSessionService->saveSessions();
	}
	releaseAllBubbles();
	clearBubblePool();
	ui.ChatListWidget->clearConversations();
}
void Frm_AIAssit::setupSignals()
{
	connect(this, &Frm_AIAssit::Answer, this, &Frm_AIAssit::getAnswerShow);
	connect(this, &Frm_AIAssit::AnswerStream, this, &Frm_AIAssit::getStreamAnswerShow);
	connect(this, &Frm_AIAssit::PushBtnChanged, ui.ChatInput, &ChatInputWidget::SetButtonEnable);
	connect(this, &Frm_AIAssit::ChangeCurModel, ui.ChatInput, &ChatInputWidget::setModelCurrIndex);
	connect(ui.AIParams, &AIParamWidget::paramsChanged, this, &Frm_AIAssit::ApplyModelParam);
	connect(ui.ChatInput, &ChatInputWidget::MessageUp, this, &Frm_AIAssit::on_pushButton_clicked);
	connect(ui.ChatInput, &ChatInputWidget::ModelSelect, this, &Frm_AIAssit::ChangeModel);
	connect(ui.ChatInput, &ChatInputWidget::addButtonSignal, this, [this]() {
		if (LLMClient) {
			LLMClient->getKnowledgeBase();
		}
	});
	connect(ui.ChatInput, &ChatInputWidget::KnowledgeBaseSelect, this, [this](const QString& kbId) {
		if (LLMClient) {
			LLMClient->ChangeKnowledgeGraph(kbId);
		}
	});
	connect(ui.ChatInput, &ChatInputWidget::InitFileFinished, this, &Frm_AIAssit::UpAllFilesToHost);
	connect(ui.ChatInput, &ChatInputWidget::RemoveFileSignal, this, [this](const QString& fileId) {
		if (LLMClient) {
			LLMClient->CancelUpdateFile(fileId);
		}
	});
	connect(ui.ChatInput, &ChatInputWidget::RemoveAllFilesSignal, this, [this](const QStringList& fileList) {
		if (LLMClient) {
			LLMClient->CancelAllUpdateFiles(fileList);
		}
	});
	connect(ui.ChatShow, &ChatShowWidget::paramSetButtonClicked, this, &Frm_AIAssit::ShowAIParam);
	connect(ui.ChatShow, &ChatShowWidget::toggleButtonClicked, this, &Frm_AIAssit::toggleSidebar);
	connect(ui.ChatListWidget, &ChatList::newConversationRequested, this, &Frm_AIAssit::createNewConversation, Qt::UniqueConnection);
	connect(ui.ChatListWidget, &ChatList::conversationChanged, this, &Frm_AIAssit::onConversationSelected);
	connect(ui.ChatListWidget, &ChatList::renameRequested, this, &Frm_AIAssit::renameCurrentConversation);
	connect(ui.ChatListWidget, &ChatList::deleteRequested, this, &Frm_AIAssit::deleteCurrentConversation);
	connect(ui.ChatListWidget, &ChatList::exportConversationRequested, this, &Frm_AIAssit::onExportConversationRequested);
	connect(ui.ChatListWidget, &ChatList::showDetailsRequested, this, &Frm_AIAssit::onShowDetailsRequested);
	
	// 设置搜索回调函数
	ui.ChatListWidget->setSearchCallback([this](const QString& conversationId) -> QString {
		if (m_chatSessionService) {
			const ChatSession* session = m_chatSessionService->session(conversationId);
			if (session) {
				return buildConversationPlainText(*session);
			}
		}
		return QString();
	});

	// 连接LLMClient的信号（需要在LLMClient创建后调用setupLLMClientSignals()）
	if (LLMClient) {
		setupLLMClientSignals();
	}
}
void Frm_AIAssit::setupLLMClientSignals()
{
	if (!LLMClient) {
		qWarning() << "LLMClient is null in setupLLMClientSignals";
		return;
	}

	// 断开之前的连接（如果有）
	disconnect(LLMClient, nullptr, this, nullptr);

	// 连接LLMClient的通用信号
	connect(LLMClient, &MessageManager::Answer, this, &Frm_AIAssit::getAnswerShow);
	connect(LLMClient, &MessageManager::AnswerStream, this, &Frm_AIAssit::getStreamAnswerShow);
	connect(LLMClient, &MessageManager::ChangeButtonStatus, ui.ChatInput, &ChatInputWidget::SetButtonEnable);
	connect(LLMClient, &MessageManager::FunctionCallSignal, this, &Frm_AIAssit::preFuncall);
	connect(LLMClient, &MessageManager::StreamEnded, this, &Frm_AIAssit::getStreamAnswerEnd);
	connect(LLMClient, &MessageManager::modelsListFetched, ui.ChatInput, &ChatInputWidget::UpdateModelList);
	connect(LLMClient, &MessageManager::KonwledgeBaseSignal, ui.ChatInput, &ChatInputWidget::onSelectKnowledgeBaseClicked);
	connect(LLMClient, &MessageManager::FollowSuggestSignal, this, &Frm_AIAssit::buildBubbleSuggest);

}
void Frm_AIAssit::ShowAIParam()
{
	ui.AIParams->setVisible(bShowParam);
	bShowParam = !bShowParam;
	QTimer::singleShot(50, this, &Frm_AIAssit::recalculateAllChatBubbles);
}
void Frm_AIAssit::initHistoryFile()
{
	if (!m_configRepository) {
		m_configRepository = std::make_unique<AppConfigRepository>();
	}
	if (!m_configRepository->ensureChatHistoryStorage()) {
		qDebug() << "目录创建失败：" << m_configRepository->chatHistoryDirectory();
		return;
	}
	ChatJsonFile = m_configRepository->chatHistoryFile();
	QFileInfo fileInfo(ChatJsonFile);
	if (!fileInfo.exists())
	{
		QFile file(ChatJsonFile);
		if (file.open(QIODevice::WriteOnly))
		{
			file.close();
			qDebug() << "文件不存在，已创建：" << ChatJsonFile;
		}
		else
		{
			qDebug() << "文件创建失败：" << file.errorString();
		}
	}
	else
	{
		qDebug() << "文件已存在：" << ChatJsonFile;
	}
	if (m_chatSessionService) {
		m_chatSessionService->setStorageFile(ChatJsonFile);
	}
}
void Frm_AIAssit::initUI()
{
	this->setWindowTitle(tr("GKG AI Assit"));
	setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
	
	ChangeCurModel(ui.AIParams->GetAIParamModel());
	ShowAIParam();
	bool loadJson = loadChatMapFromJson();
	if (!loadJson)
		createNewConversation();
	else
		PreLoadChat();
	ui.ChatShow->updateEmptyState();
	updateSidebarWidth(width());
	applyResponsiveLayout(width());

	// 初始化阶段禁用发送按钮，等待连接成功后再启用
	PushBtnChanged(false);

	// 初始化时检查服务器连接（信号连接已在 setupLLMClientSignals() 中完成）
	if (m_clientManager) {
		m_clientManager->checkConnection(5000);
	}
	else if (LLMClient) {
		LLMClient->checkServerConnectionAsync(5000);
	}
}
void Frm_AIAssit::updateSidebarWidth(int windowWidth)
{
	if (!ui.ChatListWidget)
	{
		return;
	}

	int targetWidth = kSidebarExpandWidth;
	if (windowWidth <= kTightBreakpoint)
	{
		targetWidth = kSidebarTightWidth;
	}
	else if (windowWidth <= kCompactBreakpoint)
	{
		targetWidth = kSidebarCompactWidth;
	}
	ui.ChatListWidget->setMinimumWidth(targetWidth);
	ui.ChatListWidget->setMaximumWidth(targetWidth);
	if (ui.mainHorizontalLayout)
	{
		ui.mainHorizontalLayout->setStretch(0, targetWidth > 0 ? 1 : 0);
		ui.mainHorizontalLayout->setStretch(1, 4);
	}
}
void Frm_AIAssit::applyResponsiveLayout(int windowWidth)
{
	if (!ui.ChatListWidget)
	{
		return;
	}

	const bool shouldCollapse = windowWidth <= kCollapseBreakpoint;
	updateSidebarWidth(windowWidth);

	if (shouldCollapse)
	{
		m_sidebarCollapsedByResponsive = true;
		if (ui.ChatListWidget->isVisible())
		{
			setSidebarVisible(false, false, true);
		}
	}
	else
	{
		const bool shouldRestore = m_sidebarCollapsedByResponsive && !m_sidebarManuallyHidden;
		m_sidebarCollapsedByResponsive = false;
		if (shouldRestore)
		{
			setSidebarVisible(true, false, true);
		}
	}
}
void Frm_AIAssit::setSidebarVisible(bool visible, bool animated, bool triggeredByResponsive)
{
	if (!ui.ChatListWidget)
	{
		return;
	}

	const bool currentlyVisible = ui.ChatListWidget->isVisible();
	if (currentlyVisible == visible && (!animated || triggeredByResponsive))
	{
		if (!triggeredByResponsive)
		{
			m_sidebarManuallyHidden = !visible;
		}
		return;
	}

	auto updateToggleIcon = [this, visible]()
	{
		ui.ChatShow->setToggleIcon(QIcon(visible
			? ":/QtWidgetsApp/ICONs/icon_open.png"
			: ":/QtWidgetsApp/ICONs/icon_close.png"));
	};

	if (animated)
	{
		if (visible && !currentlyVisible)
		{
			ui.ChatListWidget->setVisible(true);
		}
		auto* effect = new QGraphicsOpacityEffect(ui.ChatListWidget);
		ui.ChatListWidget->setGraphicsEffect(effect);
		auto* animation = new QPropertyAnimation(effect, "opacity", ui.ChatListWidget);
		animation->setDuration(220);
		animation->setEasingCurve(QEasingCurve::OutCubic);
		animation->setStartValue(visible ? 0.0 : 1.0);
		animation->setEndValue(visible ? 1.0 : 0.0);
		connect(animation, &QPropertyAnimation::finished, ui.ChatListWidget, [this, visible, effect]()
		{
			if (!visible)
			{
				ui.ChatListWidget->setVisible(false);
			}
			effect->deleteLater();
		});
		animation->start(QAbstractAnimation::DeleteWhenStopped);
	}
	else
	{
		ui.ChatListWidget->setVisible(visible);
		ui.ChatListWidget->setGraphicsEffect(nullptr);
	}

	updateToggleIcon();
	ui.mainHorizontalLayout->update();
	this->update();

	if (!triggeredByResponsive)
	{
		m_sidebarManuallyHidden = !visible;
	}
	m_sidebarCollapsedByResponsive = triggeredByResponsive && !visible;

	QTimer::singleShot(kResponsiveAnimationDelayMs, this, &Frm_AIAssit::recalculateAllChatBubbles);
}
void Frm_AIAssit::initParams()
{
	initHistoryFile();
	m_FunctionTools = LLMFunctionCall::Get()->Tools();
	params = std::make_unique<LLMParams>();
	params->setFunctionCallTools(m_FunctionTools);
	ui.AIParams->setLLMParams(params.get());
	const QString jsonFileName = m_configRepository ? m_configRepository->modelConfigFile() : QString();
	if (!jsonFileName.isEmpty()) {
		ui.AIParams->loadParamsFromJson(jsonFileName);
	}
	AIProvider platform = static_cast<AIProvider>(params->getLLMPlatForm());
	setLLMClient(platform);
}
void Frm_AIAssit::addChatBubble(const QString& text, bool bIsUser)
{
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	if (!chatFrame)
	{
		return;
	}

	LLMChatFrame* userBubble = acquireBubble(chatFrame);
	if (QWidget* viewport = chatFrame->viewport())
	{
		userBubble->resize(viewport->width(), userBubble->height());
	}
	LLMChatFrame::User_Type bubbleType = bIsUser ? LLMChatFrame::User_Customer : LLMChatFrame::User_Owner;
	userBubble->setUserType(bubbleType);
	//文本以及大小设置
	QSize textSize = userBubble->fontRect(text);
	userBubble->setText(text, QString::number(QDateTime::currentDateTime().toTime_t()), textSize,
		bubbleType);
	textSize = userBubble->getSize();
	//每个bubble都有唯一ID
	QString bubbleID = QUuid::createUuid().toString();
	userBubble->setBubbleID(bubbleID);
	//将最新项添加至当前对话下并滚动至最新项
	QListWidgetItem* item = new QListWidgetItem();
	chatFrame->addItem(item);
	chatFrame->setItemWidget(item, userBubble);
	userBubble->setAttribute(Qt::WA_DeleteOnClose, false);
	refreshBubbleSize(userBubble, item);
	chatFrame->scrollToBottom();
	// 添加消息后更新空状态
	ui.ChatShow->updateEmptyState();
	//用户bubble需要在此处将bubble信息存入到m_ChatMSg
	if (!bIsUser)
	{
		sSingleMsg singleMsg(text, QString::number(QDateTime::currentDateTime().toTime_t()), textSize, LLMChatFrame::User_Owner, tr("New Conversation"), "", userBubble->getBubbleID());
		if (ChatSession* session = currentSession()) {
			session->sMsg.append(singleMsg);
			session->SaveTime = QDateTime::currentDateTime();
		}
	}
}
void Frm_AIAssit::on_pushButton_clicked(ChatSendMessage msg)
{
	addChatBubble(msg.SendText, false);
	if (ui.AIParams->GetAIParambStream())
		StreamSend(msg);
	else
		send(msg);
}
void Frm_AIAssit::ApplyModelParam()
{
	if (!LLMClient) {
		qWarning() << "LLMClient is null in ApplyModelParam";
		return;
	}
	LLMClient->buildRequest();
	PushBtnChanged(false);
	if (m_clientManager) {
		m_clientManager->checkConnection(5000);
	}
	else {
		LLMClient->checkServerConnectionAsync(5000);
	}
}
QJsonObject Frm_AIAssit::parseJsonReplyToMsg(const QByteArray &data, bool isStream)
{
	QJsonDocument response_doc = QJsonDocument::fromJson(data);
	// 解析获取到的Json 获取完整json对象
	QJsonObject rsp_json = response_doc.object();
	// msg对象
	QJsonObject msg;
	const QJsonArray choices = rsp_json.value("choices").toArray();
	if (choices.isEmpty())
	{
		return msg; // 返回空对象，避免越界
	}
	const QJsonObject choice0 = choices.at(0).toObject();
	if (!isStream)
		msg = choice0.value("message").toObject();
	else
		msg = choice0.value("delta").toObject();
	return msg;
}
void Frm_AIAssit::preFuncall(QJsonObject& Content)
{
	// 处理函数调用
	QJsonArray toolCalls = Content["tool_calls"].toArray();
	if (toolCalls.isEmpty())
	{
		return;
	}

	const QJsonObject functionObj = toolCalls.at(0).toObject().value("function").toObject();
	const QString name = functionObj.value("name").toString();
	const QString argsStr = functionObj.value("arguments").toString();

	QJsonParseError parseErr;
	QJsonDocument argsDoc = QJsonDocument::fromJson(argsStr.toUtf8(), &parseErr);
	QJsonObject arguments = (parseErr.error == QJsonParseError::NoError && argsDoc.isObject())
		? argsDoc.object()
		: QJsonObject();

	QJsonObject result = LLMFunctionCall::Get()->executeFunction(name, arguments);
	Content["content"] = QString(QJsonDocument(result).toJson(QJsonDocument::Indented));
	ProcessFunctionCall(Content);
}
void Frm_AIAssit::ProcessFunctionCall(QJsonObject FunctionMsg)
{
	QJsonArray toolCalls = FunctionMsg["tool_calls"].toArray();
	if (toolCalls.isEmpty())
	{
		return;
	}
	QJsonArray toolmsg;
	QJsonObject assistantMsg;
	assistantMsg["role"] = "assistant";
	assistantMsg.insert("content", "");
	assistantMsg["tool_calls"] = toolCalls;
	toolmsg.append(assistantMsg);
	QJsonObject toolMsg;
	QJsonObject tool_call = toolCalls.at(0).toObject();
	toolMsg["tool_call_id"] = tool_call["id"];
	toolMsg["role"] = "tool";
	toolMsg["name"] = tool_call["function"].toObject()["name"].toString();
	toolMsg.insert("content", FunctionMsg["content"]);
	toolmsg.append(toolMsg);
	QJsonObject requestBody;
	requestBody.insert("model", "gkg");
	requestBody.insert("messages", toolmsg);
	requestBody["tools"] = m_FunctionTools;
	QJsonDocument doc(requestBody);
	QByteArray data = doc.toJson();
	QSslConfiguration config = QSslConfiguration::defaultConfiguration();
	config.setProtocol(QSsl::AnyProtocol);
	config.setPeerVerifyMode(QSslSocket::VerifyNone);
	LLMClient->m_NetWorkParams->clientRequest.setSslConfiguration(config);
	QNetworkReply *reply = LLMClient->m_NetWorkParams->clientNetWorkManager->post(LLMClient->m_NetWorkParams->clientRequest, data);;
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		if (reply->error())
		{
			qDebug() << "Error:" << reply->errorString();
		}
		else {
			QByteArray response_data = reply->readAll();
			if (response_data.isEmpty())
			{
				return;
			}
			QJsonDocument response_doc = QJsonDocument::fromJson(response_data);
			QJsonObject msg = parseJsonReplyToMsg(response_data);
			QString content = msg.value("content").toString();
			emit Answer(content, false);
		}
		reply->deleteLater();
	});
}
void Frm_AIAssit::getAnswerShow(const QString& word, bool bError)
{
	// 使用静态常量避免重复创建字符串
	static const QString ANSWER_HEADER = QStringLiteral("\n ### 回答 \n");
	static const QString ANSWER_HEADER_NEWLINE = QStringLiteral("\n ### 回答 \n\n");
	static const QString REASONING_HEADER = QStringLiteral("\n ### 推理 \n");
	static const QString REASONING_NOT_ENABLED = QStringLiteral("推理未开启");
	static const QString REASONING_NOT_SUPPORTED = QStringLiteral("本模型不支持推理");
	static const QString ERROR_TEXT = QStringLiteral("error");

	// 使用静态正则表达式避免重复编译
	static const QRegularExpression regexResult(
		R"(</think>(.*))",
		QRegularExpression::DotMatchesEverythingOption);
	static const QRegularExpression regexThinking(
		R"(<think>(.*)</think>)",
		QRegularExpression::DotMatchesEverythingOption);
	static const QRegularExpression regexReasoningStart(
		QStringLiteral("<think>"));
	static const QRegularExpression regexReasoningEnd(
		QStringLiteral("</think>"));

	QRegularExpressionMatch match = regexResult.match(word);
	QRegularExpressionMatch matchThinking = regexThinking.match(word);
	QString TextAnswer;
	QString TextReasoning;
	QString tempWord;

	if (bError)
	{
		// 错误情况处理
		TextAnswer = ANSWER_HEADER + word;
		TextReasoning = REASONING_HEADER + ERROR_TEXT;
		tempWord = REASONING_HEADER + ERROR_TEXT + ANSWER_HEADER + word;
	}
	else
	{
		// 检查是否包含推理标识符
		if (matchThinking.hasMatch())
		{
			// 包含 <think></think> 标识符的情况
			TextAnswer = match.captured(1).trimmed();
			TextReasoning = REASONING_HEADER + (matchThinking.captured(1).trimmed().isEmpty() ?
				REASONING_NOT_ENABLED : matchThinking.captured(1).trimmed());
			// 替换标识符
			tempWord = word;
			tempWord.replace(regexReasoningStart, REASONING_HEADER);
			tempWord.replace(regexReasoningEnd, ANSWER_HEADER);
		}
		else
		{
			// 不包含推理标识符的情况 - 将整个内容作为回答
			TextAnswer = ANSWER_HEADER_NEWLINE + word.trimmed();
			TextReasoning = REASONING_HEADER + REASONING_NOT_SUPPORTED;
			// 构造tempWord保持相同的结构
			tempWord = REASONING_HEADER + REASONING_NOT_SUPPORTED + ANSWER_HEADER + word.trimmed();
		}
	}

	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	if (!chatFrame || chatFrame->count() == 0) {
		qWarning() << "Chat frame is empty or invalid";
		return;
	}

	QListWidgetItem* latestItem = chatFrame->item(chatFrame->count() - 1);
	if (!latestItem) {
		qWarning() << "Latest item is null";
		return;
	}

	LLMChatFrame *latestWidget = qobject_cast<LLMChatFrame*>(chatFrame->itemWidget(latestItem));
	if (!latestWidget) {
		qWarning() << "Latest widget is null or invalid type";
		return;
	}

	latestWidget->fontRect(TextReasoning, TextAnswer);
	// 注意：fontRect()内部已经调用了update()，这里不需要再次调用
	QString DialogName = updateDialogName(tempWord);
	finalizeLatestBubble(latestWidget, latestItem, DialogName, TextAnswer, TextReasoning, false);
}

void Frm_AIAssit::getStreamAnswerShow(const QString& word)
{
	static const QString REASONING_END = QStringLiteral("</think>");

	m_pendingStreamChunk.append(word);
	if (word.contains(REASONING_END)) {
		m_pendingReasoningEnd = true;
	}

	const int DEFAULT_INTERVAL_MS = 75;
	const int MIN_INTERVAL_MS = 30;
	const int MAX_INTERVAL_MS = 150;

	if (!m_streamDebounceTimer) {
		m_streamDebounceTimer = new QTimer(this);
		m_streamDebounceTimer->setSingleShot(true);
		m_streamDebounceTimer->setInterval(DEFAULT_INTERVAL_MS);

		connect(m_streamDebounceTimer, &QTimer::timeout, this, [this]() {
			QListWidget* chatFrame = ui.ChatShow->getChatFrame();
			if (!chatFrame || chatFrame->count() == 0) {
				m_pendingStreamChunk.clear();
				m_pendingReasoningEnd = false;
				return;
			}

			QListWidgetItem* latestItem = chatFrame->item(chatFrame->count() - 1);
			if (!latestItem) {
				m_pendingStreamChunk.clear();
				m_pendingReasoningEnd = false;
				return;
			}

			LLMChatFrame* latestWidget = qobject_cast<LLMChatFrame*>(chatFrame->itemWidget(latestItem));
			if (!latestWidget) {
				m_pendingStreamChunk.clear();
				m_pendingReasoningEnd = false;
				return;
			}

			if (!m_pendingStreamChunk.isEmpty()) {
				const QString chunk = m_pendingStreamChunk;
				const QString reasoningEndTag = QStringLiteral("</think>");
				if (m_pendingReasoningEnd) {
					int closePos = chunk.indexOf(reasoningEndTag);
					if (closePos == -1) {
						latestWidget->appendText(chunk);
					}
					else {
						QString reasoningPart = chunk.left(closePos);
						if (!reasoningPart.isEmpty()) {
							latestWidget->appendText(reasoningPart);
						}
						latestWidget->ChangeAccpetStatus();
						QString answerPart = chunk.mid(closePos + reasoningEndTag.length());
						if (!answerPart.isEmpty()) {
							latestWidget->appendText(answerPart);
						}
					}
				}
				else {
					latestWidget->appendText(chunk);
				}
			}

			m_pendingStreamChunk.clear();
			m_pendingReasoningEnd = false;

			latestWidget->fontRect(latestWidget->getReasonRawText(), latestWidget->getRawText());
			refreshBubbleSize(latestWidget, latestItem);
			//scheduleScrollToBottom();
		});
	}
	else {
		int currentInterval = m_streamDebounceTimer->interval();
		if (word.length() > 50 && currentInterval < MAX_INTERVAL_MS) {
			m_streamDebounceTimer->setInterval(std::min(currentInterval + 20, MAX_INTERVAL_MS));
		}
		else if (word.length() < 10 && currentInterval > MIN_INTERVAL_MS) {
			m_streamDebounceTimer->setInterval(std::max(currentInterval - 10, MIN_INTERVAL_MS));
		}
	}

	m_streamDebounceTimer->start();
}

int Frm_AIAssit::send(const ChatSendMessage& msg)
{
	if (!LLMClient) {
		qWarning() << "LLMClient is null in send";
		return -1;
	}
	addChatBubble(" ", true);
	int rtn = LLMClient->send(msg);
	return rtn;
}
int Frm_AIAssit::StreamSend(const ChatSendMessage& msg)
{
	if (!LLMClient) {
		qWarning() << "LLMClient is null in StreamSend";
		return -1;
	}
	addChatBubble(" ", true);
	int rtn = LLMClient->StreamSend(msg);
	return rtn;
}
void Frm_AIAssit::getStreamAnswerEnd()
{
	//获取最新对应项
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();

	if (m_streamDebounceTimer) {
		m_streamDebounceTimer->stop();
	}

	QListWidgetItem* latestItem = chatFrame->item(chatFrame->count() - 1);
	LLMChatFrame *latestWidget = (LLMChatFrame*)chatFrame->itemWidget(latestItem);

	if (!m_pendingStreamChunk.isEmpty()) {
		latestWidget->appendText(m_pendingStreamChunk);
		m_pendingStreamChunk.clear();
	}
	if (m_pendingReasoningEnd) {
		latestWidget->ChangeAccpetStatus();
		m_pendingReasoningEnd = false;
	}

	QString DialogName = updateDialogName(latestWidget->getRawText());
	finalizeLatestBubble(latestWidget, latestItem, DialogName,
		latestWidget->getRawText(), latestWidget->getReasonRawText(), true);
	chatFrame->scrollToBottom();
}
void Frm_AIAssit::createNewConversation() {
	// 生成唯一对话ID 
	QString convId = m_chatSessionService ? m_chatSessionService->createSession(tr("New Conversation")) : QUuid::createUuid().toString();
	if (!m_chatSessionService) {
		sMsgList newMsg;
		newMsg.SaveTime = QDateTime::currentDateTime();
		sessionMap().insert(convId, newMsg);
	}
	// 使用 ChatList 的方法添加新对话
	ui.ChatListWidget->insertConversationItem(0, tr("New Conversation"), convId);
	ui.ChatListWidget->setCurrentConversation(convId);
	m_currentConversationId = convId;
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	releaseAllBubbles();
	// 新建对话后更新空状态（此时应该显示空状态）
	ui.ChatShow->updateEmptyState();
}
void Frm_AIAssit::onConversationSelected(QListWidgetItem* current, QListWidgetItem* previous) {
	if (!current) return;

	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	releaseAllBubbles();

	m_currentConversationId = current->data(Qt::UserRole).toString();
	sMsgList* session = m_chatSessionService ? m_chatSessionService->session(m_currentConversationId) : nullptr;
	if (!session) {
		return;
	}
	const sMsgList& ml = *session;

	chatFrame->setUpdatesEnabled(false);
	for (int idx = 0; idx < session->sMsg.size(); ++idx)
	{
		auto& frame = session->sMsg[idx];
		LLMChatFrame* userBubble = acquireBubble(chatFrame);
		if (QWidget* viewport = chatFrame->viewport())
		{
			userBubble->resize(viewport->width(), userBubble->height());
		}

		LLMChatFrame::User_Type Auth = frame.userType == 2 ? LLMChatFrame::User_Customer : LLMChatFrame::User_Owner;
		userBubble->setUserType(Auth);

		QSize textSize = frame.m_AllSize;
		if (!textSize.isValid() || textSize.isEmpty())
		{
			textSize = userBubble->fontRect(frame.m_ChatReasonMsg, frame.m_ChatMsg);
			frame.m_AllSize = textSize;
		}

		userBubble->setTextWithReason(frame.m_ChatReasonMsg, frame.m_ChatMsg,
			frame.m_ChatTime, textSize, Auth);
		userBubble->setTextSuccess();
		userBubble->setImportant(frame.m_IsImportant);
		userBubble->setUserNote(frame.m_Note);
		userBubble->setBubbleID(frame.m_BubbleID);

		QSize finalSize = userBubble->getSize();
		frame.m_AllSize = finalSize;

		QListWidgetItem* item = new QListWidgetItem();
		item->setSizeHint(finalSize);
		chatFrame->addItem(item);
		chatFrame->setItemWidget(item, userBubble);
	}

	chatFrame->setUpdatesEnabled(true);
	chatFrame->scrollToBottom();
	ui.ChatShow->updateEmptyState();
	if (ml.sMsg.size() > 0)
	{
		QTimer::singleShot(100, this, [this]() {
			recalculateVisibleBubbles();
		});
	}
}
bool Frm_AIAssit::loadChatMapFromJson()
{
	if (!m_chatSessionService) {
		return false;
	}
	return m_chatSessionService->loadSessions();
}
void Frm_AIAssit::PreLoadChat()
{
	// UI性能优化：禁用更新，批量加载
	QListWidget* chatList = ui.ChatListWidget->getConversationList();
	chatList->setUpdatesEnabled(false);

	// 按时间倒序排列对话
	QList<QPair<QString, sMsgList>> sortedChats;
	const ChatSessionMap& chats = sessionMap();
	for (auto it = chats.constBegin(); it != chats.constEnd(); ++it)
	{
		sortedChats.append(qMakePair(it.key(), it.value()));
	}
	// 按保存时间排序
	std::sort(sortedChats.begin(), sortedChats.end(),
		[](const QPair<QString, sMsgList>& a, const QPair<QString, sMsgList>& b) {
		return a.second.SaveTime > b.second.SaveTime;
	});
	// 添加到界面
	static const int MAX_DISPLAY_NAME_LENGTH = 12;
	static const QString ELLIPSIS = QStringLiteral("...");
	static const QString NEW_CONVERSATION = tr("New Conversation");

	for (const auto& chat : sortedChats)
	{
		QString displayName;
		if (!chat.second.sMsg.isEmpty())
		{
			displayName = chat.second.sMsg.last().m_DialogName;
			if (displayName == NEW_CONVERSATION && chat.second.sMsg.size() > 1)
			{
				// 尝试从最后一条消息生成名称
				QString lastMsg = chat.second.sMsg.last().m_ChatMsg;
				if (lastMsg.length() > MAX_DISPLAY_NAME_LENGTH)
				{
					displayName = lastMsg.left(MAX_DISPLAY_NAME_LENGTH) + ELLIPSIS;
				}
				else
				{
					displayName = lastMsg;
				}
			}
		}
		else
		{
			displayName = NEW_CONVERSATION;
		}
		ui.ChatListWidget->addConversationItem(displayName, chat.first);
	}

	// UI性能优化：重新启用更新，并一次性刷新
	chatList->setUpdatesEnabled(true);
	chatList->update();

	// 选中第一个对话
	if (ui.ChatListWidget->count() > 0)
	{
		ui.ChatListWidget->setCurrentRow(0);
	}
	QTimer::singleShot(50, this, &Frm_AIAssit::recalculateAllChatBubbles);
}
QString Frm_AIAssit::updateDialogName(const QString& dialogName)
{
	static const QString ANSWER_HEADER = QStringLiteral("\n ### 回答 \n");
	static const int MAX_NAME_LENGTH = 12;
	static const QString ELLIPSIS = QStringLiteral("...");

	int index = dialogName.indexOf(ANSWER_HEADER);
	QString tempName;
	if (index != -1)
	{
		tempName = dialogName.mid(index + ANSWER_HEADER.length() + 1, 15);
		tempName = tempName.trimmed();
		if (tempName.length() > MAX_NAME_LENGTH)
		{
			tempName = tempName.left(MAX_NAME_LENGTH) + ELLIPSIS;
		}
	}
	else
	{
		tempName = tr("New Conversation");
	}
	ui.ChatListWidget->setCurrentItemText(tempName);
	return tempName;
}

void Frm_AIAssit::attachBubbleSignals(LLMChatFrame* bubble)
{
	if (!bubble)
	{
		return;
	}
	connect(bubble, &LLMChatFrame::regenerateClicked, this, &Frm_AIAssit::AskQuestionAgain, Qt::UniqueConnection);
	connect(bubble, &LLMChatFrame::bubbleNoteChanged, this, &Frm_AIAssit::onBubbleNoteChanged, Qt::UniqueConnection);
	connect(bubble, &LLMChatFrame::bubbleImportantToggled, this, &Frm_AIAssit::onBubbleImportantToggled, Qt::UniqueConnection);
	connect(bubble, &LLMChatFrame::bubbleCollapsedToggled, this, &Frm_AIAssit::onBubbleCollapsedToggled, Qt::UniqueConnection);
}

LLMChatFrame* Frm_AIAssit::acquireBubble(QWidget* parent)
{
	QWidget* targetParent = parent;
	if (auto* listWidget = qobject_cast<QListWidget*>(parent))
	{
		targetParent = listWidget->viewport();
	}

	if (!m_enableBubblePool)
	{
		LLMChatFrame* bubble = new LLMChatFrame(targetParent);
		bubble->setAttribute(Qt::WA_DeleteOnClose, false);
		attachBubbleSignals(bubble);
		return bubble;
	}

	while (!m_bubblePool.isEmpty())
	{
		QPointer<LLMChatFrame> candidate = m_bubblePool.takeLast();
		if (candidate)
		{
			LLMChatFrame* bubble = candidate.data();
			disconnect(bubble, &QObject::destroyed, nullptr, nullptr);
			bubble->resetForReuse();
			bubble->setAttribute(Qt::WA_DeleteOnClose, false);
			if (targetParent && bubble->parent() != targetParent)
			{
				bubble->setParent(targetParent);
			}
			bubble->show();
			attachBubbleSignals(bubble);
			return bubble;
		}
	}

	LLMChatFrame* bubble = new LLMChatFrame(targetParent);
	bubble->setAttribute(Qt::WA_DeleteOnClose, false);
	attachBubbleSignals(bubble);
	return bubble;
}

void Frm_AIAssit::releaseBubble(LLMChatFrame* bubble)
{
	if (!bubble)
	{
		return;
	}

	bubble->resetForReuse();
	bubble->hide();

	if (m_enableBubblePool && m_bubblePool.size() < kBubblePoolMaxSize)
	{
		if (m_bubblePoolHost && bubble->parent() != m_bubblePoolHost)
		{
			bubble->setParent(m_bubblePoolHost);
		}
		bubble->setAttribute(Qt::WA_DeleteOnClose, false);
		m_bubblePool.append(QPointer<LLMChatFrame>(bubble));
	}
	else
	{
		disconnect(bubble, &QObject::destroyed, nullptr, nullptr);
		bubble->prepareForDeletion();
		bubble->deleteLater();
	}
}

void Frm_AIAssit::releaseAllBubbles()
{
	QListWidget* chatFrame = ui.ChatShow ? ui.ChatShow->getChatFrame() : nullptr;
	if (!chatFrame)
	{
		return;
	}

	const QSignalBlocker blocker(chatFrame);
	for (int idx = chatFrame->count() - 1; idx >= 0; --idx)
	{
		QListWidgetItem* item = chatFrame->item(idx);
		if (!item)
		{
			continue;
		}

		if (QWidget* widget = chatFrame->itemWidget(item))
		{
			chatFrame->removeItemWidget(item);
			if (auto* bubble = qobject_cast<LLMChatFrame*>(widget))
			{
				releaseBubble(bubble);
			}
			else
			{
				widget->deleteLater();
			}
		}

		QListWidgetItem* removed = chatFrame->takeItem(idx);
		delete removed;
	}

	if (!m_enableBubblePool)
	{
		m_bubblePool.clear();
	}
}

void Frm_AIAssit::clearBubblePool()
{
	if (!m_enableBubblePool)
	{
		return;
	}
	for (QPointer<LLMChatFrame>& bubblePtr : m_bubblePool)
	{
		if (LLMChatFrame* bubble = bubblePtr.data())
		{
			bubble->prepareForDeletion();
			bubble->deleteLater();
		}
	}
	m_bubblePool.clear();
}

void Frm_AIAssit::deleteCurrentConversation()
{
	QListWidgetItem* currentItem = ui.ChatListWidget->getConversationList()->currentItem();
	if (!currentItem) return;
	QString convId = currentItem->data(Qt::UserRole).toString();
	// 确认删除
	QMessageBox::StandardButton reply = QMessageBox::question(this,
		tr("Delete"),
		tr("Delete this Chat?"),
		QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes)
	{
		// 从数据中移除
		if (m_chatSessionService) {
			m_chatSessionService->removeSession(convId);
		}
		// 从界面移除
		int row = ui.ChatListWidget->getConversationList()->row(currentItem);
		ui.ChatListWidget->getConversationList()->takeItem(row);
		delete currentItem;
		// 如果是当前对话，清空聊天区域
		if (convId == m_currentConversationId)
		{
			releaseAllBubbles();
			if (ui.ChatListWidget->getConversationList()->count() > 0)
			{
				ui.ChatListWidget->getConversationList()->setCurrentRow(0);
			}
			else
			{
				createNewConversation();
			}
			// 删除对话后更新空状态
			ui.ChatShow->updateEmptyState();
		}
	}
}
void Frm_AIAssit::renameCurrentConversation()
{
	QListWidgetItem* currentItem = ui.ChatListWidget->getConversationList()->currentItem();
	if (!currentItem) return;
	bool bReNameConver;
	QString currentText = currentItem->text();
	QString cnverNewName = QInputDialog::getText(this,
		tr("Rename the Conversation"),
		tr("Please Input New Name Of the Chat:"),
		QLineEdit::Normal, currentText, &bReNameConver);
	if (!m_chatSessionService) {
		return;
	}
	sMsgList* session = m_chatSessionService->session(m_currentConversationId);
	if (!session) {
		return;
	}
	if (bReNameConver && !cnverNewName.isEmpty() && session->sMsg.size() > 1)
	{
		currentItem->setText(cnverNewName);
		session->sMsg.last().m_DialogName = cnverNewName;
	}
}
void Frm_AIAssit::toggleSidebar()
{
	const bool targetVisible = !ui.ChatListWidget->isVisible();
	setSidebarVisible(targetVisible, true, false);
}
QString Frm_AIAssit::GetLastestAsk(QString msg)
{
	if (!m_chatSessionService) {
		return QString();
	}
	sMsgList* session = m_chatSessionService->session(m_currentConversationId);
	if (!session) {
		return QString();
	}
	int index = session->GetMsgIndex(msg);
	if (index >= 0)
	{
		return session->sMsg[index - 1].m_ChatMsg;
	}
	return QString();
}
void Frm_AIAssit::AskQuestionAgain(QString msg)
{
	QString SendMsg = GetLastestAsk(msg);
	ChatSendMessage askMsg;
	askMsg.SendText = SendMsg;
	if (SendMsg.size() > 0)
		on_pushButton_clicked(askMsg);
}
void Frm_AIAssit::ChangeModel(int iModel)
{
	ui.AIParams->SetAIParamModel(iModel);
}
void Frm_AIAssit::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	applyResponsiveLayout(event->size().width());
	// 使用成员变量存储定时器
	if (!m_resizeTimer) {
		m_resizeTimer = new QTimer(this);
		m_resizeTimer->setSingleShot(true);
		connect(m_resizeTimer, &QTimer::timeout, this, [this]() {
			recalculateVisibleBubbles();
		});
	}
	m_resizeTimer->stop();
	m_resizeTimer->start(150);
}
void Frm_AIAssit::refreshBubbleSize(LLMChatFrame* bubble, QListWidgetItem* item, const QString& conversationId)
{
	if (!bubble || !item)
	{
		return;
	}
	QSize bubbleSize = bubble->getSize();
	if (!bubbleSize.isValid())
	{
		bubbleSize = bubble->size();
	}
	if (QListWidget* ownerList = item->listWidget())
	{
		const int availableWidth = ownerList->viewport()->width();
		if (availableWidth > 0)
		{
			bubble->resize(availableWidth, bubbleSize.height());
			bubbleSize.setWidth(availableWidth);
		}
	}
	item->setSizeHint(bubbleSize);
	QString convId = conversationId.isEmpty() ? m_currentConversationId : conversationId;
	if (convId.isEmpty())
	{
		return;
	}
	auto convIt = sessionMap().find(convId);
	if (convIt == sessionMap().end())
	{
		return;
	}
	const QString bubbleId = bubble->getBubbleID();
	if (bubbleId.isEmpty())
	{
		return;
	}
	for (auto& msg : convIt->sMsg)
	{
		if (msg.m_BubbleID == bubbleId)
		{
			msg.m_AllSize = bubbleSize;
			break;
		}
	}
}
void Frm_AIAssit::finalizeLatestBubble(LLMChatFrame* bubble, QListWidgetItem* item, const QString& dialogName,
	const QString& answerHtml, const QString& reasoningHtml, bool markStreamCompleted)
{
	if (!bubble || !item)
	{
		return;
	}
	bubble->setTextSuccess();
	if (markStreamCompleted)
	{
		bubble->ChangeStream();
	}
	refreshBubbleSize(bubble, item);
	PushBtnChanged(true);
	sSingleMsg singleMsg(answerHtml,
		QString::number(QDateTime::currentDateTime().toTime_t()),
		bubble->getSize(),
		LLMChatFrame::User_Customer,
		dialogName,
		reasoningHtml,
		bubble->getBubbleID(),
		bubble->isImportant(),
		bubble->userNote());
	if (ChatSession* session = currentSession()) {
		session->sMsg.append(singleMsg);
		session->SaveTime = QDateTime::currentDateTime();
	}
}
void Frm_AIAssit::recalculateVisibleBubbles()
{
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	if (!chatFrame) return;
	// 获取可见区域
	QRect viewport = chatFrame->viewport()->rect();
	for (int i = 0; i < chatFrame->count(); ++i)
	{
		QListWidgetItem* item = chatFrame->item(i);
		QRect itemRect = chatFrame->visualItemRect(item);
		// 只处理可见的项
		if (!viewport.intersects(itemRect))
		{
			continue;
		}
		LLMChatFrame* bubbleWidget = qobject_cast<LLMChatFrame*>(chatFrame->itemWidget(item));
		if (bubbleWidget && bubbleWidget->isSending())
		{
			int availableWidth = chatFrame->viewport()->width();
			bubbleWidget->resize(availableWidth, bubbleWidget->height());
			refreshBubbleSize(bubbleWidget, item);
		}
	}
	chatFrame->update();
}
void Frm_AIAssit::recalculateAllChatBubbles()
{
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	if (!chatFrame || chatFrame->count() == 0) return;
	// 批量处理
	chatFrame->setUpdatesEnabled(false);
	int availableWidth = chatFrame->viewport()->width();
	for (int i = 0; i < chatFrame->count(); ++i)
	{
		QListWidgetItem* item = chatFrame->item(i);
		LLMChatFrame* bubbleWidget = qobject_cast<LLMChatFrame*>(chatFrame->itemWidget(item));
		if (!bubbleWidget) continue;
		bubbleWidget->resize(availableWidth, bubbleWidget->height());
		refreshBubbleSize(bubbleWidget, item);
	}
	chatFrame->setUpdatesEnabled(true);
	chatFrame->update();
}
void Frm_AIAssit::setLLMClient(AIProvider platform)
{
	if (!m_clientManager) {
		m_clientManager = std::make_unique<LLMClientManager>(this);
		connect(m_clientManager.get(), &LLMClientManager::clientChanged, this, &Frm_AIAssit::attachToClient);
	}
	m_clientManager->rebuild(platform, params.get());
}
void Frm_AIAssit::SetLLMCommandFunction(std::function<QString(QString)> function)
{
	LLMFunctionCall::Get()->m_LLMCommandFunc = function;
}
void Frm_AIAssit::buildBubbleSuggest(QStringList suggestions)
{
}
void Frm_AIAssit::UpAllFilesToHost(const QString& files)
{
	if (!LLMClient) {
		qWarning() << "LLMClient is null in UpAllFilesToHost";
		return;
	}
	LLMClient->uploadFile(files);
}

void Frm_AIAssit::onBubbleNoteChanged(const QString& bubbleId, const QString& note)
{
	if (bubbleId.isEmpty())
	{
		return;
	}
	if (ChatSession* session = currentSession())
	{
		for (auto& msg : session->sMsg)
		{
			if (msg.m_BubbleID == bubbleId)
			{
				msg.m_Note = note;
				session->SaveTime = QDateTime::currentDateTime();
				break;
			}
		}
	}
}

void Frm_AIAssit::onBubbleCollapsedToggled(const QString& bubbleId, bool isCollapsed)
{
	if (bubbleId.isEmpty())
	{
		return;
	}
	
	// 找到对应的列表项并更新大小
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	if (!chatFrame)
	{
		return;
	}
	
	// 遍历所有列表项，找到对应的气泡
	for (int i = 0; i < chatFrame->count(); ++i)
	{
		QListWidgetItem* item = chatFrame->item(i);
		if (!item)
		{
			continue;
		}
		
		LLMChatFrame* bubbleWidget = qobject_cast<LLMChatFrame*>(chatFrame->itemWidget(item));
		if (bubbleWidget && bubbleWidget->getBubbleID() == bubbleId)
		{
			// 重新计算布局以获取新的大小（refreshLayoutAfterContentChange 已经在 setCollapsed 中调用）
			// 这里只需要更新列表项的大小
			refreshBubbleSize(bubbleWidget, item);
			
			// 更新会话数据中的大小
			if (ChatSession* session = currentSession())
			{
				for (auto& msg : session->sMsg)
				{
					if (msg.m_BubbleID == bubbleId)
					{
						msg.m_AllSize = bubbleWidget->getSize();
						session->SaveTime = QDateTime::currentDateTime();
						break;
					}
				}
			}
			break;
		}
	}
}

void Frm_AIAssit::onBubbleImportantToggled(const QString& bubbleId, bool isImportant)
{
	if (bubbleId.isEmpty())
	{
		return;
	}
	if (ChatSession* session = currentSession())
	{
		for (auto& msg : session->sMsg)
		{
			if (msg.m_BubbleID == bubbleId)
			{
				msg.m_IsImportant = isImportant;
				session->SaveTime = QDateTime::currentDateTime();
				break;
			}
		}
	}
}

void Frm_AIAssit::onConnectionCheckFailed(const QString& errorMessage)
{
	const QString message = errorMessage.isEmpty() ? tr("Unable to connect to the server. Please check your network or configuration.") : errorMessage;
	QMessageBox::warning(this, tr("Connection Failed"), message);
	PushBtnChanged(false);
}

void Frm_AIAssit::onConnectionCheckSucceeded()
{
	PushBtnChanged(true);
	if (m_clientManager) {
		m_clientManager->fetchModels(5000);
	}
	else if (LLMClient) {
		LLMClient->fetchModelsAsync();
	}
}

void Frm_AIAssit::onModelsFetchFailed(const QString& errorMessage)
{
	const QString message = errorMessage.isEmpty() ? tr("Failed to retrieve model list. Please try again later.") : errorMessage;
	QMessageBox::warning(this, tr("Model Fetch Failed"), message);
}

void Frm_AIAssit::onClientManagerError(const QString& context, const QString& errorMessage)
{
	const QString message = errorMessage.isEmpty() ? tr("An unknown error occurred.") : errorMessage;
	QMessageBox::warning(this, tr("Client Error"), QStringLiteral("%1: %2").arg(context, message));
}

ChatSession* Frm_AIAssit::currentSession() {
	if (!m_chatSessionService || m_currentConversationId.isEmpty()) {
		return nullptr;
	}
	return m_chatSessionService->session(m_currentConversationId);
}

const ChatSession* Frm_AIAssit::currentSession() const {
	if (!m_chatSessionService || m_currentConversationId.isEmpty()) {
		return nullptr;
	}
	return m_chatSessionService->session(m_currentConversationId);
}

ChatSessionMap& Frm_AIAssit::sessionMap() {
	static ChatSessionMap empty;
	return m_chatSessionService ? m_chatSessionService->sessions() : empty;
}

const ChatSessionMap& Frm_AIAssit::sessionMap() const {
	static ChatSessionMap empty;
	return m_chatSessionService ? m_chatSessionService->sessions() : empty;
}

void Frm_AIAssit::onExportConversationRequested(const QString& conversationId, const QString& format)
{
	if (conversationId.isEmpty() || !m_chatSessionService)
	{
		return;
	}

	const ChatSession* session = m_chatSessionService->session(conversationId);
	if (!session || session->sMsg.isEmpty())
	{
		QMessageBox::information(this, tr("No Export Content"), tr("This conversation has no messages to export."));
		return;
	}

	// 构建导出内容
	QString content;
	QString filter;
	QString suffix;
	QString dialogTitle;

	if (format == QStringLiteral("markdown"))
	{
		content = buildConversationMarkdown(*session);
		filter = tr("Markdown Files (*.md)");
		suffix = QStringLiteral("md");
		dialogTitle = tr("Export As Markdown");
	}
	else if (format == QStringLiteral("html"))
	{
		content = buildConversationHtml(*session);
		filter = tr("HTML Files (*.html *.htm)");
		suffix = QStringLiteral("html");
		dialogTitle = tr("Export As HTML");
	}
	else // text
	{
		content = buildConversationPlainText(*session);
		filter = tr("Text Files (*.txt)");
		suffix = QStringLiteral("txt");
		dialogTitle = tr("Export As Text");
	}

	if (content.isEmpty())
	{
		QMessageBox::information(this, tr("No Export Content"), tr("The conversation has no exportable content."));
		return;
	}

	// 选择保存位置
	QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
	if (defaultDir.isEmpty())
	{
		defaultDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
	}
	if (defaultDir.isEmpty())
	{
		defaultDir = QStringLiteral(".");
	}

	QString defaultName = QStringLiteral("conversation_%1.%2")
		.arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss")), suffix);
	QString defaultPath = QDir(defaultDir).filePath(defaultName);

	QString filePath = QFileDialog::getSaveFileName(this, dialogTitle, defaultPath, filter);
	if (filePath.isEmpty())
	{
		return;
	}

	QFileInfo info(filePath);
	if (info.suffix().isEmpty() && !suffix.isEmpty())
	{
		filePath += QStringLiteral(".") + suffix;
	}

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, tr("Export Failed"), tr("Failed to save file: %1").arg(file.errorString()));
		return;
	}

	QTextStream stream(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	stream.setEncoding(QStringConverter::Utf8);
#else
	stream.setCodec("UTF-8");
#endif
	stream << content;
	file.close();

	QMessageBox::information(this, tr("Export Success"), tr("Conversation exported successfully to:\n%1").arg(filePath));
}

void Frm_AIAssit::onShowDetailsRequested(const QString& conversationId)
{
	if (conversationId.isEmpty() || !m_chatSessionService)
	{
		return;
	}

	const ChatSession* session = m_chatSessionService->session(conversationId);
	if (!session)
	{
		return;
	}

	QDialog dialog(this);
	dialog.setWindowTitle(tr("Conversation Details"));
	dialog.setModal(true);
	dialog.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	dialog.setAttribute(Qt::WA_DeleteOnClose, false);

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
		"    background: transparent;"
		"}"
		"QLabel[objectName='titleLabel'] {"
		"    font-weight: 600;"
		"    font-size: 16px;"
		"    padding: 8px 0px;"
		"}"
		"QLabel[objectName='valueLabel'] {"
		"    color: #475569;"
		"    padding: 4px 0px;"
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
	);

	QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
	mainLayout->setSpacing(16);
	mainLayout->setContentsMargins(24, 24, 24, 24);
	dialog.setLayout(mainLayout);

	// 计算统计信息
	int totalMessages = session->sMsg.size();
	int userMessages = 0;
	int aiMessages = 0;
	qint64 totalChars = 0;
	for (const auto& msg : session->sMsg)
	{
		QTextDocument doc;
		doc.setHtml(msg.m_ChatMsg);
		const int msgLength = doc.toPlainText().length();
		totalChars += msgLength;
		if (msg.userType == 1) // User_Customer
		{
			userMessages++;
		}
		else
		{
			aiMessages++;
		}
	}

	// 创建详情标签
	auto addDetailRow = [&mainLayout, &dialog](const QString& title, const QString& value) {
		QLabel* titleLabel = new QLabel(title + QStringLiteral(":"), &dialog);
		titleLabel->setObjectName(QStringLiteral("titleLabel"));
		mainLayout->addWidget(titleLabel);

		QLabel* valueLabel = new QLabel(value, &dialog);
		valueLabel->setObjectName(QStringLiteral("valueLabel"));
		valueLabel->setWordWrap(true);
		mainLayout->addWidget(valueLabel);
		mainLayout->addSpacing(8);
	};

	addDetailRow(tr("Conversation ID"), conversationId);
	addDetailRow(tr("Created Time"), session->SaveTime.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")));
	addDetailRow(tr("Last Modified"), session->SaveTime.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")));
	addDetailRow(tr("Total Messages"), QString::number(totalMessages));
	addDetailRow(tr("User Messages"), QString::number(userMessages));
	addDetailRow(tr("AI Messages"), QString::number(aiMessages));
	addDetailRow(tr("Total Characters"), QString::number(totalChars));

	mainLayout->addStretch();

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->setSpacing(12);
	buttonLayout->addStretch();

	QPushButton* closeButton = new QPushButton(tr("Close"), &dialog);
	closeButton->setDefault(true);
	buttonLayout->addWidget(closeButton);
	mainLayout->addLayout(buttonLayout);

	QObject::connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

	// 调整大小并居中显示对话框
	dialog.adjustSize();
	dialog.resize(dialog.sizeHint().expandedTo(QSize(500, 400)));
	
	// 强制更新布局
	mainLayout->activate();
	dialog.updateGeometry();
	
	if (QWidget* parentWindow = window())
	{
		QPoint parentCenter = parentWindow->geometry().center();
		QRect dialogRect = dialog.geometry();
		dialog.move(parentCenter.x() - dialogRect.width() / 2, parentCenter.y() - dialogRect.height() / 2);
	}

	// 确保对话框在显示前完全布局
	dialog.show();
	QApplication::processEvents();
	dialog.hide();
	
	dialog.exec();
}

QString Frm_AIAssit::buildConversationMarkdown(const ChatSession& session) const
{
	QStringList lines;
	lines << QStringLiteral("# Conversation Export");
	lines << QString();
	lines << QStringLiteral("**Created:** ") + session.SaveTime.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
	lines << QString();

	for (const auto& msg : session.sMsg)
	{
		QTextDocument doc;
		doc.setHtml(msg.m_ChatMsg);
		QString plainText = doc.toPlainText().trimmed();

		QTextDocument reasonDoc;
		reasonDoc.setHtml(msg.m_ChatReasonMsg);
		QString reasonPlain = reasonDoc.toPlainText().trimmed();

		if (msg.userType == 1) // User_Customer
		{
			lines << QStringLiteral("## User");
			if (!reasonPlain.isEmpty())
			{
				lines << QStringLiteral("### Thinking");
				lines << reasonPlain;
				lines << QString();
			}
			lines << plainText;
		}
		else
		{
			lines << QStringLiteral("## Assistant");
			if (!reasonPlain.isEmpty())
			{
				lines << QStringLiteral("### Thinking");
				lines << reasonPlain;
				lines << QString();
			}
			lines << plainText;
		}
		lines << QString();
		lines << QStringLiteral("---");
		lines << QString();
	}

	return lines.join(QStringLiteral("\n"));
}

QString Frm_AIAssit::buildConversationHtml(const ChatSession& session) const
{
	QString html = QStringLiteral("<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n");
	html += QStringLiteral("<title>Conversation Export</title>\n");
	html += QStringLiteral("<style>body{font-family:'Microsoft YaHei UI','Segoe UI',sans-serif;max-width:800px;margin:0 auto;padding:20px;line-height:1.6;}</style>\n");
	html += QStringLiteral("</head>\n<body>\n");
	html += QStringLiteral("<h1>Conversation Export</h1>\n");
	html += QStringLiteral("<p><strong>Created:</strong> ") + session.SaveTime.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")) + QStringLiteral("</p>\n<hr>\n");

	for (const auto& msg : session.sMsg)
	{
		html += QStringLiteral("<div style=\"margin:20px 0;\">\n");
		if (msg.userType == 1) // User_Customer
		{
			html += QStringLiteral("<h2>User</h2>\n");
		}
		else
		{
			html += QStringLiteral("<h2>Assistant</h2>\n");
		}

		if (!msg.m_ChatReasonMsg.isEmpty())
		{
			html += QStringLiteral("<h3>Thinking</h3>\n");
			html += msg.m_ChatReasonMsg;
			html += QStringLiteral("\n");
		}

		html += msg.m_ChatMsg;
		html += QStringLiteral("\n</div>\n<hr>\n");
	}

	html += QStringLiteral("</body>\n</html>");
	return html;
}

QString Frm_AIAssit::buildConversationPlainText(const ChatSession& session) const
{
	QStringList lines;
	lines << QStringLiteral("Conversation Export");
	lines << QString();
	lines << QStringLiteral("Created: ") + session.SaveTime.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
	lines << QString();
	lines << QString(50, QChar('='));
	lines << QString();

	for (const auto& msg : session.sMsg)
	{
		QTextDocument doc;
		doc.setHtml(msg.m_ChatMsg);
		QString plainText = doc.toPlainText().trimmed();

		QTextDocument reasonDoc;
		reasonDoc.setHtml(msg.m_ChatReasonMsg);
		QString reasonPlain = reasonDoc.toPlainText().trimmed();

		if (msg.userType == 1) // User_Customer
		{
			lines << QStringLiteral("User:");
			if (!reasonPlain.isEmpty())
			{
				lines << QStringLiteral("Thinking:") + reasonPlain;
			}
		}
		else
		{
			lines << QStringLiteral("Assistant:");
			if (!reasonPlain.isEmpty())
			{
				lines << QStringLiteral("Thinking:") + reasonPlain;
			}
		}
		lines << plainText;
		lines << QString();
		lines << QString(50, QChar('-'));
		lines << QString();
	}

	return lines.join(QStringLiteral("\n"));
}

void Frm_AIAssit::attachToClient(MessageManager* client) {
	LLMClient = client;
	if (LLMClient) {
		setupLLMClientSignals();
	}
}
