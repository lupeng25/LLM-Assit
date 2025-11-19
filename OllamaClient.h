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

    // 实现基类的纯虚函�?
    AIProvider getProvider() const override;
    QString getProviderName() const override;
    QString getVersion() const override;
    // 构建消息�?
    QByteArray buildMessageBody(const ChatSendMessage& msg) override;
    // 发送前预处�?
    void SendPreProcess(const ChatSendMessage& msg) override;
    // 解析JSON回复为消�?
    QJsonObject parseJsonReplyToMsg(const QByteArray &data) override;
    // 发送消息（阻塞式）
    int send(const ChatSendMessage& msg) override;
    // 发送消息（流式传输�?
    int StreamSend(const ChatSendMessage& msg) override;
    // 获取错误信息
    QString GetError(const QString& errorLevel, const QString& errorContext) override;
    // 流式传输结束处理
    void processStreamEnded() override;
    // 创建API请求
    QNetworkRequest createApiRequest(const QUrl& url) override;
    // 发送API请求
    void sendApiRequest(const QNetworkRequest& request, QTimer* timeoutTimer, int timeoutMs) override;
    // 检查服务器连接（异步）
    void checkServerConnectionAsync(int timeoutMs) override;
    // 解析模型ID列表
    QStringList parseModelIds(const QByteArray &jsonData) override;
    // 获取模型列表（异步）
    void fetchModelsAsync(int timeoutMs = 5000) override;
    // 获取后续建议
    QStringList GetFollowUpSuggestions() override;
    // 获取知识库信�?
    void getKnowledgeBase() override;
    // 上传文件
    void uploadFile(const QString& filePath) override;
    // 删除文件
    void DeleteFile(const QString& fileID) override;
    // 分析阻塞式响�?
    void AnalysisBlockResponse(QJsonObject& response_obj) override;
    // 分析流式响应
    void AnalysisStreamResponse(QJsonObject& response_obj) override;
    // 更改知识图谱
    void ChangeKnowledgeGraph(const QString& kownledgeID) override;
    // 取消文件上传
    void CancelUpdateFile(const QString& file) override;
    // 取消所有文件上�?
    void CancelAllUpdateFiles(const QStringList& fileList)override;

    // 已添加的知识库集�?
    std::set<QString> addKnowledge;
private:
    // 是否正在流式传输推理内容
    bool m_isStreamingReasoning = false;

    public slots:
    // 获取模型列表完成处理
    void onFetchModelsFinished() override;
    // 获取模型列表超时处理
    void onFetchModelsTimeout() override;
    // 连接检查完成处�?
    void onCheckConnectionFinished() override;
    // 连接检查超时处�?
    void onCheckConnectionTimeout()override;
    // 获取知识库完成处�?
    void onGetKnowledgeBaseFinished() override;
    // 文件上传完成处理
    void onFileUploadFinished() override;
    // 文件删除完成处理
    void onDeleteFileFinished() override;

    private slots:
    // 处理阻塞式回�?
    void getAnswer() override;
    // 处理流式回答
    void getStreamAnswer() override;

};

