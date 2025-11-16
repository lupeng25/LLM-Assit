#pragma once

#include <QJsonObject>
#include <QString>
#include <functional>

// Vision platform and demo tools (extracted from LLMFunctionCall)
class VisionTools
{
public:
	// Set external invoker (injected by upper layer, e.g., to call vision platform commands)
	static void setInvoker(const std::function<QString(QString)>& invoker);

	// Demo tools
	static QJsonObject getWeather(const QJsonObject& arguments);
	static QJsonObject getTime(const QJsonObject& arguments);

	// Vision platform related
	static QJsonObject runVisionOrder(const QJsonObject& arguments);
	static QJsonObject openTemplateAssistant(const QJsonObject& arguments);
	static QJsonObject runCurrentScript(const QJsonObject& arguments);
	static QJsonObject closeVisionPlatform(const QJsonObject& arguments);
	static QJsonObject switchVisionTab(const QJsonObject& arguments);

private:
	static std::function<QString(QString)> s_invoker;

	static QJsonObject ok(const QString& key, const QString& value);
};


