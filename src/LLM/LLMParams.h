#pragma once
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <memory>
class LLMParams : public QObject
{
    Q_OBJECT
public:
    explicit LLMParams();
    LLMParams(const LLMParams& other);
    LLMParams& operator=(const LLMParams& other);
    ~LLMParams();
    LLMParams* clone() const;
    void copyFrom(const LLMParams& other);
    //设置模型参数
    void setChatMode(const int &iChatMode);
    void setMaxToken(const int &iMaxToken);
    void setModel(const int &iModel);
    void setTemperature(const double&dTemperature);
    void setStreamChat(const bool& bStreamChat);
    void setOpenThink(const bool& bOpenThink);
    void setOpenNetSearch(const bool& bOpenNetSearch);
    void setFunctionCallTools(const QJsonArray& tools);
    void setApiKey(const QString &apiKey);
    void setKnowledgeApi(const QString &apikey);
    void setBaseUrl(const QString &baseUrl);
    void setStreamUrl(const QString &StreamUrl);
    void setTimeout(int timeoutMs);
    void setLLMPlatForm(const int& platform);
    //获取模型参数
    int getChatMode() const;
    int getMaxToken() const;
    int getModel() const;
    double getTemperature() const;
    bool getStreamChat() const;
    bool getOpenThink() const;
    bool getOpenNetSearch() const;
    QJsonArray getFunctionCallTools() const;
    QString getApiKey() const;
    QString getKnowledgeApi() const;
    QString getBaseUrl() const;
    QString getStreamUrl() const;
    int getTimeout() const;
    int getLLMPlatForm() const;
    //序列化
    bool serialize(const QString& jsonPath);
    //反序列化
    bool deserialize(const QString& jsonPath);
private:
    //模型参数
    int iChatMode = 0;
    int iMaxToken = 4096;
    int iModel = 0;
    double dTemperature = 0.7;
    bool bStreamChat = false;
    bool bOpenThink = true;
    bool bOpenNetSearch = false;
    //网络参数
    QString chatIP = "http://172.16.1.58:8080/api/chat/completions";
    QString streamChatIP = "http://172.16.1.58:8080/api/chat/completions";
    QString apiKey = "sk-73d0cbe04fd148568420d237079acefb";
    QString KnowledgeApi = "dataset-eMXQPtWnRmhcOB1QycBglXtf";
    int iTimeOuts = 30000;
    //FunctionCall工具
    QJsonArray tools;
    //平台:OpenWebUI Dify
    int llmPlatform = 1;

};

