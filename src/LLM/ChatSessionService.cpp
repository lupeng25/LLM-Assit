#include "ChatSessionService.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

ChatSessionService::ChatSessionService(QObject* parent)
	: QObject(parent) {}

void ChatSessionService::setStorageFile(const QString& filePath) {
	if (m_storageFile == filePath)
		return;
	m_storageFile = filePath;
	emit storageFileChanged(m_storageFile);
}

QString ChatSessionService::storageFile() const {
	return m_storageFile;
}

bool ChatSessionService::loadSessions() {
	if (m_storageFile.isEmpty())
		return false;
	bool loaded = loadFromJson(m_storageFile);
	if (loaded) {
		emit sessionsChanged();
	}
	return loaded;
}

bool ChatSessionService::saveSessions() const {
	if (m_storageFile.isEmpty())
		return false;
	return saveToJson(m_storageFile);
}

QString ChatSessionService::createSession(const QString& displayNameHint) {
	const QString sessionId = QUuid::createUuid().toString();
	ChatSession session;
	session.SaveTime = QDateTime::currentDateTime();
	m_sessions.insert(sessionId, session);
	emit sessionsChanged();
	return sessionId;
}

void ChatSessionService::removeSession(const QString& sessionId) {
	if (!m_sessions.contains(sessionId))
		return;
	m_sessions.remove(sessionId);
	emit sessionsChanged();
}

ChatSession* ChatSessionService::session(const QString& sessionId) {
	auto it = m_sessions.find(sessionId);
	if (it == m_sessions.end())
		return nullptr;
	return &it.value();
}

const ChatSession* ChatSessionService::session(const QString& sessionId) const {
	auto it = m_sessions.find(sessionId);
	if (it == m_sessions.end())
		return nullptr;
	return &it.value();
}

ChatSessionMap& ChatSessionService::sessions() {
	return m_sessions;
}

const ChatSessionMap& ChatSessionService::sessions() const {
	return m_sessions;
}

void ChatSessionService::updateSession(const QString& sessionId, const ChatSession& session) {
	m_sessions.insert(sessionId, session);
	emit sessionsChanged();
}

bool ChatSessionService::loadFromJson(const QString& filePath) {
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QJsonParseError error;
	const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
	file.close();

	if (error.error != QJsonParseError::NoError || !doc.isObject())
		return false;

	m_sessions.clear();
	const QJsonObject root = doc.object();
	for (const QString& key : root.keys()) {
		ChatSession session;
		session.fromJson(root.value(key).toObject());
		m_sessions.insert(key, session);
	}
	return !m_sessions.isEmpty();
}

bool ChatSessionService::saveToJson(const QString& filePath) const {
	QJsonObject root;
	for (auto it = m_sessions.constBegin(); it != m_sessions.constEnd(); ++it) {
		root.insert(it.key(), it.value().toJson());
	}

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
	file.close();
	return true;
}

