#include "MessageManager.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QUuid>
#include <QDateTime>
#include <QUrlQuery>
#include <QDebug>

MessageManager::MessageManager(QObject *parent)
	: QObject(parent)
{
}

void MessageManager::setLLMParams(LLMParams* params)
{
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