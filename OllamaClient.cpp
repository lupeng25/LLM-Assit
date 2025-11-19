#include "OllamaClient.h"

OllamaClient::OllamaClient(QObject *parent)
    : MessageManager(parent)
{
    m_LLMParams = new LLMParams();
    m_NetWorkParams = new ClientNetWork();

    // 初始化连接测试定时器
    m_connectionCheckTimer = new QTimer(this);
    m_connectionCheckTimer->setSingleShot(true);
    connect(m_connectionCheckTimer, &QTimer::timeout, this, &OllamaClient::onCheckConnectionTimeout);

    // 初始化获取模型列表定时器
    m_fetchModelsTimer = new QTimer(this);
    m_fetchModelsTimer->setSingleShot(true);
    connect(m_fetchModelsTimer, &QTimer::timeout, this, &OllamaClient::onFetchModelsTimeout);

    m_currentRequestType = RequestType::ConnectionCheck;
}

OllamaClient::~OllamaClient()
{
    if (m_NetWorkParams && m_NetWorkParams->clientNetWorkReply)
    {
        m_NetWorkParams->clientNetWorkReply->abort();
        m_NetWorkParams->clientNetWorkReply->disconnect();
        m_NetWorkParams->clientNetWorkReply.reset();
    }
    // 注意：m_LLMParams 可能已被 setLLMParams() 设置为外部管理的对象
    // setLLMParams() 已经处理了构造函数中创建�?m_LLMParams 的删�?
    // 如果 m_LLMParams 是外部传入的，不应该在这里删除，由外部管理其生命周期
    // 因此这里不删�?m_LLMParams，避免双重释�?
    delete m_NetWorkParams;
    m_LLMParams = nullptr;
    m_NetWorkParams = nullptr;
}

AIProvider OllamaClient::getProvider() const
{
    return AIProvider::Ollama;
}

QString OllamaClient::getProviderName() const
{
    return "Ollama";
}

QString OllamaClient::getVersion() const
{
    return "1.0";
}

QByteArray OllamaClient::buildMessageBody(const ChatSendMessage& msg)
{
    QJsonObject SendMessageBody;
    QJsonObject ModelSettingsBody;
    QJsonArray messagesArray;
    QJsonObject messageObject;
    const std::map<int, QString> modelMap =
    {
        { 0, "gkg" },
        { 1, "gkg32" },
        { 2, "gkgvision" },
        { 3, "Codeinter" }
    };
    messageObject["role"] = "user";//角色

                                   //发送信�?输入信息+文档i...
    QString finalSendMsg = msg.SendText;
    if (msg.fileContext.size() > 0)
    {
        for (int i = 0; i<msg.fileContext.size(); i++)
        {
            finalSendMsg += QStringLiteral("\n 文档%1内容:").arg(i) + msg.fileContext[i];
        }
    }
    //finalSendMsg += m_LLMParams->getOpenThink() ? "\\think" : "\\no_think";
    messageObject["content"] = finalSendMsg;
    //图像:目前仅支持转base64格式
    if (msg.Image64.size() > 0)
    {
        QJsonArray ImagejsonArray;
        for (const QString &base64Str : msg.Image64)
        {
            ImagejsonArray.append(base64Str);
            qDebug() << base64Str;
            std::string testc = base64Str.toStdString();
        }
        messageObject["images"] = ImagejsonArray;
    }
    messagesArray.append(messageObject);
    //模型参数设置
    SendMessageBody["model"] = m_availableModelIds.empty() ? modelMap.find(m_LLMParams->getModel())->second :
        m_availableModelIds[m_LLMParams->getModel()];
    SendMessageBody["messages"] = messagesArray;
    SendMessageBody["think"] = m_LLMParams->getOpenThink();
    ModelSettingsBody["mode"] = (m_LLMParams->getChatMode() == 0) ? "query" : "chat";
    ModelSettingsBody["max_tokens"] = m_LLMParams->getMaxToken();
    ModelSettingsBody["temperature"] = m_LLMParams->getTemperature();
    if (m_availableModelIds[m_LLMParams->getModel()] == "qwen2.5vl:32b")
    {
        ModelSettingsBody["keep_alive"] = "24h";
    }
    SendMessageBody["options"] = ModelSettingsBody;
    if (m_LLMParams->getStreamChat())
    {
        SendMessageBody.insert("stream", true);
    }
    else
    {
        SendMessageBody.insert("stream", false);
        //SendMessageBody["tools"] = m_LLMParams->getFunctionCallTools();
    }

    return QJsonDocument(SendMessageBody).toJson();
}

void OllamaClient::SendPreProcess(const ChatSendMessage& msg)
{
    QByteArray postData = buildMessageBody(msg);
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::AnyProtocol);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    m_NetWorkParams->clientRequest.setSslConfiguration(config);
    m_NetWorkParams->clientNetWorkReply = std::unique_ptr<QNetworkReply>(m_NetWorkParams->clientNetWorkManager->post(m_NetWorkParams->clientRequest, postData));
}

QJsonObject OllamaClient::parseJsonReplyToMsg(const QByteArray &data)
{
    QJsonDocument response_doc = QJsonDocument::fromJson(data);
    if (response_doc.isNull())
        return QJsonObject();

    QJsonObject rsp_json = response_doc.object();
    return rsp_json;
}

int OllamaClient::send(const ChatSendMessage& msg)
{
    m_isStreamingReasoning = false;
    SendPreProcess(msg);
    connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished, this, &OllamaClient::getAnswer);
    return true;
}

int OllamaClient::StreamSend(const ChatSendMessage& msg)
{
    m_isStreamingReasoning = false;
    SendPreProcess(msg);
    connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::readyRead, this, &OllamaClient::getStreamAnswer, Qt::QueuedConnection);
    connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished, this, &OllamaClient::processStreamEnded);
    return true;
}

QString OllamaClient::GetError(const QString& errorLevel, const QString& errorContext)
{
    ChangeButtonStatus(true);
    static const QMap<QString, QString> contextErrorMap = {
        { "Connection timed out", tr("Connection timed out") },
        { "Permission denied", tr("Permission denied") },
        { "Connection refused",tr("Connection refused") }
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

void OllamaClient::processStreamEnded()
{
    if (m_NetWorkParams->clientNetWorkReply->error())
    {
        QString errorMsg = GetError(m_NetWorkParams->clientNetWorkReply->errorString(), m_NetWorkParams->clientNetWorkReply->readAll());
        emit Answer(errorMsg, true);
        return;
    }
    if (m_isStreamingReasoning)
    {
        emit AnswerStream(QStringLiteral("</think>"));
        m_isStreamingReasoning = false;
    }
    m_NetWorkParams->rawBuffer.clear();
    emit StreamEnded();
}

void OllamaClient::getAnswer()
{

    if (m_NetWorkParams->clientNetWorkReply->error() == QNetworkReply::NoError)
    {
        QByteArray read_data = m_NetWorkParams->clientNetWorkReply->readAll();
        QJsonObject Content = parseJsonReplyToMsg(read_data);
        QString textresponse;
        bool isFunctionCall = Content.contains("tool_calls");
        if (!isFunctionCall)
        {
            QJsonObject msgs = Content.value("message").toObject();
            const QString content = msgs.value("content").toString();
            const QString reasoning = msgs.value("thinking").toString();
            if (!content.isEmpty() || !reasoning.isEmpty())
            {
                if (!reasoning.isEmpty())
                {
                    textresponse = QStringLiteral("<think>%1</think>%2")
                        .arg(reasoning.trimmed(), content);
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

void OllamaClient::getStreamAnswer()
{
    if (m_NetWorkParams->clientNetWorkReply->error() == QNetworkReply::NoError)
    {
        QByteArray response_data = m_NetWorkParams->clientNetWorkReply->readAll();
        m_NetWorkParams->rawBuffer.append(response_data);
        QByteArray tmp_data = QByteArray(response_data);
        int pos;
        while ((pos = m_NetWorkParams->rawBuffer.indexOf("\n")) != -1)
        {
            QByteArray chunk = m_NetWorkParams->rawBuffer.left(pos);
            m_NetWorkParams->rawBuffer.remove(0, pos + 1);
            if (chunk.trimmed().isEmpty())
            {
                continue;
            }
            QJsonObject obj = parseJsonReplyToMsg(chunk);
            if (obj.contains("message"))
            {
                QJsonObject msg = obj["message"].toObject();
                const QString reasoning = msg.value("thinking").toString();
                const QString content = msg.value("content").toString();

                if (!reasoning.isEmpty()) {
                    if (!m_isStreamingReasoning) {
                        emit AnswerStream(QStringLiteral("<think>%1").arg(reasoning));
                        m_isStreamingReasoning = true;
                    }
                    else {
                        emit AnswerStream(reasoning);
                    }
                }

                if (!content.isEmpty()) {
                    if (m_isStreamingReasoning) {
                        emit AnswerStream(QStringLiteral("</think>%1").arg(content));
                        m_isStreamingReasoning = false;
                    }
                    else {
                        emit AnswerStream(content);
                    }
                }
            }
        }
    }
}

QNetworkRequest OllamaClient::createApiRequest(const QUrl& url)
{
    QNetworkRequest request(url);

    // 设置通用请求�?
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

void OllamaClient::sendApiRequest(const QNetworkRequest& request, QTimer* timeoutTimer, int timeoutMs)
{
    m_NetWorkParams->clientNetWorkReply = std::unique_ptr<QNetworkReply>(
        m_NetWorkParams->clientNetWorkManager->get(request));

    // 根据请求类型连接相应的处理函�?
    if (m_currentRequestType == RequestType::FetchModels)
    {
        connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
            this, &OllamaClient::onFetchModelsFinished);
        // 启动超时定时�?
        timeoutTimer->start(timeoutMs);
    }
    else if (m_currentRequestType == RequestType::ConnectionCheck)
    {
        connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
            this, &OllamaClient::onCheckConnectionFinished);
        // 启动超时定时�?
        timeoutTimer->start(timeoutMs);
    }
    else if (m_currentRequestType == RequestType::GetKonwledgeBase)
    {
        connect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
            this, &OllamaClient::onGetKnowledgeBaseFinished);
    }
}

QStringList OllamaClient::parseModelIds(const QByteArray &jsonData)
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
            if (modelObj.contains("model"))
            {
                QString modelId = modelObj["model"].toString();
                if (!modelId.isEmpty())
                {
                    modelIds.append(modelId);
                }
            }
        }
    }
    return modelIds;
}

void OllamaClient::checkServerConnectionAsync(int timeoutMs)
{
    // 取消之前的请�?
    cancelCurrentRequest();

    // 设置请求类型
    m_currentRequestType = RequestType::ConnectionCheck;

    // 验证URL
    if (!validateServerUrl())
    {
        emit serverConnectionCheckFinished(false, tr("URL Error"));
        return;
    }

    // 构建请求并发�?
    QUrl testUrl = buildApiUrl("/api/version");
    QNetworkRequest testRequest = createApiRequest(testUrl);
    sendApiRequest(testRequest, m_connectionCheckTimer, timeoutMs);
}

void OllamaClient::onCheckConnectionFinished()
{
    if (!m_NetWorkParams->clientNetWorkReply) {
        return;
    }

    // 检查是否超�?
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

void OllamaClient::fetchModelsAsync(int timeoutMs)
{
    // 取消之前的请�?
    cancelCurrentRequest();

    // 设置请求类型
    m_currentRequestType = RequestType::FetchModels;

    // 验证URL
    if (!validateServerUrl())
    {
        emit modelsListFetched(false, QStringList(), tr("URL Error"));
        return;
    }

    // 构建请求并发�?
    QUrl modelsUrl = buildApiUrl("/api/tags");
    QNetworkRequest modelsRequest = createApiRequest(modelsUrl);
    sendApiRequest(modelsRequest, m_fetchModelsTimer, timeoutMs);
}

void OllamaClient::onFetchModelsFinished()
{
    if (!m_NetWorkParams->clientNetWorkReply) {
        return;
    }

    // 检查是否超�?
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
                // 发送模型更新信�?
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

void OllamaClient::onCheckConnectionTimeout()
{
    if (m_NetWorkParams->clientNetWorkReply)
    {
        disconnect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
            this, &OllamaClient::onCheckConnectionFinished);

        m_NetWorkParams->clientNetWorkReply->abort();
        m_NetWorkParams->clientNetWorkReply.reset();
    }
    emit serverConnectionCheckFinished(false, tr("Please Check IP Address or Network Setting"));
}

void OllamaClient::onFetchModelsTimeout()
{
    if (m_NetWorkParams->clientNetWorkReply)
    {
        disconnect(m_NetWorkParams->clientNetWorkReply.get(), &QNetworkReply::finished,
            this, &OllamaClient::onFetchModelsFinished);

        m_NetWorkParams->clientNetWorkReply->abort();
        m_NetWorkParams->clientNetWorkReply.reset();
    }
    emit modelsListFetched(false, QStringList(), tr("Fetch ModelList Timeout"));
}

void OllamaClient::AnalysisBlockResponse(QJsonObject& response_obj)
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

void OllamaClient::AnalysisStreamResponse(QJsonObject& response_obj)
{

}

QStringList OllamaClient::GetFollowUpSuggestions()
{
    //to be continue
    return QStringList();
}

void OllamaClient::getKnowledgeBase()
{
    //Ollama NO Knowledge
}

void OllamaClient::onGetKnowledgeBaseFinished()
{

}

void OllamaClient::uploadFile(const QString& filePath)
{
}

void OllamaClient::DeleteFile(const QString& fileID)
{
}

void OllamaClient::onFileUploadFinished()
{
}

void OllamaClient::onDeleteFileFinished()
{
}

void OllamaClient::ChangeKnowledgeGraph(const QString& kownledgeID)
{
}

void OllamaClient::CancelUpdateFile(const QString& file)
{
}

void OllamaClient::CancelAllUpdateFiles(const QStringList& fileList)
{
}
