#include "UtilityTools.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QUuid>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QDebug>
#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <QDomDocument>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif

QJsonObject UtilityTools::createErrorResponse(const QString& error)
{
    QJsonObject result;
    result["success"] = false;
    result["error"] = error;
    return result;
}

QJsonObject UtilityTools::createSuccessResponse(const QJsonObject& data)
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

double UtilityTools::safeCalculate(const QString& expression, bool& success)
{
    success = false;
    
    // Remove whitespace
    QString expr = expression.trimmed();
    if (expr.isEmpty())
    {
        return 0.0;
    }
    
    // Simple validation: only allow numbers, operators, parentheses, and decimal points
    QRegularExpression validChars("^[0-9+\\-*/().\\s]+$");
    if (!validChars.match(expr).hasMatch())
    {
        return 0.0;
    }
    
    // Try to evaluate using a simple approach
    // Note: This is a basic implementation. For production, consider using a proper expression parser
    try
    {
        // Replace common math functions
        expr.replace("sin", "std::sin", Qt::CaseInsensitive);
        expr.replace("cos", "std::cos", Qt::CaseInsensitive);
        expr.replace("tan", "std::tan", Qt::CaseInsensitive);
        expr.replace("sqrt", "std::sqrt", Qt::CaseInsensitive);
        expr.replace("log", "std::log", Qt::CaseInsensitive);
        expr.replace("ln", "std::log", Qt::CaseInsensitive);
        expr.replace("exp", "std::exp", Qt::CaseInsensitive);
        expr.replace("abs", "std::abs", Qt::CaseInsensitive);
        // Note: pow replacement removed as it requires two arguments
        expr.replace(QString("pi"), QString::number(M_PI), Qt::CaseInsensitive);
        expr.replace(QString("e"), QString::number(M_E), Qt::CaseInsensitive);
        
        // Simple evaluation (this is a basic implementation)
        // For a more robust solution, you might want to use a proper expression parser library
        QRegularExpression numberRegex("\\d+\\.?\\d*");
        QRegularExpressionMatchIterator it = numberRegex.globalMatch(expr);
        
        // For now, use a simple approach: evaluate basic arithmetic
        // This is a simplified version - for production, use a proper math parser
        bool ok;
        double result = expr.toDouble(&ok);
        if (ok)
        {
            success = true;
            return result;
        }
        
        // Try to evaluate as a simple expression
        // This is a placeholder - in production, use a proper expression evaluator
        success = false;
        return 0.0;
    }
    catch (...)
    {
        success = false;
        return 0.0;
    }
}

QJsonObject UtilityTools::calculate(const QJsonObject& arguments)
{
    QString expression = arguments["expression"].toString();
    
    if (expression.isEmpty())
    {
        return createErrorResponse("Expression cannot be empty");
    }
    
    // Simple arithmetic evaluation
    // For production, consider using a proper math expression parser
    bool ok;
    double result = 0.0;
    
    // Try simple arithmetic operations
    QRegularExpression simpleExpr("^\\s*([0-9.]+)\\s*([+\\-*/])\\s*([0-9.]+)\\s*$");
    QRegularExpressionMatch match = simpleExpr.match(expression);
    
    if (match.hasMatch())
    {
        double num1 = match.captured(1).toDouble(&ok);
        if (!ok)
        {
            return createErrorResponse("Invalid number in expression");
        }
        
        QString op = match.captured(2);
        double num2 = match.captured(3).toDouble(&ok);
        if (!ok)
        {
            return createErrorResponse("Invalid number in expression");
        }
        
        if (op == "+")
        {
            result = num1 + num2;
        }
        else if (op == "-")
        {
            result = num1 - num2;
        }
        else if (op == "*")
        {
            result = num1 * num2;
        }
        else if (op == "/")
        {
            if (num2 == 0.0)
            {
                return createErrorResponse("Division by zero");
            }
            result = num1 / num2;
        }
    }
    else
    {
        // Try to evaluate as a single number
        result = expression.toDouble(&ok);
        if (!ok)
        {
            return createErrorResponse(QString("Cannot evaluate expression: %1. Only simple arithmetic (a op b) is supported.").arg(expression));
        }
    }
    
    QJsonObject data;
    data["expression"] = expression;
    data["result"] = result;
    
    return createSuccessResponse(data);
}

QJsonObject UtilityTools::unitConvert(const QJsonObject& arguments)
{
    double value = arguments["value"].toDouble();
    QString fromUnit = arguments["from_unit"].toString().toLower();
    QString toUnit = arguments["to_unit"].toString().toLower();
    
    if (fromUnit.isEmpty() || toUnit.isEmpty())
    {
        return createErrorResponse("From unit and to unit cannot be empty");
    }
    
    double result = value;
    bool converted = false;
    
    // Length conversions
    if (fromUnit == "m" && toUnit == "km")
    {
        result = value / 1000.0;
        converted = true;
    }
    else if (fromUnit == "km" && toUnit == "m")
    {
        result = value * 1000.0;
        converted = true;
    }
    else if (fromUnit == "m" && toUnit == "cm")
    {
        result = value * 100.0;
        converted = true;
    }
    else if (fromUnit == "cm" && toUnit == "m")
    {
        result = value / 100.0;
        converted = true;
    }
    else if (fromUnit == "m" && toUnit == "mm")
    {
        result = value * 1000.0;
        converted = true;
    }
    else if (fromUnit == "mm" && toUnit == "m")
    {
        result = value / 1000.0;
        converted = true;
    }
    else if (fromUnit == "in" && toUnit == "cm")
    {
        result = value * 2.54;
        converted = true;
    }
    else if (fromUnit == "cm" && toUnit == "in")
    {
        result = value / 2.54;
        converted = true;
    }
    else if (fromUnit == "ft" && toUnit == "m")
    {
        result = value * 0.3048;
        converted = true;
    }
    else if (fromUnit == "m" && toUnit == "ft")
    {
        result = value / 0.3048;
        converted = true;
    }
    // Weight conversions
    else if (fromUnit == "kg" && toUnit == "g")
    {
        result = value * 1000.0;
        converted = true;
    }
    else if (fromUnit == "g" && toUnit == "kg")
    {
        result = value / 1000.0;
        converted = true;
    }
    else if (fromUnit == "kg" && toUnit == "lb")
    {
        result = value * 2.20462;
        converted = true;
    }
    else if (fromUnit == "lb" && toUnit == "kg")
    {
        result = value / 2.20462;
        converted = true;
    }
    // Temperature conversions
    else if (fromUnit == "c" && toUnit == "f")
    {
        result = (value * 9.0 / 5.0) + 32.0;
        converted = true;
    }
    else if (fromUnit == "f" && toUnit == "c")
    {
        result = (value - 32.0) * 5.0 / 9.0;
        converted = true;
    }
    else if (fromUnit == "c" && toUnit == "k")
    {
        result = value + 273.15;
        converted = true;
    }
    else if (fromUnit == "k" && toUnit == "c")
    {
        result = value - 273.15;
        converted = true;
    }
    // Same unit
    else if (fromUnit == toUnit)
    {
        result = value;
        converted = true;
    }
    
    if (!converted)
    {
        return createErrorResponse(QString("Unsupported unit conversion: %1 to %2").arg(fromUnit, toUnit));
    }
    
    QJsonObject data;
    data["value"] = value;
    data["from_unit"] = fromUnit;
    data["to_unit"] = toUnit;
    data["result"] = result;
    
    return createSuccessResponse(data);
}

QJsonObject UtilityTools::numberFormat(const QJsonObject& arguments)
{
    double number = arguments["number"].toDouble();
    QString format = arguments["format"].toString().toLower();
    int decimals = arguments["decimals"].toInt(-1);
    
    QString formatted;
    
    if (format == "thousands" || format == "comma")
    {
        // Add thousands separator
        QString numStr = QString::number(number, 'f', decimals >= 0 ? decimals : 2);
        QRegularExpression re("(\\d)(?=(\\d{3})+(?!\\d))");
        formatted = numStr.replace(re, "\\1,");
    }
    else if (format == "currency")
    {
        formatted = QString::number(number, 'f', decimals >= 0 ? decimals : 2);
    }
    else if (format == "percent")
    {
        formatted = QString::number(number * 100, 'f', decimals >= 0 ? decimals : 2) + "%";
    }
    else if (format == "scientific")
    {
        formatted = QString::number(number, 'e', decimals >= 0 ? decimals : 6);
    }
    else
    {
        // Default: just format with decimal places
        formatted = QString::number(number, 'f', decimals >= 0 ? decimals : 2);
    }
    
    QJsonObject data;
    data["number"] = number;
    data["formatted"] = formatted;
    data["format"] = format;
    
    return createSuccessResponse(data);
}

QJsonObject UtilityTools::generateUuid(const QJsonObject& arguments)
{
    QUuid uuid = QUuid::createUuid();
    QString uuidStr = uuid.toString(QUuid::WithoutBraces);
    
    QJsonObject data;
    data["uuid"] = uuidStr;
    data["uuid_with_braces"] = uuid.toString();
    
    return createSuccessResponse(data);
}

QJsonObject UtilityTools::generateRandomString(const QJsonObject& arguments)
{
    int length = arguments["length"].toInt(16);
    QString charset = arguments["charset"].toString();
    
    if (length <= 0)
    {
        return createErrorResponse("Length must be greater than 0");
    }
    
    if (length > 1000)
    {
        return createErrorResponse("Length cannot exceed 1000");
    }
    
    if (charset.isEmpty())
    {
        charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    }
    
    QString result;
    QRandomGenerator* generator = QRandomGenerator::global();
    
    for (int i = 0; i < length; ++i)
    {
        int index = generator->bounded(charset.length());
        result.append(charset.at(index));
    }
    
    QJsonObject data;
    data["random_string"] = result;
    data["length"] = length;
    data["charset"] = charset;
    
    return createSuccessResponse(data);
}

QJsonObject UtilityTools::hashString(const QJsonObject& arguments)
{
    QString text = arguments["text"].toString();
    QString algorithm = arguments["algorithm"].toString().toLower();
    
    if (text.isEmpty())
    {
        return createErrorResponse("Text cannot be empty");
    }
    
    if (algorithm.isEmpty())
    {
        algorithm = "md5";
    }
    
    QCryptographicHash::Algorithm hashAlgorithm;
    if (algorithm == "md5")
    {
        hashAlgorithm = QCryptographicHash::Md5;
    }
    else if (algorithm == "sha1")
    {
        hashAlgorithm = QCryptographicHash::Sha1;
    }
    else if (algorithm == "sha256")
    {
        hashAlgorithm = QCryptographicHash::Sha256;
    }
    else if (algorithm == "sha512")
    {
        hashAlgorithm = QCryptographicHash::Sha512;
    }
    else
    {
        return createErrorResponse("Unsupported algorithm. Supported: md5, sha1, sha256, sha512");
    }
    
    QCryptographicHash hash(hashAlgorithm);
    hash.addData(text.toUtf8());
    QString hashResult = hash.result().toHex();
    
    QJsonObject data;
    data["text"] = text;
    data["algorithm"] = algorithm;
    data["hash"] = hashResult;
    
    return createSuccessResponse(data);
}

QJsonObject UtilityTools::validateJson(const QJsonObject& arguments)
{
    QString jsonString = arguments["json_string"].toString();
    
    if (jsonString.isEmpty())
    {
        return createErrorResponse("JSON string cannot be empty");
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    
    QJsonObject data;
    data["json_string"] = jsonString;
    data["is_valid"] = (error.error == QJsonParseError::NoError);
    
    if (error.error != QJsonParseError::NoError)
    {
        data["error_message"] = error.errorString();
        data["error_offset"] = error.offset;
    }
    else
    {
        data["is_object"] = doc.isObject();
        data["is_array"] = doc.isArray();
    }
    
    return createSuccessResponse(data);
}

QJsonObject UtilityTools::validateXml(const QJsonObject& arguments)
{
    QString xmlString = arguments["xml_string"].toString();
    
    if (xmlString.isEmpty())
    {
        return createErrorResponse("XML string cannot be empty");
    }
    
    QDomDocument doc;
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;
    
    bool isValid = doc.setContent(xmlString, false, &errorMsg, &errorLine, &errorColumn);
    
    QJsonObject data;
    data["xml_string"] = xmlString;
    data["is_valid"] = isValid;
    
    if (!isValid)
    {
        data["error_message"] = errorMsg;
        data["error_line"] = errorLine;
        data["error_column"] = errorColumn;
    }
    else
    {
        data["root_element"] = doc.documentElement().tagName();
    }
    
    return createSuccessResponse(data);
}

