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
#include <QListWidget>
#include <QStackedWidget>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QIcon>
#include <QEvent>
#include <QVector>
#include <QSet>
#include <QTableWidget>
#include "llmParams.h"

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
	// 导航与内容
	QFrame* m_navPanel;
	QListWidget* m_navList;
	QStackedWidget* m_stackWidget;
	QFrame* m_footerBar;
	QFrame* m_navIndicator = nullptr;
	QLineEdit* m_navSearchEdit;

	// UI控件
	QLineEdit* m_apiKeyEdit;
	QLineEdit* m_knowledgeApiKeyEdit;
	QLineEdit* m_chatIPEdit;
	QLineEdit* m_streamIPEdit;
	QComboBox* m_chatModeCombo;
	QComboBox* m_platformCombo;
	QDoubleSpinBox* m_temperatureSpinBox;
	QSpinBox* m_maxTokenSpinBox;
	QCheckBox* m_streamChatCheck;
	QCheckBox* m_openThinkCheck;
	QCheckBox* m_openNetSearchCheck;
	QComboBox* m_fontScaleCombo;
	QCheckBox* m_darkModeCheck;
	QPushButton* m_applyButton;
	QPushButton* m_resetButton;

	// 参数数据
	LLMParams* llmParams;
	bool m_darkModeEnabled = false;
	qreal m_fontScaleFactor = 1.0;
	qreal m_baseFontPointSize = 0.0;
	bool m_syncingUI = false;
	QSet<int> m_dirtyPages;

	// UI设置
	void setupUI();
	void setupStyles();
	void setupConnections();
	void setupToolTips();
	void setupDirtyTracking();
	void setupDefaultValues();
	void updateUIFromParams();
	void updateIPToolTips();

	// 现代化界面构建
	void buildNavigation();
	void populateNavItems();
	void createPages();
	void updateNavIndicatorPosition(int row);
	QIcon createNavIcon(const QString& glyph, const QColor& color) const;
	QWidget* buildConnectionPage();
	QWidget* buildModelPage();
	QWidget* buildFeaturePage();
	QWidget* buildShortcutsPage();
	QFrame* createSettingCard(const QString& title, const QString& description, QVBoxLayout** bodyLayout = nullptr);
	QWidget* wrapInScrollArea(QWidget* page);

	bool eventFilter(QObject* watched, QEvent* event) override;

	struct NavEntry
	{
		QString title;
		QString description;
		QString glyph;
		QColor color;
	};

	QVector<NavEntry> m_navEntries;

	void handleSearchTextChanged(const QString& text);
	void markPageDirty(int index);
	void clearDirtyMarkers();
	void updateNavItemDisplay(int index);
	void applyThemeStyles();
	void applyFontScale();
	void onFontScaleChanged(int index);
	void onDarkModeToggled(bool checked);
};