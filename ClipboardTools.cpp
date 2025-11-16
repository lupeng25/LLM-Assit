#include "ClipboardTools.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDebug>
#include<QUrl>

QJsonObject ClipboardTools::createErrorResponse(const QString& error)
{
    QJsonObject result;
    result["success"] = false;
    result["error"] = error;
    return result;
}

QJsonObject ClipboardTools::createSuccessResponse(const QJsonObject& data)
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

QJsonObject ClipboardTools::getClipboard(const QJsonObject& arguments)
{
    QClipboard* clipboard = QApplication::clipboard();
    if (!clipboard)
    {
        return createErrorResponse("Cannot access clipboard");
    }
    
    const QMimeData* mimeData = clipboard->mimeData();
    if (!mimeData)
    {
        return createErrorResponse("Clipboard data is empty");
    }
    
    QJsonObject data;
    
    // Get text content
    if (mimeData->hasText())
    {
        data["text"] = clipboard->text();
    }
    
    // Get HTML content
    if (mimeData->hasHtml())
    {
        data["html"] = clipboard->text(QClipboard::Clipboard);
    }
    
    // Get URLs
    if (mimeData->hasUrls())
    {
        QJsonArray urlsArray;
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl& url : urls)
        {
            urlsArray.append(url.toString());
        }
        data["urls"] = urlsArray;
    }
    
    // Get image (if available)
    if (mimeData->hasImage())
    {
        QPixmap pixmap = clipboard->pixmap();
        if (!pixmap.isNull())
        {
            data["has_image"] = true;
            data["image_width"] = pixmap.width();
            data["image_height"] = pixmap.height();
        }
    }
    
    // Get all MIME types
    QJsonArray formatsArray;
    QStringList formats = mimeData->formats();
    for (const QString& format : formats)
    {
        formatsArray.append(format);
    }
    data["available_formats"] = formatsArray;
    
    return createSuccessResponse(data);
}

QJsonObject ClipboardTools::setClipboard(const QJsonObject& arguments)
{
    QClipboard* clipboard = QApplication::clipboard();
    if (!clipboard)
    {
        return createErrorResponse("Cannot access clipboard");
    }
    
    QString text = arguments["text"].toString();
    QString format = arguments["format"].toString().toLower(); // text/html
    
    if (text.isEmpty())
    {
        return createErrorResponse("Content to set cannot be empty");
    }
    
    if (format == "html")
    {
        clipboard->setText(text, QClipboard::Clipboard);
        // Note: Qt's setText for HTML requires special handling
        // This is simplified, may need to use QMimeData in practice
    }
    else // text or default
    {
        clipboard->setText(text, QClipboard::Clipboard);
    }
    
    QJsonObject data;
    data["text"] = text;
    data["format"] = format;
    data["set"] = true;
    
    return createSuccessResponse(data);
}

