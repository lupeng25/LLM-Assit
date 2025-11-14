#include "PromptLibraryDialog.h"
#include <QApplication>
#include <QHeaderView>
#include <QSplitter>
#include <QAbstractItemView>

PromptLibraryDialog::PromptLibraryDialog(PromptLibrary* library, QWidget* parent)
	: QDialog(parent)
	, m_library(library)
	, m_currentEditingId()
	, m_isAddingNew(false)
{
	if (!m_library)
	{
        //qWarning() << "PromptLibraryDialog: library is null";
		return;
	}

	setupUI();
	setupStyles();
	connectSignals();
	refreshCategoryList();
	refreshPromptList();

	// 设置窗口属性
	setWindowTitle(tr("Prompt Library"));
	setModal(true);
	resize(1000, 700);
	setMinimumSize(800, 500);
}

PromptLibraryDialog::~PromptLibraryDialog()
{
}

QString PromptLibraryDialog::getSelectedPromptContent() const
{
	QListWidgetItem* item = m_promptList->currentItem();
	if (!item)
	{
		return QString();
	}

	QString id = item->data(Qt::UserRole).toString();
	PromptItem prompt = m_library->getPrompt(id);
	return prompt.content;
}

void PromptLibraryDialog::setupUI()
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(20, 20, 20, 20);
	mainLayout->setSpacing(16);

	// 创建分割器
	m_splitter = new QSplitter(Qt::Horizontal, this);
	mainLayout->addWidget(m_splitter);

	// 左侧面板
	m_leftPanel = new QWidget();
	m_leftPanel->setObjectName("leftPanel");
	QVBoxLayout* leftLayout = new QVBoxLayout(m_leftPanel);
	leftLayout->setContentsMargins(16, 16, 16, 16);
	leftLayout->setSpacing(12);

	// 分类选择
	QHBoxLayout* categoryLayout = new QHBoxLayout();
	categoryLayout->setSpacing(10);
	QLabel* categoryLabel = new QLabel(tr("Category:"), m_leftPanel);
	categoryLabel->setStyleSheet("font-weight: 600; color: #475569;");
	m_categoryCombo = new QComboBox(m_leftPanel);
	m_categoryCombo->addItem(tr("All"), QString());
	m_categoryCombo->setMinimumHeight(36);
	categoryLayout->addWidget(categoryLabel);
	categoryLayout->addWidget(m_categoryCombo);
	leftLayout->addLayout(categoryLayout);

	// 搜索框
	m_searchEdit = new QLineEdit(m_leftPanel);
	m_searchEdit->setPlaceholderText(tr("Search prompts..."));
	m_searchEdit->setMinimumHeight(40);
	leftLayout->addWidget(m_searchEdit);

	// 提示词列表
	m_promptList = new QListWidget(m_leftPanel);
	m_promptList->setSelectionMode(QAbstractItemView::SingleSelection);
	leftLayout->addWidget(m_promptList, 1);

	// 按钮组
	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->setSpacing(10);
	m_addButton = new QPushButton(tr("Add"), m_leftPanel);
	m_addButton->setStyleSheet(R"(
		QPushButton {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #10b981, stop:1 #059669);
		}
		QPushButton:hover {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #34d399, stop:1 #10b981);
		}
		QPushButton:pressed {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #059669, stop:1 #047857);
		}
	)");
	m_editButton = new QPushButton(tr("Edit"), m_leftPanel);
	m_deleteButton = new QPushButton(tr("Delete"), m_leftPanel);
	m_deleteButton->setStyleSheet(R"(
		QPushButton {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #ef4444, stop:1 #dc2626);
		}
		QPushButton:hover {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #f87171, stop:1 #ef4444);
		}
		QPushButton:pressed {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #dc2626, stop:1 #b91c1c);
		}
	)");
	m_editButton->setEnabled(false);
	m_deleteButton->setEnabled(false);
	buttonLayout->addWidget(m_addButton);
	buttonLayout->addWidget(m_editButton);
	buttonLayout->addWidget(m_deleteButton);
	buttonLayout->addStretch();
	leftLayout->addLayout(buttonLayout);

	m_splitter->addWidget(m_leftPanel);

	// 右侧面板
	m_rightPanel = new QWidget();
	m_rightPanel->setObjectName("rightPanel");
	QVBoxLayout* rightLayout = new QVBoxLayout(m_rightPanel);
	rightLayout->setContentsMargins(20, 16, 16, 16);
	rightLayout->setSpacing(16);

	// 表单
	QFormLayout* formLayout = new QFormLayout();
	formLayout->setSpacing(12);
	formLayout->setLabelAlignment(Qt::AlignRight);
	formLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);

	// 标题
	QLabel* titleLabel = new QLabel(tr("Title:"), m_rightPanel);
	titleLabel->setStyleSheet("font-weight: 600; color: #475569;");
	m_titleEdit = new QLineEdit(m_rightPanel);
	m_titleEdit->setMinimumHeight(40);
	formLayout->addRow(titleLabel, m_titleEdit);

	// 分类
	QLabel* categoryLabel2 = new QLabel(tr("Category:"), m_rightPanel);
	categoryLabel2->setStyleSheet("font-weight: 600; color: #475569;");
	m_categoryEdit = new QComboBox(m_rightPanel);
	m_categoryEdit->setEditable(true);
	m_categoryEdit->setMinimumHeight(40);
	formLayout->addRow(categoryLabel2, m_categoryEdit);

	// 内容
	QLabel* contentLabel = new QLabel(tr("Content:"), m_rightPanel);
	contentLabel->setStyleSheet("font-weight: 600; color: #475569;");
	m_contentEdit = new QTextEdit(m_rightPanel);
	m_contentEdit->setMinimumHeight(250);
	formLayout->addRow(contentLabel, m_contentEdit);

	// 描述
	QLabel* descLabel = new QLabel(tr("Description:"), m_rightPanel);
	descLabel->setStyleSheet("font-weight: 600; color: #475569;");
	m_descriptionEdit = new QTextEdit(m_rightPanel);
	m_descriptionEdit->setMaximumHeight(100);
	formLayout->addRow(descLabel, m_descriptionEdit);

	rightLayout->addLayout(formLayout, 1);

	// 按钮
	QHBoxLayout* rightButtonLayout = new QHBoxLayout();
	rightButtonLayout->setSpacing(12);
	m_useButton = new QPushButton(tr("Use"), m_rightPanel);
	m_useButton->setStyleSheet(R"(
		QPushButton {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #8b5cf6, stop:1 #7c3aed);
		}
		QPushButton:hover {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #a78bfa, stop:1 #8b5cf6);
		}
		QPushButton:pressed {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #7c3aed, stop:1 #6d28d9);
		}
	)");
	m_useButton->setEnabled(false);
	m_saveButton = new QPushButton(tr("Save"), m_rightPanel);
	m_saveButton->setEnabled(false);
	m_cancelButton = new QPushButton(tr("Cancel"), m_rightPanel);
	m_cancelButton->setStyleSheet(R"(
		QPushButton {
			background: #f1f5f9;
			color: #475569;
			border: 2px solid #e2e8f0;
		}
		QPushButton:hover {
			background: #e2e8f0;
			border: 2px solid #cbd5e1;
		}
		QPushButton:pressed {
			background: #cbd5e1;
		}
	)");
	m_cancelButton->setEnabled(false);
	rightButtonLayout->addStretch();
	rightButtonLayout->addWidget(m_useButton);
	rightButtonLayout->addWidget(m_saveButton);
	rightButtonLayout->addWidget(m_cancelButton);
	rightLayout->addLayout(rightButtonLayout);

	m_splitter->addWidget(m_rightPanel);

	// 设置分割器比例
	m_splitter->setStretchFactor(0, 1);
	m_splitter->setStretchFactor(1, 2);
	m_splitter->setSizes({ 350, 650 });
	
	// 添加面板样式
	m_leftPanel->setStyleSheet(R"(
		QWidget#leftPanel {
			background: white;
			border-radius: 12px;
			border: 2px solid #e2e8f0;
		}
	)");
	m_rightPanel->setStyleSheet(R"(
		QWidget#rightPanel {
			background: white;
			border-radius: 12px;
			border: 2px solid #e2e8f0;
		}
	)");

	// 初始状态：禁用右侧编辑
	m_titleEdit->setEnabled(false);
	m_categoryEdit->setEnabled(false);
	m_contentEdit->setEnabled(false);
	m_descriptionEdit->setEnabled(false);
}

void PromptLibraryDialog::setupStyles()
{
	setStyleSheet(R"(
		QDialog {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
				stop:0 #f0f4f8, stop:1 #e2e8f0);
			font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;
		}
		QLabel {
			color: #1e293b;
			font-size: 14px;
			font-weight: 500;
		}
		QLineEdit {
			background: white;
			border: 2px solid #e2e8f0;
			border-radius: 8px;
			padding: 10px 14px;
			font-size: 14px;
			color: #1e293b;
			selection-background-color: #3b82f6;
			selection-color: white;
		}
		QLineEdit:focus {
			border: 2px solid #3b82f6;
			background: #fafbfc;
		}
		QLineEdit::placeholder {
			color: #94a3b8;
		}
		QTextEdit {
			background: white;
			border: 2px solid #e2e8f0;
			border-radius: 8px;
			padding: 12px;
			font-size: 14px;
			color: #1e293b;
			selection-background-color: #3b82f6;
			selection-color: white;
		}
		QTextEdit:focus {
			border: 2px solid #3b82f6;
			background: #fafbfc;
		}
		QComboBox {
			background: white;
			border: 2px solid #e2e8f0;
			border-radius: 8px;
			padding: 8px 12px;
			font-size: 14px;
			color: #1e293b;
			min-height: 20px;
		}
		QComboBox:hover {
			border: 2px solid #cbd5e1;
		}
		QComboBox:focus {
			border: 2px solid #3b82f6;
		}
		QComboBox::drop-down {
			border: none;
			width: 24px;
		}
		QComboBox::down-arrow {
			image: none;
			border-left: 5px solid transparent;
			border-right: 5px solid transparent;
			border-top: 6px solid #64748b;
			width: 0;
			height: 0;
		}
		QComboBox QAbstractItemView {
			background: white;
			border: 2px solid #e2e8f0;
			border-radius: 8px;
			selection-background-color: #3b82f6;
			selection-color: white;
			padding: 4px;
		}
		QListWidget {
			background: white;
			border: 2px solid #e2e8f0;
			border-radius: 10px;
			padding: 6px;
			outline: none;
		}
		QListWidget::item {
			padding: 14px 16px;
			border-radius: 8px;
			margin: 3px;
			border: 1px solid transparent;
		}
		QListWidget::item:hover {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 rgba(59, 130, 246, 0.08), stop:1 rgba(59, 130, 246, 0.12));
			border: 1px solid rgba(59, 130, 246, 0.2);
		}
		QListWidget::item:selected {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #3b82f6, stop:1 #2563eb);
			color: white;
			border: 1px solid #1d4ed8;
			font-weight: 500;
		}
		QPushButton {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #3b82f6, stop:1 #2563eb);
			border: none;
			color: white;
			padding: 10px 20px;
			border-radius: 8px;
			font-weight: 600;
			font-size: 14px;
			min-width: 90px;
			min-height: 36px;
		}
		QPushButton:hover {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #60a5fa, stop:1 #3b82f6);
		}
		QPushButton:pressed {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 #2563eb, stop:1 #1d4ed8);
		}
		QPushButton:disabled {
			background: #e2e8f0;
			color: #94a3b8;
		}
		QSplitter::handle {
			background: #e2e8f0;
			width: 2px;
		}
		QSplitter::handle:hover {
			background: #cbd5e1;
		}
	)");
}

void PromptLibraryDialog::connectSignals()
{
	connect(m_categoryCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
		this, &PromptLibraryDialog::onCategoryChanged);
	connect(m_searchEdit, &QLineEdit::textChanged,
		this, &PromptLibraryDialog::onSearchTextChanged);
	connect(m_promptList, &QListWidget::itemSelectionChanged,
		this, &PromptLibraryDialog::onPromptItemSelectionChanged);
	connect(m_addButton, &QPushButton::clicked,
		this, &PromptLibraryDialog::onAddPrompt);
	connect(m_editButton, &QPushButton::clicked,
		this, &PromptLibraryDialog::onEditPrompt);
	connect(m_deleteButton, &QPushButton::clicked,
		this, &PromptLibraryDialog::onDeletePrompt);
	connect(m_useButton, &QPushButton::clicked,
		this, &PromptLibraryDialog::onUsePrompt);
	connect(m_saveButton, &QPushButton::clicked,
		this, &PromptLibraryDialog::saveCurrentPrompt);
	connect(m_cancelButton, &QPushButton::clicked,
		this, [this]() {
			clearForm();
			m_currentEditingId.clear();
			m_isAddingNew = false;
			m_saveButton->setEnabled(false);
			m_cancelButton->setEnabled(false);
			m_titleEdit->setEnabled(false);
			m_categoryEdit->setEnabled(false);
			m_contentEdit->setEnabled(false);
			m_descriptionEdit->setEnabled(false);
		});

	if (m_library)
	{
		connect(m_library, &PromptLibrary::libraryUpdated,
			this, &PromptLibraryDialog::refreshPromptList);
		connect(m_library, &PromptLibrary::categoriesUpdated,
			this, &PromptLibraryDialog::refreshCategoryList);
	}
}

void PromptLibraryDialog::refreshCategoryList()
{
	if (!m_library)
	{
		return;
	}

	QString currentCategory = m_categoryCombo->currentData().toString();
	
	m_categoryCombo->clear();
	m_categoryCombo->addItem(tr("All"), QString());
	
	QStringList categories = m_library->getCategories();
	for (const QString& category : categories)
	{
		m_categoryCombo->addItem(category, category);
	}

	// 恢复之前的选择
	int index = m_categoryCombo->findData(currentCategory);
	if (index >= 0)
	{
		m_categoryCombo->setCurrentIndex(index);
	}

	// 更新分类编辑框
	m_categoryEdit->clear();
	m_categoryEdit->addItems(categories);
}

void PromptLibraryDialog::refreshPromptList()
{
	if (!m_library)
	{
		return;
	}

	QString currentId = getCurrentPromptId();
	QString category = m_categoryCombo->currentData().toString();
	QString searchText = m_searchEdit->text();

	QList<PromptItem> prompts;
	if (!searchText.isEmpty())
	{
		prompts = m_library->searchPrompts(searchText);
		// 如果指定了分类，进一步筛选
		if (!category.isEmpty())
		{
			QList<PromptItem> filtered;
			for (const PromptItem& item : prompts)
			{
				if (item.category == category)
				{
					filtered.append(item);
				}
			}
			prompts = filtered;
		}
	}
	else if (!category.isEmpty())
	{
		prompts = m_library->getPromptsByCategory(category);
	}
	else
	{
		prompts = m_library->getAllPrompts();
	}

	loadPromptsToList(prompts);

	// 恢复之前的选择
	if (!currentId.isEmpty())
	{
		for (int i = 0; i < m_promptList->count(); ++i)
		{
			QListWidgetItem* item = m_promptList->item(i);
			if (item->data(Qt::UserRole).toString() == currentId)
			{
				m_promptList->setCurrentItem(item);
				break;
			}
		}
	}
}

void PromptLibraryDialog::loadPromptsToList(const QList<PromptItem>& prompts)
{
	m_promptList->clear();
	
	for (const PromptItem& item : prompts)
	{
		QListWidgetItem* listItem = new QListWidgetItem(m_promptList);
		listItem->setText(item.title);
		listItem->setData(Qt::UserRole, item.id);
		listItem->setToolTip(item.description.isEmpty() ? item.content : item.description);
	}
}

void PromptLibraryDialog::onCategoryChanged(const QString& category)
{
	Q_UNUSED(category);
	refreshPromptList();
}

void PromptLibraryDialog::onSearchTextChanged(const QString& text)
{
	Q_UNUSED(text);
	refreshPromptList();
}

void PromptLibraryDialog::onPromptItemSelectionChanged()
{
	QListWidgetItem* item = m_promptList->currentItem();
	bool hasSelection = item != nullptr;
	
	m_editButton->setEnabled(hasSelection);
	m_deleteButton->setEnabled(hasSelection);
	m_useButton->setEnabled(hasSelection);

	if (hasSelection && !m_isAddingNew)
	{
		QString id = item->data(Qt::UserRole).toString();
		PromptItem prompt = m_library->getPrompt(id);
		fillForm(prompt);
		m_currentEditingId = id;
	}
}

void PromptLibraryDialog::onAddPrompt()
{
	clearForm();
	m_isAddingNew = true;
	m_currentEditingId.clear();
	
	m_titleEdit->setEnabled(true);
	m_categoryEdit->setEnabled(true);
	m_contentEdit->setEnabled(true);
	m_descriptionEdit->setEnabled(true);
	m_saveButton->setEnabled(true);
	m_cancelButton->setEnabled(true);
	
	m_titleEdit->setFocus();
}

void PromptLibraryDialog::onEditPrompt()
{
	QString id = getCurrentPromptId();
	if (id.isEmpty())
	{
		return;
	}

	m_isAddingNew = false;
	m_currentEditingId = id;
	
	m_titleEdit->setEnabled(true);
	m_categoryEdit->setEnabled(true);
	m_contentEdit->setEnabled(true);
	m_descriptionEdit->setEnabled(true);
	m_saveButton->setEnabled(true);
	m_cancelButton->setEnabled(true);
	
	m_titleEdit->setFocus();
}

void PromptLibraryDialog::onDeletePrompt()
{
	QString id = getCurrentPromptId();
	if (id.isEmpty())
	{
		return;
	}

	PromptItem prompt = m_library->getPrompt(id);
	int ret = QMessageBox::question(this, tr("Confirm Delete"),
		tr("Are you sure you want to delete prompt \"%1\"?").arg(prompt.title),
		QMessageBox::Yes | QMessageBox::No);

	if (ret == QMessageBox::Yes)
	{
		if (m_library->deletePrompt(id))
		{
			clearForm();
			m_currentEditingId.clear();
			refreshPromptList();
		}
	}
}

void PromptLibraryDialog::onUsePrompt()
{
	QString content = getSelectedPromptContent();
	if (!content.isEmpty())
	{
		emit promptSelected(content);
		accept();
	}
}

bool PromptLibraryDialog::saveCurrentPrompt()
{
	if (!validateForm())
	{
		return false;
	}

	QString title = m_titleEdit->text().trimmed();
	QString content = m_contentEdit->toPlainText().trimmed();
	QString category = m_categoryEdit->currentText().trimmed();
	QString description = m_descriptionEdit->toPlainText().trimmed();

	bool success = false;
	if (m_isAddingNew)
	{
		QString id = m_library->addPrompt(title, content, category, description);
		success = !id.isEmpty();
		if (success)
		{
			m_currentEditingId = id;
			m_isAddingNew = false;
		}
	}
	else
	{
		success = m_library->updatePrompt(m_currentEditingId, title, content, category, description);
	}

	if (success)
	{
		refreshPromptList();
		refreshCategoryList();
		clearForm();
		m_saveButton->setEnabled(false);
		m_cancelButton->setEnabled(false);
		m_titleEdit->setEnabled(false);
		m_categoryEdit->setEnabled(false);
		m_contentEdit->setEnabled(false);
		m_descriptionEdit->setEnabled(false);
		return true;
	}
	else
	{
		QString title = tr("Save Failed");
		QString message = tr("Failed to save prompt. Please check the input content.");
        //QMessageBox::warning(this, title, message);
		return false;
	}
}

void PromptLibraryDialog::clearForm()
{
	m_titleEdit->clear();
	m_contentEdit->clear();
	m_descriptionEdit->clear();
	m_categoryEdit->setCurrentIndex(0);
}

void PromptLibraryDialog::fillForm(const PromptItem& item)
{
	m_titleEdit->setText(item.title);
	m_contentEdit->setPlainText(item.content);
	m_descriptionEdit->setPlainText(item.description);
	
	int index = m_categoryEdit->findText(item.category);
	if (index >= 0)
	{
		m_categoryEdit->setCurrentIndex(index);
	}
	else
	{
		m_categoryEdit->setCurrentText(item.category);
	}
}

bool PromptLibraryDialog::validateForm() const
{
	if (m_titleEdit->text().trimmed().isEmpty())
	{
		QString title = tr("Validation Failed");
		QString message = tr("Title cannot be empty.");
        //QMessageBox::warning(const_cast<PromptLibraryDialog*>(this), title, message);
		return false;
	}

	if (m_contentEdit->toPlainText().trimmed().isEmpty())
	{
		QString title = tr("Validation Failed");
		QString message = tr("Content cannot be empty.");
        //QMessageBox::warning(const_cast<PromptLibraryDialog*>(this), title, message);
		return false;
	}

	return true;
}

QString PromptLibraryDialog::getCurrentPromptId() const
{
	QListWidgetItem* item = m_promptList->currentItem();
	if (!item)
	{
		return QString();
	}
	return item->data(Qt::UserRole).toString();
}

