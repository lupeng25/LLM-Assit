#include "AIParamWidget.h"
#include <QResizeEvent>

AIParamWidget::AIParamWidget(QWidget* parent)
	: QWidget(parent)
	, m_scrollArea(new QScrollArea(this))
	, m_contentWidget(new QWidget)
	, m_mainLayout(new QVBoxLayout(m_contentWidget))
	, m_connectionGroup(new QGroupBox(tr("Connect Setting"), this))
	, m_modelGroup(new QGroupBox(tr("Model Param"), this))
	, m_optionsGroup(new QGroupBox(tr("Tool Select"), this))
	, m_apiKeyEdit(new QLineEdit(this))
	, m_chatIPEdit(new QLineEdit(this))
	, m_streamIPEdit(new QLineEdit(this))
	, m_chatModeCombo(new QComboBox(this))
	, m_temperatureSpinBox(new QDoubleSpinBox(this))
	, m_maxTokenSpinBox(new QSpinBox(this))
	, m_streamChatCheck(new QCheckBox(tr("Open Stream Chat"), this))
	, m_openThinkCheck(new QCheckBox(tr("Think Mode"), this))
	, m_openNetSearchCheck(new QCheckBox(tr("Web Search"), this))
	, m_applyButton(new QPushButton(tr("App Setting"), this))
	, m_resetButton(new QPushButton(tr("Reset Default"), this))
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
	llmParams = nullptr;
}

void AIParamWidget::setupUI() 
{
	// 设置滚动区域
	m_scrollArea->setWidget(m_contentWidget);
	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setFrameShape(QFrame::NoFrame);

	// 主布局
	QVBoxLayout* widgetLayout = nullptr;
	if (QLayout* existingLayout = layout())
	{
		if (auto* existingVBox = qobject_cast<QVBoxLayout*>(existingLayout))
		{
			widgetLayout = existingVBox;
		}
		else
		{
			delete existingLayout;
			widgetLayout = new QVBoxLayout();
			setLayout(widgetLayout);
		}
	}
	else
	{
		widgetLayout = new QVBoxLayout();
		setLayout(widgetLayout);
	}
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
	auto* apiKeyLabel = new QLabel(tr("API Key:"));
	apiKeyLabel->setMinimumWidth(120);
	connectionLayout->addWidget(apiKeyLabel, 0, 0);
	connectionLayout->addWidget(m_apiKeyEdit, 0, 1);

	// 对话服务地址行
	auto* chatIPLabel = new QLabel(tr("IP Address:"));
	connectionLayout->addWidget(chatIPLabel, 1, 0);
	m_chatIPEdit->setPlaceholderText(tr("Enter server address (e.g., https://api.example.com/v1)"));
	connectionLayout->addWidget(m_chatIPEdit, 1, 1);

	// 流式对话服务地址行
	auto* streamIPLabel = new QLabel(tr("Stream IP Address:"));
	connectionLayout->addWidget(streamIPLabel, 2, 0);
	m_streamIPEdit->setPlaceholderText(tr("Enter streaming server address"));
	connectionLayout->addWidget(m_streamIPEdit, 2, 1);

	// 设置列拉伸
	connectionLayout->setColumnStretch(1, 1);

	// 模型参数分组
	auto* modelLayout = new QGridLayout(m_modelGroup);
	modelLayout->setSpacing(15);
	modelLayout->setContentsMargins(20, 25, 20, 20);

	// 对话模式行
	auto* chatModeLabel = new QLabel(tr("Chat Mode:"));
	chatModeLabel->setMinimumWidth(120);
	modelLayout->addWidget(chatModeLabel, 0, 0);
	modelLayout->addWidget(m_chatModeCombo, 0, 1);

	// 温度参数行
	auto* tempLabel = new QLabel(tr("Temperature:"));
	modelLayout->addWidget(tempLabel, 1, 0);
	modelLayout->addWidget(m_temperatureSpinBox, 1, 1);

	// 最大Token行
	auto* tokenLabel = new QLabel(tr("Max Token:"));
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

void AIParamWidget::setupStyles() 
{
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
	m_apiKeyEdit->setToolTip(tr("Enter your API key to access the AI service"));
	// IP地址输入框的工具提示将在文本变化时动态更新
	updateIPToolTips();
	m_temperatureSpinBox->setToolTip(tr("Control the randomness of the model's output (0.0 = deterministic, 1.0 = random)"));
	m_maxTokenSpinBox->setToolTip(tr("Set the maximum number of Tokens per conversation"));
	m_streamChatCheck->setToolTip(tr("Once enabled, the AI response content will be displayed in real-time"));
	m_openThinkCheck->setToolTip(tr("Once enabled, the AI will display its thought process"));
	m_openNetSearchCheck->setToolTip(tr("Once activated, the AI will engage in online searches"));
	m_applyButton->setToolTip(tr("Save the current settings and apply them"));
	m_resetButton->setToolTip(tr("Reset all parameters to their default values"));

	// 连接信号以动态更新工具提示
	connect(m_chatIPEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
		QString tooltip = tr("Set the full address of the dialogue server");
		if (!text.isEmpty()) {
			tooltip += QString("\n\n%1: %2").arg(tr("Current address"), text);
		}
		m_chatIPEdit->setToolTip(tooltip);
	});

	connect(m_streamIPEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
		QString tooltip = tr("Set the full address of the streaming dialogue server");
		if (!text.isEmpty()) {
			tooltip += QString("\n\n%1: %2").arg(tr("Current address"), text);
		}
		m_streamIPEdit->setToolTip(tooltip);
	});

	connect(m_apiKeyEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
		QString tooltip = tr("Set the API KEY");
		if (!text.isEmpty()) {
			tooltip += QString("\n\n%1: %2").arg(tr("Current API KEY"), text);
		}
		m_apiKeyEdit->setToolTip(tooltip);
	});
}

void AIParamWidget::setupDefaultValues()
{
	if (llmParams == nullptr)
	{
		llmParams = new LLMParams();
	}

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
	updateIPToolTips(); // 更新工具提示以显示当前值
}

void AIParamWidget::updateIPToolTips()
{
	QString chatTooltip = tr("Set the full address of the dialogue server");
	if (!m_chatIPEdit->text().isEmpty()) {
		chatTooltip += QString("\n\n%1: %2").arg(tr("Current address"), m_chatIPEdit->text());
	}
	m_chatIPEdit->setToolTip(chatTooltip);

	QString streamTooltip = tr("Set the full address of the streaming dialogue server");
	if (!m_streamIPEdit->text().isEmpty()) {
		streamTooltip += QString("\n\n%1: %2").arg(tr("Current address"), m_streamIPEdit->text());
	}
	m_streamIPEdit->setToolTip(streamTooltip);

	QString APITooltip = tr("Set the API KEY");
	if (!m_apiKeyEdit->text().isEmpty()) {
		APITooltip += QString("\n\n%1: %2").arg(tr("Current API KEY"), m_apiKeyEdit->text());
	}
	m_apiKeyEdit->setToolTip(APITooltip);
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
	if (llmParams != nullptr && llmParams != params)
	{
		delete llmParams;
		llmParams = nullptr;
	}
	llmParams = params;
	updateUIFromParams();
}