#pragma once

#include <QJsonObject>
#include <QString>

// 日期时间工具类
class DateTimeTools
{
public:
    // 格式化日期时间
    static QJsonObject formatDateTime(const QJsonObject& arguments);
    
    // 计算日期时间（加减）
    static QJsonObject calculateDateTime(const QJsonObject& arguments);
    
    // 解析日期时间字符串
    static QJsonObject parseDateTime(const QJsonObject& arguments);
    
    // 获取时区信息
    static QJsonObject getTimezone(const QJsonObject& arguments);
    
    // 计算时间差
    static QJsonObject timeDifference(const QJsonObject& arguments);

private:
    // 辅助方法：创建错误响应
    static QJsonObject createErrorResponse(const QString& error);
    
    // 辅助方法：创建成功响应
    static QJsonObject createSuccessResponse(const QJsonObject& data = QJsonObject());
};

