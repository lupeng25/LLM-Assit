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
	if (msg.fileContext.size() > 0)
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
	if (m_LLMParams->getStreamChat())
	{
		SendMessageBody["response_mode"] = "streaming";
	}
	else
	{
		SendMessageBody["response_mode"] = "blocking";
	}

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
	if (msg.Image64.size() > 0)
	{
		QJsonArray filesArray;
		for (const QString &base64Str : msg.Image64)
		{
			QJsonObject fileObj;
			fileObj["type"] = "image";
			fileObj["transfer_method"] = "local_file"; // 或 "remote_url"
			//
			//这是进行上传文件操作并获取文件ID
			//
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
	{
		return QJsonObject();
	}

	QJsonObject rsp_json = response_doc.object();

	// Dify API直接返回完整的响应对象
	// 不需要像OpenWebUI那样提取choices数组
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
	connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished, this, &DifyClient::AnalysisStreamAnswer);
	return true;
}

QString DifyClient::GetError(const QString& errorLevel, const QString& errorContext)
{
	ChangeButtonStatus(true);

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
		return QStringLiteral("服务器地址错误");
	}

	// 解析HTTP错误
	QRegularExpression regex(R"(server replied:\s*(\w+(?:\s\w+)*))");
	QRegularExpressionMatch match = regex.match(errorLevel);
	QString errorCode = match.hasMatch() ? match.captured(1) : QString();

	if (errorCode == "Bad Request")
	{
		QJsonDocument doc = QJsonDocument::fromJson(errorContext.toUtf8());
		if (doc.isObject())
		{
			QJsonObject obj = doc.object();
			if (obj.contains("message"))
			{
				QString errorMsg = obj["message"].toString();
				return QStringLiteral("请求错误: ") + errorMsg;
			}
		}
		return QStringLiteral("请检查输入参数");
	}
	else if (errorCode == "Unauthorized")
	{
		return QStringLiteral("API密钥错误或已过期");
	}
	else if (errorCode == "Forbidden")
	{
		return QStringLiteral("访问被禁止，请检查API权限");
	}
	else if (errorCode == "Too Many Requests")
	{
		return QStringLiteral("请求过于频繁，请稍后重试");
	}

	return QStringLiteral("未知错误: ") + errorLevel;
}

void DifyClient::AnalysisAnswer(const QString& word, bool bError)
{
	// Dify不需要特殊的thinking处理，直接返回答案
	QString TextAnswer = word;
	if (bError)
	{
		TextAnswer = QStringLiteral("错误: ") + word;
	}
}

void DifyClient::AnalysisStreamAnswer()
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
	m_NetWorkParams->clientNetWorkReply = std::unique_ptr<QNetworkReply>(
		m_NetWorkParams->clientNetWorkManager->get(request));

	// 根据请求类型连接相应的处理函数
	if (m_currentRequestType == RequestType::FetchModels)
	{
		connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &DifyClient::onFetchModelsFinished);
	}
	else if (m_currentRequestType == RequestType::ConnectionCheck)
	{
		connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &DifyClient::onCheckConnectionFinished);
	}
	else if (m_currentRequestType == RequestType::FollowUpSuggest)
	{
		connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
			this, &DifyClient::onGetFollowUpSuggestFinished);
	}

	// 启动超时定时器
	timeoutTimer->start(timeoutMs);
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
		emit serverConnectionCheckFinished(false, QStringLiteral("URL错误"));
		return;
	}

	// Dify连接测试使用基础API端点
	QUrl testUrl = buildApiUrl("/v1/info");
	QNetworkRequest testRequest = createApiRequest(testUrl);
	sendApiRequest(testRequest, m_connectionCheckTimer, timeoutMs);
}

// Dify不直接提供模型列表API，通常使用固定的应用配置
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

// 简化后的获取模型列表函数 - Dify通常不提供模型列表
void DifyClient::fetchModelsAsync(int timeoutMs)
{
	// Dify不提供模型列表API，直接返回成功
	QStringList defaultApps;
	defaultApps << "Chat Application" << "Text Generator" << "Workflow" << "Agent";
	m_availableModelIds = defaultApps;

	emit modelsListFetched(true, m_availableModelIds, "Using default Dify application types");
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

		QString textresponse;

		// 解析Dify的blocking响应格式
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
	else
	{
		emit Answer(GetError(m_NetWorkParams->clientNetWorkReply->errorString(),
			m_NetWorkParams->clientNetWorkReply->readAll()), true);
	}
	m_NetWorkParams->clientNetWorkReply.reset();
	ChangeButtonStatus(true);
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

					// 处理不同的事件类型
					QString event = obj["event"].toString();

					if (event == "message")
					{
						QString answer = obj["answer"].toString();
						if (!answer.isEmpty())
						{
							emit AnswerStream(answer);
						}

						// 保存conversation_id
						if (obj.contains("conversation_id"))
						{
							m_conversationId = obj["conversation_id"].toString();
						}
					}
					else if (event == "message_end")
					{
						// 消息结束，可以获取usage信息
						if (obj.contains("metadata"))
						{
							QJsonObject metadata = obj["metadata"].toObject();
							if (metadata.contains("usage"))
							{
								QJsonObject usage = metadata["usage"].toObject();
								int totalTokens = usage["total_tokens"].toInt();
							}
						}
					}
					else if (event == "error")
					{
						QString errorMsg = obj["message"].toString();
						emit Answer("Stream error: " + errorMsg, true);
						return;
					}
				}
			}
		}
	}
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
	emit serverConnectionCheckFinished(false, QStringLiteral("请检测Dify服务器地址或网络设置"));
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
	QUrl suggestUrl = buildApiUrl(endpoint,m_userId);
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
				return ;
			// 检查是否为对象
			if (!doc.isObject()) 
				return ;
			QJsonObject obj = doc.object();

			// 检查是否包含data字段
			if (!obj.contains("data")) 
				return ;

			// 获取data字段的值
			QJsonValue dataValue = obj.value("data");

			// 检查data是否为数组
			if (!dataValue.isArray()) 
				return ;

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
