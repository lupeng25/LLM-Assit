#include "LLMFunctionCall.h"
#include <QDateTime>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include<String>
#include <QMutexLocker>
#include <QDir>
using namespace std;

LLMFunctionCall::LLMFunctionCall()
{
	FunctionFilePath = QCoreApplication::applicationDirPath() + "/AIAssit/FunctionCall.json";
	LoadFunctionCallTool(FunctionFilePath);
	m_currentRepoPath = QDir::currentPath();
	m_gitReader = std::make_unique<GitLogReader>(m_currentRepoPath.isEmpty() ? "" : m_currentRepoPath.toStdString());
}

LLMFunctionCall::~LLMFunctionCall()
{

}

QJsonObject LLMFunctionCall::QStringToQJsonObject(const QString &qStr)
{
	QJsonDocument doc = QJsonDocument::fromJson(qStr.toUtf8());
	if (doc.isNull())
	{
		return QJsonObject();
	}
	return doc.object();
}

void LLMFunctionCall::LoadFunctionCallTool(const QString &name, const QString &description, const QJsonObject &params)
{
	QJsonObject function;
	function["name"] = name;
	function["description"] = description;
	function["parameters"] = params;
	m_toolTamplate["function"] = function;
	m_tools.append(m_toolTamplate);
}

bool LLMFunctionCall::LoadFunctionCallTool(const QString &JsonFile)
{
	QFile FunctionJsonFile(JsonFile);
	if (!FunctionJsonFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return false;
	}

	QJsonParseError JsonErrorCode;
	QJsonDocument JsonDoc = QJsonDocument::fromJson(FunctionJsonFile.readAll(), &JsonErrorCode);
	FunctionJsonFile.close();
	if (JsonErrorCode.error != QJsonParseError::NoError || !JsonDoc.isObject())
	{
		return false;
	}

	QJsonObject FunctionJsonData = JsonDoc.object();
	if (FunctionJsonData.contains("tools") && FunctionJsonData["tools"].isArray())
	{
		QJsonArray toolsArray = FunctionJsonData["tools"].toArray();
		m_tools = toolsArray;
	}
}

QJsonObject LLMFunctionCall::getWeather(const QJsonObject &arguments)
{
	QString city = arguments["city"].toString();
	QJsonObject result;
	if (QStringLiteral("苏州") == city)
	{
		result["weather"] = QStringLiteral("晴天");
		result["temperature"] = QStringLiteral("19℃");
	}
	else if (QStringLiteral("杭州") == city)
	{
		result["weather"] = QStringLiteral("晴天");
		result["temperature"] = QStringLiteral("21℃");
	}
	else if (QStringLiteral("北京") == city)
	{
		result["weather"] = QStringLiteral("阴天");
		result["temperature"] = QStringLiteral("18℃");
	}
	else {
		result = QJsonObject();
	}
	return result;
}

QJsonObject LLMFunctionCall::getTime(const QJsonObject &arguments)
{
	// 获取当前的日期和时间
	QDateTime currentDateTime = QDateTime::currentDateTime();
	QJsonObject result;
	result["NowTime"] = currentDateTime.toString();
	return result;
}

QJsonObject LLMFunctionCall::RunVisionOrder(const QJsonObject &arguments)
{
	QString ScriptName = arguments["name"].toString();
	return QJsonObject();
}

QJsonObject LLMFunctionCall::OpenModelAssit(const QJsonObject &arguments)
{
	std::string reslt;
	QJsonObject result;
	result["result"] = QStringLiteral("打开模板助手成功");
	return result;
}

QJsonObject LLMFunctionCall::RunCurrentScript(const QJsonObject &arguments)
{
	std::string reslt;
	QJsonObject result;
	result["result"] = QStringLiteral("执行当前视觉脚本成功");
	return result;
}

QJsonObject LLMFunctionCall::CloseVisionPlatForm(const QJsonObject &arguments)
{
	std::string reslt;
	QJsonObject result;
	result["result"] = QStringLiteral("视觉平台已退出");
	return result;
}

QJsonObject LLMFunctionCall::SwitchVisionTab(const QJsonObject &arguments)
{
	QString ScriptName = arguments["TabName"].toString();
	std::string reslt;
	QJsonObject result;
	result["result"] = QStringLiteral("切换视觉脚本页面成功");
	return result;
}

QJsonObject LLMFunctionCall::executeFunction(const QString &name, const QJsonObject &arguments)
{
	if (name == "get_weather")
		return getWeather(arguments);
	else if (name == "get_time")
		return getTime(arguments);
	else if (name == "open_template_assistant")
		return OpenModelAssit(arguments);
	else if (name == "run_current_visionscript")
		return RunCurrentScript(arguments);
	else if (name == "close_vision_platform")
		return CloseVisionPlatForm(arguments);
	else if (name == "code_review_latest")
		return CodeReviewLatest(arguments);
	else if (name == "code_review_commit")
		return CodeReviewCommit(arguments);
	else if (name == "code_review_range")
		return CodeReviewRange(arguments);
	else if (name == "code_review_working_dir")
		return CodeReviewWorkingDir(arguments);
	else if (name == "get_commit_info")
		return GetCommitInfo(arguments);
	else if (name == "get_repo_status")
		return GetRepoStatus(arguments);
	else
		return QJsonObject();
}

QJsonObject LLMFunctionCall::CodeReviewLatest(const QJsonObject &arguments)
{
	QMutexLocker locker(&m_gitMutex);

	try
	{
		// 检查是否指定了仓库路径
		if (arguments.contains("repo_path"))
		{
			QString repoPath = arguments["repo_path"].toString();
			repoPath.replace("think","");
			if (!repoPath.isEmpty() && repoPath != m_currentRepoPath) 
			{
				if (m_currentRepoPath != repoPath)
				{
					m_currentRepoPath = repoPath;
					m_gitReader = std::make_unique<GitLogReader>(repoPath.isEmpty() ? "" : repoPath.toStdString());
				}
			}
		}

		if (!m_gitReader) 
		{
			return handleGitError("CodeReviewLatest", std::runtime_error("Git阅读器未初始化"));
		}

		// 获取最新提交的代码审查数据
		CodeReviewData reviewData = m_gitReader->getLastCommitReviewData();

		if (reviewData.isEmpty()) 
		{
			QJsonObject result;
			result["success"] = false;
			result["message"] = "没有找到可审查的代码变更";
			return result;
		}

		// 格式化审查响应
		QJsonObject result = formatCodeReviewResponse(reviewData, "latest_commit");

		// 发出信号通知有新的代码审查数据
		emit codeReviewReady(result);

		return result;
	}
	catch (const std::exception& e) 
	{
		return handleGitError("CodeReviewLatest", e);
	}
}

QJsonObject LLMFunctionCall::CodeReviewCommit(const QJsonObject &arguments)
{
	QMutexLocker locker(&m_gitMutex);

	try 
	{
		QString commitHash = arguments["commit_hash"].toString();
		if (commitHash.isEmpty()) 
		{
			QJsonObject result;
			result["success"] = false;
			result["message"] = "需要提供提交哈希值";
			return result;
		}

		if (!m_gitReader) 
		{
			return handleGitError("CodeReviewCommit", std::runtime_error("Git阅读器未初始化"));
		}

		// 获取指定提交的代码审查数据
		CodeReviewData reviewData = m_gitReader->getCodeReviewData(commitHash.toStdString());

		if (reviewData.isEmpty()) 
		{
			QJsonObject result;
			result["success"] = false;
			result["message"] = QString("提交 %1 没有找到可审查的代码变更").arg(commitHash);
			return result;
		}

		return formatCodeReviewResponse(reviewData, "specific_commit");
	}
	catch (const std::exception& e) 
	{
		return handleGitError("CodeReviewCommit", e);
	}
}

QJsonObject LLMFunctionCall::CodeReviewRange(const QJsonObject &arguments)
{
	QMutexLocker locker(&m_gitMutex);

	try 
	{
		QString fromCommit = arguments["from_commit"].toString();
		QString toCommit = arguments["to_commit"].toString();

		if (fromCommit.isEmpty() || toCommit.isEmpty()) 
		{
			QJsonObject result;
			result["success"] = false;
			result["message"] = "需要提供起始和结束提交哈希值";
			return result;
		}

		if (!m_gitReader) 
		{
			return handleGitError("CodeReviewRange", std::runtime_error("Git阅读器未初始化"));
		}

		// 获取提交范围的代码审查数据
		CodeReviewData reviewData = m_gitReader->getRangeReviewData(fromCommit.toStdString(), toCommit.toStdString());

		if (reviewData.isEmpty())
		{
			QJsonObject result;
			result["success"] = false;
			result["message"] = QString("提交范围 %1..%2 没有找到可审查的代码变更").arg(fromCommit, toCommit);
			return result;
		}

		return formatCodeReviewResponse(reviewData, "commit_range");
	}
	catch (const std::exception& e)
	{
		return handleGitError("CodeReviewRange", e);
	}
}

QJsonObject LLMFunctionCall::CodeReviewWorkingDir(const QJsonObject &arguments)
{
	QMutexLocker locker(&m_gitMutex);

	try 
	{
		if (!m_gitReader) 
		{
			return handleGitError("CodeReviewWorkingDir", std::runtime_error("Git阅读器未初始化"));
		}

		// 获取工作目录的代码审查数据
		CodeReviewData reviewData = m_gitReader->getWorkingDirectoryReviewData();

		if (reviewData.isEmpty()) 
		{
			QJsonObject result;
			result["success"] = false;
			result["message"] = "工作目录没有发现代码变更";
			return result;
		}

		return formatCodeReviewResponse(reviewData, "working_directory");
	}
	catch (const std::exception& e) 
	{
		return handleGitError("CodeReviewWorkingDir", e);
	}
}

QJsonObject LLMFunctionCall::GetCommitInfo(const QJsonObject &arguments)
{
	QMutexLocker locker(&m_gitMutex);

	try 
	{
		QString commitHash = arguments["commit_hash"].toString();

		if (!m_gitReader) 
		{
			return handleGitError("GetCommitInfo", std::runtime_error("Git阅读器未初始化"));
		}

		GitCommit commit;
		if (commitHash.isEmpty()) 
		{
			// 获取最新提交信息
			auto commits = m_gitReader->getCommits(1);
			if (!commits.empty())
			{
				commit = commits[0];
			}
		}
		else 
		{
			// 获取指定提交信息
			commit = m_gitReader->getCommit(commitHash.toStdString());
		}

		QJsonObject result;
		result["success"] = true;
		result["commit_hash"] = QString::fromStdString(commit.hash);
		result["author"] = QString::fromStdString(commit.author);
		result["email"] = QString::fromStdString(commit.email);
		result["date"] = QString::fromStdString(commit.date);
		result["message"] = QString::fromStdString(commit.message);
		result["branch"] = QString::fromStdString(commit.branch);

		return result;
	}
	catch (const std::exception& e) 
	{
		return handleGitError("GetCommitInfo", e);
	}
}

QJsonObject LLMFunctionCall::GetRepoStatus(const QJsonObject &arguments)
{
	QMutexLocker locker(&m_gitMutex);
	try 
	{
		if (!m_gitReader) 
		{
			return handleGitError("GetRepoStatus", std::runtime_error("Git阅读器未初始化"));
		}

		RepoInfo repoInfo = m_gitReader->getRepoInfo();

		QJsonObject result;
		result["success"] = true;
		result["current_branch"] = QString::fromStdString(repoInfo.currentBranch);
		result["last_commit_hash"] = QString::fromStdString(repoInfo.lastCommitHash);
		result["has_uncommitted_changes"] = repoInfo.hasUncommittedChanges;
		result["total_commits"] = repoInfo.totalCommits;
		result["repo_path"] = m_currentRepoPath;

		QJsonArray branchesArray;
		for (const auto& branch : repoInfo.branches) 
		{
			branchesArray.append(QString::fromStdString(branch));
		}
		result["branches"] = branchesArray;

		return result;
	}
	catch (const std::exception& e) 
	{
		return handleGitError("GetRepoStatus", e);
	}
}

QJsonObject LLMFunctionCall::VisionSelectByAssit(const QJsonObject &arguments)
{
	return QJsonObject{};
}

QJsonObject LLMFunctionCall::formatCodeReviewResponse(const CodeReviewData& reviewData, const QString& reviewType)
{
	QJsonObject result;
	result["success"] = true;
	result["review_type"] = reviewType;
	result["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

	// 基本信息
	QJsonObject commitInfo;
	commitInfo["hash"] = QString::fromStdString(reviewData.commitHash);
	commitInfo["range"] = QString::fromStdString(reviewData.commitRange);
	commitInfo["author"] = QString::fromStdString(reviewData.commitInfo.author);
	commitInfo["email"] = QString::fromStdString(reviewData.commitInfo.email);
	commitInfo["date"] = QString::fromStdString(reviewData.commitInfo.date);
	commitInfo["message"] = QString::fromStdString(reviewData.commitInfo.message);
	result["commit_info"] = commitInfo;

	// 统计信息
	QJsonObject stats;
	stats["total_files_changed"] = reviewData.totalFilesChanged;
	stats["code_files_changed"] = reviewData.codeFilesChanged;
	stats["total_additions"] = reviewData.totalAdditions;
	stats["total_deletions"] = reviewData.totalDeletions;
	stats["net_changes"] = reviewData.totalAdditions - reviewData.totalDeletions;
	result["statistics"] = stats;

	// 主要变更文件
	QJsonArray mainFiles;
	for (const auto& file : reviewData.mainChangedFiles)
	{
		QJsonObject fileObj;
		fileObj["filename"] = QString::fromStdString(file.filename);
		fileObj["language"] = QString::fromStdString(file.language);
		fileObj["status"] = QString::fromStdString(file.status);
		fileObj["additions"] = file.additions;
		fileObj["deletions"] = file.deletions;
		fileObj["total_changes"] = file.totalChanges();
		mainFiles.append(fileObj);
	}
	result["main_changed_files"] = mainFiles;

	// LLM格式化的审查文本
	result["llm_review_text"] = QString::fromStdString(reviewData.reviewText);
	result["formatted_for_llm"] = QString::fromStdString(m_gitReader->formatForLLM(reviewData, true));

	return result;
}

QJsonObject LLMFunctionCall::handleGitError(const QString& operation, const std::exception& e)
{
	QString errorMsg = QString("Git操作失败 [%1]: %2").arg(operation, e.what());
	qDebug() << errorMsg;

	emit gitOperationFailed(errorMsg);

	QJsonObject result;
	result["success"] = false;
	result["error"] = errorMsg;
	result["operation"] = operation;
	result["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

	return result;
}

QString LLMFunctionCall::codeReviewDataToJson(const CodeReviewData& reviewData)
{
	QJsonObject reviewObj = formatCodeReviewResponse(reviewData, "data_export");
	QJsonDocument doc(reviewObj);
	return doc.toJson(QJsonDocument::Compact);
}