#pragma once
#include "MessageManager.h"
#include <QJsonDocument>
#include <QUrlQuery>
#include <QJsonArray>
#include <set>
class OllamaClient :
	public MessageManager
{
	Q_OBJECT
public:
	explicit OllamaClient(QObject *parent = nullptr);
	~OllamaClient();

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
	void processStreamEnded() override;
	QNetworkRequest createApiRequest(const QUrl& url) override;
	void sendApiRequest(const QNetworkRequest& request, QTimer* timeoutTimer, int timeoutMs) override;
	void checkServerConnectionAsync(int timeoutMs) override;
	QStringList parseModelIds(const QByteArray &jsonData) override;
	void fetchModelsAsync(int timeoutMs = 5000) override;
	QStringList GetFollowUpSuggestions() override;
	void getKnowledgeBase() override;
	void uploadFile(const QString& filePath) override;
	void DeleteFile(const QString& fileID) override;
	void AnalysisBlockResponse(QJsonObject& response_obj) override;
	void AnalysisStreamResponse(QJsonObject& response_obj) override;
	void ChangeKnowledgeGraph(const QString& kownledgeID) override;
	void CancelUpdateFile(const QString& file) override;
	void CancelAllUpdateFiles(const QStringList& fileList)override;

	std::set<QString> addKnowledge;
private:
	bool m_isStreamingReasoning = false;

	public slots:
	void onFetchModelsFinished() override;
	void onFetchModelsTimeout() override;
	void onCheckConnectionFinished() override;
	void onCheckConnectionTimeout()override;
	void onGetKnowledgeBaseFinished() override;
	void onFileUploadFinished() override;
	void onDeleteFileFinished() override;

	private slots:
	void getAnswer() override;
	void getStreamAnswer() override;

};

