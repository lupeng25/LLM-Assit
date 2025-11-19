#pragma once
#include <QNetworkAccessManager>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QMimeDatabase>
#include <QJsonArray>
#include <QTimer>
#include <memory>
#include "ChatInputWidget.h"
#include "LLMParams.h"
enum class AIProvider //平台
{
    Dify,
    Open_WebUI,
    Ollama,
    AnythingLLM,
    Custom
};
enum class RequestType //API请求
{
    ConnectionCheck,
    FetchModels,
    ChatRequest,
    FollowUpSuggest,
    GetKonwledgeBase,
    StopStreamAns,
    FileUpload,
    FileDelete
};
struct ClientNetWork //网络
{
    QNetworkRequest clientRequest;
    std::unique_ptr<QNetworkAccessManager> clientNetWorkManager;
    std::unique_ptr<QNetworkReply> clientNetWorkReply;
    QByteArray rawBuffer;
};
struct KnowledgeBase//知识�?
{
    QString KnowledgeID;
    QString KnowledgeName;
    QString KnowledgeDescription;
};

class MessageManager : public QObject
{
    Q_OBJECT
public:
    explicit MessageManager(QObject *parent = nullptr);
    virtual ~MessageManager() = default;

    virtual void buildRequest();
    virtual QUrl buildApiUrl(const QString& apiPath);
    virtual void cancelCurrentRequest();
    virtual bool validateServerUrl();
    virtual void setLLMParams(LLMParams* params);
    virtual QStringList getAvailableModels();// 获取可用模型列表

                                             // 获取提供商信�?
    virtual AIProvider getProvider() const = 0;
    virtual QString getProviderName() const = 0;
    virtual QString getVersion() const = 0;

    // 创建body
    virtual QByteArray buildMessageBody(const ChatSendMessage& msg) = 0;
    // 提问前的预处�?
    virtual void SendPreProcess(const ChatSendMessage& msg) = 0;
    // 返回数据处理
    virtual QJsonObject parseJsonReplyToMsg(const QByteArray &data) = 0;
    // 发送消息到服务�?
    virtual int send(const ChatSendMessage& msg) = 0;
    // 发送消息到流式服务�?
    virtual int StreamSend(const ChatSendMessage& msg) = 0;
    // 错误处理
    virtual QString GetError(const QString& errorLevel, const QString& errorContext) = 0;
    // 流式输出结束处理
    virtual void processStreamEnded() = 0;
    // 创建API请求
    virtual QNetworkRequest createApiRequest(const QUrl& url) = 0;
    // 发送API请求
    virtual void sendApiRequest(const QNetworkRequest& request, QTimer* timeoutTimer, int timeoutMs) = 0;
    // 检查连�?异步)
    virtual void checkServerConnectionAsync(int timeoutMs) = 0;
    // 解析模型ID列表
    virtual QStringList parseModelIds(const QByteArray &jsonData) = 0;
    // 获取模型列表(列表)
    virtual void fetchModelsAsync(int timeoutMs = 5000) = 0;
    // 获取知识库信�?
    virtual void getKnowledgeBase() = 0;
    //上传文件
    virtual void uploadFile(const QString& filePath) = 0;
    //删除文件(根据客户端返回的文件ID)
    virtual void DeleteFile(const QString& fileID) = 0;
    // 解析blocking数据
    virtual void AnalysisBlockResponse(QJsonObject& response_obj) = 0;
    // 解析streaming数据
    virtual void AnalysisStreamResponse(QJsonObject& response_obj) = 0;
    // 更变知识�?
    virtual void ChangeKnowledgeGraph(const QString& kownledgeID) = 0;
    // 取消某个要上传的文件
    virtual void CancelUpdateFile(const QString& file) = 0;
    // 撤销所有待上传文件
    virtual void CancelAllUpdateFiles(const QStringList& fileList) = 0;

    ClientNetWork *m_NetWorkParams;
    LLMParams *m_LLMParams;
    QTimer *m_connectionCheckTimer = nullptr;
    QTimer* m_fetchModelsTimer = nullptr;
    RequestType m_currentRequestType; //记录当前请求API类型
    QStringList m_availableModelIds;  // 存储可用的模型ID列表
    std::vector<KnowledgeBase> KnowledgeInfo;
    QMap<QString, QString>m_UpFiles;

    public slots:
    virtual void onFetchModelsFinished() = 0;
    virtual void onFetchModelsTimeout() = 0;
    virtual void onCheckConnectionFinished() = 0;
    virtual void onCheckConnectionTimeout() = 0;
    virtual QStringList GetFollowUpSuggestions() = 0;
    virtual void onGetKnowledgeBaseFinished() = 0;
    virtual void onFileUploadFinished() = 0;
    virtual void onDeleteFileFinished() = 0;

    private slots:
    // 处理Blocking数据
    virtual void getAnswer() = 0;
    // 处理Streaming数据
    virtual void getStreamAnswer() = 0;

signals:
    // 发出Blocking信号
    void Answer(const QString& word, bool bError);
    // 发出Streaming信号
    void AnswerStream(const QString& word);

    void ChangeButtonStatus(bool received);

    void FunctionCallSignal(QJsonObject& Content);

    void StreamEnded();

    void serverConnectionCheckFinished(bool isConnected, const QString &errorMessage);

    void modelsListFetched(bool success, const QStringList& models, const QString& errorMessage);

    void KonwledgeBaseSignal(std::map<QString, std::pair<QString, QString>> knowBase);

    void FollowSuggestSignal(QStringList suggestions);

protected:
    // 代码质量优化：通用错误解析工具
    // �?Qt 的错误字符串中提�?HTTP 错误短语（如 "Bad Request"�?Unauthorized" 等）
    static QString extractHttpErrorCode(const QString& errorLevel);
    // �?JSON 文本中提取指定字段（顶层�?
    static QString extractJsonField(const QString& jsonText, const QString& fieldName);

};


