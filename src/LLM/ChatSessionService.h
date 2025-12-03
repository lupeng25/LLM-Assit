#pragma once

#include <QObject>
#include <QPointer>

#include "ChatSessionTypes.h"

class ChatSessionService : public QObject {
	Q_OBJECT
public:
	explicit ChatSessionService(QObject* parent = nullptr);

	void setStorageFile(const QString& filePath);
	[[nodiscard]] QString storageFile() const;

	bool loadSessions();
	bool saveSessions() const;

	QString createSession(const QString& displayNameHint = QString());
	void removeSession(const QString& sessionId);

	ChatSession* session(const QString& sessionId);
	const ChatSession* session(const QString& sessionId) const;

	ChatSessionMap& sessions();
	const ChatSessionMap& sessions() const;

	void updateSession(const QString& sessionId, const ChatSession& session);

signals:
	void storageFileChanged(const QString& filePath);
	void sessionsChanged();

private:
	bool loadFromJson(const QString& filePath);
	bool saveToJson(const QString& filePath) const;

	QString m_storageFile;
	ChatSessionMap m_sessions;
};

