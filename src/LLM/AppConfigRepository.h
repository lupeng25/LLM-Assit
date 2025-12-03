#pragma once

#include <QString>

class AppConfigRepository {
public:
	// 单例模式：获取全局唯一实例
	static AppConfigRepository* instance();
	
	// 已存在的构造函数（保留用于向后兼容）
	AppConfigRepository();

	// 配置目录和文件路径
	[[nodiscard]] QString baseConfigDir() const;
	[[nodiscard]] QString chatHistoryDirectory() const;
	[[nodiscard]] QString chatHistoryFile() const;
	
	// 配置文件路径
	[[nodiscard]] QString modelConfigFile() const;
	[[nodiscard]] QString functionCallConfigFile() const;
	[[nodiscard]] QString promptLibraryFile() const;
	
	// 通用配置路径方法：根据文件名返回完整路径
	[[nodiscard]] QString configPath(const QString& fileName) const;
	
	// 确保配置目录存在
	bool ensureChatHistoryStorage() const;
	bool ensureConfigDirectory() const;

private:
	QString m_baseDir;
	
	// 单例实例
	static AppConfigRepository* s_instance;
};

