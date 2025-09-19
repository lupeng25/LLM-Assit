#pragma once 
#include <QNetworkAccessManager> 
#include <QPropertyAnimation>
#include <QNetworkReply> 
#include <QJsonDocument> 
#include <QElapsedTimer>
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
#include "ui_Frm_AIAssit.h" 
#include "LLMChatFrame.h"
#include "LLMFunctionCall.h"
#include "LLMParams.h"
#include "MessageManager.h"
#include "Open_WebUIClient.h"
#include "DifyClient.h"
#include "GitLogReader.h"
class Frm_AIAssit : public QWidget 
{
	Q_OBJECT

public:
	Frm_AIAssit(QWidget *parent = Q_NULLPTR);
	~Frm_AIAssit();
	bool bShowParam = false;
	QString ChatJsonFile;
	QJsonArray m_FunctionTools;
	// 初始化函数 
	void recalculateVisibleBubbles();
	void initUI();
	void initParams();
	//初始化历史文件
	void initHistoryFile();
	// 显示或隐藏参数设置界面 
	void ShowAIParam();
	// 应用当前界面的模型参数 
	void ApplyModelParam();
	//添加气泡聊天框
	void addChatBubble(const QString& text, bool bIsUser);
	//使用最新回答更新对话名
	QString updateDialogName(const QString& dialogName);
	// 发送消息到服务器 
	int send(const ChatSendMessage& msg);
	// 发送消息到流式服务器 
	int StreamSend(const ChatSendMessage& msg);
	//处理functioncall结果
	void ProcessFunctionCall(QJsonObject FunctionMsg);

	void preFuncall(QJsonObject& Content);

	void setLLMClient(AIProvider platform);

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

	QJsonObject parseJsonReplyToMsg(const QByteArray &data, bool isStream = false);

	//流式数据结束处理
	void getStreamAnswerEnd();

private:
	Ui::Frm_AIAssit ui;
	
	struct sSingleMsg {
		QString m_ChatMsg = "";
		QString m_ChatReasonMsg = "";
		QString m_ChatTime;
		QSize m_AllSize;
		int userType = 1;
		QString m_DialogName;
		QString m_BubbleID;
		sSingleMsg() = default;
		sSingleMsg(QString msg, QString time, QSize size, int type, QString dialog, QString reasoningMsg,QString id="")
			: m_ChatMsg(msg), m_ChatTime(time), m_AllSize(size), userType(type), m_DialogName(dialog), m_ChatReasonMsg(reasoningMsg),m_BubbleID(id) {}

		QJsonObject toJson() const {
			QJsonObject obj;
			obj["msg"] = m_ChatMsg;
			obj["time"] = m_ChatTime;
			obj["width"] = m_AllSize.width();
			obj["height"] = m_AllSize.height();
			obj["userType"] = userType;
			obj["dialogname"] = m_DialogName;
			obj["reasoningMsg"] = m_ChatReasonMsg;
			obj["bubbleid"] = m_BubbleID;
			return obj;
		}

		void fromJson(const QJsonObject& obj) {
			m_ChatMsg = obj["msg"].toString();
			m_ChatTime = obj["time"].toString();
			int w = obj["width"].toInt();
			int h = obj["height"].toInt();
			m_AllSize = QSize(w, h);
			userType = obj["userType"].toInt();
			m_DialogName = obj["dialogname"].toString();
			m_ChatReasonMsg = obj["reasoningMsg"].toString();
			m_BubbleID = obj["bubbleid"].toString();
		}
	};

	struct sMsgList {
		QDateTime SaveTime;
		QList<sSingleMsg> sMsg;

		QJsonObject toJson() const {
			QJsonObject obj;
			obj["saveTime"] = SaveTime.toString(Qt::ISODate);

			QJsonArray msgArray;
			for (const auto& msg : sMsg) {
				msgArray.append(msg.toJson());
			}
			obj["messages"] = msgArray;

			return obj;
		}

		void fromJson(const QJsonObject& obj) {
			SaveTime = QDateTime::fromString(obj["saveTime"].toString(), Qt::ISODate);

			QJsonArray msgArray = obj["messages"].toArray();
			sMsg.clear();
			for (const QJsonValue& val : msgArray) {
				if (val.isObject()) {
					sSingleMsg msg;
					msg.fromJson(val.toObject());
					sMsg.append(msg);
				}
			}
		}

		int GetMsgIndex(const QString& msg) {
			for (int i = 0; i < sMsg.size(); i++)
			{
				if (sMsg[i].m_BubbleID == (msg))
					return i;
			}
			return -1;
		}
	};

	QHash<QString, sMsgList> m_ChatMsg; // 对话ID -> 对话数据 
	QString m_currentConversationId; // 当前激活的对话ID 
	void recalculateAllChatBubbles();
	// 设置信号和槽连接 
	void setupSignals();
	//将对话存进Json文件
	void saveChatMapToJson(const QString& filePath, const QHash<QString, sMsgList>& chatMap);
	//读取历史文件
	bool loadChatMapFromJson();
	//设置显示对话
	void PreLoadChat();
	void toggleSidebar();
	QString GetLastestAsk(QString msg);

	MessageManager* LLMClient;
	LLMParams* params;

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
	void deleteCurrentConversation();
	void renameCurrentConversation();
	void ChangeModel(int iModel);

private:
	QTimer* m_resizeTimer = nullptr;
	// 缓存字体度量对象
	QFontMetrics* m_fontMetrics = nullptr;
	// 批量加载时的优化标志
	bool m_batchLoading = false;
};