#pragma once

#include <QString>
#include <QStringList>

// 发送按钮状态（从 ChatInputWidget 中提取为公共类型）
enum class SendButtonState
{
    Disabled,   // 禁用状态
    Ready,      // 就绪状态（可以发送）
    Cancelable  // 可取消状态（正在生成，可以取消）
};

// 聊天发送消息结构（从 ChatInputWidget 中提取为公共类型）
struct ChatSendMessage
{
    QString SendText = "";     // 发送的文本内容
    QStringList Image64;       // Base64 编码的图片列表
    QStringList fileContext;   // 文件内容列表
};


