#pragma once
#include <QNetworkAccessManager> 
#include <QNetworkReply> 
#include <QJsonArray>
#include <QTimer>
#include <memory> 
#include "ChatInputWidget.h"
#include "LLMParams.h"
enum class AIProvider //平台
{
	Dify,
	Open_WebUI,
	ChatGPT,
	Claude,
	AnythingLLM,
	Custom
};
enum class RequestType //API请求
{
	ConnectionCheck,
	FetchModels,
	ChatRequest,
	FollowUpSuggest
};
struct ClientNetWork //网络
{
	QNetworkRequest clientRequest;
	std::unique_ptr<QNetworkAccessManager> clientNetWorkManager;
	std::unique_ptr<QNetworkReply> clientNetWorkReply;
	QByteArray rawBuffer;
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

	// 获取提供商信息
	virtual AIProvider getProvider() const = 0;
	virtual QString getProviderName() const = 0;
	virtual QString getVersion() const = 0;

	// 创建body
	virtual QByteArray buildMessageBody(const ChatSendMessage& msg) = 0;
	// 提问前的预处理
	virtual void SendPreProcess(const ChatSendMessage& msg) = 0;
	// 返回数据处理
	virtual QJsonObject parseJsonReplyToMsg(const QByteArray &data) = 0;
	// 发送消息到服务器 
	virtual int send(const ChatSendMessage& msg)=0;
	// 发送消息到流式服务器 
	virtual int StreamSend(const ChatSendMessage& msg)=0;
	// 错误处理
	virtual QString GetError(const QString& errorLevel, const QString& errorContext) = 0;
	// 解析blocking数据 
	virtual void AnalysisAnswer(const QString& word, bool bError) = 0;
	// 解析Streaming数据
	virtual void AnalysisStreamAnswer() = 0;
	//创建API请求
	virtual QNetworkRequest createApiRequest(const QUrl& url)=0;
	//发送API请求
	virtual void sendApiRequest(const QNetworkRequest& request, QTimer* timeoutTimer, int timeoutMs)=0;
	// 检查连接(异步)
	virtual void checkServerConnectionAsync(int timeoutMs) = 0;
	// 解析模型ID列表
	virtual QStringList parseModelIds(const QByteArray &jsonData) = 0;
	//获取模型列表(列表)
	virtual void fetchModelsAsync(int timeoutMs = 5000)=0;

	ClientNetWork *m_NetWorkParams;
	LLMParams *m_LLMParams;
	QTimer *m_connectionCheckTimer = nullptr;
	QTimer* m_fetchModelsTimer = nullptr;
	RequestType m_currentRequestType; //记录当前请求API类型
	QStringList m_availableModelIds;  // 存储可用的模型ID列表

public slots:
	virtual void onFetchModelsFinished()=0;
	virtual void onFetchModelsTimeout()=0;
	virtual void onCheckConnectionFinished() = 0;
	virtual void onCheckConnectionTimeout() = 0;
	virtual QStringList GetFollowUpSuggestions() = 0;
private slots:
	// 处理Blocking数据
	virtual void getAnswer()=0;
    // 处理Streaming数据
	virtual void getStreamAnswer()=0;

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

};

