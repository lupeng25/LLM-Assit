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
	explicit ChatInputWidget(QWidget *parent = nullptr);
	~ChatInputWidget();
	void SetButtonEnable(bool bEnable);//设置发送按钮是否可点击
	QComboBox * GetModelButton() { return optionsComboBox; };
	void setModelCurrIndex(int index);//设置模型
	void UpdateModelList(bool success, QStringList models, const QString& errorMessage);
	void onSelectKnowledgeBaseClicked(std::map<QString, std::pair<QString, QString>> knowBase);
	void updateKnowledgeBaseMenu();     // 更新知识库菜单

signals:
	void MessageUp(ChatSendMessage message);//发送信息信号
	void ModelSelect(int iModel);//模型更改信号
	void KnowledgeBaseSelect(const QString& knowledgeName); // 选择知识库信号，添加参数
	void addButtonSignal();

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
	void onOptionChanged(int index);
	void onAttachButtonClicked();
	void onAddFileClicked();
	void onKnowledgeBaseItemClicked(); // 知识库子项点击槽函数

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
	//控件设置
	void setupUI();
	//qss设置
	void setupStyles();
	//设置信号槽
	void setConnect();
	//发送信息槽函数
	void onSendButtonClicked();
	//判断附件文件有效性
	bool validateImageFile(const QString &filePath, QString &errorMsg);
	//获取文件类型
	FileType getFileType(const QString &filePath);
	//读取文本文件内容
	QString readTextFile(const QString &filePath);
	//更新附件控件
	void updateFileDisplay();
	//添加文件
	void addFile(const QString &filePath, FileType type);
	//移除单个文件及其数据
	void removeFile(const QString &filePath);
	//移除所有文件及其数据
	void clearAllFiles();

	// 重写拖放事件
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dropEvent(QDropEvent *event) override;

	//控件
	QTextEdit *textEdit;//文本输入框
	QPushButton *attachButton;//额外文件添加
	QComboBox *optionsComboBox;//模型选择combox
	QPushButton *sendButton;//发送按钮
	QMenu *attachMenu;//附件菜单
	QAction *addFileAction;//添加文件动作
	QAction *selectKnowledgeBaseAction;//选择知识库动作
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

signals:
	void InitFileFinished(const QString& file);
	void RemoveFileSignal(const QString& file);
	void RemoveAllFilesSignal(const QStringList& fileList);
};

#endif // CHATINPUTWIDGET_H