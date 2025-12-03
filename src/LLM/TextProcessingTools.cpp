#include "TextProcessingTools.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QTextCodec>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QUrl>
#include <QDebug>
#include<QStringList>
#include<QJsonArray>
QJsonObject TextProcessingTools::createErrorResponse(const QString& error)
{
    QJsonObject result;
    result["success"] = false;
    result["error"] = error;
    return result;
}

QJsonObject TextProcessingTools::createSuccessResponse(const QJsonObject& data)
{
    QJsonObject result;
    result["success"] = true;
    if (!data.isEmpty())
    {
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            result[it.key()] = it.value();
        }
    }
    return result;
}

QJsonObject TextProcessingTools::textReplace(const QJsonObject& arguments)
{
    QString text = arguments["text"].toString();
    QString oldText = arguments["old_text"].toString();
    QString newText = arguments["new_text"].toString();
    bool replaceAll = arguments["replace_all"].toBool(true);
    
    if (text.isEmpty())
    {
        return createErrorResponse("Text content cannot be empty");
    }
    
    if (oldText.isEmpty())
    {
        return createErrorResponse("Text to replace cannot be empty");
    }
    
    QString resultText;
    int replaceCount = 0;
    
    if (replaceAll)
    {
        resultText = text;
        replaceCount = resultText.count(oldText);
        resultText.replace(oldText, newText);
    }
    else
    {
        int index = text.indexOf(oldText);
        if (index >= 0)
        {
            resultText = text;
            resultText.replace(index, oldText.length(), newText);
            replaceCount = 1;
        }
        else
        {
            resultText = text;
            replaceCount = 0;
        }
    }
    
    QJsonObject data;
    data["original_text"] = text;
    data["result_text"] = resultText;
    data["replace_count"] = replaceCount;
    
    return createSuccessResponse(data);
}

QJsonObject TextProcessingTools::textStatistics(const QJsonObject& arguments)
{
    QString text = arguments["text"].toString();
    
    if (text.isEmpty())
    {
        return createErrorResponse("Text content cannot be empty");
    }
    
    int charCount = text.length();
    int byteCount = text.toUtf8().size();
    int wordCount = 0;
    int lineCount = 0;
    int paragraphCount = 0;
    int spaceCount = 0;
    int digitCount = 0;
    int letterCount = 0;
    
    // Count words (simple method: split by whitespace)
    QStringList words = text.split(QRegularExpression("\\s+"));
    words.removeAll("");
    wordCount = words.size();
    
    // Count lines
    lineCount = text.count('\n') + 1;
    
    // Count paragraphs (separated by consecutive newlines)
    QStringList paragraphs = text.split(QRegularExpression("\\n\\s*\\n"));
    paragraphs.removeAll("");
    paragraphCount = paragraphs.isEmpty() ? 1 : paragraphs.size();
    
    // Count character types
    for (QChar ch : text)
    {
        if (ch.isSpace())
        {
            spaceCount++;
        }
        if (ch.isDigit())
        {
            digitCount++;
        }
        if (ch.isLetter())
        {
            letterCount++;
        }
    }
    
    QJsonObject data;
    data["char_count"] = charCount;
    data["byte_count"] = byteCount;
    data["word_count"] = wordCount;
    data["line_count"] = lineCount;
    data["paragraph_count"] = paragraphCount;
    data["space_count"] = spaceCount;
    data["digit_count"] = digitCount;
    data["letter_count"] = letterCount;
    
    return createSuccessResponse(data);
}

QJsonObject TextProcessingTools::regexMatch(const QJsonObject& arguments)
{
    QString text = arguments["text"].toString();
    QString pattern = arguments["pattern"].toString();
    bool global = arguments["global"].toBool(false);
    
    if (text.isEmpty())
    {
        return createErrorResponse("Text content cannot be empty");
    }
    
    if (pattern.isEmpty())
    {
        return createErrorResponse("Regular expression pattern cannot be empty");
    }
    
    QRegularExpression regex(pattern);
    if (!regex.isValid())
    {
        return createErrorResponse(QString("Invalid regular expression: %1").arg(regex.errorString()));
    }
    
    QJsonArray matchesArray;
    
    if (global)
    {
        QRegularExpressionMatchIterator it = regex.globalMatch(text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            QJsonObject matchObj;
            matchObj["match"] = match.captured(0);
            matchObj["start"] = match.capturedStart(0);
            matchObj["end"] = match.capturedEnd(0);
            
            QJsonArray groupsArray;
            for (int i = 1; i <= match.lastCapturedIndex(); i++)
            {
                groupsArray.append(match.captured(i));
            }
            matchObj["groups"] = groupsArray;
            
            matchesArray.append(matchObj);
        }
    }
    else
    {
        QRegularExpressionMatch match = regex.match(text);
        if (match.hasMatch())
        {
            QJsonObject matchObj;
            matchObj["match"] = match.captured(0);
            matchObj["start"] = match.capturedStart(0);
            matchObj["end"] = match.capturedEnd(0);
            
            QJsonArray groupsArray;
            for (int i = 1; i <= match.lastCapturedIndex(); i++)
            {
                groupsArray.append(match.captured(i));
            }
            matchObj["groups"] = groupsArray;
            
            matchesArray.append(matchObj);
        }
    }
    
    QJsonObject data;
    data["pattern"] = pattern;
    data["matches"] = matchesArray;
    data["match_count"] = matchesArray.size();
    data["has_match"] = matchesArray.size() > 0;
    
    return createSuccessResponse(data);
}

QJsonObject TextProcessingTools::textExtract(const QJsonObject& arguments)
{
    QString text = arguments["text"].toString();
    QString pattern = arguments["pattern"].toString();
    bool global = arguments["global"].toBool(false);
    
    if (text.isEmpty())
    {
        return createErrorResponse("Text content cannot be empty");
    }
    
    if (pattern.isEmpty())
    {
        return createErrorResponse("Extraction pattern cannot be empty");
    }
    
    QRegularExpression regex(pattern);
    if (!regex.isValid())
    {
        return createErrorResponse(QString("Invalid regular expression: %1").arg(regex.errorString()));
    }
    
    QJsonArray extractedArray;
    
    if (global)
    {
        QRegularExpressionMatchIterator it = regex.globalMatch(text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            extractedArray.append(match.captured(0));
        }
    }
    else
    {
        QRegularExpressionMatch match = regex.match(text);
        if (match.hasMatch())
        {
            extractedArray.append(match.captured(0));
        }
    }
    
    QJsonObject data;
    data["pattern"] = pattern;
    data["extracted"] = extractedArray;
    data["count"] = extractedArray.size();
    
    return createSuccessResponse(data);
}

QJsonObject TextProcessingTools::textEncodeDecode(const QJsonObject& arguments)
{
    QString text = arguments["text"].toString();
    QString operation = arguments["operation"].toString().toLower(); // encode/decode
    QString encoding = arguments["encoding"].toString().toLower(); // base64/url
    
    if (text.isEmpty())
    {
        return createErrorResponse("Text content cannot be empty");
    }
    
    if (operation != "encode" && operation != "decode")
    {
        return createErrorResponse("Operation type must be 'encode' or 'decode'");
    }
    
    QString result;
    
    if (encoding == "base64")
    {
        if (operation == "encode")
        {
            result = text.toUtf8().toBase64();
        }
        else // decode
        {
            QByteArray decoded = QByteArray::fromBase64(text.toUtf8());
            result = QString::fromUtf8(decoded);
        }
    }
    else if (encoding == "url")
    {
        if (operation == "encode")
        {
            result = QUrl::toPercentEncoding(text);
        }
        else // decode
        {
            result = QUrl::fromPercentEncoding(text.toUtf8());
        }
    }
    else
    {
        return createErrorResponse("Unsupported encoding type, supported: base64, url");
    }
    
    QJsonObject data;
    data["original_text"] = text;
    data["result_text"] = result;
    data["operation"] = operation;
    data["encoding"] = encoding;
    
    return createSuccessResponse(data);
}

QJsonObject TextProcessingTools::textFormat(const QJsonObject& arguments)
{
    QString text = arguments["text"].toString();
    QString formatType = arguments["format_type"].toString().toLower(); // json/xml/indent
    
    if (text.isEmpty())
    {
        return createErrorResponse("Text content cannot be empty");
    }
    
    QString formattedText;
    
    if (formatType == "json")
    {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError)
        {
            return createErrorResponse(QString("JSON parse error: %1").arg(error.errorString()));
        }
        formattedText = doc.toJson(QJsonDocument::Indented);
    }
    else if (formatType == "xml")
    {
        // Simple XML formatting (Qt doesn't have built-in XML formatting, this is just an example)
        // Can use QDomDocument for actual formatting
        formattedText = text; // Placeholder, can be implemented later
    }
    else if (formatType == "indent")
    {
        // Simple indentation formatting (line by line)
        QStringList lines = text.split('\n');
        int indentLevel = 0;
        QStringList formattedLines;
        
        for (const QString& line : lines)
        {
            QString trimmed = line.trimmed();
            if (trimmed.isEmpty())
            {
                formattedLines.append("");
                continue;
            }
            
            // Simple indentation logic (detect brackets)
            if (trimmed.endsWith('{') || trimmed.endsWith('['))
            {
                formattedLines.append(QString("    ").repeated(indentLevel) + trimmed);
                indentLevel++;
            }
            else if (trimmed.startsWith('}') || trimmed.startsWith(']'))
            {
                indentLevel = qMax(0, indentLevel - 1);
                formattedLines.append(QString("    ").repeated(indentLevel) + trimmed);
            }
            else
            {
                formattedLines.append(QString("    ").repeated(indentLevel) + trimmed);
            }
        }
        
        formattedText = formattedLines.join('\n');
    }
    else
    {
        return createErrorResponse("Unsupported format type, supported: json, xml, indent");
    }
    
    QJsonObject data;
    data["original_text"] = text;
    data["formatted_text"] = formattedText;
    data["format_type"] = formatType;
    
    return createSuccessResponse(data);
}

