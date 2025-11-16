#pragma once

#include <QJsonObject>
#include <QString>

// 文本处理工具类
class TextProcessingTools
{
public:
    // 文本替换
    static QJsonObject textReplace(const QJsonObject& arguments);
    
    // 文本统计
    static QJsonObject textStatistics(const QJsonObject& arguments);
    
    // 正则表达式匹配
    static QJsonObject regexMatch(const QJsonObject& arguments);
    
    // 文本提取（使用正则表达式）
    static QJsonObject textExtract(const QJsonObject& arguments);
    
    // 文本编码/解码
    static QJsonObject textEncodeDecode(const QJsonObject& arguments);
    
    // 格式化文本（JSON/XML缩进等）
    static QJsonObject textFormat(const QJsonObject& arguments);

private:
    // 辅助方法：创建错误响应
    static QJsonObject createErrorResponse(const QString& error);
    
    // 辅助方法：创建成功响应
    static QJsonObject createSuccessResponse(const QJsonObject& data = QJsonObject());
};

