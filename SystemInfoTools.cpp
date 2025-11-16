#include "SystemInfoTools.h"
#include <QDir>
#include <QFileInfo>
#include <QStorageInfo>
#include <QOperatingSystemVersion>
#include <QSysInfo>
#include <QProcessEnvironment>
#include <QDebug>
#include <QJsonArray>
#include<QByteArray>
#include <QStorageInfo>
QJsonObject SystemInfoTools::createErrorResponse(const QString& error)
{
    QJsonObject result;
    result["success"] = false;
    result["error"] = error;
    return result;
}

QJsonObject SystemInfoTools::createSuccessResponse(const QJsonObject& data)
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

QJsonObject SystemInfoTools::getSystemInfo(const QJsonObject& arguments)
{
    QJsonObject data;
    
    // Operating system information
    data["os_type"] = QSysInfo::kernelType();
    data["os_version"] = QSysInfo::kernelVersion();
    data["product_type"] = QSysInfo::productType();
    data["product_version"] = QSysInfo::productVersion();
    
    // Architecture information
    data["cpu_architecture"] = QSysInfo::currentCpuArchitecture();
    data["build_cpu_architecture"] = QSysInfo::buildCpuArchitecture();
    
    // Hostname
    data["machine_host_name"] = QSysInfo::machineHostName();
    data["pretty_product_name"] = QSysInfo::prettyProductName();
    
    // Operating system version details
    QOperatingSystemVersion osVersion = QOperatingSystemVersion::current();
    data["os_major_version"] = osVersion.majorVersion();
    data["os_minor_version"] = osVersion.minorVersion();
    data["os_micro_version"] = osVersion.microVersion();
    
    return createSuccessResponse(data);
}

QJsonObject SystemInfoTools::getDiskSpace(const QJsonObject& arguments)
{
    QString drivePath = arguments["drive_path"].toString();
    
    QJsonArray drivesArray;
    
    if (drivePath.isEmpty())
    {
        // Get all drives
        QList<QStorageInfo> drives = QStorageInfo::mountedVolumes();
        for (const QStorageInfo& drive : drives)
        {
            if (!drive.isValid() || !drive.isReady())
            {
                continue;
            }
            
            qint64 totalBytes = drive.bytesTotal();
            qint64 freeBytes = drive.bytesFree();
            qint64 availableBytes = drive.bytesAvailable();
            qint64 usedBytes = totalBytes - freeBytes;
            
            double totalGB = totalBytes / (1024.0 * 1024.0 * 1024.0);
            double freeGB = freeBytes / (1024.0 * 1024.0 * 1024.0);
            double usedGB = usedBytes / (1024.0 * 1024.0 * 1024.0);
            double usagePercent = totalBytes > 0 ? (usedBytes * 100.0 / totalBytes) : 0.0;
            
            QJsonObject driveInfo;
            driveInfo["root_path"] = drive.rootPath();
            driveInfo["name"] = drive.name();
            driveInfo["file_system_type"] = QString::fromUtf8(drive.fileSystemType());
            driveInfo["total_bytes"] = static_cast<qint64>(totalBytes);
            driveInfo["free_bytes"] = static_cast<qint64>(freeBytes);
            driveInfo["available_bytes"] = static_cast<qint64>(availableBytes);
            driveInfo["used_bytes"] = static_cast<qint64>(usedBytes);
            driveInfo["total_gb"] = QString::number(totalGB, 'f', 2).toDouble();
            driveInfo["free_gb"] = QString::number(freeGB, 'f', 2).toDouble();
            driveInfo["used_gb"] = QString::number(usedGB, 'f', 2).toDouble();
            driveInfo["usage_percent"] = QString::number(usagePercent, 'f', 2).toDouble();
            driveInfo["is_read_only"] = drive.isReadOnly();
            // Note: isRemovable() may not be available in all Qt versions
            // driveInfo["is_removable"] = drive.isRemovable();
            
            drivesArray.append(driveInfo);
        }
    }
    else
    {
        // Get specified drive
        QStorageInfo drive(drivePath);
        if (!drive.isValid() || !drive.isReady())
        {
            return createErrorResponse(QString("Invalid drive path: %1").arg(drivePath));
        }
        
        qint64 totalBytes = drive.bytesTotal();
        qint64 freeBytes = drive.bytesFree();
        qint64 availableBytes = drive.bytesAvailable();
        qint64 usedBytes = totalBytes - freeBytes;
        
        double totalGB = totalBytes / (1024.0 * 1024.0 * 1024.0);
        double freeGB = freeBytes / (1024.0 * 1024.0 * 1024.0);
        double usedGB = usedBytes / (1024.0 * 1024.0 * 1024.0);
        double usagePercent = totalBytes > 0 ? (usedBytes * 100.0 / totalBytes) : 0.0;
        
        QJsonObject driveInfo;
        driveInfo["root_path"] = drive.rootPath();
        driveInfo["name"] = drive.name();
        driveInfo["file_system_type"] = QString::fromUtf8(drive.fileSystemType());
        driveInfo["total_bytes"] = static_cast<qint64>(totalBytes);
        driveInfo["free_bytes"] = static_cast<qint64>(freeBytes);
        driveInfo["available_bytes"] = static_cast<qint64>(availableBytes);
        driveInfo["used_bytes"] = static_cast<qint64>(usedBytes);
        driveInfo["total_gb"] = QString::number(totalGB, 'f', 2).toDouble();
        driveInfo["free_gb"] = QString::number(freeGB, 'f', 2).toDouble();
        driveInfo["used_gb"] = QString::number(usedGB, 'f', 2).toDouble();
        driveInfo["usage_percent"] = QString::number(usagePercent, 'f', 2).toDouble();
        driveInfo["is_read_only"] = drive.isReadOnly();
        // Note: isRemovable() may not be available in all Qt versions
        // driveInfo["is_removable"] = drive.isRemovable();
        
        drivesArray.append(driveInfo);
    }
    
    QJsonObject data;
    data["drives"] = drivesArray;
    data["count"] = drivesArray.size();
    
    return createSuccessResponse(data);
}

QJsonObject SystemInfoTools::getEnvironmentVariable(const QJsonObject& arguments)
{
    QString varName = arguments["var_name"].toString();
    
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
    if (varName.isEmpty())
    {
        // Return all environment variables
        QJsonObject envVars;
        QStringList keys = env.keys();
        for (const QString& key : keys)
        {
            envVars[key] = env.value(key);
        }
        
        QJsonObject data;
        data["environment_variables"] = envVars;
        data["count"] = keys.size();
        
        return createSuccessResponse(data);
    }
    else
    {
        // Return specified environment variable
        QString value = env.value(varName);
        
        QJsonObject data;
        data["var_name"] = varName;
        data["value"] = value;
        data["exists"] = !value.isEmpty();
        
        return createSuccessResponse(data);
    }
}

QJsonObject SystemInfoTools::getCurrentDirectory(const QJsonObject& arguments)
{
    QString currentDir = QDir::currentPath();
    
    QJsonObject data;
    data["current_directory"] = currentDir;
    data["absolute_path"] = QDir(currentDir).absolutePath();
    
    return createSuccessResponse(data);
}

QJsonObject SystemInfoTools::setCurrentDirectory(const QJsonObject& arguments)
{
    QString dirPath = arguments["dir_path"].toString();
    
    if (dirPath.isEmpty())
    {
        return createErrorResponse("Directory path cannot be empty");
    }
    
    QDir dir;
    if (!dir.exists(dirPath))
    {
        return createErrorResponse(QString("Directory does not exist: %1").arg(dirPath));
    }
    
    if (!QDir::setCurrent(dirPath))
    {
        return createErrorResponse(QString("Cannot change to directory: %1").arg(dirPath));
    }
    
    QJsonObject data;
    data["dir_path"] = dirPath;
    data["current_directory"] = QDir::currentPath();
    data["changed"] = true;
    
    return createSuccessResponse(data);
}

