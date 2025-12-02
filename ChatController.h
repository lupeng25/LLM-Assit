#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include "CommonTypes.h"
#include "ChatSessionTypes.h"

// 前向声明
class MessageManager;
class ChatSessionService;
class LLMFunctionCall;
class LLMParams;

/**
 * @brief ChatController - 聊天业务逻辑控制器
 * 
 * 负责处理：
 * - 消息发送（阻塞式/流式）
 * - 工具调用处理
 * - 会话管理（创建/删除/重命名/切换）
 * - 对话导出
 * 
 * 通过信号通知 UI 层进行界面更新，不直接操作 UI 组件
 */
class ChatController : public QObject
{
    Q_OBJECT

public:
    explicit ChatController(
        MessageManager* llmClient,
        ChatSessionService* sessionService,
        LLMFunctionCall* functionCall,
        LLMParams* llmParams,
        QObject* parent = nullptr
    );
    
    /**
     * @brief 更新 LLMClient（当客户端切换时调用）
     */
    void setLLMClient(MessageManager* llmClient);

    // === 消息发送 ===
    /**
     * @brief 发送阻塞式消息
     * @param msg 消息内容
     * @return 返回码（0表示成功，负数表示失败）
     */
    int sendMessage(const ChatSendMessage& msg);

    /**
     * @brief 发送流式消息
     * @param msg 消息内容
     * @return 返回码（0表示成功，负数表示失败）
     */
    int sendStreamMessage(const ChatSendMessage& msg);

    // === 会话管理 ===
    /**
     * @brief 创建新会话
     * @param nameHint 会话名称提示（可选）
     * @return 新会话的 ID
     */
    QString createNewSession(const QString& nameHint = QString());

    /**
     * @brief 删除会话
     * @param sessionId 会话 ID
     */
    void deleteSession(const QString& sessionId);

    /**
     * @brief 重命名会话
     * @param sessionId 会话 ID
     * @param newName 新名称
     */
    void renameSession(const QString& sessionId, const QString& newName);

    /**
     * @brief 切换到指定会话
     * @param sessionId 会话 ID
     */
    void switchToSession(const QString& sessionId);

    /**
     * @brief 获取当前会话 ID
     */
    QString currentSessionId() const { return m_currentSessionId; }

    /**
     * @brief 设置当前会话 ID
     */
    void setCurrentSessionId(const QString& sessionId);

    // === 工具调用 ===
    /**
     * @brief 处理工具调用
     * @param toolCall 工具调用 JSON 对象
     */
    void processToolCall(const QJsonObject& toolCall);

    // === 对话导出 ===
    /**
     * @brief 导出对话为 Markdown 格式
     * @param sessionId 会话 ID
     * @return Markdown 格式的对话内容
     */
    QString exportToMarkdown(const QString& sessionId) const;

    /**
     * @brief 导出对话为 HTML 格式
     * @param sessionId 会话 ID
     * @return HTML 格式的对话内容
     */
    QString exportToHtml(const QString& sessionId) const;

    /**
     * @brief 导出对话为纯文本格式
     * @param sessionId 会话 ID
     * @return 纯文本格式的对话内容
     */
    QString exportToPlainText(const QString& sessionId) const;

    /**
     * @brief 更新对话名称（基于最新回答内容）
     * @param sessionId 会话 ID
     * @param answerText 回答文本
     * @return 更新后的对话名称
     */
    QString updateDialogNameFromAnswer(const QString& sessionId, const QString& answerText);

    /**
     * @brief 获取指定会话
     */
    const ChatSession* getSession(const QString& sessionId) const;

    /**
     * @brief 向当前会话添加消息
     */
    void appendMessageToSession(const QString& sessionId, const ChatMessageData& message);

signals:
    // === UI 更新信号 ===
    /**
     * @brief 需要添加用户消息气泡
     * @param text 消息文本
     */
    void needAddUserBubble(const QString& text);

    /**
     * @brief 需要刷新会话列表
     */
    void needRefreshSessionList();

    /**
     * @brief 需要切换到指定会话
     * @param sessionId 会话 ID
     */
    void needSwitchToSession(const QString& sessionId);

    /**
     * @brief 需要清空聊天区域
     */
    void needClearChatArea();

    /**
     * @brief 需要更新对话名称
     * @param sessionId 会话 ID
     * @param newName 新名称
     */
    void needUpdateDialogName(const QString& sessionId, const QString& newName);

    /**
     * @brief 需要导出对话
     * @param sessionId 会话 ID
     * @param format 导出格式（"markdown", "html", "txt"）
     * @param content 导出的内容
     */
    void needExportConversation(const QString& sessionId, const QString& format, const QString& content);

    /**
     * @brief 工具调用结果已准备好
     * @param result 工具调用结果文本
     */
    void toolCallResultReady(const QString& result);

    /**
     * @brief 工具调用出错
     * @param error 错误信息
     */
    void toolCallError(const QString& error);

public slots:
	/**
	 * @brief 处理工具调用信号（从 MessageManager 接收）
	 * 注意：参数必须是非 const 引用，因为 MessageManager::FunctionCallSignal 传递的是非 const 引用
	 */
	void onToolCallReceived(QJsonObject& toolCall);

private:
    /**
     * @brief 工具调用预处理（解析并执行工具）
     */
    void preProcessToolCall(QJsonObject& content);

    /**
     * @brief 执行工具调用并发送结果
     */
    void executeToolCall(const QJsonObject& toolCall);

    /**
     * @brief 解析 JSON 回复为消息对象
     */
    QJsonObject parseJsonReplyToMsg(const QByteArray& data, bool isStream = false);

    /**
     * @brief 构建对话导出内容（内部辅助方法）
     */
    QString buildExportContent(const ChatSession& session, const QString& format) const;

    // 依赖注入的组件
    MessageManager* m_llmClient;
    ChatSessionService* m_sessionService;
    LLMFunctionCall* m_functionCall;
    LLMParams* m_llmParams;

    // 状态
    QString m_currentSessionId;
    QJsonArray m_functionTools;  // 当前可用的工具列表
};

