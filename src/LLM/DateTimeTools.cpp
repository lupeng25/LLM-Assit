#include "DateTimeTools.h"
#include <QDateTime>
#include <QTimeZone>
#include <QDebug>

QJsonObject DateTimeTools::createErrorResponse(const QString& error)
{
    QJsonObject result;
    result["success"] = false;
    result["error"] = error;
    return result;
}

QJsonObject DateTimeTools::createSuccessResponse(const QJsonObject& data)
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

QJsonObject DateTimeTools::formatDateTime(const QJsonObject& arguments)
{
    QString datetimeStr = arguments["datetime"].toString();
    QString format = arguments["format"].toString();
    
    QDateTime dt;
    if (datetimeStr.isEmpty())
    {
        dt = QDateTime::currentDateTime();
    }
    else
    {
        // Try to parse the datetime string
        dt = QDateTime::fromString(datetimeStr, Qt::ISODate);
        if (!dt.isValid())
        {
            dt = QDateTime::fromString(datetimeStr);
        }
        if (!dt.isValid())
        {
            return createErrorResponse(QString("Invalid datetime string: %1").arg(datetimeStr));
        }
    }
    
    if (format.isEmpty())
    {
        format = "yyyy-MM-dd HH:mm:ss";
    }
    
    QString formatted = dt.toString(format);
    
    QJsonObject data;
    data["original_datetime"] = dt.toString(Qt::ISODate);
    data["formatted_datetime"] = formatted;
    data["format"] = format;
    data["timestamp"] = dt.toSecsSinceEpoch();
    data["timestamp_ms"] = dt.toMSecsSinceEpoch();
    
    return createSuccessResponse(data);
}

QJsonObject DateTimeTools::calculateDateTime(const QJsonObject& arguments)
{
    QString datetimeStr = arguments["datetime"].toString();
    QString operation = arguments["operation"].toString().toLower(); // add/subtract
    qint64 value = arguments["value"].toInt();
    QString unit = arguments["unit"].toString().toLower(); // days/hours/minutes/seconds
    
    QDateTime dt;
    if (datetimeStr.isEmpty())
    {
        dt = QDateTime::currentDateTime();
    }
    else
    {
        dt = QDateTime::fromString(datetimeStr, Qt::ISODate);
        if (!dt.isValid())
        {
            dt = QDateTime::fromString(datetimeStr);
        }
        if (!dt.isValid())
        {
            return createErrorResponse(QString("Invalid datetime string: %1").arg(datetimeStr));
        }
    }
    
    if (operation != "add" && operation != "subtract")
    {
        return createErrorResponse("Operation must be 'add' or 'subtract'");
    }
    
    QDateTime resultDt = dt;
    
    if (unit == "days")
    {
        resultDt = (operation == "add") ? dt.addDays(value) : dt.addDays(-value);
    }
    else if (unit == "hours")
    {
        resultDt = (operation == "add") ? dt.addSecs(value * 3600) : dt.addSecs(-value * 3600);
    }
    else if (unit == "minutes")
    {
        resultDt = (operation == "add") ? dt.addSecs(value * 60) : dt.addSecs(-value * 60);
    }
    else if (unit == "seconds")
    {
        resultDt = (operation == "add") ? dt.addSecs(value) : dt.addSecs(-value);
    }
    else
    {
        return createErrorResponse("Unit must be 'days', 'hours', 'minutes', or 'seconds'");
    }
    
    QJsonObject data;
    data["original_datetime"] = dt.toString(Qt::ISODate);
    data["result_datetime"] = resultDt.toString(Qt::ISODate);
    data["operation"] = operation;
    data["value"] = value;
    data["unit"] = unit;
    data["timestamp"] = resultDt.toSecsSinceEpoch();
    
    return createSuccessResponse(data);
}

QJsonObject DateTimeTools::parseDateTime(const QJsonObject& arguments)
{
    QString datetimeStr = arguments["datetime"].toString();
    QString format = arguments["format"].toString();
    
    if (datetimeStr.isEmpty())
    {
        return createErrorResponse("Datetime string cannot be empty");
    }
    
    QDateTime dt;
    if (!format.isEmpty())
    {
        dt = QDateTime::fromString(datetimeStr, format);
    }
    else
    {
        // Try common formats
        dt = QDateTime::fromString(datetimeStr, Qt::ISODate);
        if (!dt.isValid())
        {
            dt = QDateTime::fromString(datetimeStr, "yyyy-MM-dd HH:mm:ss");
        }
        if (!dt.isValid())
        {
            dt = QDateTime::fromString(datetimeStr);
        }
    }
    
    if (!dt.isValid())
    {
        return createErrorResponse(QString("Cannot parse datetime string: %1").arg(datetimeStr));
    }
    
    QJsonObject data;
    data["datetime_string"] = datetimeStr;
    data["parsed_datetime"] = dt.toString(Qt::ISODate);
    data["year"] = dt.date().year();
    data["month"] = dt.date().month();
    data["day"] = dt.date().day();
    data["hour"] = dt.time().hour();
    data["minute"] = dt.time().minute();
    data["second"] = dt.time().second();
    data["timestamp"] = dt.toSecsSinceEpoch();
    data["timestamp_ms"] = dt.toMSecsSinceEpoch();
    data["day_of_week"] = dt.date().dayOfWeek();
    data["day_of_year"] = dt.date().dayOfYear();
    
    return createSuccessResponse(data);
}

QJsonObject DateTimeTools::getTimezone(const QJsonObject& arguments)
{
    QTimeZone tz = QTimeZone::systemTimeZone();
    
    QJsonObject data;
    data["timezone_id"] = QString::fromUtf8(tz.id());
    data["timezone_name"] = tz.displayName(QTimeZone::StandardTime);
    data["offset_seconds"] = tz.offsetFromUtc(QDateTime::currentDateTime());
    data["offset_hours"] = tz.offsetFromUtc(QDateTime::currentDateTime()) / 3600.0;
    data["is_dst"] = tz.isDaylightTime(QDateTime::currentDateTime());
    data["current_datetime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    data["utc_datetime"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    
    return createSuccessResponse(data);
}

QJsonObject DateTimeTools::timeDifference(const QJsonObject& arguments)
{
    QString datetime1Str = arguments["datetime1"].toString();
    QString datetime2Str = arguments["datetime2"].toString();
    
    QDateTime dt1, dt2;
    
    if (datetime1Str.isEmpty())
    {
        dt1 = QDateTime::currentDateTime();
    }
    else
    {
        dt1 = QDateTime::fromString(datetime1Str, Qt::ISODate);
        if (!dt1.isValid())
        {
            dt1 = QDateTime::fromString(datetime1Str);
        }
        if (!dt1.isValid())
        {
            return createErrorResponse(QString("Invalid datetime1: %1").arg(datetime1Str));
        }
    }
    
    if (datetime2Str.isEmpty())
    {
        dt2 = QDateTime::currentDateTime();
    }
    else
    {
        dt2 = QDateTime::fromString(datetime2Str, Qt::ISODate);
        if (!dt2.isValid())
        {
            dt2 = QDateTime::fromString(datetime2Str);
        }
        if (!dt2.isValid())
        {
            return createErrorResponse(QString("Invalid datetime2: %1").arg(datetime2Str));
        }
    }
    
    qint64 secondsDiff = dt1.secsTo(dt2);
    qint64 msDiff = dt1.msecsTo(dt2);
    
    QJsonObject data;
    data["datetime1"] = dt1.toString(Qt::ISODate);
    data["datetime2"] = dt2.toString(Qt::ISODate);
    data["difference_seconds"] = secondsDiff;
    data["difference_milliseconds"] = msDiff;
    data["difference_days"] = secondsDiff / 86400.0;
    data["difference_hours"] = secondsDiff / 3600.0;
    data["difference_minutes"] = secondsDiff / 60.0;
    data["is_datetime1_before"] = dt1 < dt2;
    data["is_datetime1_after"] = dt1 > dt2;
    
    return createSuccessResponse(data);
}

