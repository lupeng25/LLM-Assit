#include "MessageManager.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QUuid>
#include <QDateTime>
#include <QUrlQuery>
#include <QSslConfiguration>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QDebug>

MessageManager::MessageManager(QObject *parent)
    : QObject(parent)
{
}

void MessageManager::setLLMParams(LLMParams* params)
{
    // 如果之前有创建自己的 m_LLMParams，且不是传入的参数，需要先删除（避免内存泄漏）
    if (m_LLMParams != nullptr && m_LLMParams != params)
    {
        // 检查是否是构造函数中创建的（通过检查是否是默认构造的）
        // 这里简单处理：如果 m_LLMParams 存在且不是传入的参数，就删除
        // 注意：这假设构造函数中创建的 m_LLMParams 会在 setLLMParams 之前一直存在
        delete m_LLMParams;
        m_LLMParams = nullptr;
    }
    this->m_LLMParams = params;
}

void MessageManager::buildRequest()
{
    m_NetWorkParams->clientRequest.setRawHeader("accept", "text/event-stream");
    m_NetWorkParams->clientRequest.setUrl(QUrl(m_LLMParams->getBaseUrl()));
    m_NetWorkParams->clientRequest.setRawHeader("Authorization", ("Bearer " + m_LLMParams->getApiKey()).toUtf8());
    m_NetWorkParams->clientRequest.setRawHeader("Content-Type", "application/json");
}

// 取消当前请求的公共函数
void MessageManager::cancelCurrentRequest()
{
    if (m_NetWorkParams && m_NetWorkParams->clientNetWorkReply)
    {
        auto reply = std::move(m_NetWorkParams->clientNetWorkReply);
        reply->abort();
        reply->disconnect();
    }
}

// 验证服务器URL
bool MessageManager::validateServerUrl()
{
    QUrl originalUrl = m_NetWorkParams->clientRequest.url();
    return !originalUrl.isEmpty();
}

// 构建API URL的公共函数
QUrl MessageManager::buildApiUrl(const QString& apiPath)
{
    QUrl originalUrl = m_NetWorkParams->clientRequest.url();

    QUrl apiUrl;
    apiUrl.setScheme(originalUrl.scheme());
    apiUrl.setHost(originalUrl.host());
    apiUrl.setPort(originalUrl.port());
    apiUrl.setPath(apiPath);

    return apiUrl;
}

// 获取当前可用的模型列表
QStringList MessageManager::getAvailableModels()
{
    return m_availableModelIds;
}

void MessageManager::configureRequestSsl(const QSslConfiguration& config)
{
	if (!m_NetWorkParams)
	{
		return;
	}
	m_NetWorkParams->clientRequest.setSslConfiguration(config);
}

QNetworkRequest MessageManager::currentRequest() const
{
	if (!m_NetWorkParams)
	{
		return QNetworkRequest();
	}
	return m_NetWorkParams->clientRequest;
}

QNetworkReply* MessageManager::postRequest(const QByteArray& data)
{
	if (!m_NetWorkParams || !m_NetWorkParams->clientNetWorkManager)
	{
		return nullptr;
	}
	return m_NetWorkParams->clientNetWorkManager->post(m_NetWorkParams->clientRequest, data);
}

// 初始化网络管理器（用于 LLMClientManager 等外部类）
void MessageManager::initializeNetworkManager()
{
	if (!m_NetWorkParams)
	{
		return;
	}
	if (!m_NetWorkParams->clientNetWorkManager)
	{
		m_NetWorkParams->clientNetWorkManager = std::make_unique<QNetworkAccessManager>();
	}
}

// 提取 HTTP 错误短语（如 "Bad Request"、"Unauthorized" 等）
QString MessageManager::extractHttpErrorCode(const QString& errorLevel)
{
    static const QRegularExpression regex(R"(server replied:\s*(\w+(?:\s\w+)*))");
    QRegularExpressionMatch match = regex.match(errorLevel);
    return match.hasMatch() ? match.captured(1) : QString();
}

// 从 JSON 文本中提取顶层字段值（字符串类型）
QString MessageManager::extractJsonField(const QString& jsonText, const QString& fieldName)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        return QString();
    QJsonObject obj = doc.object();
    return obj.value(fieldName).toString();
}
