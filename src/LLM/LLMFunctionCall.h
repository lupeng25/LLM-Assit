#pragma once
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QStringList>
#include <QSet>
#include <QHash>
#include <QMutex>
#include <memory>
#include <functional>
#include "FunctionCallRouter.h"

// 前向声明：使用 std::unique_ptr 时只需要前向声明
class GitLogReader;
class QFileSystemWatcher;
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

	// Return tools filtered by enabled modules
	QJsonArray Tools();

	//执行函数
	QJsonObject executeFunction(const QString &name, const QJsonObject &arguments);
	std::function<QString(QString)> m_LLMCommandFunc;

	// Module switches
	void setEnabledModules(const QStringList& modules);
	void enableModule(const QString& module, bool enabled);
	QStringList enabledModules() const;

private:
	QString FunctionFilePath;
	QJsonArray m_tools; // 保持对外兼容（从路由器读取）
	QJsonObject m_toolTamplate;
	QJsonObject m_noneParam;
	FunctionCallRouter m_router;
	std::unique_ptr<QFileSystemWatcher> m_fileWatcher;
	QSet<QString> m_enabledModules;              // e.g. {"vision","git","fs","text","clipboard","system","datetime","utility","data"}
	QHash<QString, QString> m_functionToModule;  // function name -> module id

	// Git代码审查相关
	std::unique_ptr<GitLogReader> m_gitReader;
	QString m_currentRepoPath;
	QMutex m_gitMutex;

	//加载Functioncall的Tools
	void LoadFunctionCallTool(const QString &name, const QString &description, const QJsonObject &params); // 保留但不再使用
	bool LoadFunctionCallTool(const QString &JsonFile); // 读取 JSON 并交给路由器保存
	void registerAllHandlers(); // 注册所有工具到路由器
	void setupFunctionJsonWatcher(); // 热加载 FunctionCall.json
	void onFunctionJsonChanged(const QString& path); // 变更回调

	// Helper to register and track module ownership
	void registerTool(const QString& module, const QString& name, FunctionCallRouter::Handler handler);
	QJsonArray buildFilteredTools() const;

	// === Helpers (legacy) ===
	// QString -> QJsonObject
	QJsonObject QStringToQJsonObject(const QString &qStr);


protected:
	LLMFunctionCall();

signals:
	void loadMessages(QJsonObject);
	void codeReviewReady(QJsonObject reviewData);
	void gitOperationFailed(QString error);
};
