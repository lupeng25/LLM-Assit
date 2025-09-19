#ifndef CHATLIST_H
#define CHATLIST_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QMenu>
#include <QAction>

class ChatList : public QWidget
{
	Q_OBJECT

public:
	explicit ChatList(QWidget *parent = nullptr);
	~ChatList();

	// 获取组件的访问器
	QPushButton* getNewConversationButton() const { return btnNewConversation; }
	QListWidget* getConversationList() const { return m_conversationList; }

	// 列表操作方法
	void addConversationItem(const QString& text, const QString& id);
	void insertConversationItem(int index, const QString& text, const QString& id);
	void removeConversationItem(const QString& id);
	void clearConversations();

	// 选择操作
	void setCurrentConversation(const QString& id);
	QString getCurrentConversationId() const;
	QListWidgetItem* getCurrentItem() const;

	// 设置当前选中项的文本
	void setCurrentItemText(const QString& text);

	// 获取列表项数量
	int count() const { return m_conversationList->count(); }

	// 设置当前行
	void setCurrentRow(int row) { m_conversationList->setCurrentRow(row); }

signals:
	void newConversationRequested();
	void conversationSelected(const QString& conversationId);
	void conversationChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void contextMenuRequested(const QPoint& pos);
	void renameRequested();
	void deleteRequested();

	private slots:
	void onNewConversationClicked();
	void onConversationSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void showContextMenu(const QPoint& pos);

private:
	void setupUI();
	void connectSignals();

	// UI组件
	QVBoxLayout* mainLayout;
	QPushButton* btnNewConversation;
	QListWidget* m_conversationList;
};

#endif // CHATLIST_H