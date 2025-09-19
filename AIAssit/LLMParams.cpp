#include "LLMParams.h"

LLMParams::LLMParams()
{

}

LLMParams::LLMParams(const LLMParams& other) : QObject(nullptr)
{
	copyFrom(other);
}

LLMParams& LLMParams::operator=(const LLMParams& other)
{
	if (this != &other)
	{
		copyFrom(other);
	}
	return *this;
}

LLMParams::~LLMParams()
{
}

void LLMParams::copyFrom(const LLMParams& other)
{
	iChatMode = other.iChatMode;
	iMaxToken = other.iMaxToken;
	iModel = other.iModel;
	dTemperature = other.dTemperature;
	bStreamChat = other.bStreamChat;
	bOpenThink = other.bOpenThink;
	bOpenNetSearch = other.bOpenNetSearch;
	tools = other.tools;
	chatIP = other.chatIP;
	streamChatIP = other.streamChatIP;
	apiKey = other.apiKey;
	iTimeOuts = other.iTimeOuts;
}

LLMParams* LLMParams::clone() const
{
	return new LLMParams(*this);
}

void LLMParams::setChatMode(const int &iChatMode)
{
	this->iChatMode = iChatMode;
}

int LLMParams::getChatMode() const
{
	return this->iChatMode;
}

void LLMParams::setMaxToken(const int &iMaxToken)
{
	this->iMaxToken = iMaxToken;
}

int LLMParams::getMaxToken() const
{
	return this->iMaxToken;
}

void LLMParams::setModel(const int &iModel)
{
	this->iModel = iModel;
}

int LLMParams::getModel() const
{
	return this->iModel;
}

void LLMParams::setTemperature(const double& dTemperature)
{
	this->dTemperature = dTemperature;
}

double LLMParams::getTemperature() const
{
	return this->dTemperature;
}

void LLMParams::setStreamChat(const bool& bStreamChat)
{
	this->bStreamChat = bStreamChat;
}

bool LLMParams::getStreamChat() const
{
	return this->bStreamChat;
}

void LLMParams::setOpenThink(const bool& bOpenThink)
{
	this->bOpenThink = bOpenThink;
}

bool LLMParams::getOpenThink() const
{
	return this->bOpenThink;
}

void LLMParams::setOpenNetSearch(const bool& bOpenNetSearch)
{
	this->bOpenNetSearch = bOpenNetSearch;
}

bool LLMParams::getOpenNetSearch() const
{
	return this->bOpenNetSearch;
}

void LLMParams::setFunctionCallTools(const QJsonArray& tools)
{
	this->tools = tools;
}

QJsonArray LLMParams::getFunctionCallTools() const
{
	return this->tools;
}

void LLMParams::setApiKey(const QString &apiKey)
{
	this->apiKey = apiKey;
}

QString LLMParams::getApiKey() const
{
	return this->apiKey;
}

void LLMParams::setBaseUrl(const QString &baseUrl)
{
	this->chatIP = baseUrl;
}

QString LLMParams::getBaseUrl() const
{
	return this->chatIP;
}

void LLMParams::setStreamUrl(const QString &baseUrl)
{
	this->streamChatIP = baseUrl;
}

QString LLMParams::getStreamUrl() const
{
	return this->streamChatIP;
}

void LLMParams::setTimeout(int timeoutMs)
{
	this->iTimeOuts = timeoutMs;
}

int LLMParams::getTimeout() const
{
	return this->iTimeOuts;
}

void LLMParams::setLLMPlatForm(const int& platform)
{
	this->llmPlatform = platform;
}

int LLMParams::getLLMPlatForm() const
{
	return this->llmPlatform;
}

bool LLMParams::serialize(const QString& jsonPath)
{
	QJsonObject obj;
	obj["apiKey"] = this->apiKey;
	obj["chatIP"] = this->chatIP;
	obj["streamChatIP"] = this->streamChatIP;
	obj["chatMode"] = this->iChatMode;
	obj["temperature"] = this->dTemperature;
	obj["maxToken"] = this->iMaxToken;
	obj["streamChat"] = this->bStreamChat;
	obj["openThink"] = this->bOpenThink;
	obj["model"] = this->iModel;
	obj["netSearch"] = this->bOpenNetSearch;
	obj["NetTimesOut"] = this->iTimeOuts;
	obj["platform"] = this->llmPlatform;

	QJsonDocument doc(obj);
	QFile file(jsonPath);
	if (!file.open(QIODevice::WriteOnly)) 
	{
		return false;
	}
	qint64 bytesWritten = file.write(doc.toJson());
	return bytesWritten != -1;
}

bool LLMParams::deserialize(const QString& jsonPath)
{
	QFile file(jsonPath);
	if (!file.open(QIODevice::ReadOnly)) 
	{
		return false;
	}

	QByteArray data = file.readAll();
	file.close();

	if (data.isEmpty()) 
	{
		return false;
	}

	QJsonDocument doc = QJsonDocument::fromJson(data);
	if (doc.isNull() || !doc.isObject()) 
	{
		return false;
	}

	QJsonObject obj = doc.object();
	this->apiKey = obj.value("apiKey").toString(this->apiKey);
	this->chatIP = obj.value("chatIP").toString(this->chatIP);
	this->streamChatIP = obj.value("streamChatIP").toString(this->streamChatIP);
	this->iChatMode = obj.value("chatMode").toInt(this->iChatMode);
	this->dTemperature = obj.value("temperature").toDouble(this->dTemperature);
	this->iMaxToken = obj.value("maxToken").toInt(this->iMaxToken);
	this->bStreamChat = obj.value("streamChat").toBool(this->bStreamChat);
	this->bOpenThink = obj.value("openThink").toBool(this->bOpenThink);
	this->iModel = obj.value("model").toInt(this->iModel);
	this->bOpenNetSearch = obj.value("netSearch").toBool(this->bOpenNetSearch);
	this->iTimeOuts = obj.value("NetTimesOut").toInt(this->iTimeOuts);
	this->llmPlatform= obj.value("platform").toInt(this->llmPlatform);
	return true;
}