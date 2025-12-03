#include "ModelSelectorWidget.h"

#include <QApplication>
#include <functional>
#include <QColor>
#include <algorithm>
#include <QFocusEvent>
#include <QFont>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtGlobal>

namespace
{
	class ModelRowWidget : public QWidget
	{
	public:
		ModelRowWidget(const QString& name, bool favorite, QWidget* parent = nullptr)
			: QWidget(parent)
			, m_nameLabel(new QLabel(name, this))
			, m_favoriteButton(new QToolButton(this))
		{
			setObjectName("ModelRowWidget");
			setMinimumHeight(44);
			auto* layout = new QHBoxLayout(this);
			layout->setContentsMargins(12, 6, 12, 6);
			layout->setSpacing(8);

			auto* iconLabel = new QLabel(this);
			iconLabel->setFixedSize(28, 28);
			iconLabel->setAlignment(Qt::AlignCenter);
			iconLabel->setStyleSheet(
				"background: #e0f2fe;"
				"border-radius: 14px;"
				"color: #0284c7;"
				"font-weight: 600;");
			const QString badgeText = name.isEmpty() ? QStringLiteral("M") : name.left(1).toUpper();
			iconLabel->setText(badgeText);

			m_nameLabel->setObjectName("ModelName");
			m_nameLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #0f172a;");
			m_nameLabel->setWordWrap(false);

			auto* tagLabel = new QLabel(QObject::tr("LLM"), this);
			tagLabel->setObjectName("ModelTag");
			tagLabel->setStyleSheet(
				"border-radius: 8px;"
				"padding: 2px 6px;"
				"background: rgba(79, 70, 229, 0.08);"
				"color: #4338ca;"
				"font-size: 11px;"
				"font-weight: 600;");

			m_favoriteButton->setCheckable(true);
			m_favoriteButton->setCursor(Qt::PointingHandCursor);
			m_favoriteButton->setFixedSize(24, 24);
			m_favoriteButton->setStyleSheet("QToolButton { border: none; background: transparent; }");
			m_favoriteButton->setChecked(favorite);
			updateFavoriteVisual();

			connect(m_favoriteButton, &QToolButton::toggled, this, [this]() {
				updateFavoriteVisual();
			});

			layout->addWidget(iconLabel);
			layout->addWidget(m_nameLabel, 1);
			layout->addWidget(tagLabel);
			layout->addStretch();
			layout->addWidget(m_favoriteButton);

			setProperty("active", false);
			updateActiveState(false);
		}

		void setFavorite(bool favorite)
		{
			if (m_favoriteButton->isChecked() == favorite)
			{
				updateFavoriteVisual();
				return;
			}
			m_favoriteButton->setChecked(favorite);
			updateFavoriteVisual();
		}

		QToolButton* favoriteButton() const
		{
			return m_favoriteButton;
		}

		void setActive(bool active)
		{
			if (property("active") == active)
			{
				return;
			}
			setProperty("active", active);
			updateActiveState(active);
		}

		void setClickHandler(std::function<void()> handler)
		{
			m_clickHandler = std::move(handler);
		}

	private:
		void updateFavoriteVisual()
		{
			const bool favorite = m_favoriteButton->isChecked();
			const QString filledStar = QString::fromUtf16(u"\u2605");
			const QString emptyStar = QString::fromUtf16(u"\u2606");
			m_favoriteButton->setText(favorite ? filledStar : emptyStar);
			m_favoriteButton->setStyleSheet(favorite
				? "QToolButton { color: #f59e0b; font-size: 16px; border: none; background: transparent; }"
				: "QToolButton { color: #94a3b8; font-size: 16px; border: none; background: transparent; }");
		}

		void updateActiveState(bool active)
		{
			if (active)
			{
				setStyleSheet(
					"#ModelRowWidget {"
					"background: rgba(59, 130, 246, 0.12);"
					"border-radius: 12px;"
					"}");
			}
			else
			{
				setStyleSheet(
					"#ModelRowWidget {"
					"background: transparent;"
					"border-radius: 12px;"
					"}");
			}
		}

		void mousePressEvent(QMouseEvent* event) override
		{
			QWidget::mousePressEvent(event);
			if (m_favoriteButton && m_favoriteButton->geometry().contains(event->pos()))
			{
				return;
			}
			if (m_clickHandler)
			{
				m_clickHandler();
			}
		}

		QLabel* m_nameLabel = nullptr;
		QToolButton* m_favoriteButton = nullptr;
		std::function<void()> m_clickHandler;
	};
}

ModelSelectionPopup::ModelSelectionPopup(QWidget* parent)
	: QFrame(parent, Qt::Popup | Qt::FramelessWindowHint)
{
	setAttribute(Qt::WA_StyledBackground, true);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setFocusPolicy(Qt::StrongFocus);

	auto* wrapperLayout = new QVBoxLayout(this);
	wrapperLayout->setContentsMargins(0, 0, 0, 0);

	auto* card = new QFrame(this);
	card->setObjectName("ModelPopupCard");
	card->setStyleSheet(
		"#ModelPopupCard {"
		"background: #ffffff;"
		"border-radius: 16px;"
		"border: 1px solid rgba(15, 23, 42, 0.08);"
		"}");

	auto* cardLayout = new QVBoxLayout(card);
	cardLayout->setContentsMargins(20, 20, 20, 20);
	cardLayout->setSpacing(16);

	m_searchEdit = new QLineEdit(card);
	m_searchEdit->setPlaceholderText(tr("搜索模型"));
	m_searchEdit->setClearButtonEnabled(true);
	m_searchEdit->setObjectName("ModelSearchEdit");
	m_searchEdit->setMinimumHeight(36);
	m_searchEdit->setStyleSheet(
		"QLineEdit#ModelSearchEdit {"
		"border-radius: 10px;"
		"border: 1px solid rgba(148, 163, 184, 0.5);"
		"padding: 0 12px;"
		"background: rgba(248, 250, 252, 0.95);"
		"font-size: 13px;"
		"}"
		"QLineEdit#ModelSearchEdit:focus {"
		"border: 1px solid #3b82f6;"
		"background: #ffffff;"
		"}");

	m_listWidget = new QListWidget(card);
	m_listWidget->setObjectName("ModelListWidget");
	m_listWidget->setFrameShape(QFrame::NoFrame);
	m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_listWidget->setSpacing(6);
	m_listWidget->setStyleSheet(
		"#ModelListWidget {"
		"background: transparent;"
		"}"
		"#ModelListWidget::item:selected {"
		"background: transparent;"
		"}");
	m_listWidget->viewport()->installEventFilter(this);

	cardLayout->addWidget(m_searchEdit);
	cardLayout->addWidget(m_listWidget);

	wrapperLayout->addWidget(card);

	connect(m_searchEdit, &QLineEdit::textChanged, this, [this]() {
		rebuildList();
	});
	connect(m_listWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
		if (!item)
		{
			return;
		}
		const QString modelName = item->data(Qt::UserRole).toString();
		if (modelName.isEmpty())
		{
			return;
		}
		m_currentModel = modelName;
		emit modelSelected(modelName);
		hide();
	});
}

void ModelSelectionPopup::setModels(const QStringList& models)
{
	m_models = models;
	rebuildList();
}

void ModelSelectionPopup::setFavorites(const QSet<QString>& favorites)
{
	m_favorites = favorites;
	rebuildList();
}

void ModelSelectionPopup::setCurrentModel(const QString& model)
{
	m_currentModel = model;
	rebuildList();
}

void ModelSelectionPopup::focusSearchField()
{
	if (!m_searchEdit)
	{
		return;
	}
	m_searchEdit->setFocus(Qt::MouseFocusReason);
	m_searchEdit->selectAll();
}

bool ModelSelectionPopup::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::MouseButtonPress && watched == m_listWidget->viewport())
	{
		auto* mouseEvent = static_cast<QMouseEvent*>(event);
		QListWidgetItem* item = m_listWidget->itemAt(mouseEvent->pos());
		if (!item)
		{
			hide();
		}
	}
	return QFrame::eventFilter(watched, event);
}

void ModelSelectionPopup::focusOutEvent(QFocusEvent* event)
{
	QFrame::focusOutEvent(event);
	hide();
}

void ModelSelectionPopup::rebuildList()
{
	if (!m_listWidget)
	{
		return;
	}

	const QString filter = m_searchEdit ? m_searchEdit->text().trimmed() : QString();
	m_listWidget->clear();

	auto sortedFavorites = m_favorites.values();
	std::sort(sortedFavorites.begin(), sortedFavorites.end(), [](const QString& a, const QString& b) {
		return QString::localeAwareCompare(a, b) < 0;
	});

	bool hasFavoriteSection = false;
	for (const QString& model : sortedFavorites)
	{
		if (!matchesFilter(model))
		{
			continue;
		}
		if (!m_models.contains(model))
		{
			continue;
		}
		if (!hasFavoriteSection)
		{
			addSectionHeader(tr("收藏"));
			hasFavoriteSection = true;
		}
		addModelRow(model, true);
	}

	bool hasAllSection = false;
	for (const QString& model : m_models)
	{
		if (m_favorites.contains(model))
		{
			continue;
		}
		if (!matchesFilter(model))
		{
			continue;
		}
		if (!hasAllSection)
		{
			addSectionHeader(tr("全部"));
			hasAllSection = true;
		}
		addModelRow(model, false);
	}

	if (m_listWidget->count() == 0)
	{
        auto* emptyItem = new QListWidgetItem(tr("NO Match Model"), m_listWidget);
		emptyItem->setFlags(Qt::NoItemFlags);
		emptyItem->setTextAlignment(Qt::AlignCenter);
	}
}

void ModelSelectionPopup::addSectionHeader(const QString& title)
{
	auto* item = new QListWidgetItem(title, m_listWidget);
	item->setFlags(Qt::NoItemFlags);
	item->setSizeHint(QSize(item->sizeHint().width(), 26));
	item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	item->setForeground(QColor("#475569"));
	item->setFont(QFont(qApp->font().family(), 11, QFont::Bold));
}

void ModelSelectionPopup::addModelRow(const QString& modelName, bool favorite)
{
	auto* item = new QListWidgetItem(m_listWidget);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setData(Qt::UserRole, modelName);

	auto* rowWidget = new ModelRowWidget(modelName, favorite, m_listWidget);
	rowWidget->setActive(modelName == m_currentModel);
	item->setSizeHint(QSize(item->sizeHint().width(), 50));
	m_listWidget->setItemWidget(item, rowWidget);

	rowWidget->setClickHandler([this, item, modelName]() {
		if (m_listWidget)
		{
			m_listWidget->setCurrentItem(item);
		}
		m_currentModel = modelName;
		emit modelSelected(modelName);
		hide();
	});

	connect(rowWidget->favoriteButton(), &QToolButton::toggled, this, [this, modelName](bool checked) {
		if (checked)
		{
			m_favorites.insert(modelName);
		}
		else
		{
			m_favorites.remove(modelName);
		}
		emit favoritesChanged(m_favorites);
		rebuildList();
	});
}

bool ModelSelectionPopup::matchesFilter(const QString& name) const
{
	if (!m_searchEdit)
	{
		return true;
	}
	const QString filter = m_searchEdit->text().trimmed();
	if (filter.isEmpty())
	{
		return true;
	}
	return name.contains(filter, Qt::CaseInsensitive);
}

ModelComboBox::ModelComboBox(QWidget* parent)
	: QComboBox(parent)
	, m_popup(new ModelSelectionPopup(this))
	, m_settings(new QSettings(QStringLiteral("AIAssit"), QStringLiteral("ModelSelector"), this))
{
	setCursor(Qt::PointingHandCursor);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	setFocusPolicy(Qt::StrongFocus);
	setMinimumWidth(120);

	const QStringList favorites = m_settings->value(QStringLiteral("favorites")).toStringList();
	for (const QString& fav : favorites)
	{
		if (!fav.isEmpty())
		{
			m_favorites.insert(fav);
		}
	}

	m_popup->setFavorites(m_favorites);

	connect(m_popup, &ModelSelectionPopup::modelSelected, this, &ModelComboBox::handleModelPicked);
	connect(m_popup, &ModelSelectionPopup::favoritesChanged, this, &ModelComboBox::handleFavoritesChanged);
	connect(this, &QComboBox::currentTextChanged, this, &ModelComboBox::updateCollapsedWidth);
	updateCollapsedWidth();
}

void ModelComboBox::applyModelList(const QStringList& models)
{
	m_models = models;
	blockSignals(true);
	clear();
	addItems(models);
	blockSignals(false);
	ensureCurrentIndexValid();

	if (m_popup)
	{
		m_popup->setModels(models);
		m_popup->setFavorites(m_favorites);
		m_popup->setCurrentModel(currentText());
	}

	updateCollapsedWidth();
}

void ModelComboBox::showPopup()
{
	if (!m_popup)
	{
		QComboBox::showPopup();
		return;
	}

	const int popupWidth = 420;
	m_popup->setFixedWidth(popupWidth);
	m_popup->setMinimumHeight(280);
	m_popup->setCurrentModel(currentText());

	QPoint popupPos = mapToGlobal(QPoint(0, height()));
	if (QScreen* screen = QGuiApplication::screenAt(popupPos))
	{
		const QRect available = screen->availableGeometry();
		if (popupPos.x() + popupWidth > available.right() - 8)
		{
			popupPos.setX(qMax(available.right() - popupWidth - 8, available.left() + 8));
		}
		const int popupHeight = m_popup->height();
		if (popupPos.y() + popupHeight > available.bottom() - 8)
		{
			popupPos.setY(qMax(available.bottom() - popupHeight - 8, available.top() + 8));
		}
	}

	m_popup->move(popupPos);
	m_popup->show();
	m_popup->raise();
	m_popup->activateWindow();
	m_popup->focusSearchField();
}

void ModelComboBox::hidePopup()
{
	if (m_popup && m_popup->isVisible())
	{
		m_popup->hide();
	}
	else
	{
		QComboBox::hidePopup();
	}
}

void ModelComboBox::handleModelPicked(const QString& model)
{
	const int index = m_models.indexOf(model);
	if (index >= 0)
	{
		setCurrentIndex(index);
	}
	else
	{
		addItem(model);
		m_models.append(model);
		setCurrentIndex(count() - 1);
	}

	emit activated(currentIndex());
	emit currentIndexChanged(currentIndex());
	emit currentTextChanged(currentText());
}

void ModelComboBox::handleFavoritesChanged(const QSet<QString>& favorites)
{
	m_favorites = favorites;
	if (m_popup)
	{
		m_popup->setFavorites(favorites);
	}
	if (m_settings)
	{
		QStringList favoriteList = favorites.values();
		std::sort(favoriteList.begin(), favoriteList.end(), [](const QString& a, const QString& b) {
			return QString::localeAwareCompare(a, b) < 0;
		});
		m_settings->setValue(QStringLiteral("favorites"), favoriteList);
	}
}

void ModelComboBox::ensureCurrentIndexValid()
{
	if (m_models.isEmpty())
	{
		setCurrentIndex(-1);
		return;
	}
	if (currentIndex() < 0 && !m_models.isEmpty())
	{
		setCurrentIndex(0);
	}
}

void ModelComboBox::updateCollapsedWidth()
{
	const QString text = currentText();
	const QFontMetrics metrics(font());
	const int textWidth = metrics.horizontalAdvance(text.isEmpty() ? QStringLiteral(" ") : text);
	constexpr int padding = 70; // padding + arrow space
	constexpr int minWidth = 120;
	constexpr int maxWidth = 480;
	int desiredWidth = textWidth + padding;
	desiredWidth = qBound(minWidth, desiredWidth, maxWidth);
	m_collapsedWidth = desiredWidth;
	setFixedWidth(desiredWidth);
}

