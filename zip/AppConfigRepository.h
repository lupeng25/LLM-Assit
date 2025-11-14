#pragma once

#include <QString>

class AppConfigRepository {
public:
	AppConfigRepository();

	[[nodiscard]] QString chatHistoryDirectory() const;
	[[nodiscard]] QString chatHistoryFile() const;
	[[nodiscard]] QString modelConfigFile() const;

	bool ensureChatHistoryStorage() const;

private:
	QString m_baseDir;
};

