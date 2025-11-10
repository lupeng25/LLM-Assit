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
	explicit ChatShowWidget(QWidget *parent = nullptr);
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
	// 设置信号槽
	void connectSignals();
	//空状态管理
	void updateEmptyState();
	void showEmptyState();
	void hideEmptyState();

signals:
	void toggleButtonClicked();
	void paramSetButtonClicked();

protected:
	void resizeEvent(QResizeEvent* event) override;

private slots:
	void onToggleButtonClicked();
	void onParamSetButtonClicked();
	void onUpButtonClicked();
	void onDownButtonClicked();
	void onScrollPositionChanged();

private:
	void setupUI();
	void setupEmptyStateWidget(); 
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