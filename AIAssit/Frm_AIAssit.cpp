#include "Frm_AIAssit.h" 

Frm_AIAssit::Frm_AIAssit(QWidget *parent)
	: QWidget(parent) {
	ui.setupUi(this);
	initParams();
	setupSignals();
	initUI();
}

Frm_AIAssit::~Frm_AIAssit() {
	saveChatMapToJson(ChatJsonFile, m_ChatMsg);
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	chatFrame->clear();
	ui.ChatListWidget->clearConversations();
}

//设置信号和槽连接 
void Frm_AIAssit::setupSignals() 
{
	connect(this, &Frm_AIAssit::Answer, this, &Frm_AIAssit::getAnswerShow);
	connect(this, &Frm_AIAssit::AnswerStream, this, &Frm_AIAssit::getStreamAnswerShow);
	connect(this, &Frm_AIAssit::PushBtnChanged, ui.ChatInput, &ChatInputWidget::SetButtonEnable);
	connect(this, &Frm_AIAssit::ChangeCurModel, ui.ChatInput, &ChatInputWidget::setModelCurrIndex);
	connect(ui.ChatInput, &ChatInputWidget::MessageUp, this, &Frm_AIAssit::on_pushButton_clicked);
	connect(ui.ChatInput, &ChatInputWidget::ModelSelect, this, &Frm_AIAssit::ChangeModel);
	connect(ui.ChatShow, &ChatShowWidget::paramSetButtonClicked, this, &Frm_AIAssit::ShowAIParam);
	connect(ui.ChatShow, &ChatShowWidget::toggleButtonClicked, this, &Frm_AIAssit::toggleSidebar);	
	connect(ui.AIParams,&AIParamWidget::paramsChanged,this,&Frm_AIAssit::ApplyModelParam);
	connect(ui.ChatListWidget, &ChatList::newConversationRequested,this, &Frm_AIAssit::createNewConversation,Qt::UniqueConnection);
	connect(ui.ChatListWidget, &ChatList::conversationChanged,this, &Frm_AIAssit::onConversationSelected);
	connect(ui.ChatListWidget, &ChatList::renameRequested,this, &Frm_AIAssit::renameCurrentConversation);
	connect(ui.ChatListWidget, &ChatList::deleteRequested,this, &Frm_AIAssit::deleteCurrentConversation);
	connect(LLMClient, &MessageManager::Answer, this, &Frm_AIAssit::getAnswerShow);
	connect(LLMClient, &MessageManager::AnswerStream, this, &Frm_AIAssit::getStreamAnswerShow);
	connect(LLMClient, &MessageManager::ChangeButtonStatus, ui.ChatInput, &ChatInputWidget::SetButtonEnable);
	connect(LLMClient, &MessageManager::FunctionCallSignal, this, &Frm_AIAssit::preFuncall);
	connect(LLMClient, &MessageManager::StreamEnded,this,&Frm_AIAssit::getStreamAnswerEnd);
	connect(LLMClient, &MessageManager::modelsListFetched,ui.ChatInput, &ChatInputWidget::UpdateModelList);
	
}

//显示或隐藏参数设置界面 
void Frm_AIAssit::ShowAIParam() 
{
	ui.AIParams->setVisible(bShowParam);
	bShowParam = !bShowParam;
	QTimer::singleShot(50, this, &Frm_AIAssit::recalculateAllChatBubbles);
}

//加载历史文件
void Frm_AIAssit::initHistoryFile()
{
	QString dirPath = QCoreApplication::applicationDirPath() + "/AIAssit/LLMChatHistory";
	QDir dir(dirPath);

	if (!dir.exists()) {
		if (!dir.mkpath(".")) 
		{ 
			qDebug() << "目录创建失败：" << dirPath;
			return;
		}
		else 
		{
			qDebug() << "目录已创建：" << dirPath;
		}
	}

	ChatJsonFile = dirPath + "/LLMChat_" + QDate::currentDate().toString("yyyy-MM-dd") + ".json";
	QFileInfo fileInfo(ChatJsonFile);

	if (!fileInfo.exists()) 
	{
		QFile file(ChatJsonFile);

		if (file.open(QIODevice::WriteOnly))
		{
			file.close();
			qDebug() << "文件不存在，已创建：" << ChatJsonFile;
		}
		else 
		{
			qDebug() << "文件创建失败：" << file.errorString();
		}
	}
	else 
	{
		qDebug() << "文件已存在：" << ChatJsonFile;
	}
}

//初始化函数 
void Frm_AIAssit::initUI()
{
	this->setWindowTitle(QStringLiteral("GKG AI助手"));
	setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
	ChangeCurModel(ui.AIParams->GetAIParamModel());
	ShowAIParam();
	bool loadJson = loadChatMapFromJson();
	if (!loadJson)
		createNewConversation();
	else
		PreLoadChat();
	ui.ChatShow->updateEmptyState();

	connect(LLMClient, &Open_WebUIClient::serverConnectionCheckFinished,
		this, [this](bool isConnected, const QString &errorMessage) {
		if (isConnected)
		{
			LLMClient->fetchModelsAsync(5000);
		}
		else 
		{
			QMessageBox::warning(this, QStringLiteral("错误"), errorMessage);
		}
	});
	LLMClient->checkServerConnectionAsync(5000);
	//LLMClient->getKnowledgeBase();
}

void Frm_AIAssit::initParams()
{
	initHistoryFile();
	m_FunctionTools = LLMFunctionCall::Get()->Tools();
	params = new LLMParams();
	params->setFunctionCallTools(m_FunctionTools);
	ui.AIParams->setLLMParams(params);
	QString jsonFileName = QCoreApplication::applicationDirPath() + "/AIAssit/AIModelConfig.json";
	bool bLoadRight = ui.AIParams->loadParamsFromJson(jsonFileName);
	if (!bLoadRight) 
	{
		qDebug() << "Failed to load model parameters from JSON file.";
	}
	AIProvider platform = static_cast<AIProvider>(params->getLLMPlatForm());
	setLLMClient(platform);
}

void Frm_AIAssit::addChatBubble(const QString& text, bool bIsUser)
{
	LLMChatFrame* userBubble = new LLMChatFrame();
	//再生成信号连接
	connect(userBubble, &LLMChatFrame::regenerateClicked, this, &Frm_AIAssit::AskQuestionAgain);
	//文本以及大小设置
	QSize textSize = userBubble->fontRect(text);
	userBubble->setText(text, QString::number(QDateTime::currentDateTime().toTime_t()), textSize,
		bIsUser ? LLMChatFrame::User_Customer : LLMChatFrame::User_Owner);
	//每个bubble都有唯一ID
	QString bubbleID= QUuid::createUuid().toString();
	userBubble->setBubbleID(bubbleID);
	//将最新项添加至当前对话下并滚动至最新项
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	QListWidgetItem* item = new QListWidgetItem(chatFrame);
	item->setSizeHint(textSize);
	chatFrame->addItem(item);
	chatFrame->setItemWidget(item, userBubble);
	chatFrame->scrollToBottom();

	// 添加消息后更新空状态
	ui.ChatShow->updateEmptyState();
	//用户bubble需要在此处将bubble信息存入到m_ChatMSg
	if (!bIsUser)
	{
		sSingleMsg singleMsg(text, QString::number(QDateTime::currentDateTime().toTime_t()), textSize, LLMChatFrame::User_Owner, QStringLiteral("新对话"), "", userBubble->BubbleID());
		m_ChatMsg[m_currentConversationId].sMsg.append(singleMsg);
	}
}

//发送消息按钮点击事件处理 
void Frm_AIAssit::on_pushButton_clicked(ChatSendMessage msg)
{
	addChatBubble(msg.SendText, false);
	if (ui.AIParams->GetAIParambStream())
		StreamSend(msg);
	else
		send(msg);
}

void Frm_AIAssit::ApplyModelParam() 
{
	LLMClient->buildRequest();
	LLMClient->fetchModelsAsync();
}

QJsonObject Frm_AIAssit::parseJsonReplyToMsg(const QByteArray &data, bool isStream)
{
	QJsonDocument response_doc = QJsonDocument::fromJson(data);
	// 解析获取到的Json 获取完整json对象
	QJsonObject rsp_json = response_doc.object();
	// msg对象
	QJsonObject msg;

	if (!isStream)
		msg = rsp_json.value("choices").toArray()[0].toObject().value("message").toObject();
	else
		msg = rsp_json.value("choices").toArray()[0].toObject().value("delta").toObject();

	return msg;
}

void Frm_AIAssit::ProcessFunctionCall(QJsonObject FunctionMsg)
{
	QJsonArray toolCalls = FunctionMsg["tool_calls"].toArray();
	QJsonArray toolmsg;
	QJsonObject assistantMsg;
	assistantMsg["role"] = "assistant";
	assistantMsg.insert("content", "");
	assistantMsg["tool_calls"] = toolCalls;
	toolmsg.append(assistantMsg);
	QJsonObject toolMsg;
	QJsonObject tool_call = FunctionMsg["tool_calls"].toArray()[0].toObject();
	toolMsg["tool_call_id"] = tool_call["id"];
	toolMsg["role"] = "tool";
	toolMsg["name"] = tool_call["function"].toObject()["name"].toString();
	toolMsg.insert("content", FunctionMsg["content"]);
	toolmsg.append(toolMsg);

	QJsonObject requestBody;
	requestBody.insert("model", "gkg");
	requestBody.insert("messages", toolmsg);
	requestBody["tools"] = m_FunctionTools;
	QJsonDocument doc(requestBody);
	QByteArray data = doc.toJson();
	QSslConfiguration config = QSslConfiguration::defaultConfiguration();
	config.setProtocol(QSsl::AnyProtocol);
	config.setPeerVerifyMode(QSslSocket::VerifyNone);
	LLMClient->m_NetWorkParams->clientRequest.setSslConfiguration(config);
	QNetworkReply *reply = LLMClient->m_NetWorkParams->clientNetWorkManager->post(LLMClient->m_NetWorkParams->clientRequest, data);;
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		if (reply->error()) 
		{
			qDebug() << "Error:" << reply->errorString();
		}
		else {
			QByteArray response_data = reply->readAll();
			if (response_data.isEmpty()) 
			{
				return;
			}
			QJsonDocument response_doc = QJsonDocument::fromJson(response_data);
			QJsonObject msg = parseJsonReplyToMsg(response_data);
			QString content = msg.value("content").toString();
            emit Answer(content, false);		
		}
		reply->deleteLater();
	});
}

//显示服务器返回的答案 
void Frm_AIAssit::getAnswerShow(const QString& word,bool bError) 
{
	
	QRegularExpression regexResult(R"(</think>(.*))", QRegularExpression::DotMatchesEverythingOption);
	QRegularExpression regexTinking(R"(<think>(.*)</think>)", QRegularExpression::DotMatchesEverythingOption);
	QRegularExpressionMatch match = regexResult.match(word);
	QRegularExpressionMatch matchThinking = regexTinking.match(word);
	QString TextAnswer;
	QString TextReasoning;
	QString tempWord;
	if (bError)
	{
		// 错误情况处理
		TextAnswer = QStringLiteral("\n ### 回答 \n") + word;
		TextReasoning = QStringLiteral("\n ### 推理 \n") + "error";
		tempWord = QStringLiteral("\n ### 推理 \n") + "error" + QStringLiteral("\n ### 回答 \n") + word;
	}
	else 
	{
		// 检查是否包含推理标识符
		if (matchThinking.hasMatch()) 
		{
			// 包含 <think></think> 标识符的情况
			TextAnswer = QStringLiteral("\n ### 回答 \n\n") + match.captured(1).trimmed();
			TextReasoning = QStringLiteral("\n ### 推理 \n") +(matchThinking.captured(1).trimmed().isEmpty() ?
					QStringLiteral("推理未开启") : matchThinking.captured(1).trimmed());
			// 替换标识符
			tempWord = word;
			tempWord.replace(QRegularExpression("<think>"), QStringLiteral("\n ### 推理 \n"));
			tempWord.replace(QRegularExpression("</think>"), QStringLiteral("\n ### 回答 \n"));
		}
		else 
		{
			// 不包含推理标识符的情况 - 将整个内容作为回答
			TextAnswer = QStringLiteral("\n ### 回答 \n\n") + word.trimmed();
			TextReasoning = QStringLiteral("\n ### 推理 \n") + QStringLiteral("本模型不支持推理");
			// 构造tempWord保持相同的结构
			tempWord = QStringLiteral("\n ### 推理 \n") + QStringLiteral("本模型不支持推理") +
				QStringLiteral("\n ### 回答 \n") + word.trimmed();
		}
	}

	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	QListWidgetItem* latestItem = chatFrame->item(chatFrame->count() - 1);
	LLMChatFrame *latestWidget = (LLMChatFrame*)chatFrame->itemWidget(latestItem);
	QSize textSize = latestWidget->fontRect(TextReasoning, TextAnswer);
	latestItem->setSizeHint(textSize);
	latestWidget->setTextSuccess();
	QString DialogName = updateDialogName(tempWord);
	sSingleMsg singleMsg(TextAnswer, QString::number(QDateTime::currentDateTime().toTime_t()), textSize, LLMChatFrame::User_Customer, DialogName, TextReasoning, latestWidget->BubbleID());
	m_ChatMsg[m_currentConversationId].sMsg.append(singleMsg);
}

//显示服务器流式数据
void Frm_AIAssit::getStreamAnswerShow(const QString& word) 
{
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	QListWidgetItem* latestItem = chatFrame->item(chatFrame->count() - 1);
	LLMChatFrame *latestWidget = (LLMChatFrame*)chatFrame->itemWidget(latestItem);
	if (word.contains("</think>"))
	{
		latestWidget->ChangeAccpetStatus();
	}
	latestWidget->appendText(word); 
	QSize textSize = latestWidget->fontRect(latestWidget->reasonRawText(),latestWidget->RawText()); //重新计算整段文字的尺寸
	latestItem->setSizeHint(textSize);
	chatFrame->scrollToBottom();
}

//发送消息到服务器 
int Frm_AIAssit::send(const ChatSendMessage& msg)
{
	addChatBubble(" ", true);
	int rtn=LLMClient->send(msg);
	return rtn;
}

//发送消息到流式服务器 
int Frm_AIAssit::StreamSend(const ChatSendMessage& msg)
{
	addChatBubble(" ", true);
	int rtn=LLMClient->StreamSend(msg);
	return rtn;
}


//服务器流式数据结束时的数据处理
void Frm_AIAssit::getStreamAnswerEnd()
{
	PushBtnChanged(true);//发送按钮状态更新
	//获取最新对应项
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	QListWidgetItem* latestItem = chatFrame->item(chatFrame->count() - 1);
	LLMChatFrame *latestWidget = (LLMChatFrame*)chatFrame->itemWidget(latestItem);
	//改变相应状态
	latestWidget->setTextSuccess();
	latestWidget->ChangeStream();
	
	QString DialogName=updateDialogName(latestWidget->RawText());
	sSingleMsg singleMsg(latestWidget->RawText(), QString::number(QDateTime::currentDateTime().toTime_t()), latestWidget->size(),
		LLMChatFrame::User_Customer, DialogName, latestWidget->reasonRawText(),latestWidget->BubbleID());
	m_ChatMsg[m_currentConversationId].sMsg.append(singleMsg);
}

void Frm_AIAssit::createNewConversation() {
	// 生成唯一对话ID 
	QString convId = QUuid::createUuid().toString();
	sMsgList newMsg;
	newMsg.SaveTime = QDateTime::currentDateTime();
	m_ChatMsg.insert(convId, newMsg);

	// 使用 ChatList 的方法添加新对话
	ui.ChatListWidget->insertConversationItem(0, QStringLiteral("新对话"), convId);
	ui.ChatListWidget->setCurrentConversation(convId);

	m_currentConversationId = convId;
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	chatFrame->clear(); // 清空聊天列表
	chatFrame->scrollToBottom();

	// 新建对话后更新空状态（此时应该显示空状态）
	ui.ChatShow->updateEmptyState();
}

//对话切换
void Frm_AIAssit::onConversationSelected(QListWidgetItem* current, QListWidgetItem* previous) {
	if (!current) return;

	// 清空旧对话
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	chatFrame->clear();

	// 获取新对话ID
	m_currentConversationId = current->data(Qt::UserRole).toString();
	sMsgList ml = m_ChatMsg[m_currentConversationId];

	// 批量处理，避免频繁更新
	chatFrame->setUpdatesEnabled(false); // 禁用更新

	for (auto& frame : ml.sMsg) 
	{
		// bubble设置
		LLMChatFrame* userBubble = new LLMChatFrame();
		connect(userBubble, &LLMChatFrame::regenerateClicked, this, &Frm_AIAssit::AskQuestionAgain);

		LLMChatFrame::User_Type Auth = frame.userType == 2 ? LLMChatFrame::User_Customer : LLMChatFrame::User_Owner;
		userBubble->setUserType(Auth);

		// 直接使用保存的大小，避免重复计算
		QSize textSize = frame.m_AllSize;

		// 只在大小无效时才重新计算
		if (!textSize.isValid() || textSize.isEmpty())
		{
			textSize = userBubble->fontRect(frame.m_ChatReasonMsg, frame.m_ChatMsg);
			frame.m_AllSize = textSize; // 缓存计算结果
		}

		userBubble->setTextWithReason(frame.m_ChatReasonMsg, frame.m_ChatMsg,
			frame.m_ChatTime, textSize, Auth);
		userBubble->setTextSuccess();
		userBubble->setBubbleID(frame.m_BubbleID);

		// 将bubble添加进新对话
		QListWidgetItem* item = new QListWidgetItem(chatFrame);
		item->setSizeHint(textSize);
		chatFrame->addItem(item);
		chatFrame->setItemWidget(item, userBubble);
	}

	// 重新启用更新
	chatFrame->setUpdatesEnabled(true);
	chatFrame->scrollToBottom();

	// 更新空状态
	ui.ChatShow->updateEmptyState();

	// 延迟调用recalculate，避免阻塞UI
	// 使用更长的延迟，并且只在真正需要时调用
	if (ml.sMsg.size() > 0)
	{
		QTimer::singleShot(100, this, [this]() {
			recalculateVisibleBubbles(); // 只计算可见的气泡
		});
	}
}

//对话->JsonFile
void Frm_AIAssit::saveChatMapToJson(const QString& filePath, const QHash<QString, sMsgList>& chatMap) {
	QJsonObject root;
	for (auto it = chatMap.constBegin(); it != chatMap.constEnd(); ++it) 
	{
		root[it.key()] = it.value().toJson();
	}

	QJsonDocument doc(root);
	QFile file(filePath);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) 
	{
		file.write(doc.toJson(QJsonDocument::Indented));
		file.close();
	}
}

//JsonFile->对话
bool Frm_AIAssit::loadChatMapFromJson()
{
	QFile file(ChatJsonFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
	{
		return false ;
	}

	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
	file.close();
	if (error.error != QJsonParseError::NoError || !doc.isObject())
	{
		return false ;
	}
	
	QJsonObject root = doc.object();
	if (root.isEmpty()) 
	{
		return false;
	}
	for (const QString& key : root.keys()) 
	{
		sMsgList msgList;
		msgList.fromJson(root[key].toObject());
		m_ChatMsg.insert(key, msgList);
	}

	if (m_ChatMsg.isEmpty()) 
	{
		return false;
	}
	return true;
}

void Frm_AIAssit::PreLoadChat()
{
	// 按时间倒序排列对话
	QList<QPair<QString, sMsgList>> sortedChats;
	for (auto it = m_ChatMsg.constBegin(); it != m_ChatMsg.constEnd(); ++it) 
	{
		sortedChats.append(qMakePair(it.key(), it.value()));
	}

	// 按保存时间排序
	std::sort(sortedChats.begin(), sortedChats.end(),
		[](const QPair<QString, sMsgList>& a, const QPair<QString, sMsgList>& b) {
		return a.second.SaveTime > b.second.SaveTime;
	});

	// 添加到界面
	for (const auto& chat : sortedChats)
	{
		QString displayName;
		if (!chat.second.sMsg.isEmpty()) 
		{
			displayName = chat.second.sMsg.last().m_DialogName;
			if (displayName == QStringLiteral("新对话") && chat.second.sMsg.size() > 1) 
			{
				// 尝试从最后一条消息生成名称
				QString lastMsg = chat.second.sMsg.last().m_ChatMsg;
				if (lastMsg.length() > 12) 
				{
					displayName = lastMsg.left(12) + "...";
				}
				else
				{
					displayName = lastMsg;
				}
			}
		}
		else 
		{
			displayName = QStringLiteral("新对话");
		}
		ui.ChatListWidget->addConversationItem(displayName, chat.first);
	}

	// 选中第一个对话
	if (ui.ChatListWidget->count() > 0) 
	{
		ui.ChatListWidget->setCurrentRow(0);
	}
	QTimer::singleShot(50, this, &Frm_AIAssit::recalculateAllChatBubbles);
}

QString Frm_AIAssit::updateDialogName(const QString& dialogName)
{
	QString target = QStringLiteral("\n ### 回答 \n");
	int index = dialogName.indexOf(target);
	QString tempName;

	if (index != -1) 
	{
		tempName = dialogName.mid(index + target.length() + 1, 15); 
		tempName = tempName.trimmed();
		if (tempName.length() > 12) 
		{
			tempName = tempName.left(12) + "...";
		}
	}
	else 
	{
		tempName = QStringLiteral("新对话");
	}
	ui.ChatListWidget->setCurrentItemText(tempName);
	return tempName;
}

//to be continue:移动到ChatList中
void Frm_AIAssit::deleteCurrentConversation()
{
	QListWidgetItem* currentItem = ui.ChatListWidget->getConversationList()->currentItem();
	if (!currentItem) return;

	QString convId = currentItem->data(Qt::UserRole).toString();

	// 确认删除
	QMessageBox::StandardButton reply = QMessageBox::question(this,
		QStringLiteral("确认删除"),
		QStringLiteral("确定要删除这个对话吗？"),
		QMessageBox::Yes | QMessageBox::No);

	if (reply == QMessageBox::Yes) 
	{
		// 从数据中移除
		m_ChatMsg.remove(convId);

		// 从界面移除
		int row = ui.ChatListWidget->getConversationList()->row(currentItem);
		ui.ChatListWidget->getConversationList()->takeItem(row);
		delete currentItem;

		// 如果是当前对话，清空聊天区域
		if (convId == m_currentConversationId) 
		{
			QListWidget* chatFrame = ui.ChatShow->getChatFrame();
			chatFrame->clear();

			if (ui.ChatListWidget->getConversationList()->count() > 0) 
			{
				ui.ChatListWidget->getConversationList()->setCurrentRow(0);
			}
			else 
			{
				createNewConversation();
			}

			// 删除对话后更新空状态
			ui.ChatShow->updateEmptyState();
		}
	}
}

//to be continue:移动到ChatList中
void Frm_AIAssit::renameCurrentConversation()
{
	QListWidgetItem* currentItem = ui.ChatListWidget->getConversationList()->currentItem();
	if (!currentItem) return;

	bool bReNameConver;
	QString currentText = currentItem->text();
	QString cnverNewName = QInputDialog::getText(this,
		QStringLiteral("重命名对话"),
		QStringLiteral("请输入新的对话名称:"),
		QLineEdit::Normal, currentText, &bReNameConver);

	if (bReNameConver && !cnverNewName.isEmpty()&& m_ChatMsg[m_currentConversationId].sMsg.size()>1) 
	{
		currentItem->setText(cnverNewName);
		m_ChatMsg[m_currentConversationId].sMsg.last().m_DialogName= cnverNewName;
	}
}

void Frm_AIAssit::toggleSidebar()
{
	bool isVisible = ui.ChatListWidget->isVisible();

	if (isVisible) 
	{
		// 收起侧边栏
		ui.ChatListWidget->setVisible(false);
		ui.ChatShow->setToggleIcon(QIcon(":/QtWidgetsApp/ICONs/icon_close.png"));
	}
	else 
	{
		// 展开侧边栏
		ui.ChatListWidget->setVisible(true);
		ui.ChatShow->setToggleIcon(QIcon(":/QtWidgetsApp/ICONs/icon_open.png"));
	}

	// 强制更新布局
	ui.mainHorizontalLayout->update();
	this->update();
	QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
	ui.ChatListWidget->setGraphicsEffect(effect);

	QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
	animation->setDuration(300);
	animation->setStartValue(isVisible ? 1.0 : 0.0);
	animation->setEndValue(isVisible ? 0.0 : 1.0);
	animation->start(QAbstractAnimation::DeleteWhenStopped);
	QTimer::singleShot(50, this, &Frm_AIAssit::recalculateAllChatBubbles);
}

QString Frm_AIAssit::GetLastestAsk(QString msg)
{
	sMsgList ml = m_ChatMsg[m_currentConversationId];
	int index= ml.GetMsgIndex(msg);
	if (index >=0)
	{
		return ml.sMsg[index - 1].m_ChatMsg;
	}
	return QString();
}

void Frm_AIAssit::AskQuestionAgain(QString msg)
{
	QString SendMsg = GetLastestAsk(msg);
	ChatSendMessage askMsg;
	askMsg.SendText = SendMsg;
	if (SendMsg.size() > 0)
		on_pushButton_clicked(askMsg);
}

void Frm_AIAssit::ChangeModel(int iModel)
{
	ui.AIParams->SetAIParamModel(iModel);
}

void Frm_AIAssit::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	// 使用成员变量存储定时器
	if (!m_resizeTimer) {
		m_resizeTimer = new QTimer(this);
		m_resizeTimer->setSingleShot(true);
		connect(m_resizeTimer, &QTimer::timeout, this, [this]() {
			recalculateVisibleBubbles(); // 只计算可见部分
		});
	}

	// 防抖动：如果连续resize，重置定时器
	m_resizeTimer->stop();
	m_resizeTimer->start(150); // 150ms 延迟
}
void Frm_AIAssit::recalculateVisibleBubbles()
{
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	if (!chatFrame) return;

	// 获取可见区域
	QRect viewport = chatFrame->viewport()->rect();

	for (int i = 0; i < chatFrame->count(); ++i) 
	{
		QListWidgetItem* item = chatFrame->item(i);
		QRect itemRect = chatFrame->visualItemRect(item);

		// 只处理可见的项
		if (!viewport.intersects(itemRect)) 
		{
			continue;
		}

		LLMChatFrame* bubbleWidget = qobject_cast<LLMChatFrame*>(chatFrame->itemWidget(item));
		if (bubbleWidget && bubbleWidget->isSending()) 
		{
			int availableWidth = chatFrame->viewport()->width();
			bubbleWidget->resize(availableWidth, bubbleWidget->height());

			// 使用缓存的大小，避免重复计算
			QString convId = m_currentConversationId;
			QString bubbleId = bubbleWidget->BubbleID();

			// 在消息列表中查找对应的消息
			for (auto& msg : m_ChatMsg[convId].sMsg)
			{
				if (msg.m_BubbleID == bubbleId && msg.m_AllSize.isValid())
				{
					item->setSizeHint(msg.m_AllSize);
					break;
				}
			}
		}
	}
	chatFrame->update();
}
void Frm_AIAssit::recalculateAllChatBubbles() 
{
	QListWidget* chatFrame = ui.ChatShow->getChatFrame();
	if (!chatFrame || chatFrame->count() == 0) return;

	// 批量处理
	chatFrame->setUpdatesEnabled(false);

	int availableWidth = chatFrame->viewport()->width();

	for (int i = 0; i < chatFrame->count(); ++i)
	{
		QListWidgetItem* item = chatFrame->item(i);
		LLMChatFrame* bubbleWidget = qobject_cast<LLMChatFrame*>(chatFrame->itemWidget(item));

		if (!bubbleWidget) continue;

		bubbleWidget->resize(availableWidth, bubbleWidget->height());

		// 优先使用缓存的大小
		QString bubbleId = bubbleWidget->BubbleID();
		bool sizeFound = false;

		// 查找缓存的大小
		for (const auto& msg : m_ChatMsg[m_currentConversationId].sMsg)
		{
			if (msg.m_BubbleID == bubbleId && msg.m_AllSize.isValid())
			{
				item->setSizeHint(msg.m_AllSize);
				sizeFound = true;
				break;
			}
		}

		// 只有在没有缓存时才重新计算
		if (!sizeFound && bubbleWidget->isSending())
		{
			QSize newSize;
			if (bubbleWidget->userType() == LLMChatFrame::User_Customer &&
				!bubbleWidget->reasoningText().isEmpty()) 
			{
				newSize = bubbleWidget->fontRect(bubbleWidget->reasoningText(),
					bubbleWidget->text());
			}
			else
			{
				newSize = bubbleWidget->fontRect(bubbleWidget->text());
			}
			item->setSizeHint(newSize);
		}
	}
	chatFrame->setUpdatesEnabled(true);
	chatFrame->update();
}


void Frm_AIAssit::preFuncall(QJsonObject& Content)
{
	// 处理函数调用
	QJsonArray toolCalls = Content["tool_calls"].toArray();
	if (!toolCalls.isEmpty())
	{
		QJsonObject functionCall = toolCalls[0].toObject()["function"].toObject();
		QString name = functionCall["name"].toString();
		QJsonObject arguments = QJsonDocument::fromJson(functionCall["arguments"].toString().toUtf8()).object();
		QJsonObject result = LLMFunctionCall::Get()->executeFunction(name, arguments);
		Content["content"] = QString(QJsonDocument(result).toJson(QJsonDocument::Indented));
		ProcessFunctionCall(Content);
	}
}

void Frm_AIAssit::setLLMClient(AIProvider platform)
{
	if (platform == AIProvider::Dify)
	{
		LLMClient = new DifyClient();
	}
	else
	{
		LLMClient = new Open_WebUIClient();
	}		
	LLMClient->setLLMParams(params);
	LLMClient->m_NetWorkParams->clientNetWorkManager = std::make_unique<QNetworkAccessManager>();
	LLMClient->buildRequest();
}