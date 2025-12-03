#include "AIParamWidget.h"
#include "ShortcutManager.h"
#include "ShortcutEdit.h"
#include "MessageManager.h"
#include "AppConfigRepository.h"
#include <QResizeEvent>
#include <QPainter>
#include <QTimer>
#include <QHeaderView>
#include <QTableWidget>
#include <QPushButton>

AIParamWidget::AIParamWidget(QWidget* parent)
	: QWidget(parent)
	, m_navPanel(nullptr)
	, m_navList(nullptr)
	, m_stackWidget(nullptr)
	, m_footerBar(nullptr)
	, m_navSearchEdit(new QLineEdit(this))
	, m_apiKeyEdit(new QLineEdit(this))
	, m_knowledgeApiKeyEdit(new QLineEdit(this))
	, m_chatIPEdit(new QLineEdit(this))
	, m_streamIPEdit(new QLineEdit(this))
	, m_chatModeCombo(new QComboBox(this))
	, m_platformCombo(new QComboBox(this))
	, m_temperatureSpinBox(new QDoubleSpinBox(this))
	, m_maxTokenSpinBox(new QSpinBox(this))
	, m_streamChatCheck(new QCheckBox(this))
	, m_openThinkCheck(new QCheckBox(this))
	, m_openNetSearchCheck(new QCheckBox(this))
	, m_fontScaleCombo(new QComboBox(this))
	, m_darkModeCheck(new QCheckBox(this))
	, m_applyButton(new QPushButton(tr("Apply Changes"), this))
	, m_resetButton(new QPushButton(tr("Reset Default"), this))
	, llmParams(new LLMParams())
	, m_baseFontPointSize(this->font().pointSizeF() > 0 ? this->font().pointSizeF() : 14.0)
{
	setupUI();
	setupStyles();
	setupConnections();
	setupDirtyTracking();
	setupToolTips();
	setupDefaultValues();
	
	// 确保所有控件创建完成后，再次应用样式
	QTimer::singleShot(0, this, [this]() {
		applyThemeStyles();
	});
}

AIParamWidget::~AIParamWidget()
{
	llmParams = nullptr;
}

void AIParamWidget::setupUI()
{
	auto* rootLayout = new QHBoxLayout(this);
	rootLayout->setContentsMargins(0, 0, 0, 0);
	rootLayout->setSpacing(0);

	// 左侧导航
	m_navPanel = new QFrame(this);
	m_navPanel->setObjectName("NavPanel");
	m_navPanel->setMinimumWidth(220);
	m_navPanel->setMaximumWidth(260);
	auto* navLayout = new QVBoxLayout(m_navPanel);
	navLayout->setContentsMargins(28, 32, 28, 32);
	navLayout->setSpacing(24);

	auto* navTitle = new QLabel(tr("Workspace"));
	navTitle->setObjectName("NavTitle");
	navLayout->addWidget(navTitle);

	m_navSearchEdit->setObjectName("NavSearch");
	m_navSearchEdit->setPlaceholderText(tr("Search settings..."));
	m_navSearchEdit->setClearButtonEnabled(true);
	navLayout->addWidget(m_navSearchEdit);

	m_navList = new QListWidget(m_navPanel);
	m_navList->setObjectName("NavList");
	m_navList->setSelectionMode(QAbstractItemView::SingleSelection);
	m_navList->setIconSize(QSize(28, 28));
	m_navList->viewport()->installEventFilter(this);
	navLayout->addWidget(m_navList, 1);

	m_navIndicator = new QFrame(m_navPanel);
	m_navIndicator->setObjectName("NavIndicator");
	m_navIndicator->setFixedWidth(4);
	m_navIndicator->hide();
	m_navIndicator->raise();

	rootLayout->addWidget(m_navPanel);

	// 右侧内容
	auto* contentPanel = new QFrame(this);
	contentPanel->setObjectName("ContentPanel");
	auto* contentLayout = new QVBoxLayout(contentPanel);
	contentLayout->setContentsMargins(0, 0, 0, 0);
	contentLayout->setSpacing(0);

	m_stackWidget = new QStackedWidget(contentPanel);
	m_stackWidget->setObjectName("SettingsStack");
	contentLayout->addWidget(m_stackWidget, 1);

	m_footerBar = new QFrame(contentPanel);
	m_footerBar->setObjectName("FooterBar");
	auto* footerLayout = new QHBoxLayout(m_footerBar);
	footerLayout->setContentsMargins(32, 16, 32, 16);
	footerLayout->setSpacing(16);
	footerLayout->addStretch();
	m_resetButton->setProperty("type", "ghost");
	m_applyButton->setProperty("type", "primary");
	footerLayout->addWidget(m_resetButton);
	footerLayout->addWidget(m_applyButton);
	contentLayout->addWidget(m_footerBar, 0);

	rootLayout->addWidget(contentPanel, 1);

	buildNavigation();
	createPages();
	updateNavIndicatorPosition(m_navList ? m_navList->currentRow() : -1);
}

void AIParamWidget::setupStyles() 
{
	m_temperatureSpinBox->setRange(0.0, 1.0);
	m_temperatureSpinBox->setSingleStep(0.1);
	m_temperatureSpinBox->setDecimals(2);

	m_maxTokenSpinBox->setRange(1, 32768);
	m_maxTokenSpinBox->setSingleStep(1024);
	m_chatModeCombo->addItem(QStringLiteral("Query"), 0);
	m_chatModeCombo->addItem(QStringLiteral("Chat"), 1);

	m_platformCombo->clear();
	m_platformCombo->addItem(tr("Dify"), static_cast<int>(AIProvider::Dify));
	m_platformCombo->addItem(tr("Open WebUI"), static_cast<int>(AIProvider::Open_WebUI));
	m_platformCombo->addItem(tr("Ollama"), static_cast<int>(AIProvider::Ollama));
	m_platformCombo->addItem(tr("AnythingLLM"), static_cast<int>(AIProvider::AnythingLLM));
	m_platformCombo->addItem(tr("Custom"), static_cast<int>(AIProvider::Custom));
	m_platformCombo->setCurrentIndex(0);

	m_streamChatCheck->setObjectName("ToggleSwitch");
	m_openThinkCheck->setObjectName("ToggleSwitch");
	m_openNetSearchCheck->setObjectName("ToggleSwitch");

	m_fontScaleCombo->addItem(tr("Compact"), 0.9);
	m_fontScaleCombo->addItem(tr("Standard"), 1.0);
	m_fontScaleCombo->addItem(tr("Comfortable"), 1.1);
	m_fontScaleCombo->addItem(tr("Reading"), 1.25);
	m_fontScaleCombo->setCurrentIndex(1);

	m_darkModeCheck->setText(tr("Enable dark theme (beta)"));
}

void AIParamWidget::setupConnections()
{
	connect(m_applyButton, &QPushButton::clicked, this, &AIParamWidget::applyCurrentParams);
	connect(m_resetButton, &QPushButton::clicked, this, &AIParamWidget::setupDefaultValues);
	if (m_navList && m_stackWidget)
	{
		connect(m_navList, &QListWidget::currentRowChanged, this, [this](int row) {
			m_stackWidget->setCurrentIndex(row);
			updateNavIndicatorPosition(row);
		});
	}
	if (m_navSearchEdit)
	{
		connect(m_navSearchEdit, &QLineEdit::textChanged, this, &AIParamWidget::handleSearchTextChanged);
	}
	if (m_fontScaleCombo)
	{
		connect(m_fontScaleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, &AIParamWidget::onFontScaleChanged);
	}
	if (m_darkModeCheck)
	{
		connect(m_darkModeCheck, &QCheckBox::toggled, this, &AIParamWidget::onDarkModeToggled);
	}
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

	connect(m_knowledgeApiKeyEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
		QString tooltip = tr("Set the knowledge base API KEY");
		if (!text.isEmpty()) {
			tooltip += QString("\n\n%1: %2").arg(tr("Current knowledge API KEY"), text);
		}
		m_knowledgeApiKeyEdit->setToolTip(tooltip);
	});
}

void AIParamWidget::setupDirtyTracking()
{
	auto connectLineEditDirty = [this](QLineEdit* edit, int page) {
		if (!edit)
		{
			return;
		}
		connect(edit, &QLineEdit::textEdited, this, [this, page]() {
			markPageDirty(page);
		});
	};

	auto connectComboDirty = [this](QComboBox* combo, int page) {
		if (!combo)
		{
			return;
		}
		connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, page](int) {
			markPageDirty(page);
		});
	};

	auto connectDoubleSpinDirty = [this](QDoubleSpinBox* spin, int page) {
		if (!spin)
		{
			return;
		}
		connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, page](double) {
			markPageDirty(page);
		});
	};

	auto connectSpinDirty = [this](QSpinBox* spin, int page) {
		if (!spin)
		{
			return;
		}
		connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, page](int) {
			markPageDirty(page);
		});
	};

	auto connectCheckDirty = [this](QCheckBox* check, int page) {
		if (!check)
		{
			return;
		}
		connect(check, &QCheckBox::stateChanged, this, [this, page](int) {
			markPageDirty(page);
		});
	};

	connectLineEditDirty(m_apiKeyEdit, 0);
	connectLineEditDirty(m_knowledgeApiKeyEdit, 0);
	connectLineEditDirty(m_chatIPEdit, 0);
	connectLineEditDirty(m_streamIPEdit, 0);
	connectComboDirty(m_platformCombo, 0);
	connectComboDirty(m_chatModeCombo, 1);
	connectDoubleSpinDirty(m_temperatureSpinBox, 1);
	connectSpinDirty(m_maxTokenSpinBox, 1);
	connectCheckDirty(m_streamChatCheck, 2);
	connectCheckDirty(m_openThinkCheck, 2);
	connectCheckDirty(m_openNetSearchCheck, 2);
}

void AIParamWidget::setupDefaultValues()
{
	if (llmParams == nullptr)
	{
		llmParams = new LLMParams();
	}

	m_syncingUI = true;
	
	// 先设置 m_darkModeEnabled，确保样式正确应用
	m_darkModeEnabled = false;
	if (m_darkModeCheck)
	{
		m_darkModeCheck->setChecked(false);
	}
	
	updateUIFromParams();

	if (m_fontScaleCombo)
	{
		int standardIndex = m_fontScaleCombo->findData(1.0);
		if (standardIndex >= 0)
		{
			m_fontScaleCombo->setCurrentIndex(standardIndex);
		}
		m_fontScaleFactor = 1.0;
	}
	m_syncingUI = false;

	// 确保样式在最后应用，并且强制刷新
	applyFontScale();
	applyThemeStyles();
	
	// Load shortcuts configuration
	ShortcutManager::instance()->loadConfig();
	clearDirtyMarkers();
	
	// 强制更新样式
	style()->unpolish(this);
	style()->polish(this);
	update();
}

void AIParamWidget::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
}

void AIParamWidget::updateUIFromParams()
{
	m_syncingUI = true;
	m_apiKeyEdit->setText(llmParams->getApiKey());
	m_knowledgeApiKeyEdit->setText(llmParams->getKnowledgeApi());
	m_chatIPEdit->setText(llmParams->getBaseUrl());
	m_streamIPEdit->setText(llmParams->getStreamUrl());
	m_chatModeCombo->setCurrentIndex(llmParams->getChatMode());
	if (m_platformCombo)
	{
		const int platformValue = llmParams->getLLMPlatForm();
		int index = m_platformCombo->findData(platformValue);
		if (index < 0)
		{
			index = 0;
		}
		m_platformCombo->setCurrentIndex(index);
	}
	m_temperatureSpinBox->setValue(llmParams->getTemperature());
	m_maxTokenSpinBox->setValue(llmParams->getMaxToken());
	m_streamChatCheck->setChecked(llmParams->getStreamChat());
	m_openThinkCheck->setChecked(llmParams->getOpenThink());
	m_openNetSearchCheck->setChecked(llmParams->getOpenNetSearch());
	m_syncingUI = false;
	updateIPToolTips();
	clearDirtyMarkers();
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

	QString knowledgeTooltip = tr("Set the knowledge base API KEY");
	if (!m_knowledgeApiKeyEdit->text().isEmpty()) {
		knowledgeTooltip += QString("\n\n%1: %2").arg(tr("Current knowledge API KEY"), m_knowledgeApiKeyEdit->text());
	}
	m_knowledgeApiKeyEdit->setToolTip(knowledgeTooltip);
}

void AIParamWidget::applyCurrentParams()
{
	llmParams->setApiKey(m_apiKeyEdit->text());
	llmParams->setKnowledgeApi(m_knowledgeApiKeyEdit->text());
	llmParams->setBaseUrl(m_chatIPEdit->text());
	llmParams->setStreamUrl(m_streamIPEdit->text());
	llmParams->setChatMode(m_chatModeCombo->currentIndex());
	if (m_platformCombo)
	{
		llmParams->setLLMPlatForm(m_platformCombo->currentData().toInt());
	}
	llmParams->setTemperature(m_temperatureSpinBox->value());
	llmParams->setMaxToken(m_maxTokenSpinBox->value());
	llmParams->setStreamChat(m_streamChatCheck->isChecked());
	llmParams->setOpenThink(m_openThinkCheck->isChecked());
	llmParams->setOpenNetSearch(m_openNetSearchCheck->isChecked());

	llmParams->serialize(AppConfigRepository::instance()->modelConfigFile());
	
	// Save shortcuts configuration
	ShortcutManager::instance()->saveConfig();
	
	emit paramsChanged();
	clearDirtyMarkers();
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

void AIParamWidget::buildNavigation()
{
	populateNavItems();
	if (m_navList)
	{
		m_navList->setCurrentRow(0);
	}
}


void AIParamWidget::populateNavItems()
{
	if (!m_navList)
	{
		return;
	}
	m_navList->clear();
	m_navEntries = {
		{ tr("Connection"), tr("API & Endpoints"), tr("A"), QColor("#2563eb") },
		{ tr("Models"), tr("Chat mode & tuning"), tr("M"), QColor("#ec4899") },
		{ tr("Features"), tr("Runtime behavior"), tr("F"), QColor("#f97316") },
		{ tr("Shortcuts"), tr("Keyboard shortcuts"), tr("K"), QColor("#8b5cf6") }
	};

	for (int i = 0; i < m_navEntries.size(); ++i)
	{
		auto* item = new QListWidgetItem(m_navList);
		item->setSizeHint(QSize(item->sizeHint().width(), 62));
		updateNavItemDisplay(i);
	}
}

void AIParamWidget::createPages()
{
	if (!m_stackWidget)
	{
		return;
	}
	m_stackWidget->addWidget(buildConnectionPage());
	m_stackWidget->addWidget(buildModelPage());
	m_stackWidget->addWidget(buildFeaturePage());
	m_stackWidget->addWidget(buildShortcutsPage());
}

QWidget* AIParamWidget::buildConnectionPage()
{
	auto* container = new QWidget;
	container->setObjectName("SettingsPage");
	auto* layout = new QVBoxLayout(container);
	layout->setContentsMargins(32, 32, 32, 32);
	layout->setSpacing(16);

	QVBoxLayout* apiBody = nullptr;
	auto* apiCard = createSettingCard(
		tr("API Credential"),
		tr("Connect to your preferred LLM provider with a secure API key."),
		&apiBody);
	auto* apiForm = new QFormLayout;
	apiForm->setLabelAlignment(Qt::AlignLeft);
	apiForm->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
	apiForm->setHorizontalSpacing(24);
	apiForm->setVerticalSpacing(16);
	m_apiKeyEdit->setPlaceholderText(tr("sk-xxxxxxxxxxxxxxxxxxxx"));
	apiForm->addRow(tr("API Key"), m_apiKeyEdit);
	m_knowledgeApiKeyEdit->setPlaceholderText(tr("dataset-xxxxxxxxxxxx"));
	apiForm->addRow(tr("Knowledge API Key"), m_knowledgeApiKeyEdit);
	apiBody->addLayout(apiForm);
	layout->addWidget(apiCard);

	QVBoxLayout* platformBody = nullptr;
	auto* platformCard = createSettingCard(
		tr("LLM Platform"),
		tr("Select which provider or backend you would like to use when sending requests."),
		&platformBody);
	auto* platformForm = new QFormLayout;
	platformForm->setLabelAlignment(Qt::AlignLeft);
	platformForm->setHorizontalSpacing(24);
	platformForm->setVerticalSpacing(16);
	m_platformCombo->setMinimumWidth(200);
	platformForm->addRow(tr("Platform"), m_platformCombo);
	platformBody->addLayout(platformForm);
	layout->addWidget(platformCard);

	QVBoxLayout* endpointBody = nullptr;
	auto* endpointCard = createSettingCard(
		tr("Endpoints"),
		tr("Customize the base URL for normal requests and the streaming endpoint for live responses."),
		&endpointBody);
	auto* endpointForm = new QFormLayout;
	endpointForm->setLabelAlignment(Qt::AlignLeft);
	endpointForm->setHorizontalSpacing(24);
	endpointForm->setVerticalSpacing(16);
	m_chatIPEdit->setPlaceholderText(tr("https://api.example.com/v1"));
	m_streamIPEdit->setPlaceholderText(tr("https://stream.example.com/v1"));
	endpointForm->addRow(tr("Base URL"), m_chatIPEdit);
	endpointForm->addRow(tr("Stream URL"), m_streamIPEdit);
	endpointBody->addLayout(endpointForm);
	layout->addWidget(endpointCard);

	layout->addStretch();
	return wrapInScrollArea(container);
}

QWidget* AIParamWidget::buildModelPage()
{
	auto* container = new QWidget;
	container->setObjectName("SettingsPage");
	auto* layout = new QVBoxLayout(container);
	layout->setContentsMargins(32, 32, 32, 32);
	layout->setSpacing(20);

	QVBoxLayout* modeBody = nullptr;
	auto* modeCard = createSettingCard(
		tr("Conversation Mode"),
		tr("Choose between single-turn queries or multi-turn dialogue with memory."),
		&modeBody);
	auto* modeForm = new QFormLayout;
	modeForm->setHorizontalSpacing(24);
	modeForm->setVerticalSpacing(16);
	modeForm->addRow(tr("Chat Mode"), m_chatModeCombo);
	modeBody->addLayout(modeForm);
	layout->addWidget(modeCard);

	QVBoxLayout* tuningBody = nullptr;
	auto* tuningCard = createSettingCard(
		tr("Generation Controls"),
		tr("Fine-tune how creative or constrained the assistant should be."),
		&tuningBody);

	auto* tuningGrid = new QGridLayout;
	tuningGrid->setHorizontalSpacing(20);
	tuningGrid->setVerticalSpacing(16);
	tuningGrid->addWidget(new QLabel(tr("Temperature")), 0, 0);
	tuningGrid->addWidget(m_temperatureSpinBox, 0, 1);
	tuningGrid->addWidget(new QLabel(tr("Max Tokens")), 1, 0);
	tuningGrid->addWidget(m_maxTokenSpinBox, 1, 1);
	tuningBody->addLayout(tuningGrid);
	layout->addWidget(tuningCard);

	layout->addStretch();
	return wrapInScrollArea(container);
}

QWidget* AIParamWidget::buildFeaturePage()
{
	auto* container = new QWidget;
	container->setObjectName("SettingsPage");
	auto* layout = new QVBoxLayout(container);
	layout->setContentsMargins(32, 32, 32, 32);
	layout->setSpacing(20);

	QVBoxLayout* streamBody = nullptr;
	auto* streamingCard = createSettingCard(
		tr("Streaming & Thinking"),
		tr("Control how the assistant streams responses and reveals its reasoning."),
		&streamBody);

	auto addToggleRow = [](QWidget* parent, QCheckBox* check, const QString& title, const QString& desc) {
		// 如果 checkbox 已经在某个布局中，先从布局中移除
		QWidget* oldParent = check->parentWidget();
		if (oldParent)
		{
			QLayout* oldLayout = oldParent->layout();
			if (oldLayout)
			{
				oldLayout->removeWidget(check);
			}
			// 如果父控件不是目标父控件，需要重新设置
			if (oldParent != parent)
			{
				check->setParent(nullptr); // 先移除父控件
			}
		}
		// 设置新的父控件
		if (check->parent() != parent)
		{
			check->setParent(parent);
		}
		
		auto* row = new QFrame(parent);
		row->setObjectName("ToggleRow");
		row->setMinimumHeight(60); // 设置最小高度，确保有足够空间显示标题和描述
		auto* rowLayout = new QHBoxLayout(row);
		rowLayout->setContentsMargins(0, 8, 8, 8); // 上下边距 8px，右边距 8px
		rowLayout->setSpacing(16);

		auto* textLayout = new QVBoxLayout;
		textLayout->setContentsMargins(0, 0, 0, 0);
		textLayout->setSpacing(4);

		auto* titleLabel = new QLabel(title, row);
		titleLabel->setObjectName("CardTitle");
		titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		auto* descLabel = new QLabel(desc, row);
		descLabel->setObjectName("CardDescription");
		descLabel->setWordWrap(true);
		descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

		textLayout->addWidget(titleLabel);
		textLayout->addWidget(descLabel);
		rowLayout->addLayout(textLayout, 1); // 设置拉伸因子，让文本区域优先扩展

		check->setText(QString());
		// 确保 toggle switch 有足够的空间：indicator 48px + 足够的边距防止被裁剪
		check->setMinimumWidth(64);
		check->setMaximumWidth(64);
		check->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		check->setMinimumHeight(30);
		check->setMaximumHeight(30);
		rowLayout->addWidget(check, 0, Qt::AlignRight | Qt::AlignVCenter); // 右对齐，垂直居中

		return row;
	};

	streamBody->addWidget(addToggleRow(streamingCard,
		m_streamChatCheck,
		tr("Streaming Mode"),
		tr("Render responses token-by-token to see the assistant thinking in real time.")));

	streamBody->addWidget(addToggleRow(streamingCard,
		m_openThinkCheck,
		tr("Reasoning Trace"),
		tr("Display the assistant's internal reasoning before the final answer.")));

	layout->addWidget(streamingCard);

	QVBoxLayout* runtimeBody = nullptr;
	auto* runtimeCard = createSettingCard(
		tr("Runtime Options"),
		tr("Enable advanced behaviors to mirror the Chatbox assistant experience."),
		&runtimeBody);

	runtimeBody->addWidget(addToggleRow(runtimeCard,
		m_openNetSearchCheck,
		tr("Web Search"),
		tr("Allow the assistant to call real-time search when the prompt requires fresh knowledge.")));

	layout->addWidget(runtimeCard);

	QVBoxLayout* accessibilityBody = nullptr;
	auto* accessibilityCard = createSettingCard(
		tr("Accessibility & Theme"),
		tr("Adjust font readability and switch between light or dark appearance."),
		&accessibilityBody);

	auto* accessibilityForm = new QFormLayout;
	accessibilityForm->setLabelAlignment(Qt::AlignLeft);
	accessibilityForm->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
	accessibilityForm->setHorizontalSpacing(24);
	accessibilityForm->setVerticalSpacing(16);
	
	// 确保下拉框有足够的宽度
	m_fontScaleCombo->setMinimumWidth(200);
	m_fontScaleCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	accessibilityForm->addRow(tr("Font Scale"), m_fontScaleCombo);
	accessibilityBody->addLayout(accessibilityForm);

	auto* themeRow = new QHBoxLayout;
	themeRow->setContentsMargins(0, 8, 0, 0);
	themeRow->setSpacing(8);
	
	// 确保复选框有足够的空间
	m_darkModeCheck->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	themeRow->addWidget(m_darkModeCheck);
	themeRow->addStretch();
	accessibilityBody->addLayout(themeRow);

	layout->addWidget(accessibilityCard);
	layout->addStretch();
	return wrapInScrollArea(container);
}

QWidget* AIParamWidget::buildShortcutsPage()
{
	auto* container = new QWidget;
	container->setObjectName("SettingsPage");
	auto* layout = new QVBoxLayout(container);
	layout->setContentsMargins(32, 32, 32, 32);
	layout->setSpacing(20);

	QVBoxLayout* shortcutsBody = nullptr;
	auto* shortcutsCard = createSettingCard(
		tr("Keyboard Shortcuts"),
		tr("Customize keyboard shortcuts for common actions. Click on a shortcut field and press the desired key combination."),
		&shortcutsBody);
	shortcutsCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// Create table for shortcuts
	auto* shortcutsTable = new QTableWidget(shortcutsCard);
	shortcutsTable->setColumnCount(3);
	shortcutsTable->setHorizontalHeaderLabels({ tr("Action"), tr("Shortcut"), tr("Default") });
	shortcutsTable->horizontalHeader()->setStretchLastSection(true);
	shortcutsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	shortcutsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	shortcutsTable->setAlternatingRowColors(true);
	shortcutsTable->verticalHeader()->setVisible(false);
	shortcutsTable->setShowGrid(false);
	shortcutsTable->verticalHeader()->setDefaultSectionSize(45); // 设置默认行高

	ShortcutManager* manager = ShortcutManager::instance();
	
	// Populate table
	int row = 0;
	for (int action = ShortcutManager::NewConversation; action <= ShortcutManager::DeleteConversation; ++action)
	{
		ShortcutManager::ShortcutAction shortcutAction = static_cast<ShortcutManager::ShortcutAction>(action);
		
		shortcutsTable->insertRow(row);
		
		// Action name
		QString description = manager->getShortcutDescription(shortcutAction);
		auto* actionItem = new QTableWidgetItem(description);
		actionItem->setData(Qt::UserRole, action);
		shortcutsTable->setItem(row, 0, actionItem);
		
		// Current shortcut
		QKeySequence currentSeq = manager->getShortcut(shortcutAction);
		auto* shortcutEdit = new ShortcutEdit(shortcutsTable);
		shortcutEdit->setKeySequence(currentSeq);
		shortcutEdit->setProperty("action", action);
		
		connect(shortcutEdit, &ShortcutEdit::keySequenceChanged, this, [this, shortcutAction, manager](const QKeySequence& seq) {
			manager->setShortcut(shortcutAction, seq);
			markPageDirty(3); // Shortcuts page is index 3
		});
		
		shortcutsTable->setCellWidget(row, 1, shortcutEdit);
		
		// Default shortcut
		QKeySequence defaultSeq = manager->getDefaultShortcut(shortcutAction);
		auto* defaultItem = new QTableWidgetItem(defaultSeq.toString(QKeySequence::NativeText));
		defaultItem->setForeground(QColor("#64748b"));
		shortcutsTable->setItem(row, 2, defaultItem);
		
		row++;
	}
	
	shortcutsTable->resizeColumnsToContents();
	shortcutsTable->setColumnWidth(0, 200);
	shortcutsTable->setColumnWidth(1, 150);
	
	shortcutsBody->addWidget(shortcutsTable, 1); // 让表格填满可用空间
	
	// Reset button
	auto* resetButton = new QPushButton(tr("Reset to Defaults"), shortcutsCard);
	resetButton->setProperty("type", "ghost");
	connect(resetButton, &QPushButton::clicked, this, [this, manager, shortcutsTable]() {
		manager->resetToDefaults();
		
		// Update all shortcut edits
		for (int row = 0; row < shortcutsTable->rowCount(); ++row)
		{
			auto* edit = qobject_cast<ShortcutEdit*>(shortcutsTable->cellWidget(row, 1));
			if (edit)
			{
				int action = edit->property("action").toInt();
				ShortcutManager::ShortcutAction shortcutAction = static_cast<ShortcutManager::ShortcutAction>(action);
				edit->setKeySequence(manager->getShortcut(shortcutAction));
			}
			
			// Update default column
			int action = shortcutsTable->item(row, 0)->data(Qt::UserRole).toInt();
			ShortcutManager::ShortcutAction shortcutAction = static_cast<ShortcutManager::ShortcutAction>(action);
			QKeySequence defaultSeq = manager->getDefaultShortcut(shortcutAction);
			shortcutsTable->item(row, 2)->setText(defaultSeq.toString(QKeySequence::NativeText));
		}
		
		markPageDirty(3);
	});
	
	shortcutsBody->addWidget(resetButton);
	
	layout->addWidget(shortcutsCard, 1); // 使用拉伸因子让卡片填满空间
	return wrapInScrollArea(container);
}

void AIParamWidget::updateNavIndicatorPosition(int row)
{
	if (!m_navIndicator || !m_navList)
	{
		return;
	}
	if (row < 0 || row >= m_navList->count())
	{
		m_navIndicator->hide();
		return;
	}

	QListWidgetItem* item = m_navList->item(row);
	if (!item)
	{
		m_navIndicator->hide();
		return;
	}

	const QRect itemRect = m_navList->visualItemRect(item);
	if (!itemRect.isValid())
	{
		m_navIndicator->hide();
		return;
	}

	const QPoint mappedTop = m_navList->viewport()->mapTo(m_navPanel, itemRect.topLeft());
	const int indicatorHeight = qMax(36, itemRect.height() - 6);
	const int indicatorX = 12;
	const int indicatorY = mappedTop.y() + (itemRect.height() - indicatorHeight) / 2;

	m_navIndicator->setGeometry(indicatorX, indicatorY, m_navIndicator->width(), indicatorHeight);
	m_navIndicator->show();
	m_navIndicator->raise();
}

QIcon AIParamWidget::createNavIcon(const QString& glyph, const QColor& color) const
{
	const int size = 32;
	QPixmap pixmap(size, size);
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	QColor fill = color;
	fill.setAlpha(230);
	painter.setBrush(fill);
	painter.setPen(Qt::NoPen);
	painter.drawRoundedRect(pixmap.rect(), 14, 14);

	if (!glyph.isEmpty())
	{
		QFont iconFont = font();
		iconFont.setBold(true);
		iconFont.setPointSize(11);
		painter.setFont(iconFont);
		painter.setPen(Qt::white);
		painter.drawText(pixmap.rect(), Qt::AlignCenter, glyph.left(2).toUpper());
	}

	return QIcon(pixmap);
}

bool AIParamWidget::eventFilter(QObject* watched, QEvent* event)
{
	if (m_navList && watched == m_navList->viewport())
	{
		if (event->type() == QEvent::Resize || event->type() == QEvent::LayoutRequest)
		{
			updateNavIndicatorPosition(m_navList->currentRow());
		}
	}
	return QWidget::eventFilter(watched, event);
}

void AIParamWidget::handleSearchTextChanged(const QString& text)
{
	if (!m_navList)
	{
		return;
	}

	const QString keyword = text.trimmed();
	int matchedIndex = -1;
	for (int i = 0; i < m_navEntries.size(); ++i)
	{
		const auto& entry = m_navEntries[i];
		const QString combined = entry.title + " " + entry.description;
		const bool match = keyword.isEmpty() || combined.contains(keyword, Qt::CaseInsensitive);
		if (QListWidgetItem* item = m_navList->item(i))
		{
			item->setHidden(!keyword.isEmpty() && !match);
		}
		if (match && matchedIndex == -1)
		{
			matchedIndex = i;
		}
	}

	if (matchedIndex >= 0 && matchedIndex < m_navEntries.size())
	{
		m_navList->setCurrentRow(matchedIndex);
		if (m_stackWidget)
		{
			m_stackWidget->setCurrentIndex(matchedIndex);
		}
		updateNavIndicatorPosition(matchedIndex);
	}
	else if (keyword.isEmpty() && m_navList->count() > 0)
	{
		updateNavIndicatorPosition(m_navList->currentRow());
	}
}

void AIParamWidget::markPageDirty(int index)
{
	if (m_syncingUI || index < 0)
	{
		return;
	}
	if (!m_dirtyPages.contains(index))
	{
		m_dirtyPages.insert(index);
		updateNavItemDisplay(index);
	}
}

void AIParamWidget::clearDirtyMarkers()
{
	if (m_dirtyPages.isEmpty())
	{
		return;
	}
	m_dirtyPages.clear();
	for (int i = 0; i < m_navEntries.size(); ++i)
	{
		updateNavItemDisplay(i);
	}
}

void AIParamWidget::updateNavItemDisplay(int index)
{
	if (!m_navList || index < 0 || index >= m_navEntries.size())
	{
		return;
	}
	QListWidgetItem* item = m_navList->item(index);
	if (!item)
	{
		return;
	}

	const auto& entry = m_navEntries[index];
	QString title = entry.title;
	if (m_dirtyPages.contains(index))
	{
		title = QStringLiteral("* %1").arg(entry.title);
	}
	item->setText(QStringLiteral("%1\n%2").arg(title, entry.description));
	item->setIcon(createNavIcon(entry.glyph, entry.color));
}

void AIParamWidget::applyThemeStyles()
{
	const QString lightTheme = QStringLiteral(R"(
		AIParamWidget {
			background: #fbfbff;
		}
		#NavPanel {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
				stop:0 #fefefe, stop:1 #f3f8ff);
			border-right: 1px solid rgba(203, 213, 225, 0.7);
		}
		#NavTitle {
			color: #0f172a;
			font-size: 18px;
			font-weight: 600;
		}
		#NavSearch {
			border: 1px solid rgba(148, 163, 184, 0.4);
			border-radius: 14px;
			padding: 8px 12px;
			background: rgba(255, 255, 255, 0.96);
		}
		#NavSearch:focus {
			border-color: #2563eb;
		}
		#NavList {
			border: none;
			background: transparent;
			color: #4c566a;
			font-size: 14px;
		}
		#NavList::item {
			border-radius: 18px;
			padding: 10px 12px 10px 16px;
			margin-bottom: 10px;
			color: #4c566a;
		}
		#NavList::item:selected {
			background: rgba(56, 189, 248, 0.22);
			color: #0f172a;
			font-weight: 600;
			border: 1px solid rgba(59, 130, 246, 0.35);
		}
		#ContentPanel {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
				stop:0 #fdfdff, stop:1 #f1f5ff);
		}
		#SettingsViewport, #SettingsScroll, #SettingsPage {
			background: transparent;
			border: none;
		}
		#SettingCard {
			background: #ffffff;
			border-radius: 26px;
			border: none;
		}
		#CardTitle {
			font-size: 19px;
			font-weight: 600;
			color: #0f172a;
		}
		#CardDescription {
			color: #7c8aa5;
			font-size: 13px;
			font-weight: 400;
		}
		#FooterBar {
			background: rgba(255, 255, 255, 0.96);
			border-top: 1px solid rgba(148, 163, 184, 0.25);
		}
		QPushButton {
			border-radius: 14px;
			padding: 11px 30px;
			font-weight: 600;
		}
		QPushButton[type="primary"] {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #2563eb, stop:1 #38bdf8);
			color: #ffffff;
			border: none;
		}
		QPushButton[type="primary"]:hover {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #1d4ed8, stop:1 #1fb6ff);
		}
		QPushButton[type="primary"]:pressed {
			background: #1e3a8a;
		}
		QPushButton[type="ghost"] {
			background: transparent;
			color: #2563eb;
			border: 1px solid rgba(37, 99, 235, 0.3);
		}
		QPushButton[type="ghost"]:hover {
			background: rgba(37, 99, 235, 0.08);
		}
		QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
			border: 1px solid rgba(148, 163, 184, 0.45);
			border-radius: 14px;
			padding: 10px 14px;
			background: rgba(255, 255, 255, 0.98);
			font-size: 14px;
		}
		QTableWidget {
			border: none;
			background: transparent;
			font-size: 14px;
			gridline-color: rgba(148, 163, 184, 0.2);
		}
		QTableWidget::item {
			padding: 8px 12px;
			border: none;
			color: #1e293b;
			min-height: 45px;
		}
		QTableWidget::item:selected {
			background: rgba(37, 99, 235, 0.1);
			color: #0f172a;
		}
		QTableWidget::item:hover {
			background: rgba(37, 99, 235, 0.05);
		}
		QTableWidget QLineEdit {
			font-size: 13px;
			border: 1px solid rgba(148, 163, 184, 0.3);
			border-radius: 6px;
			padding: 4px 8px;
			background: rgba(255, 255, 255, 0.98);
		}
		QTableWidget QLineEdit:focus {
			border: 2px solid #3b82f6;
		}
		QHeaderView::section {
			background: rgba(241, 245, 249, 0.8);
			padding: 10px 12px;
			border: none;
			border-bottom: 2px solid rgba(148, 163, 184, 0.3);
			font-size: 14px;
			font-weight: 600;
			color: #475569;
		}
		QCheckBox#ToggleSwitch {
			spacing: 0px;
			padding: 0px;
			min-width: 64px;
			max-width: 64px;
			min-height: 30px;
			max-height: 30px;
		}
		QCheckBox#ToggleSwitch::indicator {
			width: 48px;
			height: 26px;
			border-radius: 13px;
			background: rgba(148, 163, 184, 0.35);
			border: none;
			margin: 0px;
		}
		QCheckBox#ToggleSwitch::indicator:checked {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #22d3ee, stop:1 #38bdf8);
		}
		QScrollBar:vertical {
			background: transparent;
			width: 10px;
			margin: 8px 0;
		}
		QScrollBar::handle:vertical {
			background: rgba(148, 163, 184, 0.5);
			border-radius: 4px;
		}
		#NavIndicator {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #60a5fa, stop:1 #2563eb);
			border-radius: 2px;
		}
	)");

	const QString darkTheme = QStringLiteral(R"(
		AIParamWidget {
			background: #0f172a;
			color: #e2e8f0;
		}
		#NavPanel {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
				stop:0 #111827, stop:1 #0b1220);
			border-right: 1px solid rgba(15, 23, 42, 0.6);
		}
		#NavTitle {
			color: #f8fafc;
		}
		#NavSearch {
			border: 1px solid rgba(96, 165, 250, 0.4);
			border-radius: 14px;
			padding: 8px 12px;
			background: rgba(15, 23, 42, 0.8);
			color: #f8fafc;
		}
		#NavSearch:focus {
			border-color: #60a5fa;
		}
		#NavList {
			color: #cbd5f5;
		}
		#NavList::item {
			color: #cbd5f5;
		}
		#NavList::item:selected {
			background: rgba(56, 189, 248, 0.32);
			color: #ffffff;
			border: 1px solid rgba(255, 255, 255, 0.2);
		}
		#ContentPanel {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
				stop:0 #111827, stop:1 #0f172a);
		}
		#SettingCard {
			background: #182032;
			border: 1px solid rgba(255, 255, 255, 0.06);
		}
		#CardHeader {
			border-bottom: 1px solid rgba(255, 255, 255, 0.06);
		}
		#CardTitle {
			color: #f8fafc;
		}
		#CardDescription {
			color: #94a3b8;
		}
		#FooterBar {
			background: rgba(15, 23, 42, 0.95);
			border-top: 1px solid rgba(96, 165, 250, 0.2);
		}
		QPushButton[type="primary"] {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #2563eb, stop:1 #38bdf8);
			color: #ffffff;
		}
		QPushButton[type="primary"]:hover {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #1e40af, stop:1 #0ea5e9);
		}
		QPushButton[type="ghost"] {
			color: #93c5fd;
			border: 1px solid rgba(147, 197, 253, 0.4);
		}
		QPushButton[type="ghost"]:hover {
			background: rgba(59, 130, 246, 0.12);
		}
		QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
			border: 1px solid rgba(148, 163, 184, 0.45);
			border-radius: 14px;
			padding: 10px 14px;
			background: rgba(15, 23, 42, 0.9);
			color: #e2e8f0;
		}
		QTableWidget {
			border: none;
			background: transparent;
			font-size: 14px;
			gridline-color: rgba(148, 163, 184, 0.2);
			color: #e2e8f0;
		}
		QTableWidget::item {
			padding: 8px 12px;
			border: none;
			color: #e2e8f0;
			min-height: 36px;
		}
		QTableWidget::item:selected {
			background: rgba(59, 130, 246, 0.2);
			color: #f8fafc;
		}
		QTableWidget::item:hover {
			background: rgba(59, 130, 246, 0.1);
		}
		QTableWidget QLineEdit {
			font-size: 14px;
			border: 1px solid rgba(148, 163, 184, 0.3);
			border-radius: 6px;
			padding: 4px 8px;
			background: rgba(15, 23, 42, 0.9);
			color: #e2e8f0;
		}
		QTableWidget QLineEdit:focus {
			border: 2px solid #3b82f6;
		}
		QHeaderView::section {
			background: rgba(24, 32, 50, 0.8);
			padding: 10px 12px;
			border: none;
			border-bottom: 2px solid rgba(96, 165, 250, 0.3);
			font-size: 13px;
			font-weight: 600;
			color: #cbd5e1;
		}
		QLineEdit::placeholder {
			color: #64748b;
		}
		QCheckBox#ToggleSwitch {
			spacing: 0px;
			padding: 0px;
			min-width: 64px;
			max-width: 64px;
			min-height: 30px;
			max-height: 30px;
		}
		QCheckBox#ToggleSwitch::indicator {
			width: 48px;
			height: 26px;
			border-radius: 13px;
			background: rgba(148, 163, 184, 0.5);
			border: none;
			margin: 0px;
		}
		QCheckBox#ToggleSwitch::indicator:checked {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #22d3ee, stop:1 #38bdf8);
		}
		QScrollBar::handle:vertical {
			background: rgba(148, 163, 184, 0.6);
		}
		#NavIndicator {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #38bdf8, stop:1 #2563eb);
		}
	)");

	setStyleSheet(m_darkModeEnabled ? darkTheme : lightTheme);
}

void AIParamWidget::applyFontScale()
{
	if (m_baseFontPointSize <= 0)
	{
		return;
	}
	QFont scaledFont = font();
	scaledFont.setPointSizeF(m_baseFontPointSize * m_fontScaleFactor);
	setFont(scaledFont);
	const QList<QWidget*> widgets = findChildren<QWidget*>();
	for (QWidget* child : widgets)
	{
		child->setFont(scaledFont);
	}
}

void AIParamWidget::onFontScaleChanged(int index)
{
	if (index < 0 || !m_fontScaleCombo)
	{
		return;
	}
	const QVariant scaleValue = m_fontScaleCombo->itemData(index);
	if (!scaleValue.isValid())
	{
		return;
	}
	m_fontScaleFactor = scaleValue.toDouble();
	applyFontScale();
	if (!m_syncingUI)
	{
		markPageDirty(2);
	}
}

void AIParamWidget::onDarkModeToggled(bool checked)
{
	m_darkModeEnabled = checked;
	applyThemeStyles();
	if (!m_syncingUI)
	{
		markPageDirty(2);
	}
}

QFrame* AIParamWidget::createSettingCard(const QString& title, const QString& description, QVBoxLayout** bodyLayout)
{
	auto* card = new QFrame;
	card->setObjectName("SettingCard");
	auto* shadow = new QGraphicsDropShadowEffect(card);
	shadow->setBlurRadius(28);
	shadow->setOffset(0, 12);
	shadow->setColor(QColor(15, 23, 42, 35));
	card->setGraphicsEffect(shadow);

	auto* layout = new QVBoxLayout(card);
	layout->setContentsMargins(28, 28, 28, 28);
	layout->setSpacing(12);
	layout->setSizeConstraint(QLayout::SetMinimumSize);

	auto* titleLabel = new QLabel(title);
	titleLabel->setObjectName("CardTitle");
	auto* descLabel = new QLabel(description);
	descLabel->setObjectName("CardDescription");
	descLabel->setWordWrap(true);

	layout->addWidget(titleLabel);
	layout->addWidget(descLabel);
	if (bodyLayout)
	{
		auto* body = new QVBoxLayout;
		body->setContentsMargins(0, 12, 0, 0);
		body->setSpacing(18);
		layout->addLayout(body);
		*bodyLayout = body;
	}
	return card;
}


QWidget* AIParamWidget::wrapInScrollArea(QWidget* page)
{
	auto* scroll = new QScrollArea;
	scroll->setObjectName("SettingsScroll");
	scroll->setWidget(page);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	if (scroll->viewport())
	{
		scroll->viewport()->setObjectName("SettingsViewport");
	}
	return scroll;
}