#pragma once
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QObject>
#include <QDebug>
#include <QList>
#include <QMutex>
#include <thread>
#include <memory>
#include "GitLogReader.h"
class LLMFunctionCall : public QObject
{
	Q_OBJECT
public:
	static LLMFunctionCall* Get() 
	{
		static LLMFunctionCall ft;
		return &ft;
	}

	~LLMFunctionCall();

	QJsonArray Tools() { return m_tools; }

	//执行函数
	QJsonObject executeFunction(const QString &name, const QJsonObject &arguments);
	std::function<QString(QString)> m_LLMCommandFunc;
private:
	QString FunctionFilePath;
	QJsonArray m_tools;
	QJsonObject m_toolTamplate;
	QJsonObject m_noneParam;

	// Git代码审查相关
	std::unique_ptr<GitLogReader> m_gitReader;
	QString m_currentRepoPath;
	QMutex m_gitMutex;

	//加载Functioncall的Tools
	void LoadFunctionCallTool(const QString &name, const QString &description, const QJsonObject &params);
	bool LoadFunctionCallTool(const QString &JsonFile);
	
	// ==FunctionCall测试函数 ==
	//获取天气
	QJsonObject getWeather(const QJsonObject &arguments);
	//获取当前时间
	QJsonObject getTime(const QJsonObject &arguments = QJsonObject());
	// ===视觉平台功能函数 ===
	//执行视觉命令
	QJsonObject RunVisionOrder(const QJsonObject &arguments);
	//打开模板助手
	QJsonObject OpenModelAssit(const QJsonObject &arguments);
	//执行视觉平台当前脚本
	QJsonObject RunCurrentScript(const QJsonObject &arguments);
	//关闭视觉平台
	QJsonObject CloseVisionPlatForm(const QJsonObject &arguments);
	//切换视觉平台脚本页
	QJsonObject SwitchVisionTab(const QJsonObject &arguments);

	// === Git代码审查功能函数 ===
	//对最新的Git提交进行AI代码审查
	QJsonObject CodeReviewLatest(const QJsonObject &arguments);
	//对指定的Git提交进行AI代码审查
	QJsonObject CodeReviewCommit(const QJsonObject &arguments);
	//对Git提交范围进行AI代码审查，比较两个提交之间的所有变更
	QJsonObject CodeReviewRange(const QJsonObject &arguments);
	//对当前工作目录的未提交变更进行AI代码审查
	QJsonObject CodeReviewWorkingDir(const QJsonObject &arguments);
	//获取Git提交的详细信息
	QJsonObject GetCommitInfo(const QJsonObject &arguments);
	//获取Git仓库的状态信息
	QJsonObject GetRepoStatus(const QJsonObject &arguments);

	// == 选型助手相关功能 ==
	//通过选型助手获取选型结果
	QJsonObject VisionSelectByAssit(const QJsonObject &arguments);
	// === 辅助方法 ===
	//字符串转json
	QJsonObject QStringToQJsonObject(const QString &qStr);
	//Git数据格式化
	QJsonObject formatCodeReviewResponse(const CodeReviewData& reviewData, const QString& reviewType);
	//处理Git异常
	QJsonObject handleGitError(const QString& operation, const std::exception& e);
	//Git数据转Json
	QString codeReviewDataToJson(const CodeReviewData& reviewData);


protected:
	LLMFunctionCall();

signals:
	void loadMessages(QJsonObject);
	void codeReviewReady(QJsonObject reviewData);
	void gitOperationFailed(QString error);
};