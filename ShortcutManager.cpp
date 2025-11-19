#include "ShortcutManager.h"
#include <QApplication>

ShortcutManager* ShortcutManager::s_instance = nullptr;

ShortcutManager* ShortcutManager::instance()
{
	if (!s_instance)
	{
		s_instance = new ShortcutManager(qApp);
	}
	return s_instance;
}

ShortcutManager::ShortcutManager(QObject* parent)
	: QObject(parent)
{
	initializeDefaults();
	loadConfig();
}

ShortcutManager::~ShortcutManager()
{
	saveConfig();
	clearShortcuts();
}

void ShortcutManager::clearShortcuts()
{
	// 清理所有快捷键对象
	for (auto it = m_shortcutObjects.begin(); it != m_shortcutObjects.end(); ++it)
	{
		QPointer<QShortcut> shortcut = it.value();
		if (shortcut)
		{
			shortcut->deleteLater();
		}
	}
	m_shortcutObjects.clear();
}

void ShortcutManager::initializeDefaults()
{
	// 设置默认快捷键
	m_defaultShortcuts[NewConversation] = QKeySequence("Ctrl+N");
	m_defaultShortcuts[SearchConversations] = QKeySequence("Ctrl+K");
	m_defaultShortcuts[ToggleSidebar] = QKeySequence("Ctrl+B");
	m_defaultShortcuts[FocusInput] = QKeySequence("Ctrl+L");
	m_defaultShortcuts[SendMessage] = QKeySequence("Ctrl+Return");
	m_defaultShortcuts[StopGeneration] = QKeySequence("Ctrl+.");
	m_defaultShortcuts[Settings] = QKeySequence("Ctrl+,");
	m_defaultShortcuts[ClearCurrentChat] = QKeySequence("Ctrl+Shift+D");
	m_defaultShortcuts[ExportConversation] = QKeySequence("Ctrl+E");
	m_defaultShortcuts[DeleteConversation] = QKeySequence("Delete");

	// 设置描述
	m_descriptions[NewConversation] = tr("New Conversation");
	m_descriptions[SearchConversations] = tr("Search Conversations");
	m_descriptions[ToggleSidebar] = tr("Toggle Sidebar");
	m_descriptions[FocusInput] = tr("Focus Input");
	m_descriptions[SendMessage] = tr("Send Message");
	m_descriptions[StopGeneration] = tr("Stop Generation");
	m_descriptions[Settings] = tr("Open Settings");
	m_descriptions[ClearCurrentChat] = tr("Clear Current Chat");
	m_descriptions[ExportConversation] = tr("Export Conversation");
	m_descriptions[DeleteConversation] = tr("Delete Conversation");

	// 初始化当前快捷键为默认值
	m_shortcuts = m_defaultShortcuts;
}

void ShortcutManager::registerShortcut(ShortcutAction action, QWidget* parent)
{
	// 如果已存在且对象仍然有效，先删除
	if (m_shortcutObjects.contains(action))
	{
		QPointer<QShortcut> existing = m_shortcutObjects[action];
		if (existing)
		{
			existing->deleteLater(); // 使用 deleteLater 更安全
		}
		m_shortcutObjects.remove(action); // 从 map 中移除
	}

	QShortcut* shortcut = new QShortcut(m_shortcuts[action], parent);
	shortcut->setContext(Qt::ApplicationShortcut);

	connect(shortcut, &QShortcut::activated, this, [this, action]() {
		emit shortcutTriggered(action);
	});

	m_shortcutObjects[action] = QPointer<QShortcut>(shortcut);
}

void ShortcutManager::setShortcut(ShortcutAction action, const QKeySequence& sequence)
{
	if (m_shortcuts[action] == sequence)
	{
		return;
	}

	m_shortcuts[action] = sequence;

	// 更新已注册的快捷键对象
	if (m_shortcutObjects.contains(action))
	{
		QPointer<QShortcut> shortcut = m_shortcutObjects[action];
		if (shortcut)
		{
			shortcut->setKey(sequence);
		}
		else
		{
			// 对象已被删除，从 map 中移除
			m_shortcutObjects.remove(action);
		}
	}

	emit shortcutsChanged();
}

QKeySequence ShortcutManager::getShortcut(ShortcutAction action) const
{
	return m_shortcuts.value(action, m_defaultShortcuts.value(action));
}

QKeySequence ShortcutManager::getDefaultShortcut(ShortcutAction action) const
{
	return m_defaultShortcuts.value(action);
}

QString ShortcutManager::getShortcutDescription(ShortcutAction action) const
{
	return m_descriptions.value(action, QString());
}

QMap<ShortcutManager::ShortcutAction, QKeySequence> ShortcutManager::getAllShortcuts() const
{
	return m_shortcuts;
}

void ShortcutManager::resetToDefaults()
{
	m_shortcuts = m_defaultShortcuts;

	// 更新所有已注册的快捷键对象
	auto it = m_shortcutObjects.begin();
	while (it != m_shortcutObjects.end())
	{
		QPointer<QShortcut> shortcut = it.value();
		if (shortcut)
		{
			shortcut->setKey(m_shortcuts[it.key()]);
			++it;
		}
		else
		{
			// 对象已被删除，从 map 中移除
			it = m_shortcutObjects.erase(it);
		}
	}

	emit shortcutsChanged();
}

QString ShortcutManager::getConfigPath() const
{
	QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
	if (configDir.isEmpty())
	{
		configDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/AIAssit";
	}

	QDir dir;
	if (!dir.exists(configDir))
	{
		dir.mkpath(configDir);
	}

	return configDir + "/shortcuts.json";
}

bool ShortcutManager::saveConfig()
{
	QJsonObject json;

	for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it)
	{
		json[actionToString(it.key())] = it.value().toString();
	}

	QJsonDocument doc(json);
	QFile file(getConfigPath());

	if (!file.open(QIODevice::WriteOnly))
	{
		return false;
	}

	file.write(doc.toJson());
	file.close();

	return true;
}

bool ShortcutManager::loadConfig()
{
	QFile file(getConfigPath());

	if (!file.exists() || !file.open(QIODevice::ReadOnly))
	{
		return false;
	}

	QByteArray data = file.readAll();
	file.close();

	QJsonDocument doc = QJsonDocument::fromJson(data);
	if (doc.isNull() || !doc.isObject())
	{
		return false;
	}

	QJsonObject json = doc.object();

	for (auto it = json.begin(); it != json.end(); ++it)
	{
		ShortcutAction action = stringToAction(it.key());
		// Check if action is valid (not the invalid indicator)
		if (action >= NewConversation && action <= DeleteConversation)
		{
			QKeySequence seq(it.value().toString());
			if (!seq.isEmpty())
			{
				m_shortcuts[action] = seq;
			}
		}
	}

	return true;
}

QString ShortcutManager::actionToString(ShortcutAction action) const
{
	switch (action)
	{
	case NewConversation: return "NewConversation";
	case SearchConversations: return "SearchConversations";
	case ToggleSidebar: return "ToggleSidebar";
	case FocusInput: return "FocusInput";
	case SendMessage: return "SendMessage";
	case StopGeneration: return "StopGeneration";
	case Settings: return "Settings";
	case ClearCurrentChat: return "ClearCurrentChat";
	case ExportConversation: return "ExportConversation";
	case DeleteConversation: return "DeleteConversation";
	default: return QString();
	}
}

ShortcutManager::ShortcutAction ShortcutManager::stringToAction(const QString& str) const
{
	if (str == "NewConversation") return NewConversation;
	if (str == "SearchConversations") return SearchConversations;
	if (str == "ToggleSidebar") return ToggleSidebar;
	if (str == "FocusInput") return FocusInput;
	if (str == "SendMessage") return SendMessage;
	if (str == "StopGeneration") return StopGeneration;
	if (str == "Settings") return Settings;
	if (str == "ClearCurrentChat") return ClearCurrentChat;
	if (str == "ExportConversation") return ExportConversation;
	if (str == "DeleteConversation") return DeleteConversation;
	// Return first enum value as invalid indicator
	return static_cast<ShortcutAction>(static_cast<int>(NewConversation) - 1);
}

