#include "AppConfigRepository.h"

#include <QCoreApplication>
#include <QDate>
#include <QDir>

namespace {
constexpr auto kAppFolderName = "AIAssit";
constexpr auto kChatHistoryFolder = "LLMChatHistory";
constexpr auto kModelConfigFile = "AIModelConfig.json";
}

AppConfigRepository::AppConfigRepository() {
	m_baseDir = QCoreApplication::applicationDirPath() + "/" + kAppFolderName;
}

QString AppConfigRepository::chatHistoryDirectory() const {
	return m_baseDir + "/" + kChatHistoryFolder;
}

QString AppConfigRepository::chatHistoryFile() const {
	return chatHistoryDirectory() + "/LLMChat_" + QDate::currentDate().toString("yyyy-MM-dd") + ".json";
}

QString AppConfigRepository::modelConfigFile() const {
	return m_baseDir + "/" + kModelConfigFile;
}

bool AppConfigRepository::ensureChatHistoryStorage() const {
	QDir dir(chatHistoryDirectory());
	if (dir.exists()) {
		return true;
	}
	return dir.mkpath(".");
}

