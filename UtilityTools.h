#pragma once

#include <QJsonObject>
#include <QString>

// 实用工具类
class UtilityTools
{
public:
    // 数学计算
    static QJsonObject calculate(const QJsonObject& arguments);
    
    // 单位转换
    static QJsonObject unitConvert(const QJsonObject& arguments);
    
    // 数字格式化
    static QJsonObject numberFormat(const QJsonObject& arguments);
    
    // 生成UUID
    static QJsonObject generateUuid(const QJsonObject& arguments);
    
    // 生成随机字符串
    static QJsonObject generateRandomString(const QJsonObject& arguments);
    
    // 字符串哈希
    static QJsonObject hashString(const QJsonObject& arguments);
    
    // 验证JSON格式
    static QJsonObject validateJson(const QJsonObject& arguments);
    
    // 验证XML格式
    static QJsonObject validateXml(const QJsonObject& arguments);

private:
    // 辅助方法：创建错误响应
    static QJsonObject createErrorResponse(const QString& error);
    
    // 辅助方法：创建成功响应
    static QJsonObject createSuccessResponse(const QJsonObject& data = QJsonObject());
    
    // 辅助方法：安全计算数学表达式
    static double safeCalculate(const QString& expression, bool& success);
};

