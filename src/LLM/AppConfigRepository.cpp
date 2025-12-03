#include "AppConfigRepository.h"

#include <QCoreApplication>
#include <QDate>
#include <QDir>

namespace {
constexpr auto kAppFolderName = "AIAssit";
constexpr auto kChatHistoryFolder = "LLMChatHistory";
constexpr auto kModelConfigFile = "AIModelConfig.json";
constexpr auto kFunctionCallConfigFile = "FunctionCall.json";
constexpr auto kPromptLibraryFile = "PromptLibrary.json";
}

// 单例实例初始化
AppConfigRepository* AppConfigRepository::s_instance = nullptr;

AppConfigRepository* AppConfigRepository::instance()
{
	if (!s_instance) {
		s_instance = new AppConfigRepository();
	}
	return s_instance;
}

AppConfigRepository::AppConfigRepository() 
{
	m_baseDir = QCoreApplication::applicationDirPath() + "/" + kAppFolderName;
}

QString AppConfigRepository::baseConfigDir() const
{
	return m_baseDir;
}

QString AppConfigRepository::chatHistoryDirectory() const
 {
	return m_baseDir + "/" + kChatHistoryFolder;
}

QString AppConfigRepository::chatHistoryFile() const 
{
	return chatHistoryDirectory() + "/LLMChat_" + QDate::currentDate().toString("yyyy-MM-dd") + ".json";
}

QString AppConfigRepository::modelConfigFile() const
 {
	return m_baseDir + "/" + kModelConfigFile;
}

QString AppConfigRepository::functionCallConfigFile() const
{
	return m_baseDir + "/" + kFunctionCallConfigFile;
}

QString AppConfigRepository::promptLibraryFile() const
{
	return m_baseDir + "/" + kPromptLibraryFile;
}

QString AppConfigRepository::configPath(const QString& fileName) const
{
	return m_baseDir + "/" + fileName;
}

bool AppConfigRepository::ensureChatHistoryStorage() const 
{
	QDir dir(chatHistoryDirectory());
	if (dir.exists()) {
		return true;
	}
	return dir.mkpath(".");
}

bool AppConfigRepository::ensureConfigDirectory() const
{
	QDir dir(m_baseDir);
	if (dir.exists()) {
		return true;
	}
	return dir.mkpath(".");
}

