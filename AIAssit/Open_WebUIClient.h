#pragma once
#include "MessageManager.h"
#include <QJsonDocument>
#include <QUrlQuery>
#include <QJsonArray>
class Open_WebUIClient :
	public MessageManager
{
	Q_OBJECT
public:
	explicit Open_WebUIClient(QObject *parent = nullptr);
	~Open_WebUIClient();

	// 实现基类的纯虚函数
	AIProvider getProvider() const override;
	QString getProviderName() const override;
	QString getVersion() const override;
	QByteArray buildMessageBody(const ChatSendMessage& msg) override;
	void SendPreProcess(const ChatSendMessage& msg) override;
	QJsonObject parseJsonReplyToMsg(const QByteArray &data) override;
	int send(const ChatSendMessage& msg) override;
	int StreamSend(const ChatSendMessage& msg) override;
	QString GetError(const QString& errorLevel, const QString& errorContext) override;
	void AnalysisAnswer(const QString& word, bool bError) override;
	void AnalysisStreamAnswer() override;
	QNetworkRequest createApiRequest(const QUrl& url) override;
	void sendApiRequest(const QNetworkRequest& request, QTimer* timeoutTimer, int timeoutMs) override;
	void checkServerConnectionAsync(int timeoutMs) override;
	QStringList parseModelIds(const QByteArray &jsonData) override;
	void fetchModelsAsync(int timeoutMs = 5000) override;
	QStringList GetFollowUpSuggestions() override;
public slots:
    void onFetchModelsFinished() override;
    void onFetchModelsTimeout() override;
    void onCheckConnectionFinished() override;
    void onCheckConnectionTimeout()override;

private slots:
	void getAnswer() override;
	void getStreamAnswer() override;
	
};

