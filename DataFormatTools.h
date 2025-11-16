#pragma once

#include <QJsonObject>
#include <QString>

// 数据格式处理工具类
class DataFormatTools
{
public:
    // 解析JSON
    static QJsonObject parseJson(const QJsonObject& arguments);
    
    // 格式化JSON
    static QJsonObject formatJson(const QJsonObject& arguments);
    
    // 解析CSV
    static QJsonObject parseCsv(const QJsonObject& arguments);
    
    // 转换为CSV
    static QJsonObject toCsv(const QJsonObject& arguments);

private:
    // 辅助方法：创建错误响应
    static QJsonObject createErrorResponse(const QString& error);
    
    // 辅助方法：创建成功响应
    static QJsonObject createSuccessResponse(const QJsonObject& data = QJsonObject());
};

