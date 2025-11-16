#include "LLMFunctionCall.h"
#include <QDateTime>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include<String>
#include <QMutexLocker>
#include <QDir>
#include <QTimer>
using namespace std;

LLMFunctionCall::LLMFunctionCall()
{
	FunctionFilePath = QCoreApplication::applicationDirPath() + "/AIAssit/FunctionCall.json";
	LoadFunctionCallTool(FunctionFilePath);
	registerAllHandlers();
	setupFunctionJsonWatcher();
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
	// 使用路由器读取工具定义（解耦）
	const bool ok = m_router.loadToolsDefinition(JsonFile);
	m_tools = m_router.toolsDefinition();
	return ok;
}

void LLMFunctionCall::registerAllHandlers()
{
	// 把外部视觉平台调用器注入给 VisionTools
	VisionTools::setInvoker(m_LLMCommandFunc);

	// 演示/视觉平台相关
	m_router.registerTool(QStringLiteral("get_weather"), VisionTools::getWeather);
	m_router.registerTool(QStringLiteral("get_time"), VisionTools::getTime);
	m_router.registerTool(QStringLiteral("open_template_assistant"), VisionTools::openTemplateAssistant);
	m_router.registerTool(QStringLiteral("run_current_visionscript"), VisionTools::runCurrentScript);
	m_router.registerTool(QStringLiteral("close_vision_platform"), VisionTools::closeVisionPlatform);
	m_router.registerTool(QStringLiteral("run_vision_order"), VisionTools::runVisionOrder);
	m_router.registerTool(QStringLiteral("switch_vision_tab"), VisionTools::switchVisionTab);

	// Git 代码审查（由 GitTools 管理 GitLogReader 生命周期）
	m_router.registerTool(QStringLiteral("code_review_latest"), GitTools::codeReviewLatest);
	m_router.registerTool(QStringLiteral("code_review_commit"), GitTools::codeReviewCommit);
	m_router.registerTool(QStringLiteral("code_review_range"), GitTools::codeReviewRange);
	m_router.registerTool(QStringLiteral("code_review_working_dir"), GitTools::codeReviewWorkingDir);
	m_router.registerTool(QStringLiteral("get_commit_info"), GitTools::getCommitInfo);
	m_router.registerTool(QStringLiteral("get_repo_status"), GitTools::getRepoStatus);

	// 文件系统
	m_router.registerTool(QStringLiteral("read_file"), FileSystemTools::readFile);
	m_router.registerTool(QStringLiteral("write_file"), FileSystemTools::writeFile);
	m_router.registerTool(QStringLiteral("list_directory"), FileSystemTools::listDirectory);
	m_router.registerTool(QStringLiteral("search_in_files"), FileSystemTools::searchInFiles);
	m_router.registerTool(QStringLiteral("create_directory"), FileSystemTools::createDirectory);
	m_router.registerTool(QStringLiteral("delete_file"), FileSystemTools::deleteFile);
	m_router.registerTool(QStringLiteral("delete_directory"), FileSystemTools::deleteDirectory);
	m_router.registerTool(QStringLiteral("copy_file"), FileSystemTools::copyFile);
	m_router.registerTool(QStringLiteral("move_file"), FileSystemTools::moveFile);
	m_router.registerTool(QStringLiteral("get_file_info"), FileSystemTools::getFileInfo);
	m_router.registerTool(QStringLiteral("find_files"), FileSystemTools::findFiles);
	m_router.registerTool(QStringLiteral("count_lines"), FileSystemTools::countLines);
	m_router.registerTool(QStringLiteral("path_exists"), FileSystemTools::pathExists);
	m_router.registerTool(QStringLiteral("join_path"), FileSystemTools::joinPath);
	m_router.registerTool(QStringLiteral("normalize_path"), FileSystemTools::normalizePath);
	m_router.registerTool(QStringLiteral("get_file_name"), FileSystemTools::getFileName);
	m_router.registerTool(QStringLiteral("get_directory"), FileSystemTools::getDirectory);
	m_router.registerTool(QStringLiteral("get_file_extension"), FileSystemTools::getFileExtension);

	// 文本处理
	m_router.registerTool(QStringLiteral("text_replace"), TextProcessingTools::textReplace);
	m_router.registerTool(QStringLiteral("text_statistics"), TextProcessingTools::textStatistics);
	m_router.registerTool(QStringLiteral("regex_match"), TextProcessingTools::regexMatch);
	m_router.registerTool(QStringLiteral("text_extract"), TextProcessingTools::textExtract);
	m_router.registerTool(QStringLiteral("text_encode_decode"), TextProcessingTools::textEncodeDecode);
	m_router.registerTool(QStringLiteral("text_format"), TextProcessingTools::textFormat);

	// 剪贴板
	m_router.registerTool(QStringLiteral("get_clipboard"), ClipboardTools::getClipboard);
	m_router.registerTool(QStringLiteral("set_clipboard"), ClipboardTools::setClipboard);

	// 系统信息
	m_router.registerTool(QStringLiteral("get_system_info"), SystemInfoTools::getSystemInfo);
	m_router.registerTool(QStringLiteral("get_disk_space"), SystemInfoTools::getDiskSpace);
	m_router.registerTool(QStringLiteral("get_environment_variable"), SystemInfoTools::getEnvironmentVariable);
	m_router.registerTool(QStringLiteral("get_current_directory"), SystemInfoTools::getCurrentDirectory);
	m_router.registerTool(QStringLiteral("set_current_directory"), SystemInfoTools::setCurrentDirectory);

	// 日期时间
	m_router.registerTool(QStringLiteral("format_datetime"), DateTimeTools::formatDateTime);
	m_router.registerTool(QStringLiteral("calculate_datetime"), DateTimeTools::calculateDateTime);
	m_router.registerTool(QStringLiteral("parse_datetime"), DateTimeTools::parseDateTime);
	m_router.registerTool(QStringLiteral("get_timezone"), DateTimeTools::getTimezone);
	m_router.registerTool(QStringLiteral("time_difference"), DateTimeTools::timeDifference);

	// 实用工具
	m_router.registerTool(QStringLiteral("calculate"), UtilityTools::calculate);
	m_router.registerTool(QStringLiteral("unit_convert"), UtilityTools::unitConvert);
	m_router.registerTool(QStringLiteral("number_format"), UtilityTools::numberFormat);
	m_router.registerTool(QStringLiteral("generate_uuid"), UtilityTools::generateUuid);
	m_router.registerTool(QStringLiteral("generate_random_string"), UtilityTools::generateRandomString);
	m_router.registerTool(QStringLiteral("hash_string"), UtilityTools::hashString);
	m_router.registerTool(QStringLiteral("validate_json"), UtilityTools::validateJson);
	m_router.registerTool(QStringLiteral("validate_xml"), UtilityTools::validateXml);

	// 数据格式
	m_router.registerTool(QStringLiteral("parse_json"), DataFormatTools::parseJson);
	m_router.registerTool(QStringLiteral("format_json"), DataFormatTools::formatJson);
	m_router.registerTool(QStringLiteral("parse_csv"), DataFormatTools::parseCsv);
	m_router.registerTool(QStringLiteral("to_csv"), DataFormatTools::toCsv);
}

void LLMFunctionCall::setupFunctionJsonWatcher()
{
	m_fileWatcher = std::make_unique<QFileSystemWatcher>();
	if (!FunctionFilePath.isEmpty())
	{
		m_fileWatcher->addPath(FunctionFilePath);
	}
	connect(m_fileWatcher.get(), &QFileSystemWatcher::fileChanged, this, &LLMFunctionCall::onFunctionJsonChanged);
}

void LLMFunctionCall::onFunctionJsonChanged(const QString& path)
{
	// 某些平台会触发两次变更事件，做简单防抖
	QTimer::singleShot(200, this, [this]() {
		LoadFunctionCallTool(FunctionFilePath);
		// 变更后继续监听（有的编辑器会先删除再写入）
		if (m_fileWatcher && !m_fileWatcher->files().contains(FunctionFilePath))
		{
			m_fileWatcher->addPath(FunctionFilePath);
		}
	});
}

// ==== Legacy demo/vision methods -> delegate to VisionTools ====
QJsonObject LLMFunctionCall::getWeather(const QJsonObject &arguments)
{
	return VisionTools::getWeather(arguments);
}

QJsonObject LLMFunctionCall::getTime(const QJsonObject &arguments)
{
	return VisionTools::getTime(arguments);
}

QJsonObject LLMFunctionCall::RunVisionOrder(const QJsonObject &arguments)
{
	return VisionTools::runVisionOrder(arguments);
}

QJsonObject LLMFunctionCall::OpenModelAssit(const QJsonObject &arguments)
{
	return VisionTools::openTemplateAssistant(arguments);
}

QJsonObject LLMFunctionCall::RunCurrentScript(const QJsonObject &arguments)
{
	return VisionTools::runCurrentScript(arguments);
}

QJsonObject LLMFunctionCall::CloseVisionPlatForm(const QJsonObject &arguments)
{
	return VisionTools::closeVisionPlatform(arguments);
}

QJsonObject LLMFunctionCall::SwitchVisionTab(const QJsonObject &arguments)
{
	return VisionTools::switchVisionTab(arguments);
}

QJsonObject LLMFunctionCall::executeFunction(const QString &name, const QJsonObject &arguments)
{
	return m_router.execute(name, arguments);
}

// Legacy Git review functions have been removed (migrated to GitTools)