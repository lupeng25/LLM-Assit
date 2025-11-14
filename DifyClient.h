#pragma once
#include "MessageManager.h"
#include <QJsonDocument>
#include <QUrlQuery>
#include <QJsonArray>
class DifyClient :
    public MessageManager
{
    Q_OBJECT
public:
    explicit DifyClient(QObject *parent = nullptr);
    ~DifyClient();

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
    QUrl buildApiUrl(const QString& apiPath) override;
    QUrl buildApiUrl(const QString& apiPath,const QString& UserID);
    void getKnowledgeBase() override;
    QStringList GetFollowUpSuggestions()override;
    void uploadFile(const QString& filePath) override;
    void DeleteFile(const QString& fileID) override;
    void AnalysisBlockResponse(QJsonObject& response_obj) override;
    void AnalysisStreamResponse(QJsonObject& response_obj) override;
    void ChangeKnowledgeGraph(const QString& kownledgeID) override;
    void CancelUpdateFile(const QString& file) override;
    void CancelAllUpdateFiles(const QStringList& fileList)override;

    //Dify定制
    void setConversationId(const QString& conversationId);
    void setMessageId(const QString& messageId);
    void setTaskId(const QString& taskId);
    void setUserId(const QString& userId);
    QString getConversationId() const;
    QString getMessageId() const;
    QString getTaskId() const;
    QString getUserId() const;
    void resetConversationId();
    void resetMessageId();
    void resetTaskId();
    void resetUserId();
    QByteArray buildStopAnswerBody();
    void StopGenerateStreamAns();

public slots:
    void onFetchModelsFinished() override;
    void onFetchModelsTimeout() override;
    void onCheckConnectionFinished() override;
    void onCheckConnectionTimeout()override;
    void onGetKnowledgeBaseFinished() override;
    void onFileUploadFinished() override;
    void onDeleteFileFinished() override;
    void onGetFollowUpSuggestFinished() ;
    void onStopStreamAnsFinished();

private slots:
    void getAnswer() override;
    void getStreamAnswer() override;

private:
    QString m_conversationId;
    QString m_messageId;
    QString m_taskId;
    QString m_userId = "GKG";
};

