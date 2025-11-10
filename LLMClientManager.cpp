#include <QNetworkAccessManager>
#include "LLMClientManager.h"

#include "DifyClient.h"
#include "OllamaClient.h"
#include "Open_WebUIClient.h"

LLMClientManager::LLMClientManager(QObject* parent)
	: QObject(parent) {
	m_connectionRetry.timer = new QTimer(this);
	m_connectionRetry.timer->setSingleShot(true);
	connect(m_connectionRetry.timer, &QTimer::timeout, this, &LLMClientManager::retryConnection);

	m_modelsRetry.timer = new QTimer(this);
	m_modelsRetry.timer->setSingleShot(true);
	connect(m_modelsRetry.timer, &QTimer::timeout, this, &LLMClientManager::retryModels);
}

void LLMClientManager::initialize(AIProvider provider, LLMParams* params) {
	if (m_client) {
		disconnect(m_client.get(), nullptr, this, nullptr);
	}

	m_client = createClient(provider);
	if (!m_client) {
		emit clientChanged(nullptr);
		return;
	}
	m_client->setLLMParams(params);
	m_client->m_NetWorkParams->clientNetWorkManager = std::make_unique<QNetworkAccessManager>();
	m_client->buildRequest();
	connectClientSignals();
	resetRetryContexts();
	emit clientChanged(m_client.get());
}

void LLMClientManager::rebuild(AIProvider provider, LLMParams* params) {
	initialize(provider, params);
}

MessageManager* LLMClientManager::client() const {
	return m_client.get();
}

std::unique_ptr<MessageManager> LLMClientManager::takeClient() {
	return std::move(m_client);
}

void LLMClientManager::setConnectionRetryPolicy(int maxAttempts, int backoffMs) {
	m_connectionRetry.maxAttempts = std::max(1, maxAttempts);
	m_connectionRetry.backoffMs = std::max(0, backoffMs);
}

void LLMClientManager::setFetchModelsRetryPolicy(int maxAttempts, int backoffMs) {
	m_modelsRetry.maxAttempts = std::max(1, maxAttempts);
	m_modelsRetry.backoffMs = std::max(0, backoffMs);
}

void LLMClientManager::checkConnection(int timeoutMs) {
	if (!m_client) {
		emit clientError(tr("Connection Check"), tr("LLM client is not initialized."));
		return;
	}
	m_connectionRetry.timer->stop();
	m_connectionRetry.attempts = 0;
	m_connectionRetry.timeoutMs = timeoutMs;
	startConnectionAttempt();
}

void LLMClientManager::fetchModels(int timeoutMs) {
	if (!m_client) {
		emit clientError(tr("Model Fetch"), tr("LLM client is not initialized."));
		return;
	}
	m_modelsRetry.timer->stop();
	m_modelsRetry.attempts = 0;
	m_modelsRetry.timeoutMs = timeoutMs;
	startModelsAttempt();
}

void LLMClientManager::connectClientSignals() {
	if (!m_client) {
		return;
	}
	connect(m_client.get(), &MessageManager::serverConnectionCheckFinished,
		this, &LLMClientManager::handleConnectionCheckFinished);
	connect(m_client.get(), &MessageManager::modelsListFetched,
		this, &LLMClientManager::handleModelsListFetched);
}

void LLMClientManager::resetRetryContexts() {
	m_connectionRetry.attempts = 0;
	m_modelsRetry.attempts = 0;
	m_connectionRetry.timer->stop();
	m_modelsRetry.timer->stop();
}

void LLMClientManager::startConnectionAttempt() {
	if (!m_client) {
		emit clientError(tr("Connection Check"), tr("LLM client is not initialized."));
		return;
	}
	if (m_connectionRetry.attempts >= m_connectionRetry.maxAttempts) {
		emit connectionCheckFailed(tr("Maximum retry attempts reached."));
		emit clientError(tr("Connection Check"), tr("Maximum retry attempts reached."));
		return;
	}
	++m_connectionRetry.attempts;
	m_client->checkServerConnectionAsync(m_connectionRetry.timeoutMs);
}

void LLMClientManager::startModelsAttempt() {
	if (!m_client) {
		emit clientError(tr("Model Fetch"), tr("LLM client is not initialized."));
		return;
	}
	if (m_modelsRetry.attempts >= m_modelsRetry.maxAttempts) {
		emit modelsFetchFailed(tr("Maximum retry attempts reached."));
		emit clientError(tr("Model Fetch"), tr("Maximum retry attempts reached."));
		return;
	}
	++m_modelsRetry.attempts;
	m_client->fetchModelsAsync(m_modelsRetry.timeoutMs);
}

void LLMClientManager::scheduleConnectionRetry(const QString& errorMessage) {
	if (!m_connectionRetry.timer) {
		return;
	}
	if (m_connectionRetry.attempts >= m_connectionRetry.maxAttempts) {
		emit connectionCheckFailed(errorMessage);
		emit clientError(tr("Connection Check"), errorMessage);
		return;
	}
	m_connectionRetry.timer->start(m_connectionRetry.backoffMs);
}

void LLMClientManager::scheduleModelsRetry(const QString& errorMessage) {
	if (!m_modelsRetry.timer) {
		return;
	}
	if (m_modelsRetry.attempts >= m_modelsRetry.maxAttempts) {
		emit modelsFetchFailed(errorMessage);
		emit clientError(tr("Model Fetch"), errorMessage);
		return;
	}
	m_modelsRetry.timer->start(m_modelsRetry.backoffMs);
}

void LLMClientManager::handleConnectionCheckFinished(bool isConnected, const QString& errorMessage) {
	m_connectionRetry.timer->stop();
	if (isConnected) {
		m_connectionRetry.attempts = 0;
		emit connectionCheckSucceeded();
		m_connectionRetry.timer->stop();
	}
	else {
		scheduleConnectionRetry(errorMessage.isEmpty() ? tr("Server connection failed.") : errorMessage);
	}
}

void LLMClientManager::handleModelsListFetched(bool success, const QStringList& models, const QString& errorMessage) {
	m_modelsRetry.timer->stop();
	if (success) {
		m_modelsRetry.attempts = 0;
		emit modelsFetchSucceeded(models);
	}
	else {
		QString err = errorMessage;
		if (err.isEmpty()) {
			err = tr("Failed to fetch model list.");
		}
		scheduleModelsRetry(err);
	}
}

void LLMClientManager::retryConnection() {
	startConnectionAttempt();
}

void LLMClientManager::retryModels() {
	startModelsAttempt();
}

std::unique_ptr<MessageManager> LLMClientManager::createClient(AIProvider provider) const {
	switch (provider) {
	case AIProvider::Dify:
		return std::make_unique<DifyClient>();
	case AIProvider::Open_WebUI:
		return std::make_unique<Open_WebUIClient>();
	default:
		return std::make_unique<OllamaClient>();
	}
}

