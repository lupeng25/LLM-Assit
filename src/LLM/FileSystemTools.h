#pragma once

#include <QJsonObject>
#include <QString>

// 文件系统操作工具类
class FileSystemTools
{
public:
    // 读取文件内容
    static QJsonObject readFile(const QJsonObject& arguments);
    
    // 写入文件内容
    static QJsonObject writeFile(const QJsonObject& arguments);
    
    // 列出目录内容
    static QJsonObject listDirectory(const QJsonObject& arguments);
    
    // 在文件中搜索文本
    static QJsonObject searchInFiles(const QJsonObject& arguments);
    
    // 创建目录
    static QJsonObject createDirectory(const QJsonObject& arguments);
    
    // 删除文件
    static QJsonObject deleteFile(const QJsonObject& arguments);
    
    // 删除目录
    static QJsonObject deleteDirectory(const QJsonObject& arguments);
    
    // 复制文件
    static QJsonObject copyFile(const QJsonObject& arguments);
    
    // 移动/重命名文件
    static QJsonObject moveFile(const QJsonObject& arguments);
    
    // 获取文件信息
    static QJsonObject getFileInfo(const QJsonObject& arguments);
    
    // 查找文件
    static QJsonObject findFiles(const QJsonObject& arguments);
    
    // 统计文件行数
    static QJsonObject countLines(const QJsonObject& arguments);
    
    // 检查路径是否存在
    static QJsonObject pathExists(const QJsonObject& arguments);
    
    // 拼接路径
    static QJsonObject joinPath(const QJsonObject& arguments);
    
    // 规范化路径
    static QJsonObject normalizePath(const QJsonObject& arguments);
    
    // 获取文件名
    static QJsonObject getFileName(const QJsonObject& arguments);
    
    // 获取目录路径
    static QJsonObject getDirectory(const QJsonObject& arguments);
    
    // 获取文件扩展名
    static QJsonObject getFileExtension(const QJsonObject& arguments);

private:
    // 辅助方法：创建错误响应
    static QJsonObject createErrorResponse(const QString& error);
    
    // 辅助方法：创建成功响应
    static QJsonObject createSuccessResponse(const QJsonObject& data = QJsonObject());
};

