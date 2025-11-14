#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>

// 提示词数据结构
struct PromptItem
{
	QString id;              // 唯一标识符
	QString title;           // 标题
	QString content;         // 提示词内容
	QString category;        // 分类
	QString description;     // 描述
	QDateTime createdAt;     // 创建时间
	QDateTime updatedAt;     // 更新时间

	// 转换为JSON对象
	QJsonObject toJson() const;
	// 从JSON对象创建
	static PromptItem fromJson(const QJsonObject& obj);
	// 验证数据有效性
	bool isValid() const;
};

// 提示词库管理类
class PromptLibrary : public QObject
{
	Q_OBJECT

public:
	// 构造函数
	explicit PromptLibrary(QObject* parent = nullptr);
	// 析构函数
	~PromptLibrary();

	// 加载提示词库
	bool load();
	// 保存提示词库
	bool save() const;
	// 获取存储文件路径
	QString getStoragePath() const;

	// 提示词管理
	// 添加提示词
	QString addPrompt(const QString& title, const QString& content, 
		const QString& category = QString(), const QString& description = QString());
	// 更新提示词
	bool updatePrompt(const QString& id, const QString& title, const QString& content,
		const QString& category = QString(), const QString& description = QString());
	// 删除提示词
	bool deletePrompt(const QString& id);
	// 获取提示词
	PromptItem getPrompt(const QString& id) const;
	// 获取所有提示词
	QList<PromptItem> getAllPrompts() const;
	// 根据分类获取提示词
	QList<PromptItem> getPromptsByCategory(const QString& category) const;
	// 搜索提示词
	QList<PromptItem> searchPrompts(const QString& keyword) const;

	// 分类管理
	// 获取所有分类
	QStringList getCategories() const;
	// 添加分类
	bool addCategory(const QString& category);
	// 删除分类（会删除该分类下的所有提示词）
	bool deleteCategory(const QString& category);
	// 重命名分类
	bool renameCategory(const QString& oldName, const QString& newName);
	// 获取分类下的提示词数量
	int getCategoryCount(const QString& category) const;

	// 统计信息
	// 获取提示词总数
	int getTotalCount() const;
	// 获取分类总数
	int getTotalCategoryCount() const;

signals:
	// 提示词库已更新信号
	void libraryUpdated();
	// 提示词已添加信号
	void promptAdded(const QString& id);
	// 提示词已更新信号
	void promptUpdated(const QString& id);
	// 提示词已删除信号
	void promptDeleted(const QString& id);
	// 分类已更新信号
	void categoriesUpdated();

private:
	// 生成唯一ID
	QString generateId() const;
	// 确保存储目录存在
	bool ensureStorageDirectory() const;
	// 初始化默认提示词
	void initializeDefaultPrompts();
	// 验证分类名称
	bool isValidCategoryName(const QString& category) const;

	QList<PromptItem> m_prompts;  // 提示词列表
	QString m_storagePath;        // 存储文件路径
};

