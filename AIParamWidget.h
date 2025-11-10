#pragma once

#include <QWidget>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QVariant>
#include <QString>
#include <QDebug>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QMessageBox>
#include <QApplication>
#include <QFrame>
#include <QScrollArea>
#include "llmParams.h"

QT_BEGIN_NAMESPACE
class QLineEdit;
class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QVBoxLayout;
class QGridLayout;
class QGroupBox;
class QScrollArea;
QT_END_NAMESPACE

class AIParamWidget : public QWidget {
	Q_OBJECT

public:
	explicit AIParamWidget(QWidget* parent = nullptr);
	~AIParamWidget();
	// 公共接口
	bool loadParamsFromJson(const QString& jsonPath);
	void applyCurrentParams();
	void setLLMParams(LLMParams* params);
	// 参数获取
	int GetAIParamChatMode() { return llmParams->getChatMode(); };
	int GetAIParamMaxToken() { return llmParams->getMaxToken(); };
	int GetAIParamModel() { return llmParams->getModel(); };
	void SetAIParamModel(int iModel) { llmParams->setModel(iModel); };
	double GetAIParamTemperature() { return llmParams->getTemperature(); };
	bool GetAIParambStream() { return llmParams->getStreamChat(); };
	bool GetAIParambOpenThink() { return llmParams->getOpenThink(); };
	QString GetAIParamBaseUrl() { return llmParams->getBaseUrl(); };
	QString GetAIParamStreamUrl() { return llmParams->getStreamUrl(); };
	QString GetAIParamApiKey() { return llmParams->getApiKey(); };

signals:
	void paramsChanged();

protected:
	void resizeEvent(QResizeEvent* event) override;

private:
	// 主布局容器
	QScrollArea* m_scrollArea;
	QWidget* m_contentWidget;
	QVBoxLayout* m_mainLayout;

	// 分组框
	QGroupBox* m_connectionGroup;
	QGroupBox* m_modelGroup;
	QGroupBox* m_optionsGroup;

	// UI控件
	QLineEdit* m_apiKeyEdit;
	QLineEdit* m_chatIPEdit;
	QLineEdit* m_streamIPEdit;
	QComboBox* m_chatModeCombo;
	QDoubleSpinBox* m_temperatureSpinBox;
	QSpinBox* m_maxTokenSpinBox;
	QCheckBox* m_streamChatCheck;
	QCheckBox* m_openThinkCheck;
	QCheckBox* m_openNetSearchCheck;
	QPushButton* m_applyButton;
	QPushButton* m_resetButton;

	// 参数数据
	LLMParams* llmParams;

	// UI设置
	void setupUI();//ui
	void setupStyles();//qss
	void setupConnections();//信号槽
	void setupToolTips();//控件提示
	void setupDefaultValues();
	void updateUIFromParams();
	void updateIPToolTips(); // 更新IP地址输入框的工具提示
};