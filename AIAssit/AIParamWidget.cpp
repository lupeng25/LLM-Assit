#include "AIParamWidget.h"
#include <QResizeEvent>

AIParamWidget::AIParamWidget(QWidget* parent)
	: QWidget(parent)
	, m_scrollArea(new QScrollArea(this))
	, m_contentWidget(new QWidget)
	, m_mainLayout(new QVBoxLayout(m_contentWidget))
	, m_connectionGroup(new QGroupBox(QStringLiteral("连接设置"), this))
	, m_modelGroup(new QGroupBox(QStringLiteral("模型参数"), this))
	, m_optionsGroup(new QGroupBox(QStringLiteral("功能选项"), this))
	, m_apiKeyEdit(new QLineEdit(this))
	, m_chatIPEdit(new QLineEdit(this))
	, m_streamIPEdit(new QLineEdit(this))
	, m_chatModeCombo(new QComboBox(this))
	, m_temperatureSpinBox(new QDoubleSpinBox(this))
	, m_maxTokenSpinBox(new QSpinBox(this))
	, m_streamChatCheck(new QCheckBox(QStringLiteral("启用流式对话"), this))
	, m_openThinkCheck(new QCheckBox(QStringLiteral("思考模式"), this))
	, m_openNetSearchCheck(new QCheckBox(QStringLiteral("联网搜索"),this))
	, m_applyButton(new QPushButton(QStringLiteral("应用设置"), this))
	, m_resetButton(new QPushButton(QStringLiteral("重置默认"), this))
	, llmParams(new LLMParams())
{
	setupUI();
	setupStyles();
	setupConnections();
	setupToolTips();
	setupDefaultValues();
}

AIParamWidget::~AIParamWidget() 
{
	delete llmParams; 
}

void AIParamWidget::setupUI() {
	// 设置滚动区域
	m_scrollArea->setWidget(m_contentWidget);
	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setFrameShape(QFrame::NoFrame);

	// 主布局
	auto* widgetLayout = new QVBoxLayout(this);
	widgetLayout->setContentsMargins(0, 0, 0, 0);
	widgetLayout->addWidget(m_scrollArea);

	// 设置内容边距
	m_mainLayout->setContentsMargins(20, 20, 20, 20);
	m_mainLayout->setSpacing(20);

	// 连接设置分组
	auto* connectionLayout = new QGridLayout(m_connectionGroup);
	connectionLayout->setSpacing(15);
	connectionLayout->setContentsMargins(20, 25, 20, 20);

	// API Key行
	auto* apiKeyLabel = new QLabel(QStringLiteral("API Key:"));
	apiKeyLabel->setMinimumWidth(120);
	connectionLayout->addWidget(apiKeyLabel, 0, 0);
	connectionLayout->addWidget(m_apiKeyEdit, 0, 1);

	// 对话服务地址行
	auto* chatIPLabel = new QLabel(QStringLiteral("对话服务地址:"));
	connectionLayout->addWidget(chatIPLabel, 1, 0);
	connectionLayout->addWidget(m_chatIPEdit, 1, 1);

	// 流式对话服务地址行
	auto* streamIPLabel = new QLabel(QStringLiteral("流式服务地址:"));
	connectionLayout->addWidget(streamIPLabel, 2, 0);
	connectionLayout->addWidget(m_streamIPEdit, 2, 1);

	// 设置列拉伸
	connectionLayout->setColumnStretch(1, 1);

	// 模型参数分组
	auto* modelLayout = new QGridLayout(m_modelGroup);
	modelLayout->setSpacing(15);
	modelLayout->setContentsMargins(20, 25, 20, 20);

	// 对话模式行
	auto* chatModeLabel = new QLabel(QStringLiteral("对话模式:"));
	chatModeLabel->setMinimumWidth(120);
	modelLayout->addWidget(chatModeLabel, 0, 0);
	modelLayout->addWidget(m_chatModeCombo, 0, 1);

	// 温度参数行
	auto* tempLabel = new QLabel(QStringLiteral("温度参数:"));
	modelLayout->addWidget(tempLabel, 1, 0);
	modelLayout->addWidget(m_temperatureSpinBox, 1, 1);

	// 最大Token行
	auto* tokenLabel = new QLabel(QStringLiteral("最大Token:"));
	modelLayout->addWidget(tokenLabel, 2, 0);
	modelLayout->addWidget(m_maxTokenSpinBox, 2, 1);

	// 设置列拉伸
	modelLayout->setColumnStretch(1, 1);

	// 功能选项分组
	auto* optionsLayout = new QVBoxLayout(m_optionsGroup);
	optionsLayout->setSpacing(15);
	optionsLayout->setContentsMargins(20, 25, 20, 20);
	optionsLayout->addWidget(m_streamChatCheck);
	optionsLayout->addWidget(m_openThinkCheck);
	optionsLayout->addWidget(m_openNetSearchCheck);

	// 按钮区域
	auto* buttonLayout = new QHBoxLayout;
	buttonLayout->setSpacing(15);
	buttonLayout->addStretch();
	buttonLayout->addWidget(m_resetButton);
	buttonLayout->addWidget(m_applyButton);

	// 添加到主布局
	m_mainLayout->addWidget(m_connectionGroup);
	m_mainLayout->addWidget(m_modelGroup);
	m_mainLayout->addWidget(m_optionsGroup);
	m_mainLayout->addLayout(buttonLayout);
	m_mainLayout->addStretch();
}

void AIParamWidget::setupStyles() {
	// 设置控件参数
	m_temperatureSpinBox->setRange(0.0, 1.0);
	m_temperatureSpinBox->setSingleStep(0.1);
	m_temperatureSpinBox->setDecimals(2);

	m_maxTokenSpinBox->setRange(1, 32768);
	m_maxTokenSpinBox->setSingleStep(1024);

	m_chatModeCombo->addItem(QStringLiteral("Query"), 0);
	m_chatModeCombo->addItem(QStringLiteral("Chat"), 1);
}

void AIParamWidget::setupConnections() 
{
	connect(m_applyButton, &QPushButton::clicked, this, &AIParamWidget::applyCurrentParams);
	connect(m_resetButton, &QPushButton::clicked, this, &AIParamWidget::setupDefaultValues);
}

void AIParamWidget::setupToolTips() 
{
	m_apiKeyEdit->setToolTip(QStringLiteral("输入您的API密钥以访问AI服务"));
	m_chatIPEdit->setToolTip(QStringLiteral("设置对话服务器的完整地址"));
	m_streamIPEdit->setToolTip(QStringLiteral("设置流式对话服务器的完整地址"));
	m_temperatureSpinBox->setToolTip(QStringLiteral("控制模型输出的随机性 (0.0=确定性, 1.0=随机性)"));
	m_maxTokenSpinBox->setToolTip(QStringLiteral("设置单次对话的最大Token数量"));
	m_streamChatCheck->setToolTip(QStringLiteral("启用后将实时显示AI回复内容"));
	m_openThinkCheck->setToolTip(QStringLiteral("启用后AI将显示思考过程"));
	m_openNetSearchCheck->setToolTip(QStringLiteral("启用后AI联网搜索"));
	m_applyButton->setToolTip(QStringLiteral("保存当前配置并应用"));
	m_resetButton->setToolTip(QStringLiteral("重置所有参数为默认值"));
}

void AIParamWidget::setupDefaultValues() 
{
	llmParams = new LLMParams();
	updateUIFromParams();
}

void AIParamWidget::resizeEvent(QResizeEvent* event) 
{
	QWidget::resizeEvent(event);
	m_scrollArea->resize(size());
}

void AIParamWidget::updateUIFromParams()
{
	m_apiKeyEdit->setText(llmParams->getApiKey());
	m_chatIPEdit->setText(llmParams->getBaseUrl());
	m_streamIPEdit->setText(llmParams->getStreamUrl());
	m_chatModeCombo->setCurrentIndex(llmParams->getChatMode());
	m_temperatureSpinBox->setValue(llmParams->getTemperature());
	m_maxTokenSpinBox->setValue(llmParams->getMaxToken());
	m_streamChatCheck->setChecked(llmParams->getStreamChat());
	m_openThinkCheck->setChecked(llmParams->getOpenThink());
	m_openNetSearchCheck->setChecked(llmParams->getOpenNetSearch());
}

void AIParamWidget::applyCurrentParams()
{
	llmParams->setApiKey(m_apiKeyEdit->text());
	llmParams->setBaseUrl(m_chatIPEdit->text());
	llmParams->setStreamUrl(m_streamIPEdit->text());
	llmParams->setChatMode(m_chatModeCombo->currentIndex());
	llmParams->setTemperature(m_temperatureSpinBox->value());
	llmParams->setMaxToken(m_maxTokenSpinBox->value());
	llmParams->setStreamChat(m_streamChatCheck->isChecked());
	llmParams->setOpenThink(m_openThinkCheck->isChecked());
	llmParams->setOpenNetSearch(m_openNetSearchCheck->isChecked());

	llmParams->serialize(QCoreApplication::applicationDirPath() + "/AIAssit/AIModelConfig.json");
	emit paramsChanged();
}

bool AIParamWidget::loadParamsFromJson(const QString& jsonPath) 
{
	if (!llmParams->deserialize(jsonPath)) 
	{
		return false;
	}
	updateUIFromParams();
	return true;
}

void AIParamWidget::setLLMParams(LLMParams* params)
{
	llmParams = params;
	updateUIFromParams();
}