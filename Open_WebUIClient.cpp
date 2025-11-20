#include "Open_WebUIClient.h"

Open_WebUIClient::Open_WebUIClient(QObject *parent)
	: MessageManager(parent)
{
	m_LLMParams = new LLMParams();
	m_NetWorkParams = new ClientNetWork();

	// 初始化连接测试定时器
	m_connectionCheckTimer = new QTimer(this);
	m_connectionCheckTimer->setSingleShot(true);
	connect(m_connectionCheckTimer, &QTimer::timeout, this, &Open_WebUIClient::onCheckConnectionTimeout);

	// 初始化获取模型列表定时器
	m_fetchModelsTimer = new QTimer(this);
	m_fetchModelsTimer->setSingleShot(true);
	connect(m_fetchModelsTimer, &QTimer::timeout, this, &Open_WebUIClient::onFetchModelsTimeout);

	m_currentRequestType = RequestType::ConnectionCheck;
}

Open_WebUIClient::~Open_WebUIClient()
{
	if (m_NetWorkParams && m_NetWorkParams->clientNetWorkReply)
	{
		m_NetWorkParams->clientNetWorkReply->abort();
		m_NetWorkParams->clientNetWorkReply->disconnect();
		m_NetWorkParams->clientNetWorkReply.reset();
	}
	// 注意：m_LLMParams 可能已被 setLLMParams() 设置为外部管理的对象
	// setLLMParams() 已经处理了构造函数中创建的 m_LLMParams 的删除
	// 如果 m_LLMParams 是外部传入的，不应该在这里删除，由外部管理其生命周期
	// 因此这里不删除 m_LLMParams，避免双重释放
	delete m_NetWorkParams;
	m_LLMParams = nullptr;
	m_NetWorkParams = nullptr;
}

AIProvider Open_WebUIClient::getProvider() const
{
	return AIProvider::Open_WebUI;
}

QString Open_WebUIClient::getProviderName() const
{
	return "OpenWebUI";
}

QString Open_WebUIClient::getVersion() const
{
	return "1.0";
}

QByteArray Open_WebUIClient::buildMessageBody(const ChatSendMessage& msg)
{
	QJsonObject SendMessageBody;
	QJsonArray messagesArray;
	QJsonObject messageObject;
	QJsonObject knowledgeObject;
	QJsonArray knowledgeArray;
	QJsonArray UpFilesArray;
	QJsonObject UpFileObject;
	for (auto KnowledgeID : addKnowledge)
	{
		knowledgeObject["type"] = QStringLiteral("collection");
		knowledgeObject["id"] = KnowledgeID;
		knowledgeArray.append(knowledgeObject);
	}
	for (auto it = m_UpFiles.begin(); it != m_UpFiles.end(); ++it)
	{
		QString fileName = it.key();
		QString fileID = it.value();
		UpFileObject["type"] = "file";
		UpFileObject["id"] = fileID;
		UpFilesArray.append(UpFileObject);
	}
	const std::map<int, QString> modelMap =
	{
		{ 0, "gkg" },
		{ 1, "gkg32" },
		{ 2, "gkgvision" },
		{ 3, "Codeinter" }
	};
	messageObject["role"] = "user";//角色
								   //发送信息:输入信息+文档i...
	QString finalSendMsg = msg.SendText;
	if (msg.fileContext.size() > 0)
	{
		for (int i = 0; i<msg.fileContext.size(); i++)
		{
			finalSendMsg += QStringLiteral("\n 文档%1内容:").arg(i) + msg.fileContext[i];
		}
	}
	finalSendMsg += m_LLMParams->getOpenThink() ? "\\think" : "\\no_think";
	messageObject["content"] = finalSendMsg;
	//图像:目前仅支持转base64格式
	if (msg.Image64.size() > 0)
	{
		QJsonArray ImagejsonArray;
		for (const QString &base64Str : msg.Image64)
		{
			ImagejsonArray.append(base64Str);
		}
		messageObject["images"] = ImagejsonArray;
	}
	messagesArray.append(messageObject);
	//模型参数设置
	SendMessageBody["model"] = m_availableModelIds.empty() ? modelMap.find(m_LLMParams->getModel())->second :
		m_availableModelIds[m_LLMParams->getModel()];
	SendMessageBody["messages"] = messagesArray;
	SendMessageBody["mode"] = (m_LLMParams->getChatMode() == 0) ? "query" : "chat";
	SendMessageBody["max_tokens"] = m_LLMParams->getMaxToken();
	SendMessageBody["temperature"] = m_LLMParams->getTemperature();
	SendMessageBody["files"] = UpFilesArray;
	if (m_LLMParams->getStreamChat())
	{
		SendMessageBody.insert("stream", true);
		QJsonObject stream_options;
		stream_options.insert("include_usage", true);
		SendMessageBody.insert("stream_options", stream_options);
	}
	else
	{
		SendMessageBody["tools"] = m_LLMParams->getFunctionCallTools();
	}

	return QJsonDocument(SendMessageBody).toJson();
}

void Open_WebUIClient::SendPreProcess(const ChatSendMessage& msg)
{
	QByteArray postData = buildMessageBody(msg);
	QSslConfiguration config = QSslConfiguration::defaultConfiguration();
	config.setProtocol(QSsl::AnyProtocol);
	config.setPeerVerifyMode(QSslSocket::VerifyNone);
	m_NetWorkParams->clientRequest.setSslConfiguration(config);
	m_NetWorkParams->clientNetWorkReply = std::unique_ptr<QNetworkReply>(m_NetWorkParams->clientNetWorkManager->post(m_NetWorkParams->clientRequest, postData));
}

QJsonObject Open_WebUIClient::parseJsonReplyToMsg(const QByteArray &data)
{
	QJsonObject msg;

	// 检查输入数据是否为空
	if (data.isEmpty())
	{
		return msg;
	}

	QJsonParseError parseError;
	QJsonDocument response_doc = QJsonDocument::fromJson(data, &parseError);

	// 检查JSON解析是否成功
	if (parseError.error != QJsonParseError::NoError)
	{
		return msg;
	}

	// 检查文档是否为空或不是对象类型
	if (response_doc.isNull() || !response_doc.isObject())
	{
		return msg;
	}

	// 获取根JSON对象
	QJsonObject rsp_json = response_doc.object();

	// 检查是否包含choices字段
	if (!rsp_json.contains("choices"))
	{
		return msg;
	}

	// 获取choices数组
	QJsonValue choicesValue = rsp_json.value("choices");
	if (!choicesValue.isArray())
	{
		return msg;
	}

	QJsonArray choicesArray = choicesValue.toArray();

	// 检查choices数组是否为空
	if (choicesArray.isEmpty())
	{
		return msg;
	}

	// 获取第一个choice对象
	QJsonValue firstChoice = choicesArray[0];
	if (!firstChoice.isObject())
	{
		return msg;
	}

	QJsonObject choiceObj = firstChoice.toObject();

	// 根据流式或非流式模式获取消息对象
	QString messageKey = m_LLMParams->getStreamChat() ? "delta" : "message";

	if (!choiceObj.contains(messageKey))
	{
		return msg;
	}

	QJsonValue messageValue = choiceObj.value(messageKey);
	if (!messageValue.isObject())
	{
		return msg;
	}

	msg = messageValue.toObject();

	return msg;
}

int Open_WebUIClient::send(const ChatSendMessage& msg)
{
	SendPreProcess(msg);
	connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished, this, &Open_WebUIClient::getAnswer);
	return true;
}

int Open_WebUIClient::StreamSend(const ChatSendMessage& msg)
{
	m_isStreamingReasoning = false;
	SendPreProcess(msg);
	connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::readyRead, this, &Open_WebUIClient::getStreamAnswer, Qt::QueuedConnection);
	connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished, this, &Open_WebUIClient::processStreamEnded);
	return true;
}

QString Open_WebUIClient::GetError(const QString& errorLevel, const QString& errorContext)
{
	ChangeButtonStatus(true);
	static const QMap<QString, QString> contextErrorMap = {
		{ "Connection timed out", QStringLiteral("连接请求超时") },
		{ "Permission denied",    QStringLiteral("权限不足") },
		{ "Connection refused",QStringLiteral("服务器拒绝连接") }
	};

	if (contextErrorMap.contains(errorLevel))
	{
		return contextErrorMap.value(errorLevel);
	}
	else if (errorLevel.contains("not found"))
		return tr("IP Address Error");

	QString errorCode = MessageManager::extractHttpErrorCode(errorLevel);

	if (errorCode == "Bad Request")
	{
		const QString errorMsg = MessageManager::extractJsonField(errorContext, QStringLiteral("error"));
		if (!errorMsg.isEmpty())
		{
			if (errorMsg == "Message is empty")
				return tr("Please Input Useful Message");
			return tr("Error") + errorMsg;
		}
		return tr("Please Check Input or IP Address");
	}
	else if (errorCode == "Forbidden")
	{
		return tr("API Key Error");
	}

	return tr("Unkonw Error");
}

void Open_WebUIClient::processStreamEnded()
{
	if (m_NetWorkParams->clientNetWorkReply->error())
	{
		QString errorMsg = GetError(m_NetWorkParams->clientNetWorkReply->errorString(), m_NetWorkParams->clientNetWorkReply->readAll());
		emit Answer(errorMsg, true);
		return;
	}
	if (m_isStreamingReasoning) {
		emit AnswerStream(QStringLiteral("---REASONING_END---"));
		m_isStreamingReasoning = false;
	}
	m_NetWorkParams->rawBuffer.clear();
	emit StreamEnded();
}

void Open_WebUIClient::getAnswer()
{

	if (m_NetWorkParams->clientNetWorkReply->error() == QNetworkReply::NoError)
	{
		QByteArray read_data = m_NetWorkParams->clientNetWorkReply->readAll();
		QJsonObject Content = parseJsonReplyToMsg(read_data);
		QString textresponse;
		bool isFunctionCall = Content.contains("tool_calls");
		if (!isFunctionCall)
		{
			const QString reasoning = Content.value("reasoning_content").toString();
			const QString content = Content.value("content").toString();
			const bool shouldShowReasoning = m_LLMParams->getOpenThink() && !reasoning.isEmpty();

			if (!content.isEmpty() || !reasoning.isEmpty())
			{
				if (shouldShowReasoning)
				{
					textresponse = reasoning.trimmed() +
						QStringLiteral("\n\n---REASONING_END---\n\n") + content;
				}
				else
				{
					textresponse = content;
				}
				emit Answer(textresponse, false);
			}
			else
			{
				emit Answer("Invalid response format", false);
			}
		}
		else
		{
			emit FunctionCallSignal(Content);

		}
	}
	else
	{
		emit Answer(GetError(m_NetWorkParams->clientNetWorkReply->errorString(), m_NetWorkParams->clientNetWorkReply->readAll()), true);
	}
	m_NetWorkParams->clientNetWorkReply.reset();
	ChangeButtonStatus(true);
}

void Open_WebUIClient::getStreamAnswer()
{
	if (m_NetWorkParams->clientNetWorkReply->error() == QNetworkReply::NoError)
	{
		QByteArray response_data = m_NetWorkParams->clientNetWorkReply->readAll();
		m_NetWorkParams->rawBuffer.append(response_data);
		QByteArray tmp_data = QByteArray(response_data);
		int pos = response_data.indexOf("\n\n");
		while (pos != -1)
		{
			QByteArray chunk = tmp_data.left(pos);
			if (chunk.startsWith("data: ")) {
				chunk = chunk.mid(6);
			}
			const QString trimmed = QString::fromUtf8(chunk).trimmed();
			if (trimmed.isEmpty()) {
				if (pos + 2 >= tmp_data.size()) break;
				tmp_data = tmp_data.mid(pos + 2);
				pos = tmp_data.indexOf("\n\n");
				continue;
			}
			if (trimmed == "[DONE]") {
				if (pos + 2 >= tmp_data.size()) break;
				tmp_data = tmp_data.mid(pos + 2);
				pos = tmp_data.indexOf("\n\n");
				continue;
			}

			QJsonObject deltaObj = parseJsonReplyToMsg(chunk);
			const QString reasoningChunk = deltaObj.value("reasoning_content").toString();
			const QString contentChunk = deltaObj.value("content").toString();
			const bool shouldShowReasoning = m_LLMParams->getOpenThink();

			if (!reasoningChunk.isEmpty() && shouldShowReasoning) {
				if (!m_isStreamingReasoning) {
					emit AnswerStream(QStringLiteral("---REASONING_START---") + reasoningChunk);
					m_isStreamingReasoning = true;
				}
				else {
					emit AnswerStream(reasoningChunk);
				}
			}

			if (!contentChunk.isEmpty()) {
				if (m_isStreamingReasoning && shouldShowReasoning) {
					emit AnswerStream(QStringLiteral("---REASONING_END---") + contentChunk);
					m_isStreamingReasoning = false;
				}
				else {
					emit AnswerStream(contentChunk);
				}
			}

			if (pos + 2 >= tmp_data.size()) break;
			tmp_data = tmp_data.mid(pos + 2);
			pos = tmp_data.indexOf("\n\n");
		}
	}
}

QNetworkRequest Open_WebUIClient::createApiRequest(const QUrl& url)
{
	QNetworkRequest request(url);

	// 设置通用请求头
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setHeader(QNetworkRequest::UserAgentHeader, "OpenWebUIClient/1.0");

	// 配置SSL
	QSslConfiguration config = QSslConfiguration::defaultConfiguration();
	config.setProtocol(QSsl::AnyProtocol);
	config.setPeerVerifyMode(QSslSocket::VerifyNone);
	request.setSslConfiguration(config);

	// 复制认证信息
	QVariant authHeader = m_NetWorkParams->clientRequest.rawHeader("Authorization");
	if (authHeader.isValid())
	{
		request.setRawHeader("Authorization", authHeader.toByteArray());
	}

	return request;
}

void Open_WebUIClient::sendApiRequest(const QNetworkRequest& request, QTimer* timeoutTimer, int timeoutMs)
{
	m_NetWorkParams->clientNetWorkReply = std::unique_ptr<QNetworkReply>(
		m_NetWorkParams->clientNetWorkManager->get(request));

	// 根据请求类型连接相应的处理函数
	if (m_currentRequestType == RequestType::FetchModels)
	{
		connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &Open_WebUIClient::onFetchModelsFinished);
		// 启动超时定时器
		timeoutTimer->start(timeoutMs);
	}
	else if (m_currentRequestType == RequestType::ConnectionCheck)
	{
		connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &Open_WebUIClient::onCheckConnectionFinished);
		// 启动超时定时器
		timeoutTimer->start(timeoutMs);
	}
	else if (m_currentRequestType == RequestType::GetKonwledgeBase)
	{
		connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &Open_WebUIClient::onGetKnowledgeBaseFinished);
	}
}

QStringList Open_WebUIClient::parseModelIds(const QByteArray &jsonData)
{
	QStringList modelIds;

	QJsonParseError parseError;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

	if (parseError.error != QJsonParseError::NoError)
	{
		return modelIds;
	}

	QJsonObject jsonObj = jsonDoc.object();
	QJsonArray modelsArray;

	// 检查不同的JSON结构
	if (jsonObj.contains("data"))
	{
		modelsArray = jsonObj["data"].toArray();
	}
	else if (jsonObj.contains("models"))
	{
		modelsArray = jsonObj["models"].toArray();
	}
	else if (jsonDoc.isArray())
	{
		modelsArray = jsonDoc.array();
	}

	for (const QJsonValue &modelValue : modelsArray)
	{
		if (modelValue.isObject())
		{
			QJsonObject modelObj = modelValue.toObject();
			if (modelObj.contains("id"))
			{
				QString modelId = modelObj["id"].toString();
				if (!modelId.isEmpty())
				{
					modelIds.append(modelId);
				}
			}
		}
	}
	return modelIds;
}

void Open_WebUIClient::checkServerConnectionAsync(int timeoutMs)
{
	// 取消之前的请求
	cancelCurrentRequest();

	// 设置请求类型
	m_currentRequestType = RequestType::ConnectionCheck;

	// 验证URL
	if (!validateServerUrl())
	{
		emit serverConnectionCheckFinished(false, tr("URL Error"));
		return;
	}

	// 构建请求并发送
	QUrl testUrl = buildApiUrl("/api/models");
	QNetworkRequest testRequest = createApiRequest(testUrl);
	sendApiRequest(testRequest, m_connectionCheckTimer, timeoutMs);
}

void Open_WebUIClient::onCheckConnectionFinished()
{
	if (!m_NetWorkParams->clientNetWorkReply) {
		return;
	}

	// 检查是否超时
	if (!m_connectionCheckTimer->isActive()) {
		m_NetWorkParams->clientNetWorkReply.reset();
		return;
	}

	m_connectionCheckTimer->stop();

	bool isConnected = false;
	QString errorMessage;

	if (m_NetWorkParams->clientNetWorkReply->error() == QNetworkReply::NoError)
	{
		int statusCode = m_NetWorkParams->clientNetWorkReply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (statusCode == 200)
		{
			isConnected = true;
			errorMessage = "Connection successful";
		}
		else
		{
			errorMessage = QString("HTTP Error: %1").arg(statusCode);
		}
	}
	else
	{
		errorMessage = m_NetWorkParams->clientNetWorkReply->errorString();
	}

	m_NetWorkParams->clientNetWorkReply.reset();
	emit serverConnectionCheckFinished(isConnected, errorMessage);
}

void Open_WebUIClient::fetchModelsAsync(int timeoutMs)
{
	// 取消之前的请求
	cancelCurrentRequest();

	// 设置请求类型
	m_currentRequestType = RequestType::FetchModels;

	// 验证URL
	if (!validateServerUrl())
	{
		emit modelsListFetched(false, QStringList(), tr("URL Error"));
		return;
	}

	// 构建请求并发送
	QUrl modelsUrl = buildApiUrl("/api/models");
	QNetworkRequest modelsRequest = createApiRequest(modelsUrl);
	sendApiRequest(modelsRequest, m_fetchModelsTimer, timeoutMs);
}

void Open_WebUIClient::onFetchModelsFinished()
{
	if (!m_NetWorkParams->clientNetWorkReply) {
		return;
	}

	// 检查是否超时
	if (!m_fetchModelsTimer->isActive()) {
		m_NetWorkParams->clientNetWorkReply.reset();
		return;
	}

	m_fetchModelsTimer->stop();

	bool success = false;
	QString errorMessage;
	QStringList models;

	if (m_NetWorkParams->clientNetWorkReply->error() == QNetworkReply::NoError)
	{
		int statusCode = m_NetWorkParams->clientNetWorkReply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (statusCode == 200)
		{
			QByteArray responseData = m_NetWorkParams->clientNetWorkReply->readAll();
			models = parseModelIds(responseData);

			if (!models.isEmpty())
			{
				m_availableModelIds = models;
				success = true;
				errorMessage = QString("Successfully fetched %1 models").arg(models.size());
				// 发送模型更新信号
				emit modelsListFetched(success, m_availableModelIds, errorMessage);
			}
			else
			{
				errorMessage = "No models found in response";
			}
		}
		else
		{
			errorMessage = QString("HTTP Error: %1").arg(statusCode);
		}
	}
	else
	{
		errorMessage = m_NetWorkParams->clientNetWorkReply->errorString();
	}

	m_NetWorkParams->clientNetWorkReply.reset();
	emit modelsListFetched(success, models, errorMessage);
}

void Open_WebUIClient::onCheckConnectionTimeout()
{
	if (m_NetWorkParams->clientNetWorkReply)
	{
		disconnect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &Open_WebUIClient::onCheckConnectionFinished);

		m_NetWorkParams->clientNetWorkReply->abort();
		m_NetWorkParams->clientNetWorkReply.reset();
	}
	emit serverConnectionCheckFinished(false, tr("Please Check IP Address or Network Setting"));
}

void Open_WebUIClient::onFetchModelsTimeout()
{
	if (m_NetWorkParams->clientNetWorkReply)
	{
		disconnect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &Open_WebUIClient::onFetchModelsFinished);

		m_NetWorkParams->clientNetWorkReply->abort();
		m_NetWorkParams->clientNetWorkReply.reset();
	}
	emit modelsListFetched(false, QStringList(), tr("Fetch ModelList Timeout"));
}

QStringList Open_WebUIClient::GetFollowUpSuggestions()
{
	//to be continue
	return QStringList();
}

void Open_WebUIClient::getKnowledgeBase()
{
	// 取消之前的请求
	if (KnowledgeInfo.size() > 0)
		return;
	cancelCurrentRequest();

	// 设置请求类型
	m_currentRequestType = RequestType::GetKonwledgeBase;

	// 验证URL
	if (!validateServerUrl())
		return;

	// 构建请求并发送
	QUrl modelsUrl = buildApiUrl("/api/v1/knowledge/");
	QNetworkRequest modelsRequest = createApiRequest(modelsUrl);
	sendApiRequest(modelsRequest, m_fetchModelsTimer, 50000);
}

void Open_WebUIClient::onGetKnowledgeBaseFinished()
{
	if (!m_NetWorkParams->clientNetWorkReply)
		return;

	bool success = false;
	QString errorMessage;

	if (m_NetWorkParams->clientNetWorkReply->error() == QNetworkReply::NoError)
	{
		int statusCode = m_NetWorkParams->clientNetWorkReply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (statusCode == 200)
		{
			QByteArray responseData = m_NetWorkParams->clientNetWorkReply->readAll();
			QJsonParseError error;
			QJsonDocument doc = QJsonDocument::fromJson(responseData, &error);

			if (error.error != QJsonParseError::NoError || !doc.isArray())
				return;

			QJsonArray jsonArray = doc.array();
			std::map<QString, std::pair<QString, QString>> knowBase;
			for (const QJsonValue& value : jsonArray)
			{
				if (!value.isObject())
					continue;
				QJsonObject obj = value.toObject();

				// 检查必需字段是否存在
				if (!obj.contains("id") || !obj.contains("name") || !obj.contains("description"))
					continue;

				KnowledgeBase kb;
				kb.KnowledgeID = obj["id"].toString();
				kb.KnowledgeName = obj["name"].toString();
				kb.KnowledgeDescription = obj["description"].toString();
				knowBase.insert(std::make_pair<QString, std::pair<QString, QString>>(obj["id"].toString(), std::make_pair<QString, QString>(obj["name"].toString(), obj["description"].toString())));
				// 验证字段不为空
				if (kb.KnowledgeID.isEmpty() || kb.KnowledgeName.isEmpty())
					continue;
				KnowledgeInfo.push_back(kb);
			}
			KonwledgeBaseSignal(knowBase);
		}
		else
		{
			errorMessage = QString("HTTP Error: %1").arg(statusCode);
		}
	}
	else
	{
		errorMessage = m_NetWorkParams->clientNetWorkReply->errorString();
	}
	m_NetWorkParams->clientNetWorkReply.reset();
}

void Open_WebUIClient::uploadFile(const QString& filePath)
{
	// 取消之前的请求
	cancelCurrentRequest();

	// 设置请求类型
	m_currentRequestType = RequestType::FileUpload;

	// 验证URL
	if (!validateServerUrl())
		return;
	QFileInfo fileInfo(filePath);
	QFile* file = new QFile(filePath);
	if (!file->open(QIODevice::ReadOnly))
	{
		delete file;
		return;
	}

	// 构建文件上传URL
	QUrl uploadUrl = buildApiUrl("/api/v1/files/");
	QNetworkRequest uploadRequest(uploadUrl);

	// 配置SSL
	QSslConfiguration config = QSslConfiguration::defaultConfiguration();
	config.setProtocol(QSsl::AnyProtocol);
	config.setPeerVerifyMode(QSslSocket::VerifyNone);
	uploadRequest.setSslConfiguration(config);

	// 设置Bearer token认证
	QVariant authHeader = m_NetWorkParams->clientRequest.rawHeader("Authorization");
	if (authHeader.isValid())
	{
		uploadRequest.setRawHeader("Authorization", authHeader.toByteArray());
	}
	// 创建multipart/form-data
	QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

	// 添加文件部分
	QMimeDatabase mimeDb;

	// 先尝试根据数据内容检测
	QMimeType mimeType = mimeDb.mimeTypeForFile(filePath);
	QString filenametype = mimeType.name();
	QHttpPart filePart;
	filePart.setHeader(QNetworkRequest::ContentTypeHeader, filenametype.toUtf8());
	filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
		QString("form-data; name=\"file\"; filename=\"%1\"")
		.arg(QFileInfo(filePath).fileName()));

	filePart.setBodyDevice(file);
	file->setParent(multiPart); // 确保文件对象的生命周期由multiPart管理

	multiPart->append(filePart);

	// 发送POST请求
	m_NetWorkParams->clientNetWorkReply = std::unique_ptr<QNetworkReply>(
		m_NetWorkParams->clientNetWorkManager->post(uploadRequest, multiPart));

	// multiPart会在reply删除时自动删除
	multiPart->setParent(m_NetWorkParams->clientNetWorkReply.get());

	// 连接信号
	connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
		this, &Open_WebUIClient::onFileUploadFinished);
}

void Open_WebUIClient::DeleteFile(const QString& fileID)
{
	// 取消之前的请求
	cancelCurrentRequest();

	// 设置请求类型
	m_currentRequestType = RequestType::FileDelete;

	// 验证URL
	if (!validateServerUrl())
		return;
	// 构建文件删除URL
	QUrl DeleteFileUrl = buildApiUrl("/api/v1/files/" + fileID);
	QNetworkRequest DeleteFileRequest(DeleteFileUrl);

	// 配置SSL
	QSslConfiguration config = QSslConfiguration::defaultConfiguration();
	config.setProtocol(QSsl::AnyProtocol);
	config.setPeerVerifyMode(QSslSocket::VerifyNone);
	DeleteFileRequest.setSslConfiguration(config);

	// 设置Bearer token认证
	QVariant authHeader = m_NetWorkParams->clientRequest.rawHeader("Authorization");
	if (authHeader.isValid())
	{
		DeleteFileRequest.setRawHeader("Authorization", authHeader.toByteArray());
	}
	// 发送POST请求
	m_NetWorkParams->clientNetWorkReply = std::unique_ptr<QNetworkReply>(
		m_NetWorkParams->clientNetWorkManager->deleteResource(DeleteFileRequest));

	// 连接信号
	connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
		this, &Open_WebUIClient::onDeleteFileFinished);
}

void Open_WebUIClient::onFileUploadFinished()
{
	if (!m_NetWorkParams->clientNetWorkReply)
		return;
	bool success = false;
	QString errorMessage;
	QString fileId;
	QString fileName;

	if (m_NetWorkParams->clientNetWorkReply->error() == QNetworkReply::NoError)
	{
		int statusCode = m_NetWorkParams->clientNetWorkReply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (statusCode == 200 || statusCode == 201)
		{
			QByteArray responseData = m_NetWorkParams->clientNetWorkReply->readAll();
			QJsonParseError error;
			QJsonDocument doc = QJsonDocument::fromJson(responseData, &error);

			if (error.error == QJsonParseError::NoError && doc.isObject())
			{
				QJsonObject obj = doc.object();

				// 解析Dify文件上传响应
				if (obj.contains("id"))
				{
					fileId = obj["id"].toString();
					fileName = obj["filename"].toString();
					m_UpFiles.insert(fileName, fileId);
					success = true;
					errorMessage = "文件上传成功";
				}
				else
				{
					errorMessage = "响应中未找到文件ID";
				}
			}
			else
			{
				errorMessage = "响应解析失败: " + error.errorString();
			}
		}
		else
		{
			QByteArray responseData = m_NetWorkParams->clientNetWorkReply->readAll();
			QJsonDocument doc = QJsonDocument::fromJson(responseData);
			if (doc.isObject())
			{
				QJsonObject obj = doc.object();
				if (obj.contains("message"))
				{
					errorMessage = QString("HTTP %1: %2").arg(statusCode).arg(obj["message"].toString());
				}
				else
				{
					errorMessage = QString("HTTP错误: %1").arg(statusCode);
				}
			}
			else
			{
				errorMessage = QString("HTTP错误: %1").arg(statusCode);
			}
		}
	}
	else
	{
		errorMessage = GetError(m_NetWorkParams->clientNetWorkReply->errorString(),
			m_NetWorkParams->clientNetWorkReply->readAll());
	}

	m_NetWorkParams->clientNetWorkReply.reset();
}

void Open_WebUIClient::onDeleteFileFinished()
{
	if (!m_NetWorkParams->clientNetWorkReply)
		return;
	bool success = false;
	QString errorMessage;
	if (m_NetWorkParams->clientNetWorkReply->error() == QNetworkReply::NoError)
	{
		int statusCode = m_NetWorkParams->clientNetWorkReply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (statusCode == 200 || statusCode == 201)
		{
			QByteArray responseData = m_NetWorkParams->clientNetWorkReply->readAll();
			QJsonParseError error;
			QJsonDocument doc = QJsonDocument::fromJson(responseData, &error);

			if (error.error == QJsonParseError::NoError && doc.isObject())
			{
				QJsonObject obj = doc.object();
				errorMessage = obj["message"].toString();
				qDebug() << errorMessage;
			}
			else
			{
				errorMessage = "响应解析失败: " + error.errorString();
			}
		}
	}
	else
	{
		errorMessage = GetError(m_NetWorkParams->clientNetWorkReply->errorString(),
			m_NetWorkParams->clientNetWorkReply->readAll());
	}

	m_NetWorkParams->clientNetWorkReply.reset();
}

void Open_WebUIClient::AnalysisBlockResponse(QJsonObject& response_obj)
{
	QString textresponse;
	bool isFunctionCall = response_obj.contains("tool_calls");
	if (!isFunctionCall)
	{
		QJsonValue value;
		value = response_obj["content"].toString();
		if (value.isString())
		{
			textresponse = value.toString();
			emit Answer(textresponse, false);
		}
		else
		{
			emit Answer("Invalid response format", false);
		}
	}
	else
	{
		emit FunctionCallSignal(response_obj);

	}
}

void Open_WebUIClient::AnalysisStreamResponse(QJsonObject& response_obj)
{

}

void Open_WebUIClient::ChangeKnowledgeGraph(const QString& kownledgeID)
{
	if (addKnowledge.find(kownledgeID) == addKnowledge.end())
		addKnowledge.insert(kownledgeID);
	else
		addKnowledge.erase(kownledgeID);
}

void Open_WebUIClient::CancelUpdateFile(const QString& file)
{
	QFileInfo fileInfo(file);
	QString fileName = fileInfo.fileName();
	auto it = m_UpFiles.find(fileName);
	QString fileID = it.value();
	if (it != m_UpFiles.end())
	{
		m_UpFiles.erase(it);
		qDebug() << "Delete File" + fileName;
	}
	else
	{
		qDebug() << "NO File";
	}
	DeleteFile(fileID);
}

void Open_WebUIClient::CancelAllUpdateFiles(const QStringList& fileList)
{
	for (auto file : fileList)
	{
		CancelUpdateFile(file);
	}
}