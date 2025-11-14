#pragma once

#include <QDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSplitter>
#include <QListWidgetItem>
#include <QMessageBox>
#include "PromptLibrary.h"

// 提示词库管理对话框
class PromptLibraryDialog : public QDialog
{
	Q_OBJECT

public:
	// 构造函数
	explicit PromptLibraryDialog(PromptLibrary* library, QWidget* parent = nullptr);
	// 析构函数
	~PromptLibraryDialog();

	// 获取选中的提示词内容
	QString getSelectedPromptContent() const;

signals:
	// 提示词已选择信号
	void promptSelected(const QString& content);

private slots:
	// 分类选择改变
	void onCategoryChanged(const QString& category);
	// 提示词列表项选择改变
	void onPromptItemSelectionChanged();
	// 添加新提示词
	void onAddPrompt();
	// 编辑当前提示词
	void onEditPrompt();
	// 删除当前提示词
	void onDeletePrompt();
	// 使用选中的提示词
	void onUsePrompt();
	// 搜索提示词
	void onSearchTextChanged(const QString& text);
	// 刷新列表
	void refreshPromptList();
	// 刷新分类列表
	void refreshCategoryList();

private:
	// 初始化UI
	void setupUI();
	// 设置样式
	void setupStyles();
	// 连接信号槽
	void connectSignals();
	// 加载提示词到列表
	void loadPromptsToList(const QList<PromptItem>& prompts);
	// 清空表单
	void clearForm();
	// 填充表单
	void fillForm(const PromptItem& item);
	// 验证表单
	bool validateForm() const;
	// 获取当前选中的提示词ID
	QString getCurrentPromptId() const;
	// 保存当前编辑的提示词
	bool saveCurrentPrompt();

	PromptLibrary* m_library;  // 提示词库对象

	// UI组件
	QSplitter* m_splitter;
	QWidget* m_leftPanel;
	QWidget* m_rightPanel;
	
	// 左侧面板
	QComboBox* m_categoryCombo;
	QLineEdit* m_searchEdit;
	QListWidget* m_promptList;
	QPushButton* m_addButton;
	QPushButton* m_editButton;
	QPushButton* m_deleteButton;
	
	// 右侧面板
	QLineEdit* m_titleEdit;
	QComboBox* m_categoryEdit;
	QTextEdit* m_contentEdit;
	QTextEdit* m_descriptionEdit;
	QPushButton* m_saveButton;
	QPushButton* m_cancelButton;
	QPushButton* m_useButton;

	QString m_currentEditingId;  // 当前正在编辑的提示词ID
	bool m_isAddingNew;          // 是否正在添加新提示词
};

