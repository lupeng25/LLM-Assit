#pragma once

#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtCore/QSet>
#include <QtCore/QStringList>

class QLineEdit;
class QListWidget;
class QSettings;

class ModelSelectionPopup : public QFrame
{
	Q_OBJECT
public:
	explicit ModelSelectionPopup(QWidget* parent = nullptr);

	void setModels(const QStringList& models);
	void setFavorites(const QSet<QString>& favorites);
	void setCurrentModel(const QString& model);
	void focusSearchField();

signals:
	void modelSelected(const QString& model);
	void favoritesChanged(const QSet<QString>& favorites);

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void focusOutEvent(QFocusEvent* event) override;

private:
	void rebuildList();
	void addSectionHeader(const QString& title);
	void addModelRow(const QString& modelName, bool favorite);
	bool matchesFilter(const QString& name) const;

	QStringList m_models;
	QSet<QString> m_favorites;
	QString m_currentModel;

	QLineEdit* m_searchEdit = nullptr;
	QListWidget* m_listWidget = nullptr;
};

class ModelComboBox : public QComboBox
{
	Q_OBJECT
public:
	explicit ModelComboBox(QWidget* parent = nullptr);

	void applyModelList(const QStringList& models);

protected:
	void showPopup() override;
	void hidePopup() override;

private slots:
	void handleModelPicked(const QString& model);
	void handleFavoritesChanged(const QSet<QString>& favorites);

private:
	void ensureCurrentIndexValid();
	void updateCollapsedWidth();
	int collapsedWidth() const { return m_collapsedWidth; }

	ModelSelectionPopup* m_popup = nullptr;
	QStringList m_models;
	QSet<QString> m_favorites;
	QSettings* m_settings = nullptr;
	int m_collapsedWidth = 0;
};

