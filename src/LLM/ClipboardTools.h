#pragma once

#include <QJsonObject>
#include <QJsonArray>
#include <QPixmap>
#include <QString>

// 剪贴板操作工具类
class ClipboardTools
{
public:
    // 获取剪贴板内容
    static QJsonObject getClipboard(const QJsonObject& arguments);
    
    // 设置剪贴板内容
    static QJsonObject setClipboard(const QJsonObject& arguments);

private:
    // 辅助方法：创建错误响应
    static QJsonObject createErrorResponse(const QString& error);
    
    // 辅助方法：创建成功响应
    static QJsonObject createSuccessResponse(const QJsonObject& data = QJsonObject());
};

