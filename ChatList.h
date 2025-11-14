#ifndef CHATLIST_H
#define CHATLIST_H
#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QMenu>
#include <QAction>
#include <QString>
#include <QPoint>

class ChatList : public QWidget
{
    Q_OBJECT

public:
    // 构造函数
    explicit ChatList(QWidget *parent = nullptr);
    // 析构函数
    ~ChatList();

    // 获取组件的访问器
    QPushButton* getNewConversationButton() const { return btnNewConversation; }
    QListWidget* getConversationList() const { return m_conversationList; }
    // 列表操作方法
    // 添加对话项
    void addConversationItem(const QString& text, const QString& id);
    // 插入对话项
    void insertConversationItem(int index, const QString& text, const QString& id);
    // 移除对话项
    void removeConversationItem(const QString& id);
    // 清空所有对话
    void clearConversations();
    // 选择操作
    // 设置当前对话
    void setCurrentConversation(const QString& id);
    // 获取当前对话ID
    QString getCurrentConversationId() const;
    // 获取当前项
    QListWidgetItem* getCurrentItem() const;
    // 设置当前选中项的文本
    void setCurrentItemText(const QString& text);
    // 设置对话时间戳
    void setConversationTimestamp(const QString& id, const QString& timestamp);
    // 获取列表项数量
    int count() const { return m_conversationList->count(); }
    // 设置当前行
    void setCurrentRow(int row) { m_conversationList->setCurrentRow(row); }
    // 对话项数据角色枚举
    enum ConversationRole
    {
        IdRole = Qt::UserRole,        // ID角色
        TimestampRole = Qt::UserRole + 1  // 时间戳角色
    };

signals:
    // 新建对话请求信号
    void newConversationRequested();
    // 对话选择信号
    void conversationSelected(const QString& conversationId);
    // 对话改变信号
    void conversationChanged(QListWidgetItem* current, QListWidgetItem* previous);
    // 上下文菜单请求信号
    void contextMenuRequested(const QPoint& pos);
    // 重命名请求信号
    void renameRequested();
    // 删除请求信号
    void deleteRequested();
    // 导出对话请求信号
    void exportConversationRequested(const QString& conversationId, const QString& format);
    // 显示详情请求信号
    void showDetailsRequested(const QString& conversationId);

    private slots:
    // 新建对话按钮点击处理
    void onNewConversationClicked();
    // 对话选择改变处理
    void onConversationSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous);
    // 显示上下文菜单
    void showContextMenu(const QPoint& pos);

private:
    // 设置UI
    void setupUI();
    // 连接信号槽
    void connectSignals();
    // 根据ID查找列表项
    QListWidgetItem* findItemById(const QString& id) const;
    // UI组件
    QVBoxLayout* mainLayout;
    QPushButton* btnNewConversation;
    QListWidget* m_conversationList;


};

#endif // CHATLIST_H
