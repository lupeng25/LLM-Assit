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
	// default: enable all modules
	m_enabledModules = QSet<QString>::fromList({
		QStringLiteral("vision"), QStringLiteral("git"), QStringLiteral("fs"), QStringLiteral("text"),
		QStringLiteral("clipboard"), QStringLiteral("system"), QStringLiteral("datetime"),
		QStringLiteral("utility"), QStringLiteral("data")
	});
	LoadFunctionCallTool(FunctionFilePath);
	registerAllHandlers();
	setupFunctionJsonWatcher();
	m_currentRepoPath = QDir::currentPath();
	m_gitReader = std::make_unique<GitLogReader>(m_currentRepoPath.isEmpty() ? "" : m_currentRepoPath.toStdString());
}

LLMFunctionCall::~LLMFunctionCall()
{

}

QJsonArray LLMFunctionCall::Tools()
{
	return buildFilteredTools();
}

QStringList LLMFunctionCall::enabledModules() const
{
	return m_enabledModules.values();
}

void LLMFunctionCall::setEnabledModules(const QStringList& modules)
{
	m_enabledModules = QSet<QString>(modules.begin(), modules.end());
}

void LLMFunctionCall::enableModule(const QString& module, bool enabled)
{
	if (enabled) m_enabledModules.insert(module);
	else m_enabledModules.remove(module);
}

QJsonArray LLMFunctionCall::buildFilteredTools() const
{
	QJsonArray all = m_router.toolsDefinition();
	if (m_enabledModules.isEmpty()) return QJsonArray();
	QJsonArray filtered;
	for (const QJsonValue& v : all)
	{
		QJsonObject obj = v.toObject();
		QJsonObject fn = obj.value("function").toObject();
		const QString name = fn.value("name").toString();
		const QString module = m_functionToModule.value(name);
		if (module.isEmpty() || m_enabledModules.contains(module))
		{
			filtered.append(obj);
		}
	}
	return filtered;
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

void LLMFunctionCall::registerTool(const QString& module, const QString& name, FunctionCallRouter::Handler handler)
{
	m_functionToModule.insert(name, module);
	m_router.registerTool(name, handler);
}

void LLMFunctionCall::registerAllHandlers()
{
	// Inject external invoker into VisionTools
	VisionTools::setInvoker(m_LLMCommandFunc);

	// Vision/Demo
	registerTool("vision", "get_weather", VisionTools::getWeather);
	registerTool("vision", "get_time", VisionTools::getTime);
	registerTool("vision", "open_template_assistant", VisionTools::openTemplateAssistant);
	registerTool("vision", "run_current_visionscript", VisionTools::runCurrentScript);
	registerTool("vision", "close_vision_platform", VisionTools::closeVisionPlatform);
	registerTool("vision", "run_vision_order", VisionTools::runVisionOrder);
	registerTool("vision", "switch_vision_tab", VisionTools::switchVisionTab);

	// Git
	registerTool("git", "code_review_latest", GitTools::codeReviewLatest);
	registerTool("git", "code_review_commit", GitTools::codeReviewCommit);
	registerTool("git", "code_review_range", GitTools::codeReviewRange);
	registerTool("git", "code_review_working_dir", GitTools::codeReviewWorkingDir);
	registerTool("git", "get_commit_info", GitTools::getCommitInfo);
	registerTool("git", "get_repo_status", GitTools::getRepoStatus);

	// File system
	registerTool("fs", "read_file", FileSystemTools::readFile);
	registerTool("fs", "write_file", FileSystemTools::writeFile);
	registerTool("fs", "list_directory", FileSystemTools::listDirectory);
	registerTool("fs", "search_in_files", FileSystemTools::searchInFiles);
	registerTool("fs", "create_directory", FileSystemTools::createDirectory);
	registerTool("fs", "delete_file", FileSystemTools::deleteFile);
	registerTool("fs", "delete_directory", FileSystemTools::deleteDirectory);
	registerTool("fs", "copy_file", FileSystemTools::copyFile);
	registerTool("fs", "move_file", FileSystemTools::moveFile);
	registerTool("fs", "get_file_info", FileSystemTools::getFileInfo);
	registerTool("fs", "find_files", FileSystemTools::findFiles);
	registerTool("fs", "count_lines", FileSystemTools::countLines);
	registerTool("fs", "path_exists", FileSystemTools::pathExists);
	registerTool("fs", "join_path", FileSystemTools::joinPath);
	registerTool("fs", "normalize_path", FileSystemTools::normalizePath);
	registerTool("fs", "get_file_name", FileSystemTools::getFileName);
	registerTool("fs", "get_directory", FileSystemTools::getDirectory);
	registerTool("fs", "get_file_extension", FileSystemTools::getFileExtension);

	// Text
	registerTool("text", "text_replace", TextProcessingTools::textReplace);
	registerTool("text", "text_statistics", TextProcessingTools::textStatistics);
	registerTool("text", "regex_match", TextProcessingTools::regexMatch);
	registerTool("text", "text_extract", TextProcessingTools::textExtract);
	registerTool("text", "text_encode_decode", TextProcessingTools::textEncodeDecode);
	registerTool("text", "text_format", TextProcessingTools::textFormat);

	// Clipboard
	registerTool("clipboard", "get_clipboard", ClipboardTools::getClipboard);
	registerTool("clipboard", "set_clipboard", ClipboardTools::setClipboard);

	// System
	registerTool("system", "get_system_info", SystemInfoTools::getSystemInfo);
	registerTool("system", "get_disk_space", SystemInfoTools::getDiskSpace);
	registerTool("system", "get_environment_variable", SystemInfoTools::getEnvironmentVariable);
	registerTool("system", "get_current_directory", SystemInfoTools::getCurrentDirectory);
	registerTool("system", "set_current_directory", SystemInfoTools::setCurrentDirectory);

	// Datetime
	registerTool("datetime", "format_datetime", DateTimeTools::formatDateTime);
	registerTool("datetime", "calculate_datetime", DateTimeTools::calculateDateTime);
	registerTool("datetime", "parse_datetime", DateTimeTools::parseDateTime);
	registerTool("datetime", "get_timezone", DateTimeTools::getTimezone);
	registerTool("datetime", "time_difference", DateTimeTools::timeDifference);

	// Utility
	registerTool("utility", "calculate", UtilityTools::calculate);
	registerTool("utility", "unit_convert", UtilityTools::unitConvert);
	registerTool("utility", "number_format", UtilityTools::numberFormat);
	registerTool("utility", "generate_uuid", UtilityTools::generateUuid);
	registerTool("utility", "generate_random_string", UtilityTools::generateRandomString);
	registerTool("utility", "hash_string", UtilityTools::hashString);
	registerTool("utility", "validate_json", UtilityTools::validateJson);
	registerTool("utility", "validate_xml", UtilityTools::validateXml);

	// Data format
	registerTool("data", "parse_json", DataFormatTools::parseJson);
	registerTool("data", "format_json", DataFormatTools::formatJson);
	registerTool("data", "parse_csv", DataFormatTools::parseCsv);
	registerTool("data", "to_csv", DataFormatTools::toCsv);
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
	// Some editors trigger multiple change events; debounce it
	QTimer::singleShot(200, this, [this]() {
		LoadFunctionCallTool(FunctionFilePath);
		// Re-add watch if the file was recreated
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