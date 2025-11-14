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
#include <QLineEdit>
#include <QTimer>
#include <functional>

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
    // 设置搜索回调函数，用于搜索对话内容
    void setSearchCallback(std::function<QString(const QString& conversationId)> callback);
    // 对话项数据角色枚举
    enum ConversationRole
    {
        IdRole = Qt::UserRole,        // ID角色
        TimestampRole = Qt::UserRole + 1,  // 时间戳角色
        SearchMatchRole = Qt::UserRole + 2  // 搜索匹配角色
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
    // 搜索文本改变处理
    void onSearchTextChanged(const QString& text);
    // 执行搜索
    void performSearch();

private:
    // 设置UI
    void setupUI();
    // 连接信号槽
    void connectSignals();
    // 根据ID查找列表项
    QListWidgetItem* findItemById(const QString& id) const;
    // 搜索对话内容
    bool searchInConversation(const QString& conversationId, const QString& searchText) const;
    // 显示/隐藏对话项
    void setItemVisible(QListWidgetItem* item, bool visible);
    // UI组件
    QVBoxLayout* mainLayout;
    QPushButton* btnNewConversation;
    QLineEdit* searchEdit;  // 搜索输入框
    QListWidget* m_conversationList;
    // 搜索相关
    QTimer* searchTimer;  // 搜索防抖定时器
    std::function<QString(const QString&)> searchCallback;  // 搜索回调函数
    QStringList allConversationIds;  // 保存所有对话ID，用于搜索


};

#endif // CHATLIST_H
