#include "PromptLibraryDialog.h"
#include <QApplication>
#include <QHeaderView>
#include <QSplitter>
#include <QAbstractItemView>
#include <QScrollArea>
#include <QSpacerItem>
#include <QSizePolicy>

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
	mainLayout->setContentsMargins(24, 24, 24, 24);
	mainLayout->setSpacing(0);

	// 标题区域
	QWidget* headerWidget = new QWidget(this);
	headerWidget->setObjectName("headerWidget");
	headerWidget->setFixedHeight(70);
	QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
	headerLayout->setContentsMargins(0, 0, 0, 0);
	headerLayout->setSpacing(16);
	
	QLabel* titleLabel = new QLabel(tr("Prompt Library"), headerWidget);
	titleLabel->setObjectName("dialogTitle");
	titleLabel->setAlignment(Qt::AlignCenter);
	headerLayout->addStretch();
	headerLayout->addWidget(titleLabel);
	headerLayout->addStretch();
	
	mainLayout->addWidget(headerWidget);

	// 创建分割器
	m_splitter = new QSplitter(Qt::Horizontal, this);
	m_splitter->setObjectName("mainSplitter");
	mainLayout->addWidget(m_splitter, 1);

	// 左侧面板
	m_leftPanel = new QWidget();
	m_leftPanel->setObjectName("leftPanel");
	QVBoxLayout* leftLayout = new QVBoxLayout(m_leftPanel);
	leftLayout->setContentsMargins(20, 20, 20, 20);
	leftLayout->setSpacing(16);

	// 分类选择
	QLabel* categoryLabel = new QLabel(tr("Category"), m_leftPanel);
	categoryLabel->setObjectName("sectionLabel");
	m_categoryCombo = new QComboBox(m_leftPanel);
	m_categoryCombo->setObjectName("categoryCombo");
	m_categoryCombo->addItem(tr("All Categories"), QString());
	m_categoryCombo->setItemData(0, tr("Show all prompts"), Qt::ToolTipRole);
	m_categoryCombo->setMinimumHeight(42);
	leftLayout->addWidget(categoryLabel);
	leftLayout->addWidget(m_categoryCombo);

	// 搜索框
	QLabel* searchLabel = new QLabel(tr("Search"), m_leftPanel);
	searchLabel->setObjectName("sectionLabel");
	m_searchEdit = new QLineEdit(m_leftPanel);
	m_searchEdit->setObjectName("searchEdit");
	m_searchEdit->setPlaceholderText(tr("Search by title, content, or description..."));
	m_searchEdit->setMinimumHeight(42);
	leftLayout->addWidget(searchLabel);
	leftLayout->addWidget(m_searchEdit);

	// 提示词列表
	QLabel* listLabel = new QLabel(tr("Prompts"), m_leftPanel);
	listLabel->setObjectName("sectionLabel");
	m_promptList = new QListWidget(m_leftPanel);
	m_promptList->setObjectName("promptList");
	m_promptList->setSelectionMode(QAbstractItemView::SingleSelection);
	leftLayout->addWidget(listLabel);
	leftLayout->addWidget(m_promptList, 1);

	// 按钮组
	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->setSpacing(10);
	m_addButton = new QPushButton(tr("Add"), m_leftPanel);
	m_addButton->setObjectName("addButton");
	m_editButton = new QPushButton(tr("Edit"), m_leftPanel);
	m_editButton->setObjectName("editButton");
	m_deleteButton = new QPushButton(tr("Delete"), m_leftPanel);
	m_deleteButton->setObjectName("deleteButton");
	m_editButton->setEnabled(false);
	m_deleteButton->setEnabled(false);
	buttonLayout->addWidget(m_addButton);
	buttonLayout->addWidget(m_editButton);
	buttonLayout->addWidget(m_deleteButton);
	leftLayout->addLayout(buttonLayout);

	m_splitter->addWidget(m_leftPanel);

	// 右侧面板
	m_rightPanel = new QWidget();
	m_rightPanel->setObjectName("rightPanel");
	QVBoxLayout* rightLayout = new QVBoxLayout(m_rightPanel);
	rightLayout->setContentsMargins(24, 24, 24, 24);
	rightLayout->setSpacing(20);

	// 表单标题
	QLabel* formTitle = new QLabel(tr("Prompt Details"), m_rightPanel);
	formTitle->setObjectName("formTitle");
	rightLayout->addWidget(formTitle);

	// 创建滚动区域来包装表单
	QScrollArea* scrollArea = new QScrollArea(m_rightPanel);
	scrollArea->setWidgetResizable(true);
	scrollArea->setFrameShape(QFrame::NoFrame);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
	
	// 表单容器
	QWidget* formContainer = new QWidget();
	formContainer->setStyleSheet("background: transparent;");
	QFormLayout* formLayout = new QFormLayout(formContainer);
	formLayout->setSpacing(16);
	formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
	formLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
	formLayout->setVerticalSpacing(20);
	formLayout->setContentsMargins(0, 0, 0, 0);

	// 标题
	QLabel* titleFormLabel = new QLabel(tr("Title"), formContainer);
	titleFormLabel->setObjectName("formLabel");
	m_titleEdit = new QLineEdit(formContainer);
	m_titleEdit->setObjectName("formInput");
	m_titleEdit->setPlaceholderText(tr("Enter a descriptive title for this prompt"));
	m_titleEdit->setMinimumHeight(42);
	m_titleEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	formLayout->addRow(titleFormLabel, m_titleEdit);

	// 分类
	QLabel* categoryFormLabel = new QLabel(tr("Category"), formContainer);
	categoryFormLabel->setObjectName("formLabel");
	m_categoryEdit = new QComboBox(formContainer);
	m_categoryEdit->setObjectName("formInput");
	m_categoryEdit->setEditable(true);
	m_categoryEdit->setMinimumHeight(42);
	m_categoryEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_categoryEdit->lineEdit()->setPlaceholderText(tr("Select or enter a category"));
	formLayout->addRow(categoryFormLabel, m_categoryEdit);

	// 内容
	QLabel* contentLabel = new QLabel(tr("Content"), formContainer);
	contentLabel->setObjectName("formLabel");
	m_contentEdit = new QTextEdit(formContainer);
	m_contentEdit->setObjectName("formTextArea");
	m_contentEdit->setPlaceholderText(tr("Enter the prompt content here. You can use variables like {{variable}} for dynamic content."));
	m_contentEdit->setMinimumHeight(200);
	m_contentEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	formLayout->addRow(contentLabel, m_contentEdit);

	// 描述
	QLabel* descLabel = new QLabel(tr("Description"), formContainer);
	descLabel->setObjectName("formLabel");
	m_descriptionEdit = new QTextEdit(formContainer);
	m_descriptionEdit->setObjectName("formTextArea");
	m_descriptionEdit->setPlaceholderText(tr("Optional: Add a brief description to help identify this prompt"));
	m_descriptionEdit->setFixedHeight(100);
	m_descriptionEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	formLayout->addRow(descLabel, m_descriptionEdit);
	
	// 添加弹性空间
	formLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	scrollArea->setWidget(formContainer);
	rightLayout->addWidget(scrollArea, 1);

	// 按钮
	QHBoxLayout* rightButtonLayout = new QHBoxLayout();
	rightButtonLayout->setSpacing(12);
	m_useButton = new QPushButton(tr("Use Prompt"), m_rightPanel);
	m_useButton->setObjectName("primaryButton");
	m_useButton->setEnabled(false);
	m_saveButton = new QPushButton(tr("Save"), m_rightPanel);
	m_saveButton->setObjectName("primaryButton");
	m_saveButton->setEnabled(false);
	m_cancelButton = new QPushButton(tr("Cancel"), m_rightPanel);
	m_cancelButton->setObjectName("ghostButton");
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
	m_splitter->setSizes({ 380, 720 });

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
				stop:0 #eef2ff, stop:1 #e0f2fe);
			border-radius: 20px;
			font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;
			font-size: 14px;
			color: #0f172a;
		}
		
		/* 标题区域 */
		#headerWidget {
			background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
				stop:0 rgba(255, 255, 255, 0.96), stop:1 rgba(241, 245, 249, 0.94));
			border: none;
			border-radius: 20px 20px 0 0;
			border-bottom: 1px solid rgba(226, 232, 240, 0.75);
		}
		
		#dialogTitle {
			color: #0f172a;
			font-weight: 700;
			font-size: 22px;
			background: transparent;
			padding: 0;
		}
		
		/* 左侧面板 */
		#leftPanel {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
				stop:0 #fefefe, stop:1 #f3f8ff);
			border-radius: 18px;
			border: 1px solid rgba(203, 213, 225, 0.5);
		}
		
		#sectionLabel {
			color: #475569;
			font-size: 13px;
			font-weight: 600;
			letter-spacing: 0.3px;
			text-transform: uppercase;
			background: transparent;
			margin-bottom: 4px;
		}
		
		/* 右侧面板 */
		#rightPanel {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
				stop:0 #fdfdff, stop:1 #f1f5ff);
			border-radius: 18px;
			border: 1px solid rgba(203, 213, 225, 0.5);
		}
		
		#formTitle {
			color: #0f172a;
			font-size: 18px;
			font-weight: 600;
			background: transparent;
			margin-bottom: 8px;
		}
		
		#formLabel {
			color: #475569;
			font-size: 14px;
			font-weight: 600;
			background: transparent;
			margin-bottom: 6px;
		}
		
		/* 输入框样式 */
		#searchEdit, #categoryCombo, #formInput {
			border: 1px solid rgba(148, 163, 184, 0.45);
			border-radius: 14px;
			padding: 10px 16px;
			background: rgba(255, 255, 255, 0.98);
			font-size: 14px;
			color: #1e293b;
		}
		
		#searchEdit:focus, #categoryCombo:focus, #formInput:focus {
			border-color: #2563eb;
			background: rgba(255, 255, 255, 1);
			box-shadow: 0 0 0 3px rgba(37, 99, 235, 0.1);
		}
		
		#searchEdit:hover, #categoryCombo:hover, #formInput:hover {
			border-color: rgba(148, 163, 184, 0.65);
			background: rgba(255, 255, 255, 1);
		}
		
		#searchEdit::placeholder {
			color: #94a3b8;
		}
		
		#formTextArea {
			border: 1px solid rgba(148, 163, 184, 0.45);
			border-radius: 14px;
			padding: 14px 16px;
			background: rgba(255, 255, 255, 0.98);
			font-size: 14px;
			color: #1e293b;
			line-height: 1.6;
		}
		
		#formTextArea:focus {
			border-color: #2563eb;
			background: rgba(255, 255, 255, 1);
			box-shadow: 0 0 0 3px rgba(37, 99, 235, 0.1);
		}
		
		#formTextArea:hover {
			border-color: rgba(148, 163, 184, 0.65);
			background: rgba(255, 255, 255, 1);
		}
		
		#formTextArea::placeholder {
			color: #94a3b8;
		}
		
		/* ComboBox 下拉箭头 */
		#categoryCombo::drop-down, #formInput::drop-down {
			border: none;
			width: 28px;
		}
		
		#categoryCombo::down-arrow, #formInput::down-arrow {
			image: none;
			border-left: 5px solid transparent;
			border-right: 5px solid transparent;
			border-top: 6px solid #64748b;
			width: 0;
			height: 0;
		}
		
		#categoryCombo QAbstractItemView, #formInput QAbstractItemView {
			background: rgba(255, 255, 255, 0.98);
			border: 1px solid rgba(203, 213, 225, 0.8);
			border-radius: 12px;
			selection-background-color: rgba(37, 99, 235, 0.15);
			selection-color: #0f172a;
			padding: 6px;
		}
		
		/* 列表样式 */
		#promptList {
			background: rgba(255, 255, 255, 0.6);
			border: 1px solid rgba(203, 213, 225, 0.4);
			border-radius: 14px;
			padding: 8px;
			outline: none;
		}
		
		#promptList::item {
			border-radius: 12px;
			margin: 6px 0;
			border: 1px solid transparent;
			background: transparent;
		}
		
		#promptList::item:hover {
			background: transparent;
		}
		
		#promptList::item:selected {
			background: transparent;
		}
		
		/* 列表项widget样式 */
		#promptItemWidget {
			background: rgba(255, 255, 255, 0.8);
			border: 1px solid rgba(203, 213, 225, 0.4);
			border-radius: 12px;
			margin: 0;
		}
		
		#promptList::item:hover #promptItemWidget {
			background: rgba(255, 255, 255, 0.95);
			border-color: rgba(37, 99, 235, 0.3);
			box-shadow: 0 2px 8px rgba(37, 99, 235, 0.08);
		}
		
		/* 空状态样式 */
		#emptyStateWidget {
			background: transparent;
			border: none;
		}
		
		#emptyIcon {
			font-size: 48px;
			background: transparent;
			opacity: 0.5;
		}
		
		#emptyText {
			color: #94a3b8;
			font-size: 15px;
			font-weight: 500;
			background: transparent;
		}
		
		#emptyHint {
			color: #cbd5e1;
			font-size: 13px;
			background: transparent;
		}
		
		/* 按钮样式 */
		#primaryButton {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #2563eb, stop:1 #38bdf8);
			border: none;
			color: #ffffff;
			padding: 11px 28px;
			border-radius: 14px;
			font-weight: 600;
			font-size: 14px;
			min-width: 100px;
			min-height: 42px;
		}
		
		#primaryButton:hover {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #1d4ed8, stop:1 #1fb6ff);
		}
		
		#primaryButton:pressed {
			background: #1e3a8a;
		}
		
		#primaryButton:disabled {
			background: #cbd5e1;
			color: #94a3b8;
		}
		
		#ghostButton {
			background: transparent;
			color: #475569;
			border: 1px solid rgba(148, 163, 184, 0.5);
			padding: 11px 28px;
			border-radius: 14px;
			font-weight: 600;
			font-size: 14px;
			min-width: 100px;
			min-height: 42px;
		}
		
		#ghostButton:hover {
			background: rgba(241, 245, 249, 0.8);
			border-color: rgba(148, 163, 184, 0.7);
		}
		
		#ghostButton:pressed {
			background: rgba(226, 232, 240, 0.9);
		}
		
		#ghostButton:disabled {
			background: transparent;
			color: #cbd5e1;
			border-color: #cbd5e1;
		}
		
		#addButton {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #10b981, stop:1 #34d399);
			border: none;
			color: #ffffff;
			padding: 10px 20px;
			border-radius: 12px;
			font-weight: 600;
			font-size: 13px;
			min-height: 38px;
		}
		
		#addButton:hover {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #059669, stop:1 #10b981);
		}
		
		#addButton:pressed {
			background: #047857;
		}
		
		#editButton {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #2563eb, stop:1 #38bdf8);
			border: none;
			color: #ffffff;
			padding: 10px 20px;
			border-radius: 12px;
			font-weight: 600;
			font-size: 13px;
			min-height: 38px;
		}
		
		#editButton:hover {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #1d4ed8, stop:1 #1fb6ff);
		}
		
		#editButton:pressed {
			background: #1e3a8a;
		}
		
		#editButton:disabled {
			background: #cbd5e1;
			color: #94a3b8;
		}
		
		#deleteButton {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #ef4444, stop:1 #f87171);
			border: none;
			color: #ffffff;
			padding: 10px 20px;
			border-radius: 12px;
			font-weight: 600;
			font-size: 13px;
			min-height: 38px;
		}
		
		#deleteButton:hover {
			background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
				stop:0 #dc2626, stop:1 #ef4444);
		}
		
		#deleteButton:pressed {
			background: #b91c1c;
		}
		
		#deleteButton:disabled {
			background: #cbd5e1;
			color: #94a3b8;
		}
		
		/* 分割器 */
		#mainSplitter::handle {
			background: rgba(203, 213, 225, 0.4);
			width: 2px;
		}
		
		#mainSplitter::handle:hover {
			background: rgba(148, 163, 184, 0.6);
		}
		
		/* 滚动条样式 */
		QScrollBar:vertical {
			background: transparent;
			width: 10px;
			margin: 8px 0;
		}
		
		QScrollBar::handle:vertical {
			background: rgba(148, 163, 184, 0.5);
			border-radius: 4px;
			min-height: 30px;
		}
		
		QScrollBar::handle:vertical:hover {
			background: rgba(59, 130, 246, 0.7);
		}
		
		QScrollBar::add-line:vertical,
		QScrollBar::sub-line:vertical,
		QScrollBar::add-page:vertical,
		QScrollBar::sub-page:vertical {
			height: 0;
			width: 0;
		}
		
		QScrollBar:horizontal {
			background: transparent;
			height: 10px;
			margin: 0 8px;
		}
		
		QScrollBar::handle:horizontal {
			background: rgba(148, 163, 184, 0.5);
			border-radius: 4px;
			min-width: 30px;
		}
		
		QScrollBar::handle:horizontal:hover {
			background: rgba(59, 130, 246, 0.7);
		}
		
		QScrollBar::add-line:horizontal,
		QScrollBar::sub-line:horizontal,
		QScrollBar::add-page:horizontal,
		QScrollBar::sub-page:horizontal {
			height: 0;
			width: 0;
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
	m_categoryCombo->addItem(tr("All Categories"), QString());
	
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
	
	if (prompts.isEmpty())
	{
		// 显示空状态
		QListWidgetItem* emptyItem = new QListWidgetItem(m_promptList);
		emptyItem->setFlags(Qt::NoItemFlags);
		emptyItem->setSizeHint(QSize(0, 200));
		
		QWidget* emptyWidget = new QWidget();
		emptyWidget->setObjectName("emptyStateWidget");
		QVBoxLayout* emptyLayout = new QVBoxLayout(emptyWidget);
		emptyLayout->setContentsMargins(20, 40, 20, 40);
		emptyLayout->setSpacing(12);
		emptyLayout->setAlignment(Qt::AlignCenter);
		
		QLabel* emptyIcon = new QLabel(emptyWidget);
		emptyIcon->setObjectName("emptyIcon");
		emptyIcon->setText("📝");
		emptyIcon->setAlignment(Qt::AlignCenter);
		emptyIcon->setStyleSheet("font-size: 48px; background: transparent;");
		
		QLabel* emptyText = new QLabel(tr("No prompts found"), emptyWidget);
		emptyText->setObjectName("emptyText");
		emptyText->setAlignment(Qt::AlignCenter);
		emptyText->setStyleSheet(R"(
			color: #94a3b8;
			font-size: 15px;
			font-weight: 500;
			background: transparent;
		)");
		
		QLabel* emptyHint = new QLabel(tr("Click 'Add' to create your first prompt"), emptyWidget);
		emptyHint->setObjectName("emptyHint");
		emptyHint->setAlignment(Qt::AlignCenter);
		emptyHint->setStyleSheet(R"(
			color: #cbd5e1;
			font-size: 13px;
			background: transparent;
		)");
		
		emptyLayout->addWidget(emptyIcon);
		emptyLayout->addWidget(emptyText);
		emptyLayout->addWidget(emptyHint);
		
		m_promptList->setItemWidget(emptyItem, emptyWidget);
		return;
	}
	
	for (const PromptItem& item : prompts)
	{
		QListWidgetItem* listItem = new QListWidgetItem(m_promptList);
		listItem->setData(Qt::UserRole, item.id);
		listItem->setSizeHint(QSize(0, 80));
		
		// 创建自定义widget
		QWidget* itemWidget = new QWidget();
		itemWidget->setObjectName("promptItemWidget");
		QVBoxLayout* itemLayout = new QVBoxLayout(itemWidget);
		itemLayout->setContentsMargins(16, 12, 16, 12);
		itemLayout->setSpacing(6);
		
		// 标题行
		QHBoxLayout* titleLayout = new QHBoxLayout();
		titleLayout->setContentsMargins(0, 0, 0, 0);
		titleLayout->setSpacing(8);
		
		QLabel* titleLabel = new QLabel(item.title.isEmpty() ? tr("Untitled") : item.title, itemWidget);
		titleLabel->setObjectName("promptTitle");
		titleLabel->setStyleSheet(R"(
			color: #0f172a;
			font-size: 15px;
			font-weight: 600;
			background: transparent;
		)");
		
		// 分类标签
		if (!item.category.isEmpty())
		{
			QLabel* categoryLabel = new QLabel(item.category, itemWidget);
			categoryLabel->setObjectName("promptCategory");
			categoryLabel->setStyleSheet(R"(
				color: #64748b;
				font-size: 11px;
				font-weight: 500;
				background: rgba(148, 163, 184, 0.15);
				border-radius: 10px;
				padding: 2px 8px;
			)");
			titleLayout->addWidget(categoryLabel);
		}
		
		titleLayout->addWidget(titleLabel, 1);
		itemLayout->addLayout(titleLayout);
		
		// 描述或内容预览
		QString preview = item.description.isEmpty() ? item.content : item.description;
		if (!preview.isEmpty())
		{
			// 限制预览长度
			if (preview.length() > 80)
			{
				preview = preview.left(80) + "...";
			}
			
			QLabel* descLabel = new QLabel(preview, itemWidget);
			descLabel->setObjectName("promptDescription");
			descLabel->setWordWrap(true);
			descLabel->setStyleSheet(R"(
				color: #64748b;
				font-size: 13px;
				background: transparent;
				line-height: 1.4;
			)");
			itemLayout->addWidget(descLabel);
		}
		
		m_promptList->setItemWidget(listItem, itemWidget);
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
	// 更新所有列表项的样式
	for (int i = 0; i < m_promptList->count(); ++i)
	{
		QListWidgetItem* listItem = m_promptList->item(i);
		if (!listItem || listItem->flags() == Qt::NoItemFlags)
			continue;
			
		QWidget* itemWidget = m_promptList->itemWidget(listItem);
		if (itemWidget)
		{
			bool isSelected = (listItem == m_promptList->currentItem());
			QString style = isSelected 
				? R"(
					background: rgba(37, 99, 235, 0.08);
					border: 1px solid rgba(37, 99, 235, 0.4);
					box-shadow: 0 2px 12px rgba(37, 99, 235, 0.12);
				)"
				: R"(
					background: rgba(255, 255, 255, 0.8);
					border: 1px solid rgba(203, 213, 225, 0.4);
				)";
			itemWidget->setStyleSheet("QWidget#promptItemWidget { " + style + " }");
			
			// 更新标题颜色
			QLabel* titleLabel = itemWidget->findChild<QLabel*>("promptTitle");
			if (titleLabel)
			{
				titleLabel->setStyleSheet(QString(R"(
					color: %1;
					font-size: 15px;
					font-weight: 600;
					background: transparent;
				)").arg(isSelected ? "#1e40af" : "#0f172a"));
			}
			
			// 更新分类标签颜色
			QLabel* categoryLabel = itemWidget->findChild<QLabel*>("promptCategory");
			if (categoryLabel)
			{
				categoryLabel->setStyleSheet(QString(R"(
					color: %1;
					font-size: 11px;
					font-weight: 500;
					background: %2;
					border-radius: 10px;
					padding: 2px 8px;
				)").arg(isSelected ? "#1e40af" : "#64748b")
				  .arg(isSelected ? "rgba(37, 99, 235, 0.15)" : "rgba(148, 163, 184, 0.15)"));
			}
			
			// 更新描述颜色
			QLabel* descLabel = itemWidget->findChild<QLabel*>("promptDescription");
			if (descLabel)
			{
				descLabel->setStyleSheet(QString(R"(
					color: %1;
					font-size: 13px;
					background: transparent;
					line-height: 1.4;
				)").arg(isSelected ? "#475569" : "#64748b"));
			}
		}
	}
	
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

