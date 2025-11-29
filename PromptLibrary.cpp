#include "PromptLibrary.h"
#include "AppConfigRepository.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <QDebug>
#include <algorithm>

namespace {
	const QString kDefaultCategory = QStringLiteral("Default");
}

// PromptItem 实现
QJsonObject PromptItem::toJson() const
{
	QJsonObject obj;
	obj["id"] = id;
	obj["title"] = title;
	obj["content"] = content;
	obj["category"] = category.isEmpty() ? kDefaultCategory : category;
	obj["description"] = description;
	obj["createdAt"] = createdAt.toString(Qt::ISODate);
	obj["updatedAt"] = updatedAt.toString(Qt::ISODate);
	return obj;
}

PromptItem PromptItem::fromJson(const QJsonObject& obj)
{
	PromptItem item;
	item.id = obj["id"].toString();
	item.title = obj["title"].toString();
	item.content = obj["content"].toString();
	item.category = obj["category"].toString();
	if (item.category.isEmpty())
	{
		item.category = kDefaultCategory;
	}
	item.description = obj["description"].toString();
	item.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
	item.updatedAt = QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate);
	return item;
}

bool PromptItem::isValid() const
{
	return !id.isEmpty() && !title.isEmpty() && !content.isEmpty();
}

// PromptLibrary 实现
PromptLibrary::PromptLibrary(QObject* parent)
	: QObject(parent)
{
	m_storagePath = AppConfigRepository::instance()->promptLibraryFile();
	load();
}

PromptLibrary::~PromptLibrary()
{
	save();
}

QString PromptLibrary::getStoragePath() const
{
	return m_storagePath;
}

bool PromptLibrary::ensureStorageDirectory() const
{
	QFileInfo fileInfo(m_storagePath);
	QDir dir = fileInfo.absoluteDir();
	if (!dir.exists())
	{
		return dir.mkpath(".");
	}
	return true;
}

QString PromptLibrary::generateId() const
{
	return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool PromptLibrary::isValidCategoryName(const QString& category) const
{
	if (category.isEmpty())
	{
		return false;
	}
	// 检查是否包含非法字符
	return !category.contains('/') && !category.contains('\\') && !category.contains(':');
}

bool PromptLibrary::load()
{
	if (!ensureStorageDirectory())
	{
		qWarning() << "Failed to create storage directory for prompt library";
		return false;
	}

	QFile file(m_storagePath);
	if (!file.exists())
	{
		// 文件不存在，初始化默认提示词
		initializeDefaultPrompts();
		save();
		return true;
	}

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qWarning() << "Failed to open prompt library file:" << m_storagePath;
		return false;
	}

	QByteArray data = file.readAll();
	file.close();

	if (data.isEmpty())
	{
		initializeDefaultPrompts();
		save();
		return true;
	}

	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(data, &error);
	if (error.error != QJsonParseError::NoError || !doc.isObject())
	{
		qWarning() << "Failed to parse prompt library JSON:" << error.errorString();
		initializeDefaultPrompts();
		save();
		return false;
	}

	QJsonObject root = doc.object();
	QJsonArray promptsArray = root["prompts"].toArray();
	
	m_prompts.clear();
	for (const QJsonValue& value : promptsArray)
	{
		if (!value.isObject())
		{
			continue;
		}
		PromptItem item = PromptItem::fromJson(value.toObject());
		if (item.isValid())
		{
			m_prompts.append(item);
		}
	}

	return true;
}

bool PromptLibrary::save() const
{
	if (!ensureStorageDirectory())
	{
		qWarning() << "Failed to create storage directory for prompt library";
		return false;
	}

	QJsonObject root;
	QJsonArray promptsArray;
	
	for (const PromptItem& item : m_prompts)
	{
		promptsArray.append(item.toJson());
	}
	
	root["prompts"] = promptsArray;
	root["version"] = "1.0";
	root["lastUpdated"] = QDateTime::currentDateTime().toString(Qt::ISODate);

	QJsonDocument doc(root);
	QFile file(m_storagePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qWarning() << "Failed to open prompt library file for writing:" << m_storagePath;
		return false;
	}

	QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
	qint64 bytesWritten = file.write(jsonData);
	file.close();

	if (bytesWritten == -1)
	{
		qWarning() << "Failed to write prompt library file";
		return false;
	}

	return true;
}

void PromptLibrary::initializeDefaultPrompts()
{
	m_prompts.clear();
	
	QDateTime now = QDateTime::currentDateTime();
	
	// 添加一些默认提示词
	PromptItem item1;
	item1.id = generateId();
	item1.title = QStringLiteral("Code Review");
	item1.content = QStringLiteral("Please review the following code and point out potential issues, performance optimization suggestions, and best practices:\n\n```\n{code}\n```");
	item1.category = QStringLiteral("Development");
	item1.description = QStringLiteral("Template for code review prompts");
	item1.createdAt = now;
	item1.updatedAt = now;
	m_prompts.append(item1);

	PromptItem item2;
	item2.id = generateId();
	item2.title = QStringLiteral("Code Explanation");
	item2.content = QStringLiteral("Please explain in detail the functionality, logic, and implementation principles of the following code:\n\n```\n{code}\n```");
	item2.category = QStringLiteral("Development");
	item2.description = QStringLiteral("Template for code explanation prompts");
	item2.createdAt = now;
	item2.updatedAt = now;
	m_prompts.append(item2);

	PromptItem item3;
	item3.id = generateId();
	item3.title = QStringLiteral("Translation");
	item3.content = QStringLiteral("Please translate the following text into {target language}, maintaining the original meaning and tone:\n\n{text}");
	item3.category = QStringLiteral("General");
	item3.description = QStringLiteral("Template for text translation prompts");
	item3.createdAt = now;
	item3.updatedAt = now;
	m_prompts.append(item3);

	PromptItem item4;
	item4.id = generateId();
	item4.title = QStringLiteral("Summary");
	item4.content = QStringLiteral("Please summarize the following content and extract key information:\n\n{content}");
	item4.category = QStringLiteral("General");
	item4.description = QStringLiteral("Template for content summarization prompts");
	item4.createdAt = now;
	item4.updatedAt = now;
	m_prompts.append(item4);

	PromptItem item5;
	item5.id = generateId();
	item5.title = QStringLiteral("Writing Assistant");
	item5.content = QStringLiteral("Please help me {writing type} with the following requirements: {requirements}");
	item5.category = QStringLiteral("Writing");
	item5.description = QStringLiteral("Template for writing assistance prompts");
	item5.createdAt = now;
	item5.updatedAt = now;
	m_prompts.append(item5);
}

QString PromptLibrary::addPrompt(const QString& title, const QString& content,
	const QString& category, const QString& description)
{
	if (title.isEmpty() || content.isEmpty())
	{
		return QString();
	}

	PromptItem item;
	item.id = generateId();
	item.title = title;
	item.content = content;
	item.category = category.isEmpty() ? kDefaultCategory : category;
	item.description = description;
	item.createdAt = QDateTime::currentDateTime();
	item.updatedAt = item.createdAt;

	m_prompts.append(item);
	save();
	emit promptAdded(item.id);
	emit libraryUpdated();
	return item.id;
}

bool PromptLibrary::updatePrompt(const QString& id, const QString& title, const QString& content,
	const QString& category, const QString& description)
{
	if (id.isEmpty() || title.isEmpty() || content.isEmpty())
	{
		return false;
	}

	for (auto& item : m_prompts)
	{
		if (item.id == id)
		{
			item.title = title;
			item.content = content;
			item.category = category.isEmpty() ? kDefaultCategory : category;
			item.description = description;
			item.updatedAt = QDateTime::currentDateTime();
			save();
			emit promptUpdated(id);
			emit libraryUpdated();
			return true;
		}
	}

	return false;
}

bool PromptLibrary::deletePrompt(const QString& id)
{
	if (id.isEmpty())
	{
		return false;
	}

	auto it = std::remove_if(m_prompts.begin(), m_prompts.end(),
		[&id](const PromptItem& item) { return item.id == id; });

	if (it != m_prompts.end())
	{
		m_prompts.erase(it, m_prompts.end());
		save();
		emit promptDeleted(id);
		emit libraryUpdated();
		return true;
	}

	return false;
}

PromptItem PromptLibrary::getPrompt(const QString& id) const
{
	for (const PromptItem& item : m_prompts)
	{
		if (item.id == id)
		{
			return item;
		}
	}
	return PromptItem();
}

QList<PromptItem> PromptLibrary::getAllPrompts() const
{
	return m_prompts;
}

QList<PromptItem> PromptLibrary::getPromptsByCategory(const QString& category) const
{
	QList<PromptItem> result;
	QString targetCategory = category.isEmpty() ? kDefaultCategory : category;
	
	for (const PromptItem& item : m_prompts)
	{
		if (item.category == targetCategory)
		{
			result.append(item);
		}
	}
	return result;
}

QList<PromptItem> PromptLibrary::searchPrompts(const QString& keyword) const
{
	if (keyword.isEmpty())
	{
		return m_prompts;
	}

	QString lowerKeyword = keyword.toLower();
	QList<PromptItem> result;

	for (const PromptItem& item : m_prompts)
	{
		if (item.title.toLower().contains(lowerKeyword) ||
			item.content.toLower().contains(lowerKeyword) ||
			item.description.toLower().contains(lowerKeyword) ||
			item.category.toLower().contains(lowerKeyword))
		{
			result.append(item);
		}
	}

	return result;
}

QStringList PromptLibrary::getCategories() const
{
	QStringList categories;
	for (const PromptItem& item : m_prompts)
	{
		QString cat = item.category.isEmpty() ? kDefaultCategory : item.category;
		if (!categories.contains(cat))
		{
			categories.append(cat);
		}
	}
	categories.sort();
	return categories;
}

bool PromptLibrary::addCategory(const QString& category)
{
	if (!isValidCategoryName(category))
	{
		return false;
	}

	QStringList categories = getCategories();
	if (categories.contains(category))
	{
		return false; // 分类已存在
	}

	emit categoriesUpdated();
	return true;
}

bool PromptLibrary::deleteCategory(const QString& category)
{
	if (category.isEmpty())
	{
		return false;
	}

	// 删除该分类下的所有提示词
	bool deleted = false;
	auto it = m_prompts.begin();
	while (it != m_prompts.end())
	{
		if (it->category == category)
		{
			it = m_prompts.erase(it);
			deleted = true;
		}
		else
		{
			++it;
		}
	}

	if (deleted)
	{
		save();
		emit libraryUpdated();
		emit categoriesUpdated();
	}

	return deleted;
}

bool PromptLibrary::renameCategory(const QString& oldName, const QString& newName)
{
	if (oldName.isEmpty() || newName.isEmpty() || !isValidCategoryName(newName))
	{
		return false;
	}

	if (oldName == newName)
	{
		return true; // 名称相同，无需重命名
	}

	bool renamed = false;
	for (auto& item : m_prompts)
	{
		if (item.category == oldName)
		{
			item.category = newName;
			item.updatedAt = QDateTime::currentDateTime();
			renamed = true;
		}
	}

	if (renamed)
	{
		save();
		emit libraryUpdated();
		emit categoriesUpdated();
	}

	return renamed;
}

int PromptLibrary::getCategoryCount(const QString& category) const
{
	int count = 0;
	QString targetCategory = category.isEmpty() ? kDefaultCategory : category;
	
	for (const PromptItem& item : m_prompts)
	{
		if (item.category == targetCategory)
		{
			count++;
		}
	}
	return count;
}

int PromptLibrary::getTotalCount() const
{
	return m_prompts.size();
}

int PromptLibrary::getTotalCategoryCount() const
{
	return getCategories().size();
}

