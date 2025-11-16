#pragma once

#include <QHash>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <functional>

// 统一的工具路由/注册中心
class FunctionCallRouter
{
public:
	// 处理函数签名：输入参数 JSON，输出结果 JSON
	using Handler = std::function<QJsonObject(const QJsonObject&)>;

	// 注册工具处理函数
	void registerTool(const QString& name, Handler handler);

	// 执行指定工具
	QJsonObject execute(const QString& name, const QJsonObject& arguments) const;

	// 加载工具清单（FunctionCall.json），仅保存透传给上层的 tools 列表
	bool loadToolsDefinition(const QString& jsonFile);

	// 获取 tools 数组（用于上层发送给模型）
	QJsonArray toolsDefinition() const;

private:
	QHash<QString, Handler> m_handlers;
	QJsonArray m_tools; // 原 FunctionCall.json 中的 tools 节点

	// 工具内部响应封装
	static QJsonObject makeError(const QString& error);
};


