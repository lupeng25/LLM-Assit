#include "SyntaxHighlighter.h"

// 静态成员初始化
int SyntaxHighlighter::s_codeBlockCounter = 0;

SyntaxHighlighter::SyntaxHighlighter()
	: m_theme(darkTheme())
{
	initLanguageMap();
}

QString SyntaxHighlighter::highlightCode(const QString& code, const QString& language)
{
	if (code.isEmpty()) return QString();

	Language lang = stringToLanguage(language.toLower());

	switch (lang) 
	{
	case Json:
		return highlightJson(code);
	case Cpp:
		return highlightCpp(code);
	case Python:
		return highlightPython(code);
	case JavaScript:
		return highlightJavaScript(code);
	default:
		return highlightGeneric(code);
	}
}

QString SyntaxHighlighter::highlightCodeBlock(const QString& code, const QString& language)
{
	QString highlightedCode = highlightCode(code, language);
	QString toolbar = createToolbar(language, code);

	return QString(
		"<div style=\"background-color: %1; border-radius: 6px; margin: 8px 0; "
		"border: 1px solid #3e3e42; font-family: 'Consolas', 'Monaco', monospace; "
		"font-size: 11px; overflow: hidden;\">"
		"%2" // 工具栏
		"<div style=\"padding: 12px; background-color: %1;\">"
		"<pre style=\"margin: 0; color: %3; white-space: pre-wrap; word-wrap: break-word; "
		"line-height: 1.0;\">%4</pre>"
		"</div>"
		"</div>"
	).arg(m_theme.background, toolbar, m_theme.text, highlightedCode);
}

QString SyntaxHighlighter::highlightInlineCode(const QString& code)
{
	return QString(
		"<span style=\"background-color: %1; color: %2; padding: 2px 4px; "
		"border-radius: 3px; font-family: 'Consolas', 'Monaco', monospace; font-size: 11px;\">%3</span>"
	).arg(m_theme.background, m_theme.string, escapeHtml(code));
}

QString SyntaxHighlighter::createToolbar(const QString& language, const QString& code)
{
	QString uniqueId = generateUniqueId();
	QString displayLanguage = language.isEmpty() ? "Code" : language.toUpper();

	// 转义代码内容用于JavaScript
	QString escapedCode = code;
	escapedCode.replace("\\", "\\\\");
	escapedCode.replace("\"", "\\\"");
	escapedCode.replace("\n", "\\n");
	escapedCode.replace("\r", "");

	// 分步构建字符串，避免arg()参数过多的问题
	QString toolbarHtml = QString(
		"<div style=\"background-color: %1; padding: 8px 12px; border-bottom: 1px solid #3e3e42; "
		"display: flex; justify-content: space-between; align-items: center;\">"
		"<span style=\"color: %2; font-size: 12px; font-weight: 500;\">%3</span>"
	).arg(m_theme.toolbar, m_theme.toolbarText, displayLanguage);

	QString buttonHtml = QString(
		"<button id=\"copy-btn-%1\" "
		"style=\"background-color: %2; color: %3; border: 1px solid #565656; "
		"border-radius: 4px; padding: 4px 8px; font-size: 11px; cursor: pointer; "
		"font-family: inherit;\" "
		"onmouseover=\"this.style.backgroundColor='%4'\" "
		"onmouseout=\"this.style.backgroundColor='%2'\" "
	).arg(uniqueId, m_theme.button, m_theme.toolbarText, m_theme.buttonHover);

	QString onclickScript = QString(
		"onclick=\""
		"try {"
		"  if (navigator.clipboard && navigator.clipboard.writeText) {"
		"    navigator.clipboard.writeText('%1').then(() => {"
		"      this.textContent = 'Copied!'; "
		"      setTimeout(() => { this.textContent = 'Copy'; }, 1000);"
		"    }).catch(() => {"
		"      fallbackCopy('%1', this);"
		"    });"
		"  } else {"
		"    fallbackCopy('%1', this);"
		"  }"
		"} catch(e) {"
		"  fallbackCopy('%1', this);"
		"}"
		"function fallbackCopy(text, btn) {"
		"  const textArea = document.createElement('textarea');"
		"  textArea.value = text;"
		"  textArea.style.position = 'fixed';"
		"  textArea.style.opacity = '0';"
		"  document.body.appendChild(textArea);"
		"  textArea.select();"
		"  try {"
		"    document.execCommand('copy');"
		"    btn.textContent = 'Copied!';"
		"    setTimeout(() => { btn.textContent = 'Copy'; }, 1000);"
		"  } catch(e) {"
		"    btn.textContent = 'Failed';"
		"    setTimeout(() => { btn.textContent = 'Copy'; }, 1000);"
		"  }"
		"  document.body.removeChild(textArea);"
		"}\""
		"></button>"
		"</div>"
	).arg(escapedCode);

	return toolbarHtml + buttonHtml + onclickScript;
}

QString SyntaxHighlighter::generateUniqueId()
{
	return QString("code_%1_%2").arg(++s_codeBlockCounter).arg(QDateTime::currentMSecsSinceEpoch());
}

QString SyntaxHighlighter::extractPlainCode(const QString& htmlCode)
{
	QString plainCode = htmlCode;

	// 移除HTML标签
	QRegularExpression htmlTagRegex("<[^>]*>");
	plainCode.remove(htmlTagRegex);

	// 解码HTML实体
	plainCode.replace("&lt;", "<");
	plainCode.replace("&gt;", ">");
	plainCode.replace("&amp;", "&");
	plainCode.replace("&quot;", "\"");
	plainCode.replace("&#39;", "'");

	return plainCode;
}

QString SyntaxHighlighter::highlightJson(const QString& code)
{
	QString result = escapeHtml(code);

	// JSON 字符串 (键和值)
	QRegularExpression stringRegex("\"([^\"\\\\]|\\\\.)*\"");
	result = applyPatternHighlight(result, stringRegex, m_theme.string);

	// JSON 数字
	QRegularExpression numberRegex("\\b-?\\d+(?:\\.\\d+)?(?:[eE][+-]?\\d+)?\\b");
	result = applyPatternHighlight(result, numberRegex, m_theme.number);

	// JSON 关键字
	QRegularExpression keywordRegex("\\b(true|false|null)\\b");
	result = applyPatternHighlight(result, keywordRegex, m_theme.keyword, true);

	// JSON 标点符号
	QRegularExpression punctRegex("([{}\\[\\]:,])");
	result = applyPatternHighlight(result, punctRegex, m_theme.punctuation, true);

	return result;
}

QString SyntaxHighlighter::highlightCpp(const QString& code)
{
	QString result = escapeHtml(code);

	// 单行和多行注释
	QRegularExpression singleCommentRegex("//.*$");
	QRegularExpression multiCommentRegex("/\\*.*?\\*/");
	result = applyPatternHighlight(result, singleCommentRegex, m_theme.comment, false, true);
	result = applyPatternHighlight(result, multiCommentRegex, m_theme.comment, false, true);

	// 字符串和字符
	QRegularExpression stringRegex("\"([^\"\\\\]|\\\\.)*\"|'([^'\\\\]|\\\\.)*'");
	result = applyPatternHighlight(result, stringRegex, m_theme.string);

	// 预处理指令
	QRegularExpression preprocessorRegex("^\\s*#.*$");
	result = applyPatternHighlight(result, preprocessorRegex, m_theme.preprocessor);

	// C++ 关键字
	QStringList keywords = getCppKeywords();
	for (const QString& keyword : keywords)
	{
		QRegularExpression keywordRegex(QString("\\b%1\\b").arg(QRegularExpression::escape(keyword)));
		result = applyPatternHighlight(result, keywordRegex, m_theme.keyword, true);
	}

	// 数字
	QRegularExpression numberRegex("\\b\\d+(?:\\.\\d+)?[fFlL]?\\b");
	result = applyPatternHighlight(result, numberRegex, m_theme.number);

	// 函数调用
	QRegularExpression functionRegex("\\b([a-zA-Z_][a-zA-Z0-9_]*)\\s*(?=\\()");
	result = applyPatternHighlight(result, functionRegex, m_theme.function);

	return result;
}

QString SyntaxHighlighter::highlightPython(const QString& code)
{
	QString result = escapeHtml(code);

	// Python 注释
	QRegularExpression commentRegex("#.*$");
	result = applyPatternHighlight(result, commentRegex, m_theme.comment, false, true);

	// Python 字符串 (包括三引号字符串)
	QRegularExpression tripleStringRegex("\"\"\".*?\"\"\"|'''.*?'''");
	QRegularExpression stringRegex("\"([^\"\\\\]|\\\\.)*\"|'([^'\\\\]|\\\\.)*'");
	result = applyPatternHighlight(result, tripleStringRegex, m_theme.string);
	result = applyPatternHighlight(result, stringRegex, m_theme.string);

	// Python 关键字
	QStringList keywords = getPythonKeywords();
	for (const QString& keyword : keywords) 
	{
		QRegularExpression keywordRegex(QString("\\b%1\\b").arg(QRegularExpression::escape(keyword)));
		result = applyPatternHighlight(result, keywordRegex, m_theme.keyword, true);
	}

	// 函数定义
	QRegularExpression funcDefRegex("\\bdef\\s+([a-zA-Z_][a-zA-Z0-9_]*)");
	result = applyPatternHighlight(result, funcDefRegex, m_theme.function, true);

	// 类定义
	QRegularExpression classDefRegex("\\bclass\\s+([a-zA-Z_][a-zA-Z0-9_]*)");
	result = applyPatternHighlight(result, classDefRegex, m_theme.type, true);

	// 数字
	QRegularExpression numberRegex("\\b\\d+(?:\\.\\d+)?(?:[eE][+-]?\\d+)?\\b");
	result = applyPatternHighlight(result, numberRegex, m_theme.number);

	return result;
}

QString SyntaxHighlighter::highlightJavaScript(const QString& code)
{
	QString result = escapeHtml(code);

	// JavaScript 注释
	QRegularExpression singleCommentRegex("//.*$");
	QRegularExpression multiCommentRegex("/\\*.*?\\*/");
	result = applyPatternHighlight(result, singleCommentRegex, m_theme.comment, false, true);
	result = applyPatternHighlight(result, multiCommentRegex, m_theme.comment, false, true);

	// JavaScript 字符串 (包括模板字符串)
	QRegularExpression templateStringRegex("`([^`\\\\]|\\\\.)*`");
	QRegularExpression stringRegex("\"([^\"\\\\]|\\\\.)*\"|'([^'\\\\]|\\\\.)*'");
	result = applyPatternHighlight(result, templateStringRegex, m_theme.string);
	result = applyPatternHighlight(result, stringRegex, m_theme.string);

	// JavaScript 关键字
	QStringList keywords = getJavaScriptKeywords();
	for (const QString& keyword : keywords)
	{
		QRegularExpression keywordRegex(QString("\\b%1\\b").arg(QRegularExpression::escape(keyword)));
		result = applyPatternHighlight(result, keywordRegex, m_theme.keyword, true);
	}

	// 函数定义
	QRegularExpression funcRegex("\\bfunction\\s+([a-zA-Z_$][a-zA-Z0-9_$]*)");
	result = applyPatternHighlight(result, funcRegex, m_theme.function, true);

	// 数字
	QRegularExpression numberRegex("\\b\\d+(?:\\.\\d+)?(?:[eE][+-]?\\d+)?\\b");
	result = applyPatternHighlight(result, numberRegex, m_theme.number);

	return result;
}

QString SyntaxHighlighter::highlightGeneric(const QString& code)
{
	QString result = escapeHtml(code);

	// 通用字符串高亮
	QRegularExpression stringRegex("\"([^\"\\\\]|\\\\.)*\"|'([^'\\\\]|\\\\.)*'");
	result = applyPatternHighlight(result, stringRegex, m_theme.string);

	// 通用数字高亮
	QRegularExpression numberRegex("\\b\\d+(?:\\.\\d+)?\\b");
	result = applyPatternHighlight(result, numberRegex, m_theme.number);

	// 通用注释高亮
	QRegularExpression commentRegex("//.*$|#.*$|/\\*.*?\\*/|<!--.*?-->");
	result = applyPatternHighlight(result, commentRegex, m_theme.comment, false, true);

	return result;
}

// 辅助函数实现
QString SyntaxHighlighter::escapeHtml(const QString& text)
{
	return QString(text)
		.replace("&", "&amp;")
		.replace("<", "&lt;")
		.replace(">", "&gt;")
		.replace("\"", "&quot;")
		.replace("'", "&#39;");
}

QString SyntaxHighlighter::wrapWithSpan(const QString& text, const QString& color, bool bold, bool italic)
{
	QString style = QString("color: %1;").arg(color);
	if (bold) style += " font-weight: bold;";
	if (italic) style += " font-style: italic;";

	return QString("<span style=\"%1\">%2</span>").arg(style, text);
}

QString SyntaxHighlighter::applyPatternHighlight(const QString& code, const QRegularExpression& pattern,
	const QString& color, bool bold, bool italic)
{
	QString result = code;
	QRegularExpressionMatchIterator iterator = pattern.globalMatch(result);

	// 从后往前替换，避免位置偏移
	QList<QRegularExpressionMatch> matches;
	while (iterator.hasNext()) {
		matches.prepend(iterator.next());
	}

	for (const auto& match : matches) 
	{
		QString highlighted = wrapWithSpan(match.captured(0), color, bold, italic);
		result.replace(match.capturedStart(), match.capturedLength(), highlighted);
	}

	return result;
}

// 主题相关函数
void SyntaxHighlighter::setTheme(const Theme& theme)
{
	m_theme = theme;
}

SyntaxHighlighter::Theme SyntaxHighlighter::getTheme() const
{
	return m_theme;
}

SyntaxHighlighter::Theme SyntaxHighlighter::darkTheme()
{
	Theme theme;
	theme.keyword = "#569cd6";
	theme.string = "#ce9178";
	theme.comment = "#6a9955";
	theme.number = "#b5cea8";
	theme.function = "#dcdcaa";
	theme.type = "#4ec9b0";
	theme.operator_ = "#d4d4d4";
	theme.background = "#1e1e1e";
	theme.text = "#d4d4d4";
	theme.preprocessor = "#c586c0";
	theme.punctuation = "#d4d4d4";
	theme.toolbar = "#2d2d30";
	theme.toolbarText = "#cccccc";
	theme.button = "#3c3c3c";
	theme.buttonHover = "#464647";
	return theme;
}

SyntaxHighlighter::Theme SyntaxHighlighter::lightTheme()
{
	Theme theme;
	theme.keyword = "#0000ff";
	theme.string = "#a31515";
	theme.comment = "#008000";
	theme.number = "#098658";
	theme.function = "#795e26";
	theme.type = "#267f99";
	theme.operator_ = "#000000";
	theme.background = "#ffffff";
	theme.text = "#000000";
	theme.preprocessor = "#af00db";
	theme.punctuation = "#000000";
	theme.toolbar = "#f3f3f3";
	theme.toolbarText = "#333333";
	theme.button = "#e1e1e1";
	theme.buttonHover = "#d4d4d4";
	return theme;
}

// 语言检测和管理
void SyntaxHighlighter::initLanguageMap()
{
	m_languageMap["json"] = Json;
	m_languageMap["cpp"] = Cpp;
	m_languageMap["c++"] = Cpp;
	m_languageMap["c"] = Cpp;
	m_languageMap["python"] = Python;
	m_languageMap["py"] = Python;
	m_languageMap["javascript"] = JavaScript;
	m_languageMap["js"] = JavaScript;
	m_languageMap["html"] = Html;
	m_languageMap["css"] = Css;
	m_languageMap["sql"] = Sql;
}

SyntaxHighlighter::Language SyntaxHighlighter::stringToLanguage(const QString& langStr)
{
	return m_languageMap.value(langStr.toLower(), Generic);
}

// 关键字列表
QStringList SyntaxHighlighter::getCppKeywords()
{
	return{
		"alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit",
		"atomic_noexcept", "auto", "bitand", "bitor", "bool", "break", "case", "catch",
		"char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "const",
		"consteval", "constexpr", "constinit", "const_cast", "continue", "co_await",
		"co_return", "co_yield", "decltype", "default", "delete", "do", "double",
		"dynamic_cast", "else", "enum", "explicit", "export", "extern", "false",
		"float", "for", "friend", "goto", "if", "inline", "int", "long", "mutable",
		"namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or",
		"or_eq", "private", "protected", "public", "reflexpr", "register",
		"reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static",
		"static_assert", "static_cast", "struct", "switch", "synchronized", "template",
		"this", "thread_local", "throw", "true", "try", "typedef", "typeid", "typename",
		"union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while",
		"xor", "xor_eq", "include", "define", "ifdef", "ifndef", "endif", "pragma"
	};
}

QStringList SyntaxHighlighter::getPythonKeywords()
{
	return{
		"False", "None", "True", "and", "as", "assert", "async", "await", "break",
		"class", "continue", "def", "del", "elif", "else", "except", "finally",
		"for", "from", "global", "if", "import", "in", "is", "lambda", "nonlocal",
		"not", "or", "pass", "raise", "return", "try", "while", "with", "yield"
	};
}

QStringList SyntaxHighlighter::getJavaScriptKeywords()
{
	return{
		"abstract", "arguments", "await", "boolean", "break", "byte", "case", "catch",
		"char", "class", "const", "continue", "debugger", "default", "delete", "do",
		"double", "else", "enum", "eval", "export", "extends", "false", "final",
		"finally", "float", "for", "function", "goto", "if", "implements", "import",
		"in", "instanceof", "int", "interface", "let", "long", "native", "new",
		"null", "package", "private", "protected", "public", "return", "short",
		"static", "super", "switch", "synchronized", "this", "throw", "throws",
		"transient", "true", "try", "typeof", "var", "void", "volatile", "while",
		"with", "yield", "async", "of"
	};
}