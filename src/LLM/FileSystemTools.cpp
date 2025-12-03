#include "FileSystemTools.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QJsonArray>
#include <QDateTime>

QJsonObject FileSystemTools::createErrorResponse(const QString& error)
{
    QJsonObject result;
    result["success"] = false;
    result["error"] = error;
    return result;
}

QJsonObject FileSystemTools::createSuccessResponse(const QJsonObject& data)
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

QJsonObject FileSystemTools::readFile(const QJsonObject& arguments)
{
    QString filePath = arguments["file_path"].toString();
    if (filePath.isEmpty())
    {
        return createErrorResponse("File path cannot be empty");
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists())
    {
        return createErrorResponse(QString("File does not exist: %1").arg(filePath));
    }
    
    if (!fileInfo.isFile())
    {
        return createErrorResponse(QString("Path is not a file: %1").arg(filePath));
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return createErrorResponse(QString("Cannot open file: %1").arg(file.errorString()));
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString content = in.readAll();
    file.close();
    
    QJsonObject data;
    data["content"] = content;
    data["size"] = static_cast<qint64>(fileInfo.size());
    data["encoding"] = "UTF-8";
    data["modified_time"] = fileInfo.lastModified().toString(Qt::ISODate);
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::writeFile(const QJsonObject& arguments)
{
    QString filePath = arguments["file_path"].toString();
    QString content = arguments["content"].toString();
    bool append = arguments["append"].toBool(false);
    
    if (filePath.isEmpty())
    {
        return createErrorResponse("File path cannot be empty");
    }
    
    QFile file(filePath);
    QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;
    if (append)
    {
        mode = QIODevice::Append | QIODevice::Text;
    }
    
    if (!file.open(mode))
    {
        return createErrorResponse(QString("Cannot open file: %1").arg(file.errorString()));
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << content;
    file.close();
    
    QJsonObject data;
    data["file_path"] = filePath;
    data["bytes_written"] = static_cast<qint64>(content.toUtf8().size());
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::listDirectory(const QJsonObject& arguments)
{
    QString dirPath = arguments["dir_path"].toString();
    if (dirPath.isEmpty())
    {
        dirPath = QDir::currentPath();
    }
    
    bool recursive = arguments["recursive"].toBool(false);
    QString filter = arguments["filter"].toString(); // e.g. "*.cpp", "*.h"
    
    QDir dir(dirPath);
    if (!dir.exists())
    {
        return createErrorResponse(QString("Directory does not exist: %1").arg(dirPath));
    }
    
    QJsonArray filesArray;
    QJsonArray dirsArray;
    
    QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    if (!recursive)
    {
        filters |= QDir::NoSymLinks;
    }
    
    QStringList nameFilters;
    if (!filter.isEmpty())
    {
        nameFilters << filter;
    }
    
    if (recursive)
    {
        QDirIterator it(dirPath, nameFilters, filters, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            QString path = it.next();
            QFileInfo info(path);
            
            QJsonObject item;
            item["path"] = path;
            item["name"] = info.fileName();
            item["size"] = static_cast<qint64>(info.size());
            item["modified_time"] = info.lastModified().toString(Qt::ISODate);
            
            if (info.isFile())
            {
                item["type"] = "file";
                filesArray.append(item);
            }
            else if (info.isDir())
            {
                item["type"] = "directory";
                dirsArray.append(item);
            }
        }
    }
    else
    {
        QFileInfoList entries = dir.entryInfoList(nameFilters, filters);
        for (const QFileInfo& info : entries)
        {
            QJsonObject item;
            item["path"] = info.absoluteFilePath();
            item["name"] = info.fileName();
            item["size"] = static_cast<qint64>(info.size());
            item["modified_time"] = info.lastModified().toString(Qt::ISODate);
            
            if (info.isFile())
            {
                item["type"] = "file";
                filesArray.append(item);
            }
            else if (info.isDir())
            {
                item["type"] = "directory";
                dirsArray.append(item);
            }
        }
    }
    
    QJsonObject data;
    data["directory"] = dirPath;
    data["files"] = filesArray;
    data["directories"] = dirsArray;
    data["file_count"] = filesArray.size();
    data["directory_count"] = dirsArray.size();
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::searchInFiles(const QJsonObject& arguments)
{
    QString searchPath = arguments["search_path"].toString();
    QString keyword = arguments["keyword"].toString();
    QString filePattern = arguments["file_pattern"].toString(""); // e.g. "*.cpp", "*.h"
    bool caseSensitive = arguments["case_sensitive"].toBool(false);
    bool recursive = arguments["recursive"].toBool(true);
    
    if (searchPath.isEmpty())
    {
        searchPath = QDir::currentPath();
    }
    
    if (keyword.isEmpty())
    {
        return createErrorResponse("Search keyword cannot be empty");
    }
    
    QDir dir(searchPath);
    if (!dir.exists())
    {
        return createErrorResponse(QString("Search path does not exist: %1").arg(searchPath));
    }
    
    Qt::CaseSensitivity sensitivity = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    QRegularExpression regex(keyword, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
    
    QJsonArray matchesArray;
    int totalMatches = 0;
    
    QStringList nameFilters;
    if (!filePattern.isEmpty())
    {
        nameFilters << filePattern;
    }
    else
    {
        nameFilters << "*";
    }
    
    QDirIterator::IteratorFlags flags = recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
    QDirIterator it(searchPath, nameFilters, QDir::Files | QDir::NoSymLinks, flags);
    
    while (it.hasNext())
    {
        QString filePath = it.next();
        QFile file(filePath);
        
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            continue;
        }
        
        QTextStream in(&file);
        in.setCodec("UTF-8");
        int lineNumber = 0;
        QJsonArray fileMatches;
        
        while (!in.atEnd())
        {
            QString line = in.readLine();
            lineNumber++;
            
            if (line.contains(keyword, sensitivity))
            {
                QJsonObject match;
                match["line_number"] = lineNumber;
                match["line_content"] = line.trimmed();
                fileMatches.append(match);
                totalMatches++;
            }
        }
        
        file.close();
        
        if (fileMatches.size() > 0)
        {
            QJsonObject fileMatch;
            fileMatch["file_path"] = filePath;
            fileMatch["matches"] = fileMatches;
            fileMatch["match_count"] = fileMatches.size();
            matchesArray.append(fileMatch);
        }
    }
    
    QJsonObject data;
    data["search_path"] = searchPath;
    data["keyword"] = keyword;
    data["matches"] = matchesArray;
    data["total_files_matched"] = matchesArray.size();
    data["total_matches"] = totalMatches;
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::createDirectory(const QJsonObject& arguments)
{
    QString dirPath = arguments["dir_path"].toString();
    if (dirPath.isEmpty())
    {
        return createErrorResponse("Directory path cannot be empty");
    }
    
    QDir dir;
    if (!dir.mkpath(dirPath))
    {
        return createErrorResponse(QString("Cannot create directory: %1").arg(dirPath));
    }
    
    QJsonObject data;
    data["dir_path"] = dirPath;
    data["created"] = true;
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::deleteFile(const QJsonObject& arguments)
{
    QString filePath = arguments["file_path"].toString();
    if (filePath.isEmpty())
    {
        return createErrorResponse("File path cannot be empty");
    }
    
    QFile file(filePath);
    if (!file.exists())
    {
        return createErrorResponse(QString("File does not exist: %1").arg(filePath));
    }
    
    if (!file.remove())
    {
        return createErrorResponse(QString("Cannot delete file: %1").arg(file.errorString()));
    }
    
    QJsonObject data;
    data["file_path"] = filePath;
    data["deleted"] = true;
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::deleteDirectory(const QJsonObject& arguments)
{
    QString dirPath = arguments["dir_path"].toString();
    bool recursive = arguments["recursive"].toBool(false);
    
    if (dirPath.isEmpty())
    {
        return createErrorResponse("Directory path cannot be empty");
    }
    
    QDir dir(dirPath);
    if (!dir.exists())
    {
        return createErrorResponse(QString("Directory does not exist: %1").arg(dirPath));
    }
    
    bool success = false;
    if (recursive)
    {
        success = dir.removeRecursively();
    }
    else
    {
        success = dir.rmdir(dirPath);
    }
    
    if (!success)
    {
        return createErrorResponse(QString("Cannot delete directory: %1").arg(dirPath));
    }
    
    QJsonObject data;
    data["dir_path"] = dirPath;
    data["deleted"] = true;
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::copyFile(const QJsonObject& arguments)
{
    QString sourcePath = arguments["source_path"].toString();
    QString destPath = arguments["dest_path"].toString();
    
    if (sourcePath.isEmpty() || destPath.isEmpty())
    {
        return createErrorResponse("Source path and destination path cannot be empty");
    }
    
    QFile sourceFile(sourcePath);
    if (!sourceFile.exists())
    {
        return createErrorResponse(QString("Source file does not exist: %1").arg(sourcePath));
    }
    
    // If destination path is a directory, use source file name
    QFileInfo destInfo(destPath);
    if (destInfo.isDir())
    {
        QFileInfo sourceInfo(sourcePath);
        destPath = QDir(destPath).filePath(sourceInfo.fileName());
    }
    
    // If destination file exists, remove it first
    if (QFile::exists(destPath))
    {
        QFile::remove(destPath);
    }
    
    if (!QFile::copy(sourcePath, destPath))
    {
        return createErrorResponse(QString("Cannot copy file: %1 -> %2").arg(sourcePath, destPath));
    }
    
    QJsonObject data;
    data["source_path"] = sourcePath;
    data["dest_path"] = destPath;
    data["copied"] = true;
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::moveFile(const QJsonObject& arguments)
{
    QString sourcePath = arguments["source_path"].toString();
    QString destPath = arguments["dest_path"].toString();
    
    if (sourcePath.isEmpty() || destPath.isEmpty())
    {
        return createErrorResponse("Source path and destination path cannot be empty");
    }
    
    QFile sourceFile(sourcePath);
    if (!sourceFile.exists())
    {
        return createErrorResponse(QString("Source file does not exist: %1").arg(sourcePath));
    }
    
    // If destination path is a directory, use source file name
    QFileInfo destInfo(destPath);
    if (destInfo.isDir())
    {
        QFileInfo sourceInfo(sourcePath);
        destPath = QDir(destPath).filePath(sourceInfo.fileName());
    }
    
    // If destination file exists, remove it first
    if (QFile::exists(destPath))
    {
        QFile::remove(destPath);
    }
    
    if (!sourceFile.rename(destPath))
    {
        return createErrorResponse(QString("Cannot move file: %1 -> %2").arg(sourcePath, destPath));
    }
    
    QJsonObject data;
    data["source_path"] = sourcePath;
    data["dest_path"] = destPath;
    data["moved"] = true;
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::getFileInfo(const QJsonObject& arguments)
{
    QString filePath = arguments["file_path"].toString();
    if (filePath.isEmpty())
    {
        return createErrorResponse("File path cannot be empty");
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists())
    {
        return createErrorResponse(QString("File or directory does not exist: %1").arg(filePath));
    }
    
    QJsonObject data;
    data["path"] = fileInfo.absoluteFilePath();
    data["name"] = fileInfo.fileName();
    data["size"] = static_cast<qint64>(fileInfo.size());
    data["is_file"] = fileInfo.isFile();
    data["is_dir"] = fileInfo.isDir();
    data["extension"] = fileInfo.suffix();
    data["created_time"] = fileInfo.birthTime().toString(Qt::ISODate);
    data["modified_time"] = fileInfo.lastModified().toString(Qt::ISODate);
    data["readable"] = fileInfo.isReadable();
    data["writable"] = fileInfo.isWritable();
    data["executable"] = fileInfo.isExecutable();
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::findFiles(const QJsonObject& arguments)
{
    QString searchPath = arguments["search_path"].toString();
    QString fileNamePattern = arguments["file_name_pattern"].toString();
    bool recursive = arguments["recursive"].toBool(true);
    
    if (searchPath.isEmpty())
    {
        searchPath = QDir::currentPath();
    }
    
    if (fileNamePattern.isEmpty())
    {
        fileNamePattern = "*";
    }
    
    QDir dir(searchPath);
    if (!dir.exists())
    {
        return createErrorResponse(QString("Search path does not exist: %1").arg(searchPath));
    }
    
    QJsonArray filesArray;
    
    QStringList nameFilters;
    nameFilters << fileNamePattern;
    
    QDirIterator::IteratorFlags flags = recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
    QDirIterator it(searchPath, nameFilters, QDir::Files | QDir::NoSymLinks, flags);
    
    while (it.hasNext())
    {
        QString filePath = it.next();
        QFileInfo info(filePath);
        
        QJsonObject file;
        file["path"] = filePath;
        file["name"] = info.fileName();
        file["size"] = static_cast<qint64>(info.size());
        file["modified_time"] = info.lastModified().toString(Qt::ISODate);
        filesArray.append(file);
    }
    
    QJsonObject data;
    data["search_path"] = searchPath;
    data["pattern"] = fileNamePattern;
    data["files"] = filesArray;
    data["count"] = filesArray.size();
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::countLines(const QJsonObject& arguments)
{
    QString filePath = arguments["file_path"].toString();
    if (filePath.isEmpty())
    {
        return createErrorResponse("File path cannot be empty");
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return createErrorResponse(QString("Cannot open file: %1").arg(file.errorString()));
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    int lineCount = 0;
    int emptyLineCount = 0;
    int nonEmptyLineCount = 0;
    
    while (!in.atEnd())
    {
        QString line = in.readLine();
        lineCount++;
        if (line.trimmed().isEmpty())
        {
            emptyLineCount++;
        }
        else
        {
            nonEmptyLineCount++;
        }
    }
    
    file.close();
    
    QJsonObject data;
    data["file_path"] = filePath;
    data["total_lines"] = lineCount;
    data["empty_lines"] = emptyLineCount;
    data["non_empty_lines"] = nonEmptyLineCount;
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::pathExists(const QJsonObject& arguments)
{
    QString path = arguments["path"].toString();
    if (path.isEmpty())
    {
        return createErrorResponse("Path cannot be empty");
    }
    
    QFileInfo info(path);
    QJsonObject data;
    data["path"] = path;
    data["exists"] = info.exists();
    data["is_file"] = info.isFile();
    data["is_dir"] = info.isDir();
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::joinPath(const QJsonObject& arguments)
{
    QJsonArray pathsArray = arguments["paths"].toArray();
    if (pathsArray.isEmpty())
    {
        return createErrorResponse("Paths array cannot be empty");
    }
    
    QStringList paths;
    for (const QJsonValue& value : pathsArray)
    {
        paths << value.toString();
    }
    
    QDir dir;
    QString joinedPath = dir.cleanPath(paths.join(QDir::separator()));
    
    QJsonObject data;
    data["joined_path"] = joinedPath;
    data["paths"] = pathsArray;
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::normalizePath(const QJsonObject& arguments)
{
    QString path = arguments["path"].toString();
    if (path.isEmpty())
    {
        return createErrorResponse("Path cannot be empty");
    }
    
    QDir dir;
    QString normalizedPath = dir.cleanPath(path);
    
    QJsonObject data;
    data["original_path"] = path;
    data["normalized_path"] = normalizedPath;
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::getFileName(const QJsonObject& arguments)
{
    QString filePath = arguments["file_path"].toString();
    if (filePath.isEmpty())
    {
        return createErrorResponse("File path cannot be empty");
    }
    
    QFileInfo info(filePath);
    QJsonObject data;
    data["file_path"] = filePath;
    data["file_name"] = info.fileName();
    data["base_name"] = info.baseName();
    data["complete_base_name"] = info.completeBaseName();
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::getDirectory(const QJsonObject& arguments)
{
    QString filePath = arguments["file_path"].toString();
    if (filePath.isEmpty())
    {
        return createErrorResponse("File path cannot be empty");
    }
    
    QFileInfo info(filePath);
    QJsonObject data;
    data["file_path"] = filePath;
    data["directory"] = info.absolutePath();
    data["absolute_path"] = info.absoluteFilePath();
    
    return createSuccessResponse(data);
}

QJsonObject FileSystemTools::getFileExtension(const QJsonObject& arguments)
{
    QString filePath = arguments["file_path"].toString();
    if (filePath.isEmpty())
    {
        return createErrorResponse("File path cannot be empty");
    }
    
    QFileInfo info(filePath);
    QJsonObject data;
    data["file_path"] = filePath;
    data["extension"] = info.suffix();
    data["complete_suffix"] = info.completeSuffix();
    
    return createSuccessResponse(data);
}

