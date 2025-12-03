#include "DataFormatTools.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStringList>
#include <QDebug>

QJsonObject DataFormatTools::createErrorResponse(const QString& error)
{
    QJsonObject result;
    result["success"] = false;
    result["error"] = error;
    return result;
}

QJsonObject DataFormatTools::createSuccessResponse(const QJsonObject& data)
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

QJsonObject DataFormatTools::parseJson(const QJsonObject& arguments)
{
    QString jsonString = arguments["json_string"].toString();
    
    if (jsonString.isEmpty())
    {
        return createErrorResponse("JSON string cannot be empty");
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError)
    {
        return createErrorResponse(QString("JSON parse error: %1 at position %2").arg(error.errorString()).arg(error.offset));
    }
    
    QJsonObject data;
    data["json_string"] = jsonString;
    data["is_object"] = doc.isObject();
    data["is_array"] = doc.isArray();
    
    if (doc.isObject())
    {
        data["parsed_data"] = doc.object();
    }
    else if (doc.isArray())
    {
        data["parsed_data"] = doc.array();
    }
    
    return createSuccessResponse(data);
}

QJsonObject DataFormatTools::formatJson(const QJsonObject& arguments)
{
    QString jsonString = arguments["json_string"].toString();
    int indent = arguments["indent"].toInt(4);
    
    if (jsonString.isEmpty())
    {
        return createErrorResponse("JSON string cannot be empty");
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError)
    {
        return createErrorResponse(QString("JSON parse error: %1").arg(error.errorString()));
    }
    
    QJsonDocument::JsonFormat format = (indent > 0) ? QJsonDocument::Indented : QJsonDocument::Compact;
    QString formatted = doc.toJson(format);
    
    QJsonObject data;
    data["original_json"] = jsonString;
    data["formatted_json"] = formatted;
    data["indent"] = indent;
    
    return createSuccessResponse(data);
}

QJsonObject DataFormatTools::parseCsv(const QJsonObject& arguments)
{
    QString csvString = arguments["csv_string"].toString();
    QString delimiter = arguments["delimiter"].toString();
    bool hasHeader = arguments["has_header"].toBool(true);
    
    if (csvString.isEmpty())
    {
        return createErrorResponse("CSV string cannot be empty");
    }
    
    if (delimiter.isEmpty())
    {
        delimiter = ",";
    }
    
    QStringList lines = csvString.split('\n');
    lines.removeAll("");
    if (lines.isEmpty())
    {
        return createErrorResponse("CSV string has no lines");
    }
    
    QJsonArray rowsArray;
    QStringList headers;
    
    int startIndex = 0;
    if (hasHeader && !lines.isEmpty())
    {
        headers = lines[0].split(delimiter);
        startIndex = 1;
    }
    
    for (int i = startIndex; i < lines.size(); ++i)
    {
        QStringList fields = lines[i].split(delimiter);
        QJsonObject row;
        
        if (hasHeader && !headers.isEmpty())
        {
            for (int j = 0; j < fields.size() && j < headers.size(); ++j)
            {
                row[headers[j].trimmed()] = fields[j].trimmed();
            }
        }
        else
        {
            QJsonArray fieldsArray;
            for (const QString& field : fields)
            {
                fieldsArray.append(field.trimmed());
            }
            row["fields"] = fieldsArray;
        }
        
        rowsArray.append(row);
    }
    
    QJsonObject data;
    data["csv_string"] = csvString;
    data["rows"] = rowsArray;
    data["row_count"] = rowsArray.size();
    if (hasHeader && !headers.isEmpty())
    {
        QJsonArray headersArray;
        for (const QString& header : headers)
        {
            headersArray.append(header.trimmed());
        }
        data["headers"] = headersArray;
    }
    
    return createSuccessResponse(data);
}

QJsonObject DataFormatTools::toCsv(const QJsonObject& arguments)
{
    QJsonArray dataArray = arguments["data"].toArray();
    QString delimiter = arguments["delimiter"].toString();
    bool includeHeader = arguments["include_header"].toBool(true);
    
    if (dataArray.isEmpty())
    {
        return createErrorResponse("Data array cannot be empty");
    }
    
    if (delimiter.isEmpty())
    {
        delimiter = ",";
    }
    
    QStringList csvLines;
    QStringList headers;
    
    // Get headers from first row
    if (includeHeader && !dataArray.isEmpty())
    {
        QJsonObject firstRow = dataArray[0].toObject();
        headers = firstRow.keys();
        if (!headers.isEmpty())
        {
            csvLines.append(headers.join(delimiter));
        }
    }
    
    // Convert rows to CSV
    for (const QJsonValue& value : dataArray)
    {
        if (value.isObject())
        {
            QJsonObject row = value.toObject();
            QStringList fields;
            
            if (includeHeader && !headers.isEmpty())
            {
                for (const QString& header : headers)
                {
                    fields.append(row[header].toString());
                }
            }
            else
            {
                for (auto it = row.begin(); it != row.end(); ++it)
                {
                    fields.append(it.value().toString());
                }
            }
            
            csvLines.append(fields.join(delimiter));
        }
        else if (value.isArray())
        {
            QJsonArray rowArray = value.toArray();
            QStringList fields;
            for (const QJsonValue& fieldValue : rowArray)
            {
                fields.append(fieldValue.toString());
            }
            csvLines.append(fields.join(delimiter));
        }
    }
    
    QString csvResult = csvLines.join("\n");
    
    QJsonObject data;
    data["csv"] = csvResult;
    data["row_count"] = csvLines.size() - (includeHeader ? 1 : 0);
    data["delimiter"] = delimiter;
    
    return createSuccessResponse(data);
}

