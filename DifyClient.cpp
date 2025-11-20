#include "DifyClient.h"

DifyClient::DifyClient(QObject *parent)
	: MessageManager(parent)
{
	m_LLMParams = new LLMParams();
	m_NetWorkParams = new ClientNetWork();

	// 初始化连接测试定时器
	m_connectionCheckTimer = new QTimer(this);
	m_connectionCheckTimer->setSingleShot(true);
	connect(m_connectionCheckTimer, &QTimer::timeout, this, &DifyClient::onCheckConnectionTimeout);

	// 初始化获取模型列表定时器
	m_fetchModelsTimer = new QTimer(this);
	m_fetchModelsTimer->setSingleShot(true);
	connect(m_fetchModelsTimer, &QTimer::timeout, this, &DifyClient::onFetchModelsTimeout);

	m_currentRequestType = RequestType::ConnectionCheck;
}

DifyClient::~DifyClient()
{
}

AIProvider DifyClient::getProvider() const
{
	return AIProvider::Dify;
}

QString DifyClient::getProviderName() const
{
	return "Dify";
}

QString DifyClient::getVersion() const
{
	return "1.0";
}

void DifyClient::setConversationId(const QString& conversationId)
{
	m_conversationId = conversationId;
}

void DifyClient::setMessageId(const QString& messageId)
{
	m_messageId = messageId;
}

void DifyClient::setTaskId(const QString& taskId)
{
	m_taskId = taskId;
}

void DifyClient::setUserId(const QString& userId)
{
	m_userId = userId;
}

QString DifyClient::getConversationId() const
{
	return m_conversationId;
}

QString DifyClient::getUserId() const
{
	return m_userId;
}

QString DifyClient::getTaskId() const
{
	return m_taskId;
}

QString DifyClient::getMessageId() const
{
	return m_messageId;
}

void DifyClient::resetConversationId()
{
	m_conversationId.clear();
}

void DifyClient::resetUserId()
{
	m_userId.clear();
}

void DifyClient::resetTaskId()
{
	m_taskId.clear();
}

void DifyClient::resetMessageId()
{
	m_messageId.clear();
}

QByteArray DifyClient::buildMessageBody(const ChatSendMessage& msg)
{
	QJsonObject SendMessageBody;

	// Dify API的基本参数
	QJsonObject inputs; // Dify的输入参数，通常用于传递变量

						// 构建query内容：输入信息+文档内容
	QString finalQuery = msg.SendText;
	if (!msg.fileContext.isEmpty())
	{
		for (int i = 0; i < msg.fileContext.size(); i++)
		{
			finalQuery += QStringLiteral("\n 文档%1内容:").arg(i) + msg.fileContext[i];
		}
	}

	// 添加思考模式控制
	finalQuery += m_LLMParams->getOpenThink() ? "\\think" : "\\no_think";

	SendMessageBody["query"] = finalQuery;
	SendMessageBody["inputs"] = inputs; // 空的inputs对象，可根据需要添加变量

										// 响应模式：流式或阻塞式
	SendMessageBody["response_mode"] = m_LLMParams->getStreamChat() ? "streaming" : "blocking";

	// 会话ID（如果有的话）
	if (!m_conversationId.isEmpty())
	{
		SendMessageBody["conversation_id"] = m_conversationId;
	}

	// 用户标识（可选）
	if (!m_userId.isEmpty())
	{
		SendMessageBody["user"] = m_userId;
	}

	// 处理图像 - Dify通常通过files参数传递文件
	if (!msg.Image64.isEmpty())
	{
		QJsonArray filesArray;
		for (const QString &base64Str : msg.Image64)
		{
			QJsonObject fileObj;
			fileObj["type"] = "image";
			fileObj["transfer_method"] = "local_file"; // 或 "remote_url"

													   //这是进行上传文件操作并获取文件ID
			fileObj["upload_file_id"] = base64Str; // 实际使用中需要先上传文件获取file_id
			filesArray.append(fileObj);
		}
		SendMessageBody["files"] = filesArray;
	}

	// Dify特有的参数
	SendMessageBody["auto_generate_name"] = true; // 自动生成对话名称

												  // 模型相关参数（如果Dify应用支持模型切换）
												  // 注意：Dify通常在应用层配置模型，API层不直接指定模型
												  // 但可以通过inputs传递模型相关的参数
	if (m_LLMParams->getMaxToken() > 0)
	{
		inputs["max_tokens"] = m_LLMParams->getMaxToken();
	}
	if (m_LLMParams->getTemperature() >= 0)
	{
		inputs["temperature"] = m_LLMParams->getTemperature();
	}

	SendMessageBody["inputs"] = inputs;

	return QJsonDocument(SendMessageBody).toJson();
}

QByteArray DifyClient::buildStopAnswerBody()
{
	QJsonObject SendMessageBody;
	SendMessageBody["user"] = m_userId;
	return QJsonDocument(SendMessageBody).toJson();
}

void DifyClient::SendPreProcess(const ChatSendMessage& msg)
{
	QByteArray postData = buildMessageBody(msg);
	QSslConfiguration config = QSslConfiguration::defaultConfiguration();
	config.setProtocol(QSsl::AnyProtocol);
	config.setPeerVerifyMode(QSslSocket::VerifyNone);
	m_NetWorkParams->clientRequest.setSslConfiguration(config);
	m_NetWorkParams->clientNetWorkReply = std::unique_ptr<QNetworkReply>(m_NetWorkParams->clientNetWorkManager->post(m_NetWorkParams->clientRequest, postData));
}

QJsonObject DifyClient::parseJsonReplyToMsg(const QByteArray &data)
{
	QJsonDocument response_doc = QJsonDocument::fromJson(data);
	if (response_doc.isNull())
		return QJsonObject();

	QJsonObject rsp_json = response_doc.object();
	return rsp_json;
}

int DifyClient::send(const ChatSendMessage& msg)
{
	SendPreProcess(msg);
	connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished, this, &DifyClient::getAnswer);
	return true;
}

int DifyClient::StreamSend(const ChatSendMessage& msg)
{
	SendPreProcess(msg);
	connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::readyRead, this, &DifyClient::getStreamAnswer, Qt::QueuedConnection);
	connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished, this, &DifyClient::processStreamEnded);
	return true;
}

QString DifyClient::GetError(const QString& errorLevel, const QString& errorContext)
{
	ChangeButtonStatus(ChatInputWidget::SendButtonState::Ready);

	static const QMap<QString, QString> contextErrorMap = {
		{ "Connection timed out", QStringLiteral("连接请求超时") },
		{ "Permission denied", QStringLiteral("权限不足") },
		{ "Connection refused", QStringLiteral("服务器拒绝连接") }
	};

	if (contextErrorMap.contains(errorLevel))
	{
		return contextErrorMap.value(errorLevel);
	}
	else if (errorLevel.contains("not found"))
	{
		return tr("IP Address Error");
	}

	// 解析HTTP错误
	QString errorCode = MessageManager::extractHttpErrorCode(errorLevel);

	if (errorCode == "Bad Request")
	{
		const QString errorMsg = MessageManager::extractJsonField(errorContext, QStringLiteral("message"));
		if (!errorMsg.isEmpty())
		{
			return tr("Request Error: ") + errorMsg;
		}
		return tr("Please Check Input Setting");
	}
	else if (errorCode == "Unauthorized")
	{
		return tr("API Key Error");
	}
	else if (errorCode == "Forbidden")
	{
		return tr("Forbidden");
	}
	else if (errorCode == "Too Many Requests")
	{
		return tr("Too Many Requests,Please retry after mintues");
	}

	return tr("Unkonw Error") + errorLevel;
}

void DifyClient::processStreamEnded()
{
	if (m_NetWorkParams->clientNetWorkReply->error())
	{
		QString errorMsg = GetError(m_NetWorkParams->clientNetWorkReply->errorString(),
			m_NetWorkParams->clientNetWorkReply->readAll());
		emit Answer(errorMsg, true);
		return;
	}
	m_NetWorkParams->rawBuffer.clear();
	emit StreamEnded();
}

QNetworkRequest DifyClient::createApiRequest(const QUrl& url)
{
	QNetworkRequest request(url);

	// 设置通用请求头
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setHeader(QNetworkRequest::UserAgentHeader, "DifyClient/1.0");

	// 配置SSL
	QSslConfiguration config = QSslConfiguration::defaultConfiguration();
	config.setProtocol(QSsl::AnyProtocol);
	config.setPeerVerifyMode(QSslSocket::VerifyNone);
	request.setSslConfiguration(config);

	// 设置Bearer token认证
	QVariant authHeader = m_NetWorkParams->clientRequest.rawHeader("Authorization");
	if (authHeader.isValid())
	{
		request.setRawHeader("Authorization", authHeader.toByteArray());
	}

	return request;
}

void DifyClient::sendApiRequest(const QNetworkRequest& request, QTimer* timeoutTimer, int timeoutMs)
{
	if (m_currentRequestType == RequestType::StopStreamAns)
	{
		QByteArray postData = buildStopAnswerBody();
		m_NetWorkParams->clientNetWorkReply = std::unique_ptr<QNetworkReply>(
			m_NetWorkParams->clientNetWorkManager->post(request, postData));
		connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &DifyClient::onStopStreamAnsFinished);
		return;
	}
	m_NetWorkParams->clientNetWorkReply = std::unique_ptr<QNetworkReply>(
		m_NetWorkParams->clientNetWorkManager->get(request));

	// 根据请求类型连接相应的处理函数
	if (m_currentRequestType == RequestType::FetchModels)
	{
		connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &DifyClient::onFetchModelsFinished);
		// 启动超时定时器
		timeoutTimer->start(timeoutMs);
	}
	else if (m_currentRequestType == RequestType::ConnectionCheck)
	{
		connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &DifyClient::onCheckConnectionFinished);
		// 启动超时定时器
		timeoutTimer->start(timeoutMs);
	}
	else if (m_currentRequestType == RequestType::FollowUpSuggest)
	{
		connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &DifyClient::onGetFollowUpSuggestFinished);
	}
	else if (m_currentRequestType == RequestType::GetKonwledgeBase)
	{
		connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &DifyClient::onGetKnowledgeBaseFinished);
	}
}

QStringList DifyClient::parseModelIds(const QByteArray &jsonData)
{
	QStringList modelIds;

	// Dify通常不返回模型列表，而是使用预配置的应用
	// 这里返回一个默认列表或者解析应用列表
	QJsonParseError parseError;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

	if (parseError.error != QJsonParseError::NoError) {
		// 返回默认的Dify应用类型
		modelIds << "Chat Application" << "Text Generator" << "Agent";
		return modelIds;
	}

	QJsonObject jsonObj = jsonDoc.object();

	// 如果是应用列表响应
	if (jsonObj.contains("data"))
	{
		QJsonArray appsArray = jsonObj["data"].toArray();
		for (const QJsonValue &appValue : appsArray)
		{
			if (appValue.isObject())
			{
				QJsonObject appObj = appValue.toObject();
				if (appObj.contains("name"))
				{
					QString appName = appObj["name"].toString();
					if (!appName.isEmpty())
					{
						modelIds.append(appName);
					}
				}
			}
		}
	}

	// 如果没有找到应用，返回默认列表
	if (modelIds.isEmpty())
	{
		modelIds << "Default Chat App" << "Custom Application";
	}
	return modelIds;
}

QUrl DifyClient::buildApiUrl(const QString& endpoint)
{
	QUrl originalUrl = m_NetWorkParams->clientRequest.url();
	QUrl apiUrl;
	apiUrl.setScheme(originalUrl.scheme());
	apiUrl.setHost(originalUrl.host());
	apiUrl.setPath(endpoint);
	qDebug() << apiUrl.toString();
	return apiUrl;
}

QUrl DifyClient::buildApiUrl(const QString& apiPath, const QString& User)
{
	QUrl originalUrl = m_NetWorkParams->clientRequest.url();
	QUrl apiUrl;
	apiUrl.setScheme(originalUrl.scheme());
	apiUrl.setHost(originalUrl.host());
	apiUrl.setPath(apiPath);
	apiUrl.setQuery("user=" + User);
	qDebug() << apiUrl.toString();
	return apiUrl;
}

void DifyClient::getAnswer()
{
	if (m_NetWorkParams->clientNetWorkReply->error() == QNetworkReply::NoError)
	{
		QByteArray read_data = m_NetWorkParams->clientNetWorkReply->readAll();
		QJsonDocument response_doc = QJsonDocument::fromJson(read_data);
		QJsonObject response_obj = response_doc.object();

		AnalysisBlockResponse(response_obj);
	}
	else
	{
		emit Answer(GetError(m_NetWorkParams->clientNetWorkReply->errorString(),
			m_NetWorkParams->clientNetWorkReply->readAll()), true);
	}
	m_NetWorkParams->clientNetWorkReply.reset();
	ChangeButtonStatus(ChatInputWidget::SendButtonState::Ready);
}

void DifyClient::AnalysisBlockResponse(QJsonObject& response_obj)
{
	QString textresponse;

	// 解析Dify的响应格式
	if (response_obj.contains("answer"))
	{
		textresponse = response_obj["answer"].toString();

		// 保存conversation_id用于后续对话
		if (response_obj.contains("conversation_id"))
		{
			m_conversationId = response_obj["conversation_id"].toString();
		}
		if (response_obj.contains("message_id"))
		{
			m_messageId = response_obj["message_id"].toString();
		}
		if (response_obj.contains("task_id"))
		{
			m_taskId = response_obj["task_id"].toString();
		}

		emit Answer(textresponse, false);
	}
	else if (response_obj.contains("message"))
	{
		textresponse = response_obj["message"].toString();
		emit Answer(textresponse, true);
	}
	else
	{
		emit Answer("Invalid response format", true);
	}
}

void DifyClient::getStreamAnswer()
{
	if (m_NetWorkParams->clientNetWorkReply->error() == QNetworkReply::NoError)
	{
		QByteArray response_data = m_NetWorkParams->clientNetWorkReply->readAll();
		m_NetWorkParams->rawBuffer.append(response_data);

		// Dify流式响应格式: data: {...}\n\n
		QString dataStr = QString::fromUtf8(response_data);
		QStringList lines = dataStr.split('\n');

		for (const QString& line : lines)
		{
			if (line.startsWith("data: "))
			{
				QString jsonStr = line.mid(6); // 移除"data: "前缀
				if (jsonStr.trimmed() == "[DONE]")
				{
					continue; // 跳过结束标记
				}

				QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
				if (!doc.isNull())
				{
					QJsonObject obj = doc.object();
					AnalysisStreamResponse(obj);
				}
			}
		}
	}
}

void DifyClient::AnalysisStreamResponse(QJsonObject& eventObj)
{
	// 处理不同的事件类型
	QString event = eventObj["event"].toString();

	if (event == "message")
	{
		QString answer = eventObj["answer"].toString();
		if (!answer.isEmpty())
		{
			emit AnswerStream(answer);
		}

		// 保存conversation_id
		if (eventObj.contains("conversation_id"))
		{
			m_conversationId = eventObj["conversation_id"].toString();
		}
		if (eventObj.contains("message_id"))
		{
			m_messageId = eventObj["message_id"].toString();
		}
		if (eventObj.contains("task_id"))
		{
			m_taskId = eventObj["task_id"].toString();
		}
	}
	else if (event == "message_end")
	{
		// 消息结束，可以获取usage信息
		if (eventObj.contains("metadata"))
		{
			QJsonObject metadata = eventObj["metadata"].toObject();
			if (metadata.contains("usage"))
			{
				QJsonObject usage = metadata["usage"].toObject();
				int totalTokens = usage["total_tokens"].toInt();
			}
		}
	}
	else if (event == "error")
	{
		QString errorMsg = eventObj["message"].toString();
		emit Answer("Stream error: " + errorMsg, true);
		return;
	}
}

void DifyClient::checkServerConnectionAsync(int timeoutMs)
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

	// Dify连接测试使用基础API端点
	QUrl testUrl = buildApiUrl("/v1/info");
	QNetworkRequest testRequest = createApiRequest(testUrl);
	sendApiRequest(testRequest, m_connectionCheckTimer, timeoutMs);
}

void DifyClient::onCheckConnectionFinished()
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
			errorMessage = "Dify connection successful";
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

void DifyClient::fetchModelsAsync(int timeoutMs)
{
	// Dify不提供模型列表API，直接返回成功
	QStringList defaultApps;
	defaultApps << "Chat Application" << "Text Generator" << "Workflow" << "Agent";
	m_availableModelIds = defaultApps;

	emit modelsListFetched(true, m_availableModelIds, "Using default Dify application types");
}

void DifyClient::onFetchModelsFinished()
{
	// Dify通常不需要实际获取模型列表，直接使用默认应用类型
	QStringList models;
	models << "Chat Application" << "Text Generator" << "Workflow" << "Agent";
	m_availableModelIds = models;

	emit modelsListFetched(true, m_availableModelIds, "Dify application types loaded");
}

void DifyClient::onCheckConnectionTimeout()
{
	if (m_NetWorkParams->clientNetWorkReply)
	{
		disconnect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &DifyClient::onCheckConnectionFinished);

		m_NetWorkParams->clientNetWorkReply->abort();
		m_NetWorkParams->clientNetWorkReply.reset();
	}
	emit serverConnectionCheckFinished(false, tr("Please Check IP Address or Network Setting"));
}

void DifyClient::onFetchModelsTimeout()
{
	// Dify不需要实际超时处理，直接返回默认列表
	QStringList models;
	models << "Chat Application" << "Text Generator" << "Workflow" << "Agent";
	emit modelsListFetched(true, models, "Using default Dify application types");
}

QStringList DifyClient::GetFollowUpSuggestions()
{
	// 取消之前的请求
	cancelCurrentRequest();

	// 设置请求类型
	m_currentRequestType = RequestType::FollowUpSuggest;

	if (!validateServerUrl())
	{
		return QStringList();
	}
	QString endpoint = "/v1/messages/" + m_messageId + "/" + "suggested";
	QUrl suggestUrl = buildApiUrl(endpoint, m_userId);
	QNetworkRequest suggestRequest = createApiRequest(suggestUrl);
	sendApiRequest(suggestRequest, m_fetchModelsTimer, 5000);
}

void DifyClient::onGetFollowUpSuggestFinished()
{
	if (!m_NetWorkParams->clientNetWorkReply) {
		return;
	}

	bool success = false;
	QString errorMessage;
	QStringList suggestions;

	if (m_NetWorkParams->clientNetWorkReply->error() == QNetworkReply::NoError)
	{
		int statusCode = m_NetWorkParams->clientNetWorkReply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (statusCode == 200)
		{
			QByteArray responseData = m_NetWorkParams->clientNetWorkReply->readAll();
			QJsonParseError error;
			QJsonDocument doc = QJsonDocument::fromJson(responseData, &error);

			// 检查解析是否成功
			if (error.error != QJsonParseError::NoError)
				return;
			// 检查是否为对象
			if (!doc.isObject())
				return;
			QJsonObject obj = doc.object();

			// 检查是否包含data字段
			if (!obj.contains("data"))
				return;

			// 获取data字段的值
			QJsonValue dataValue = obj.value("data");

			// 检查data是否为数组
			if (!dataValue.isArray())
				return;

			QJsonArray dataArray = dataValue.toArray();

			// 遍历数组，将字符串添加到QStringList
			for (const QJsonValue& value : dataArray)
			{
				if (value.isString())
				{
					suggestions.append(value.toString());
				}
			}
			if (!suggestions.isEmpty())
			{
				success = true;
				errorMessage = QString("Successfully fetched %1 suggestions").arg(suggestions.size());
				FollowSuggestSignal(suggestions);
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
}

void DifyClient::getKnowledgeBase()
{
	// 取消之前的请求
	cancelCurrentRequest();

	// 设置请求类型
	m_currentRequestType = RequestType::GetKonwledgeBase;

	if (!validateServerUrl())
		return;

	QUrl KnowledgeUrl = buildApiUrl("/v1/datasets");
	QNetworkRequest request(KnowledgeUrl);

	// 设置通用请求头
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setHeader(QNetworkRequest::UserAgentHeader, "DifyClient/1.0");

	// 配置SSL
	QSslConfiguration config = QSslConfiguration::defaultConfiguration();
	config.setProtocol(QSsl::AnyProtocol);
	config.setPeerVerifyMode(QSslSocket::VerifyNone);
	request.setSslConfiguration(config);

	// 设置Bearer token认证
	QVariant authHeader = m_LLMParams->getKnowledgeApi();
	if (authHeader.isValid())
	{
		request.setRawHeader("Authorization", ("Bearer " + m_LLMParams->getKnowledgeApi()).toUtf8());
	}
	sendApiRequest(request, m_fetchModelsTimer, 50000);
}

void DifyClient::onGetKnowledgeBaseFinished()
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
			QJsonObject jsonObj = doc.object();
			QJsonArray jsonArray = jsonObj.value("data").toArray();

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

				// 验证字段不为空
				if (kb.KnowledgeID.isEmpty() || kb.KnowledgeName.isEmpty())
					continue;
				KnowledgeInfo.push_back(kb);
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
}

void DifyClient::StopGenerateStreamAns()
{
	// 取消之前的请求
	cancelCurrentRequest();

	// 设置请求类型
	m_currentRequestType = RequestType::StopStreamAns;

	if (!validateServerUrl())
		return;

	QString endpoint = "/v1/messages/" + m_taskId + "/" + "stop";
	QUrl suggestUrl = buildApiUrl(endpoint, m_userId);
	QNetworkRequest suggestRequest = createApiRequest(suggestUrl);
	sendApiRequest(suggestRequest, m_fetchModelsTimer, 50000);
}

void DifyClient::onStopStreamAnsFinished()
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
			errorMessage = "success";
	}
}

void DifyClient::uploadFile(const QString& filePath)
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
	QUrl uploadUrl = buildApiUrl("/v1/files/upload");
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

	// 添加user参数（如果有的话）
	if (!m_userId.isEmpty())
	{
		QHttpPart userPart;
		userPart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"user\"");
		userPart.setBody(m_userId.toUtf8());
		multiPart->append(userPart);
	}

	// 发送POST请求
	m_NetWorkParams->clientNetWorkReply = std::unique_ptr<QNetworkReply>(
		m_NetWorkParams->clientNetWorkManager->post(uploadRequest, multiPart));

	// multiPart会在reply删除时自动删除
	multiPart->setParent(m_NetWorkParams->clientNetWorkReply.get());

	// 连接信号
	connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
		this, &DifyClient::onFileUploadFinished);
}

void DifyClient::DeleteFile(const QString& fileID)
{

}

void DifyClient::onFileUploadFinished()
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
					fileName = obj["name"].toString();
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

void DifyClient::onDeleteFileFinished()
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

void DifyClient::ChangeKnowledgeGraph(const QString& kownledgeID)
{

}

void DifyClient::CancelUpdateFile(const QString& file)
{
	QFileInfo fileInfo(file);
	QString fileName = fileInfo.fileName();
	auto it = m_UpFiles.find(fileName);
	if (it != m_UpFiles.end())
		m_UpFiles.erase(it);
}

void DifyClient::CancelAllUpdateFiles(const QStringList& fileList)
{
	for (auto file : fileList)
	{
		CancelUpdateFile(file);
	}
}