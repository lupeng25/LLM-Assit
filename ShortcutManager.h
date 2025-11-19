#pragma once

#include <QObject>
#include <QShortcut>
#include <QKeySequence>
#include <QMap>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QPointer>

class ShortcutManager : public QObject
{
	Q_OBJECT

public:
	enum ShortcutAction
	{
		NewConversation,      // Ctrl+N
		SearchConversations,  // Ctrl+K
		ToggleSidebar,        // Ctrl+B
		FocusInput,           // Ctrl+L
		SendMessage,          // Ctrl+Enter
		StopGeneration,       // Ctrl+.
		Settings,             // Ctrl+,
		ClearCurrentChat,     // Ctrl+Shift+D
		ExportConversation,   // Ctrl+E
		DeleteConversation    // Delete
	};

	static ShortcutManager* instance();

	// 注册快捷键
	void registerShortcut(ShortcutAction action, QWidget* parent);

	// 设置快捷键
	void setShortcut(ShortcutAction action, const QKeySequence& sequence);

	// 获取快捷键
	QKeySequence getShortcut(ShortcutAction action) const;

	// 获取默认快捷键
	QKeySequence getDefaultShortcut(ShortcutAction action) const;

	// 获取快捷键描述
	QString getShortcutDescription(ShortcutAction action) const;

	// 获取所有快捷键
	QMap<ShortcutAction, QKeySequence> getAllShortcuts() const;

	// 重置为默认值
	void resetToDefaults();

	// 保存配置
	bool saveConfig();

	// 加载配置
	bool loadConfig();

	// 获取配置文件路径
	QString getConfigPath() const;

	// 清理所有快捷键对象（用于插件卸载时）
	void clearShortcuts();

signals:
	void shortcutTriggered(ShortcutAction action);
	void shortcutsChanged();

private:
	explicit ShortcutManager(QObject* parent = nullptr);
	~ShortcutManager();

	void initializeDefaults();
	QString actionToString(ShortcutAction action) const;
	ShortcutAction stringToAction(const QString& str) const;

	QMap<ShortcutAction, QKeySequence> m_shortcuts;
	QMap<ShortcutAction, QKeySequence> m_defaultShortcuts;
	QMap<ShortcutAction, QString> m_descriptions;
	QMap<ShortcutAction, QPointer<QShortcut>> m_shortcutObjects;

	static ShortcutManager* s_instance;
};

