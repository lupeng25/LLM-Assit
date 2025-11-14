#pragma once 
#include <QNetworkAccessManager> 
#include <QPropertyAnimation>
#include <QNetworkReply> 
#include <QJsonDocument> 
#include <QInputDialog>
#include <QMessageBox>
#include <QJsonObject> 
#include <QJsonArray> 
#include <QFileInfo>
#include <QObject> 
#include <QWidget> 
#include <QDebug> 
#include <QTimer>
#include <QFile> 
#include <QUuid>
#include <QDir>
#include <memory> 
#include <QVector>
#include <QPointer>
#include "LLMParams.h"
#include "ui_Frm_AIAssit.h" 
#include "LLMChatFrame.h"
#include "LLMFunctionCall.h"
#include "GitLogReader.h"
#include "MessageManager.h"
#include "ChatSessionService.h"
#include "ChatSessionTypes.h"
#include "AppConfigRepository.h"
#include "LLMClientManager.h"
class Frm_AIAssit : public QWidget
{
	Q_OBJECT
public:
	Frm_AIAssit(QWidget *parent = Q_NULLPTR);
	~Frm_AIAssit();
	void SetLLMCommandFunction(std::function<QString(QString)> function);
	public slots:
	// 显示服务器返回的答案 
	void getAnswerShow(const QString& word, bool bError);
	//显示服务器流式数据
	void getStreamAnswerShow(const QString& word);
	//获取最新的提问内容
	void AskQuestionAgain(QString msg);
protected:
	void resizeEvent(QResizeEvent *event) override;
	private slots:
	// 发送消息按钮点击事件处理 
	void on_pushButton_clicked(ChatSendMessage msg);
	//新建对话
	void createNewConversation();
	//选中新对话
	void onConversationSelected(QListWidgetItem* current, QListWidgetItem* previous);
	//信息格式转换
	QJsonObject parseJsonReplyToMsg(const QByteArray &data, bool isStream = false);
	//流式数据结束处理
	void getStreamAnswerEnd();
private:
	// 初始化函数 
	void recalculateVisibleBubbles();
	//UI初始化
	void initUI();
	void applyResponsiveLayout(int windowWidth);
	void setSidebarVisible(bool visible, bool animated = true, bool triggeredByResponsive = false);
	void updateSidebarWidth(int windowWidth);
	//参数初始化
	void initParams();
	//初始化历史文件
	void initHistoryFile();
	//读取历史文件
	bool loadChatMapFromJson();
	// 显示或隐藏参数设置界面 
	void ShowAIParam();
	// 设置信号和槽连接 
	void setupSignals();
	// 设置LLMClient的信号槽连接
	void setupLLMClientSignals();
	// 应用当前界面的模型参数 
	void ApplyModelParam();
	//设置显示对话
	void PreLoadChat();
	//侧边栏
	void toggleSidebar();
	//添加气泡聊天框
	void addChatBubble(const QString& text, bool bIsUser);
	//重新计算所有对话气泡
	void recalculateAllChatBubbles();
	void refreshBubbleSize(LLMChatFrame* bubble, QListWidgetItem* item, const QString& conversationId = QString());
	void finalizeLatestBubble(LLMChatFrame* bubble, QListWidgetItem* item, const QString& dialogName,
		const QString& answerHtml, const QString& reasoningHtml, bool markStreamCompleted);
	//使用最新回答更新对话名
	QString updateDialogName(const QString& dialogName);
	void attachBubbleSignals(LLMChatFrame* bubble);
	// 气泡对象池
	LLMChatFrame* acquireBubble(QWidget* parent);
	void releaseBubble(LLMChatFrame* bubble);
	void releaseAllBubbles();
	void clearBubblePool();
	// 发送消息到服务器 
	int send(const ChatSendMessage& msg);
	// 发送消息到流式服务器 
	int StreamSend(const ChatSendMessage& msg);
	//处理functioncall结果
	void ProcessFunctionCall(QJsonObject FunctionMsg);
	//functioncall预处理
	void preFuncall(QJsonObject& Content);
	//设置LLM客户端
	void setLLMClient(AIProvider platform);
	//构建推荐问题气泡
	void buildBubbleSuggest(QStringList suggest);
	//获取最新的回答Msg
	QString GetLastestAsk(QString msg);
	//上传File
	void UpAllFilesToHost(const QString& files);
	Ui::Frm_AIAssit ui;

	bool bShowParam = false;
	QString ChatJsonFile;
	QJsonArray m_FunctionTools;
	QString m_currentConversationId; // 当前激活的对话ID 
	MessageManager* LLMClient = nullptr;
	std::unique_ptr<LLMParams> params;
	QTimer* m_resizeTimer = nullptr;
	bool m_sidebarManuallyHidden = false;
	bool m_sidebarCollapsedByResponsive = false;
	QTimer* m_scrollTimer = nullptr; // UI性能优化：流式更新时的滚动定时器
	QTimer* m_streamDebounceTimer = nullptr;
	QString m_pendingStreamChunk;
	bool m_pendingReasoningEnd = false;
	std::unique_ptr<AppConfigRepository> m_configRepository;
	std::unique_ptr<ChatSessionService> m_chatSessionService;
	std::unique_ptr<LLMClientManager> m_clientManager;
	QVector<QPointer<LLMChatFrame>> m_bubblePool;
	QWidget* m_bubblePoolHost = nullptr;
	static constexpr int kBubblePoolMaxSize = 64;
	bool m_enableBubblePool = false;
	ChatSession* currentSession();
	const ChatSession* currentSession() const;
	ChatSessionMap& sessionMap();
	const ChatSessionMap& sessionMap() const;
	void attachToClient(MessageManager* client);
	QString buildConversationMarkdown(const ChatSession& session) const;
	QString buildConversationHtml(const ChatSession& session) const;
	QString buildConversationPlainText(const ChatSession& session) const;
	using sSingleMsg = ChatMessageData;
	using sMsgList = ChatSession;
signals:
	//发送按钮状态改变
	void PushBtnChanged(bool bEnable);
	//改变当前模型
	void ChangeCurModel(int iModel);
	// 发出答案信号 
	void Answer(const QString& word, bool bError);
	//发出流式信号
	void AnswerStream(const QString& word);
	private slots:
	void onConnectionCheckFailed(const QString& errorMessage);
	void onConnectionCheckSucceeded();
	void onModelsFetchFailed(const QString& errorMessage);
	void onClientManagerError(const QString& context, const QString& errorMessage);
	void deleteCurrentConversation();
	void renameCurrentConversation();
	void ChangeModel(int iModel);
	void onBubbleNoteChanged(const QString& bubbleId, const QString& note);
	void onBubbleImportantToggled(const QString& bubbleId, bool isImportant);
	void onExportConversationRequested(const QString& conversationId, const QString& format);
	void onShowDetailsRequested(const QString& conversationId);
};
