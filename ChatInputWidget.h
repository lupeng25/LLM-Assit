#ifndef CHATINPUTWIDGET_H
#define CHATINPUTWIDGET_H

#include <QUrl>
#include <QMenu>
#include <QLabel>
#include <QAction>
#include <QBuffer>
#include <QPixmap>
#include <QWidget>
#include <QToolTip>
#include <QComboBox>
#include <QTextEdit>
#include <QKeyEvent>
#include <QMimeData>
#include <QDropEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QScrollArea>
#include <QFileDialog>
#include <QImageReader>
#include <QDragEnterEvent>
#include <QGraphicsDropShadowEffect>
#include "PromptLibrary.h"
#include "PromptLibraryDialog.h"

//输入信息:文本，图片，文件
struct ChatSendMessage
{
	QString SendText="";
	QStringList Image64;
	QStringList fileContext;
};

//多功能输入框
class ChatInputWidget : public QWidget
{
	Q_OBJECT

public:
	// 构造函数
	explicit ChatInputWidget(QWidget *parent = nullptr);
	// 析构函数
	~ChatInputWidget();
	// 设置发送按钮是否可点击
	void SetButtonEnable(bool bEnable);
	// 获取模型选择下拉框
	QComboBox * GetModelButton() { return optionsComboBox; };
	// 设置当前模型索引
	void setModelCurrIndex(int index);
	// 更新模型列表
	void UpdateModelList(bool success, QStringList models, const QString& errorMessage);
	// 选择知识库点击处理
	void onSelectKnowledgeBaseClicked(std::map<QString, std::pair<QString, QString>> knowBase);
	// 更新知识库菜单
	void updateKnowledgeBaseMenu();
	// 设置提示词库
	void setPromptLibrary(PromptLibrary* library);

signals:
	// 发送信息信号
	void MessageUp(ChatSendMessage message);
	// 模型更改信号
	void ModelSelect(int iModel);
	// 选择知识库信号
	void KnowledgeBaseSelect(const QString& knowledgeName);
	// 添加按钮信号
	void addButtonSignal();

protected:
	// 事件过滤器
	bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
	// 选项改变处理
	void onOptionChanged(int index);
	// 附件按钮点击处理
	void onAttachButtonClicked();
	// 添加文件点击处理
	void onAddFileClicked();
	// 知识库子项点击处理
	void onKnowledgeBaseItemClicked();
	// 提示词库按钮点击处理
	void onPromptLibraryClicked();
	// 提示词已选择处理
	void onPromptSelected(const QString& content);

private:
	//文件类型
	enum FileType
	{
		IMAGE_FILE,
		TEXT_FILE
	};
	//附件
	struct AttachedFile
	{
		QString filePath;
		FileType fileType;
		QString displayName;
		bool operator==(const AttachedFile& other) const {
			return (filePath == other.filePath);
		}
	};
	// 控件设置
	void setupUI();
	// 样式设置
	void setupStyles();
	// 设置信号槽连接
	void setConnect();
	// 发送按钮点击处理
	void onSendButtonClicked();
	// 判断附件文件有效性
	bool validateImageFile(const QString &filePath, QString &errorMsg);
	// 获取文件类型
	FileType getFileType(const QString &filePath);
	// 读取文本文件内容
	QString readTextFile(const QString &filePath);
	// 更新附件控件显示
	void updateFileDisplay();
	// 添加文件
	void addFile(const QString &filePath, FileType type);
	// 移除单个文件及其数据
	void removeFile(const QString &filePath);
	// 移除所有文件及其数据
	void clearAllFiles();

	// 拖放进入事件
	void dragEnterEvent(QDragEnterEvent *event) override;
	// 拖放释放事件
	void dropEvent(QDropEvent *event) override;

	//控件
	QTextEdit *textEdit;//文本输入框
	QPushButton *attachButton;//额外文件添加
	QComboBox *optionsComboBox;//模型选择combox
	QPushButton *sendButton;//发送按钮
	QMenu *attachMenu;//附件菜单
	QAction *addFileAction;//添加文件动作
	QAction *selectKnowledgeBaseAction;//选择知识库动作
	QAction *promptLibraryAction;//提示词库动作
	QPushButton *promptLibraryButton;//提示词库按钮
    //布局
	QGridLayout *mainGridLayout;
	QHBoxLayout *bottomGridLayout;
	QGridLayout *buttonGridLayout;
	QWidget *bottomWidget;
	QSpacerItem *horizontalSpacer;
	// 附件相关
	static const int MAX_IMAGES = 5;//最多文件数量
	static const int MAX_FILE_SIZE = 10 * 1024 * 1024;//单个文件最大大小
	QScrollArea *imageScrollArea;
	QWidget *imageContainer;
	QHBoxLayout *imageLayout;
	QStringList ImageBase64;//图像base64
	QStringList attachedTextContents;//文本  
	QList<AttachedFile> attachedFiles;//文件

	QMenu *knowledgeBaseMenu;           // 知识库子菜单
	QList<QAction*> knowledgeBaseActions; // 知识库动作列表
	std::map<QString, std::pair<QString,QString>>  knowledgeBaseList;      // 知识库名称列表

	PromptLibrary* m_promptLibrary;     // 提示词库对象
	PromptLibraryDialog* m_promptDialog;// 提示词库对话框

signals:
	// 文件初始化完成信号
	void InitFileFinished(const QString& file);
	// 移除文件信号
	void RemoveFileSignal(const QString& file);
	// 移除所有文件信号
	void RemoveAllFilesSignal(const QStringList& fileList);
};

#endif // CHATINPUTWIDGET_H