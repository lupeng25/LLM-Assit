#include "VisionTools.h"
#include <QDateTime>

std::function<QString(QString)> VisionTools::s_invoker = nullptr;

void VisionTools::setInvoker(const std::function<QString(QString)>& invoker)
{
	s_invoker = invoker;
}

QJsonObject VisionTools::ok(const QString& key, const QString& value)
{
	QJsonObject r;
	r["success"] = true;
	r[key] = value;
	return r;
}

QJsonObject VisionTools::getWeather(const QJsonObject& arguments)
{
	QString city = arguments["city"].toString();
	QJsonObject result;
	if (QStringLiteral("Suzhou") == city)
	{
		result["weather"] = QStringLiteral("Sunny");
		result["temperature"] = QStringLiteral("19°C");
	}
	else if (QStringLiteral("Hangzhou") == city)
	{
		result["weather"] = QStringLiteral("Sunny");
		result["temperature"] = QStringLiteral("21°C");
	}
	else if (QStringLiteral("Beijing") == city)
	{
		result["weather"] = QStringLiteral("Cloudy");
		result["temperature"] = QStringLiteral("18°C");
	}
	else {
		result = QJsonObject();
	}
	return result;
}

QJsonObject VisionTools::getTime(const QJsonObject&)
{
	QJsonObject result;
	result["NowTime"] = QDateTime::currentDateTime().toString();
	return result;
}

QJsonObject VisionTools::runVisionOrder(const QJsonObject&)
{
	if (s_invoker) s_invoker(QStringLiteral("RunOnce"));
	return ok(QStringLiteral("result"), QStringLiteral("Executed successfully"));
}

QJsonObject VisionTools::openTemplateAssistant(const QJsonObject&)
{
	if (s_invoker) s_invoker(QStringLiteral("ModelAssist"));
	return ok(QStringLiteral("result"), QStringLiteral("Template assistant opened"));
}

QJsonObject VisionTools::runCurrentScript(const QJsonObject&)
{
	if (s_invoker) s_invoker(QStringLiteral("ClickButton"));
	return ok(QStringLiteral("result"), QStringLiteral("Current vision script executed"));
}

QJsonObject VisionTools::closeVisionPlatform(const QJsonObject&)
{
	if (s_invoker) s_invoker(QStringLiteral("ExitApplication"));
	return ok(QStringLiteral("result"), QStringLiteral("Vision platform closed"));
}

QJsonObject VisionTools::switchVisionTab(const QJsonObject& arguments)
{
	Q_UNUSED(arguments);
	// If needed, read TabName from arguments and pass through
	if (s_invoker) s_invoker(QStringLiteral("SwitchTab"));
	return ok(QStringLiteral("result"), QStringLiteral("Switched vision tab successfully"));
}


