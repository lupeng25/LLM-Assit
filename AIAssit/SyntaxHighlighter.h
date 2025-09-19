#pragma once
#include <QRegularExpression>
#include <QStringList>
#include <QDateTime>
#include <QString>
#include <QMap>

class SyntaxHighlighter
{
public:
	// 配色主题结构
	struct Theme 
	{
		QString keyword = "#569cd6";      // 关键字 - 蓝色
		QString string = "#ce9178";       // 字符串 - 橙色  
		QString comment = "#6a9955";      // 注释 - 绿色
		QString number = "#b5cea8";       // 数字 - 淡绿色
		QString function = "#dcdcaa";     // 函数 - 黄色
		QString type = "#4ec9b0";         // 类型 - 青色
		QString operator_ = "#d4d4d4";    // 操作符 - 白色
		QString background = "#1e1e1e";   // 背景 - 深色
		QString text = "#d4d4d4";         // 普通文本 - 淡灰色
		QString preprocessor = "#c586c0"; // 预处理指令 - 紫色
		QString punctuation = "#d4d4d4";  // 标点符号 - 淡灰色
		QString toolbar = "#2d2d30";      // 工具栏背景 - 深灰色
		QString toolbarText = "#00A9D0";  // 工具栏文本 - 浅灰色
		QString button = "#3c3c3c";       // 按钮背景 - 中灰色
		QString buttonHover = "#464647";  // 按钮悬停 - 稍亮灰色
	};

	// 支持的语言枚举
	enum Language 
	{
		Unknown,
		Json,
		Cpp,
		Python,
		JavaScript,
		Html,
		Css,
		Sql,
		Generic
	};

public:
	explicit SyntaxHighlighter();
	~SyntaxHighlighter() = default;

	// 主要接口
	QString highlightCode(const QString& code, const QString& language);
	QString highlightCodeBlock(const QString& code, const QString& language);
	QString highlightInlineCode(const QString& code);

	// 主题管理
	void setTheme(const Theme& theme);
	Theme getTheme() const;

	// 预设主题
	static Theme darkTheme();
	static Theme lightTheme();

	// 语言检测
	Language stringToLanguage(const QString& langStr);

	// 获取纯代码内容（去除HTML标签）
	QString extractPlainCode(const QString& htmlCode);

private:
	// 语言特定的高亮函数
	QString highlightJson(const QString& code);
	QString highlightCpp(const QString& code);
	QString highlightPython(const QString& code);
	QString highlightJavaScript(const QString& code);
	QString highlightGeneric(const QString& code);

	// 辅助函数
	QString escapeHtml(const QString& text);
	QString wrapWithSpan(const QString& text, const QString& color, bool bold = false, bool italic = false);
	QString applyPatternHighlight(const QString& code, const QRegularExpression& pattern, const QString& color, bool bold = false, bool italic = false);

	// 创建工具栏HTML
	QString createToolbar(const QString& language, const QString& code);
	QString generateUniqueId();

	// 获取语言关键字
	QStringList getCppKeywords();
	QStringList getPythonKeywords();
	QStringList getJavaScriptKeywords();

private:
	Theme m_theme;
	QMap<QString, Language> m_languageMap;
	static int s_codeBlockCounter; // 用于生成唯一ID
	void initLanguageMap();
};