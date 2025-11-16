#pragma once

#include <QJsonObject>
#include <QString>

// 系统信息工具类
class SystemInfoTools
{
public:
    // 获取系统信息
    static QJsonObject getSystemInfo(const QJsonObject& arguments);
    
    // 获取磁盘空间
    static QJsonObject getDiskSpace(const QJsonObject& arguments);
    
    // 获取环境变量
    static QJsonObject getEnvironmentVariable(const QJsonObject& arguments);
    
    // 获取当前工作目录
    static QJsonObject getCurrentDirectory(const QJsonObject& arguments);
    
    // 设置当前工作目录
    static QJsonObject setCurrentDirectory(const QJsonObject& arguments);

private:
    // 辅助方法：创建错误响应
    static QJsonObject createErrorResponse(const QString& error);
    
    // 辅助方法：创建成功响应
    static QJsonObject createSuccessResponse(const QJsonObject& data = QJsonObject());
};

