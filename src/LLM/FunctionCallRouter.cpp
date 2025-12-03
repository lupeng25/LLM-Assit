#include "FunctionCallRouter.h"
#include <QFile>
#include <QJsonDocument>

void FunctionCallRouter::registerTool(const QString& name, Handler handler)
{
	if (name.isEmpty() || !handler) return;
	m_handlers.insert(name, handler);
}

QJsonObject FunctionCallRouter::execute(const QString& name, const QJsonObject& arguments) const
{
	const Handler handler = m_handlers.value(name, nullptr);
	if (!handler)
	{
		return makeError(QStringLiteral("Unknown function name: %1").arg(name));
	}
	return handler(arguments);
}

bool FunctionCallRouter::loadToolsDefinition(const QString& jsonFile)
{
	QFile f(jsonFile);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		m_tools = QJsonArray();
		return false;
	}
	QJsonParseError err;
	const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
	f.close();
	if (err.error != QJsonParseError::NoError || !doc.isObject())
	{
		m_tools = QJsonArray();
		return false;
	}
	const QJsonObject obj = doc.object();
	if (obj.contains(QStringLiteral("tools")) && obj.value(QStringLiteral("tools")).isArray())
	{
		m_tools = obj.value(QStringLiteral("tools")).toArray();
		return true;
	}
	m_tools = QJsonArray();
	return false;
}

QJsonArray FunctionCallRouter::toolsDefinition() const
{
	return m_tools;
}

QJsonObject FunctionCallRouter::makeError(const QString& error)
{
	QJsonObject o;
	o.insert(QStringLiteral("success"), false);
	o.insert(QStringLiteral("error"), error);
	return o;
}


