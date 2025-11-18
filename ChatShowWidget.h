#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QWheelEvent>
#include <QScrollBar>
#include <QStackedWidget> 
#include <QPainter>
class ChatShowWidget : public QWidget
{
	Q_OBJECT
public:
	// 构造函数
	explicit ChatShowWidget(QWidget *parent = nullptr);
	// 析构函数
	~ChatShowWidget();
	// 获取组件的访问器
	QLabel* getChatTitle() const { return chatTitle; }
	QPushButton* getToggleButton() const { return toggleButton; }
	QPushButton* getParamSetButton() const { return btnParamSet; }
	QListWidget* getChatFrame() const { return listWgChatFrame; }
	// 设置标题
	void setChatTitle(const QString& title);
	// 设置切换按钮图标
	void setToggleIcon(const QIcon& icon);
	// 设置信号槽连接
	void connectSignals();
	// 空状态管理
	// 更新空状态
	void updateEmptyState();
	// 显示空状态
	void showEmptyState();
	// 隐藏空状态
	void hideEmptyState();

signals:
	// 切换按钮点击信号
	void toggleButtonClicked();
	// 参数设置按钮点击信号
	void paramSetButtonClicked();

protected:
	// 大小改变事件
	void resizeEvent(QResizeEvent* event) override;
	// 滚轮事件（优化滚动性能）
	bool eventFilter(QObject* obj, QEvent* event) override;

	private slots:
	// 切换按钮点击处理
	void onToggleButtonClicked();
	// 参数设置按钮点击处理
	void onParamSetButtonClicked();
	// 向上按钮点击处理
	void onUpButtonClicked();
	// 向下按钮点击处理
	void onDownButtonClicked();
	// 滚动位置改变处理
	void onScrollPositionChanged();

private:
	// 设置UI
	void setupUI();
	// 设置空状态组件
	void setupEmptyStateWidget();
	// 应用样式
	void applyStyles();
	 // UI组件
	QVBoxLayout* mainLayout;
	QWidget* headerWidget;
	QHBoxLayout* headerLayout;
	QFrame* chatFrame;
	QVBoxLayout* frameLayout;
	// Header组件
	QLabel* chatTitle;
	QPushButton* toggleButton;
	QPushButton* btnParamSet;
	QPushButton* UpButton;
	QPushButton* DownButton;
	// Chat组件
	QListWidget* listWgChatFrame;
	// 空状态组件
	QStackedWidget* stackedWidget;
	QWidget* emptyStateWidget;
	QLabel* emptyIconLabel;
	QLabel* emptyTextLabel;
};

#endif // CHATWIDGET_H