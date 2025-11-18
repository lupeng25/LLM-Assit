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
#include <QDialog>
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
	// 构造函数
	Frm_AIAssit(QWidget *parent = Q_NULLPTR);
	// 析构函数
	~Frm_AIAssit();
	// 设置LLM命令执行函数
	void SetLLMCommandFunction(std::function<QString(QString)> function);
	public slots:
	// 显示服务器返回的答案 
	void getAnswerShow(const QString& word, bool bError);
	//显示服务器流式数据
	void getStreamAnswerShow(const QString& word);
	//获取最新的提问内容
	void AskQuestionAgain(QString msg);
protected:
	// 窗口大小改变事件处理
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
	// 应用响应式布局
	void applyResponsiveLayout(int windowWidth);
	// 设置侧边栏可见性
	void setSidebarVisible(bool visible, bool animated = true, bool triggeredByResponsive = false);
	// 更新侧边栏宽度
	void updateSidebarWidth(int windowWidth);
	//参数初始化
	void initParams();
	//初始化历史文件
	void initHistoryFile();
	//读取历史文件
	bool loadChatMapFromJson();
	// 显示或隐藏参数设置界面 
	void ShowAIParam();
	void ensureParamDialog();
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
	// 刷新气泡大小
	void refreshBubbleSize(LLMChatFrame* bubble, QListWidgetItem* item, const QString& conversationId = QString());
	// 完成最新气泡的最终化处理
	void finalizeLatestBubble(LLMChatFrame* bubble, QListWidgetItem* item, const QString& dialogName,
		const QString& answerHtml, const QString& reasoningHtml, bool markStreamCompleted);
	// 使用最新回答更新对话名
	QString updateDialogName(const QString& dialogName);
	// 连接气泡信号
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

	QDialog* m_paramDialog = nullptr;
	AIParamWidget* m_paramWidget = nullptr;
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
	// 获取当前会话（非const版本）
	ChatSession* currentSession();
	// 获取当前会话（const版本）
	const ChatSession* currentSession() const;
	// 获取会话映射（非const版本）
	ChatSessionMap& sessionMap();
	// 获取会话映射（const版本）
	const ChatSessionMap& sessionMap() const;
	// 附加到客户端
	void attachToClient(MessageManager* client);
	// 构建会话的Markdown格式内容
	QString buildConversationMarkdown(const ChatSession& session) const;
	// 构建会话的HTML格式内容
	QString buildConversationHtml(const ChatSession& session) const;
	// 构建会话的纯文本格式内容
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
	// 连接检查失败处理
	void onConnectionCheckFailed(const QString& errorMessage);
	// 连接检查成功处理
	void onConnectionCheckSucceeded();
	// 模型获取失败处理
	void onModelsFetchFailed(const QString& errorMessage);
	// 客户端管理器错误处理
	void onClientManagerError(const QString& context, const QString& errorMessage);
	// 删除当前对话
	void deleteCurrentConversation();
	// 重命名当前对话
	void renameCurrentConversation();
	// 更改模型
	void ChangeModel(int iModel);
	// 气泡笔记改变处理
	void onBubbleNoteChanged(const QString& bubbleId, const QString& note);
	// 气泡重要性切换处理
	void onBubbleImportantToggled(const QString& bubbleId, bool isImportant);
	// 气泡折叠状态切换处理
	void onBubbleCollapsedToggled(const QString& bubbleId, bool isCollapsed);
	// 导出对话请求处理
	void onExportConversationRequested(const QString& conversationId, const QString& format);
	// 显示详情请求处理
	void onShowDetailsRequested(const QString& conversationId);
};
