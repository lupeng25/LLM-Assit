#pragma once

#include <QDateTime>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QSize>
#include <QString>

struct ChatMessageData {
	QString m_ChatMsg;
	QString m_ChatReasonMsg;
	QString m_ChatTime;
	QSize m_AllSize;
	int userType = 1;
	QString m_DialogName;
	QString m_BubbleID;

	ChatMessageData() = default;

	ChatMessageData(QString message,
		QString time,
		QSize size,
		int type,
		QString dialog,
		QString reasoning,
		QString id = QString())
		: m_ChatMsg(std::move(message))
		, m_ChatReasonMsg(std::move(reasoning))
		, m_ChatTime(std::move(time))
		, m_AllSize(size)
		, userType(type)
		, m_DialogName(std::move(dialog))
		, m_BubbleID(std::move(id)) {}

	QJsonObject toJson() const {
		QJsonObject obj;
		obj["msg"] = m_ChatMsg;
		obj["time"] = m_ChatTime;
		obj["width"] = m_AllSize.width();
		obj["height"] = m_AllSize.height();
		obj["userType"] = userType;
		obj["dialogname"] = m_DialogName;
		obj["reasoningMsg"] = m_ChatReasonMsg;
		obj["bubbleid"] = m_BubbleID;
		return obj;
	}

	void fromJson(const QJsonObject& obj) {
		m_ChatMsg = obj["msg"].toString();
		m_ChatTime = obj["time"].toString();
		m_AllSize = QSize(obj["width"].toInt(), obj["height"].toInt());
		userType = obj["userType"].toInt();
		m_DialogName = obj["dialogname"].toString();
		m_ChatReasonMsg = obj["reasoningMsg"].toString();
		m_BubbleID = obj["bubbleid"].toString();
	}
};

struct ChatSession {
	QDateTime SaveTime;
	QList<ChatMessageData> sMsg;

	QJsonObject toJson() const {
		QJsonObject obj;
		obj["saveTime"] = SaveTime.toString(Qt::ISODate);

		QJsonArray msgArray;
		for (const auto& msg : sMsg) {
			msgArray.append(msg.toJson());
		}

		obj["messages"] = msgArray;
		return obj;
	}

	void fromJson(const QJsonObject& obj) {
		SaveTime = QDateTime::fromString(obj["saveTime"].toString(), Qt::ISODate);
		QJsonArray msgArray = obj["messages"].toArray();
		sMsg.clear();
		for (const QJsonValue& val : msgArray) {
			if (!val.isObject()) {
				continue;
			}
			ChatMessageData msg;
			msg.fromJson(val.toObject());
			sMsg.append(msg);
		}
	}

	int messageIndex(const QString& bubbleId) const {
		for (int i = 0; i < sMsg.size(); ++i) {
			if (sMsg[i].m_BubbleID == bubbleId) {
				return i;
			}
		}
		return -1;
	}

	int GetMsgIndex(const QString& bubbleId) const {
		return messageIndex(bubbleId);
	}
};

using ChatSessionMap = QHash<QString, ChatSession>;

