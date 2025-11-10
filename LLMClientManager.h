#pragma once

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <memory>
#include <algorithm>
#include "LLMParams.h"
#include "MessageManager.h"

class LLMClientManager : public QObject {
	Q_OBJECT
public:
	explicit LLMClientManager(QObject* parent = nullptr);

	void initialize(AIProvider provider, LLMParams* params);
	void rebuild(AIProvider provider, LLMParams* params);

	MessageManager* client() const;
	std::unique_ptr<MessageManager> takeClient();

	void setConnectionRetryPolicy(int maxAttempts, int backoffMs);
	void setFetchModelsRetryPolicy(int maxAttempts, int backoffMs);

	void checkConnection(int timeoutMs);
	void fetchModels(int timeoutMs);

signals:
	void clientChanged(MessageManager* client);
	void connectionCheckSucceeded();
	void connectionCheckFailed(const QString& errorMessage);
	void modelsFetchSucceeded(const QStringList& models);
	void modelsFetchFailed(const QString& errorMessage);
	void clientError(const QString& context, const QString& errorMessage);

private:
	struct RetryContext {
		int attempts = 0;
		int maxAttempts = 3;
		int backoffMs = 1500;
		int timeoutMs = 5000;
		QTimer* timer = nullptr;
	};

	void connectClientSignals();
	void resetRetryContexts();
	void startConnectionAttempt();
	void startModelsAttempt();
	void scheduleConnectionRetry(const QString& errorMessage);
	void scheduleModelsRetry(const QString& errorMessage);

	private slots:
	void handleConnectionCheckFinished(bool isConnected, const QString& errorMessage);
	void handleModelsListFetched(bool success, const QStringList& models, const QString& errorMessage);
	void retryConnection();
	void retryModels();

private:

	std::unique_ptr<MessageManager> createClient(AIProvider provider) const;

	std::unique_ptr<MessageManager> m_client;
	RetryContext m_connectionRetry;
	RetryContext m_modelsRetry;
};

