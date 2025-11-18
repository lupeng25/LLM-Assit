#include "ChatList.h"
#include <QPropertyAnimation>
#include <QTimer>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QStyleOptionViewItem>
#include <QFileDialog>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextDocument>

namespace
{
    class ChatListDelegate : public QStyledItemDelegate
    {
    public:
        explicit ChatListDelegate(QObject* parent = nullptr)
            : QStyledItemDelegate(parent)
        {
        }

        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
        {
            Q_UNUSED(index);
            const int baseHeight = 60;
            return QSize(option.rect.width(), baseHeight);
        }

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
        {
            if (!painter)
            {
                return;
            }

            painter->save();

            QStyleOptionViewItem opt(option);
            initStyleOption(&opt, index);

            const QRectF cardRect = opt.rect.adjusted(6, 4, -6, -4);
            const bool isSelected = opt.state & QStyle::State_Selected;
            const bool isHovered = opt.state & QStyle::State_MouseOver;

            QColor baseColor(248, 250, 252);
            QColor borderColor(226, 232, 240, 180);
            QColor shadowColor(15, 23, 42, 24);

            if (isSelected)
            {
                baseColor = QColor(37, 99, 235);
                borderColor = QColor(30, 64, 175, 220);
                shadowColor = QColor(37, 99, 235, 55);
            }
            else if (isHovered)
            {
                baseColor = QColor(241, 245, 249);
                borderColor = QColor(191, 219, 254, 200);
                shadowColor = QColor(59, 130, 246, 40);
            }

            // 缁樺埗闃村奖
            if (shadowColor.alpha() > 0)
            {
                QPainterPath shadowPath;
                QRectF shadowRect = cardRect.adjusted(0, 2, 0, 8);
                shadowPath.addRoundedRect(shadowRect, 18, 18);
                painter->setPen(Qt::NoPen);
                painter->setBrush(shadowColor);
                painter->drawPath(shadowPath);
            }

            QPainterPath cardPath;
            cardPath.addRoundedRect(cardRect, 18, 18);
            painter->setPen(Qt::NoPen);
            painter->setBrush(baseColor);
            painter->drawPath(cardPath);

            painter->setPen(QPen(borderColor, 1));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(cardPath);

            const QString title = index.data(Qt::DisplayRole).toString();
            const QString timestamp = index.data(ChatList::TimestampRole).toString();
            const bool hasTimestamp = !timestamp.trimmed().isEmpty();

            const QColor titleColor = isSelected ? QColor(248, 250, 252)
                : QColor(15, 23, 42);
            const QColor timeColor = isSelected ? QColor(226, 232, 240, 240)
                : QColor(100, 116, 139);

            QRectF textRect = cardRect.adjusted(18, 14, -18, -14);
            const int timeWidth = hasTimestamp ? opt.fontMetrics.horizontalAdvance(timestamp) + 6 : 0;
            const QRectF timeRect = hasTimestamp
                ? QRectF(textRect.right() - timeWidth, textRect.top(), timeWidth, 20)
                : QRectF();

            QFont titleFont = opt.font;
            titleFont.setPointSizeF(titleFont.pointSizeF() + 0.4);
            titleFont.setWeight(QFont::DemiBold);

            if (hasTimestamp)
            {
                painter->setPen(timeColor);
                painter->setFont(opt.font);
                painter->drawText(timeRect, Qt::AlignRight | Qt::AlignVCenter, timestamp);
            }

            QRectF titleRect = textRect;
            if (hasTimestamp)
            {
                titleRect.setRight(timeRect.left() - 12);
            }

            painter->setPen(titleColor);
            painter->setFont(titleFont);
            painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, title);

            painter->restore();
        }
    };
}

ChatList::ChatList(QWidget *parent)
    : QWidget(parent)
    , mainLayout(nullptr)
    , btnNewConversation(nullptr)
    , searchEdit(nullptr)
    , m_conversationList(nullptr)
    , searchTimer(nullptr)
    , searchCallback(nullptr)
    , footerDivider(nullptr)
    , footerWidget(nullptr)
    , footerLayout(nullptr)
    , btnParamSetting(nullptr)
{
    setupUI();
    connectSignals();
}

ChatList::~ChatList()
{
    m_conversationList->clear();
}

void ChatList::setupUI()
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setMinimumWidth(220);
    setMaximumWidth(320);

    if (QLayout* existingLayout = layout())
    {
        if (auto* existing = qobject_cast<QVBoxLayout*>(existingLayout))
        {
            mainLayout = existing;
        }
        else
        {
            delete existingLayout;
            mainLayout = new QVBoxLayout();
            setLayout(mainLayout);
        }
    }
    else
    {
        mainLayout = new QVBoxLayout();
        setLayout(mainLayout);
    }
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    btnNewConversation = new QPushButton(this);
    btnNewConversation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    btnNewConversation->setMinimumHeight(64);
    btnNewConversation->setText(tr("New Conversation"));
    btnNewConversation->setObjectName("btnNewConversation");
    btnNewConversation->setCursor(Qt::PointingHandCursor);

    m_conversationList = new QListWidget(this);
    m_conversationList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_conversationList->setObjectName("m_conversationList");
    m_conversationList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_conversationList->setMouseTracking(true);
    m_conversationList->setSpacing(14);
    m_conversationList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_conversationList->setFrameShape(QFrame::NoFrame);
    m_conversationList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_conversationList->setItemDelegate(new ChatListDelegate(m_conversationList));

    // 创建搜索框
    QWidget* searchWidget = new QWidget(this);
    QHBoxLayout* searchLayout = new QHBoxLayout(searchWidget);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->setSpacing(8);
    
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText(tr("Search conversations..."));
    searchEdit->setObjectName("searchEdit");
    searchEdit->setStyleSheet(R"(
        QLineEdit {
            background: rgba(255, 255, 255, 0.9);
            border: 1px solid rgba(203, 213, 225, 0.8);
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 13px;
            color: #1e293b;
        }
        QLineEdit:focus {
            border-color: #3b82f6;
            background: rgba(255, 255, 255, 1.0);
        }
        QLineEdit::placeholder {
            color: #94a3b8;
        }
    )");
    
    searchLayout->addWidget(searchEdit);
    searchWidget->setLayout(searchLayout);

    // 创建搜索防抖定时器
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(300);  // 300ms 防抖

    // 添加到布局
    mainLayout->addWidget(btnNewConversation);
    mainLayout->addWidget(searchWidget);
    mainLayout->addWidget(m_conversationList);

    footerDivider = new QFrame(this);
    footerDivider->setObjectName("footerDivider");
    footerDivider->setFixedHeight(1);
    footerDivider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mainLayout->addWidget(footerDivider);

    footerWidget = new QWidget(this);
    footerWidget->setObjectName("footerWidget");
    footerLayout = new QHBoxLayout(footerWidget);
    footerLayout->setContentsMargins(4, 12, 4, 0);
    footerLayout->setSpacing(10);

    btnParamSetting = new QPushButton(tr("Param Setting"), footerWidget);
    btnParamSetting->setObjectName("btnParamSetting");
    btnParamSetting->setCursor(Qt::PointingHandCursor);
    btnParamSetting->setMinimumHeight(44);

    footerLayout->addStretch(1);
    footerLayout->addWidget(btnParamSetting);
    footerLayout->addStretch(1);

    mainLayout->addWidget(footerWidget);

    applyStyles();
}

void ChatList::connectSignals()
{
    connect(btnNewConversation, &QPushButton::clicked,
        this, &ChatList::onNewConversationClicked);

    // 杩炴帴鍒楄〃閫夋嫨鏀瑰彉淇″彿
    connect(m_conversationList, &QListWidget::currentItemChanged,
        this, &ChatList::onConversationSelectionChanged);

    connect(m_conversationList, &QListWidget::customContextMenuRequested,
        this, &ChatList::showContextMenu);
    
    // 连接搜索框信号
    if (searchEdit && searchTimer) {
        connect(searchEdit, &QLineEdit::textChanged,
            this, &ChatList::onSearchTextChanged);
        connect(searchTimer, &QTimer::timeout,
            this, &ChatList::performSearch);
    }

    if (btnParamSetting)
    {
        connect(btnParamSetting, &QPushButton::clicked,
            this, &ChatList::onParamSettingClicked);
    }
}

void ChatList::addConversationItem(const QString& text, const QString& id)
{
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(text);
    item->setData(IdRole, id);
    item->setFlags(item->flags() | Qt::ItemIsSelectable);
    item->setData(SearchMatchRole, true);  // 默认显示
    m_conversationList->addItem(item);
    
    // 保存对话ID
    if (!allConversationIds.contains(id))
    {
        allConversationIds.append(id);
    }
}

void ChatList::insertConversationItem(int index, const QString& text, const QString& id)
{
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(text);
    item->setData(IdRole, id);
    item->setFlags(item->flags() | Qt::ItemIsSelectable);
    item->setData(SearchMatchRole, true);  // 默认显示
    m_conversationList->insertItem(index, item);
    
    // 保存对话ID
    if (!allConversationIds.contains(id))
    {
        allConversationIds.append(id);
    }

    QTimer::singleShot(10, [this, item]() {
        QPropertyAnimation* animation = new QPropertyAnimation();
        animation->setTargetObject(m_conversationList);
        animation->setPropertyName("geometry");
        animation->setDuration(200);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void ChatList::removeConversationItem(const QString& id)
{
    for (int i = 0; i < m_conversationList->count(); ++i)
    {
        QListWidgetItem* item = m_conversationList->item(i);
        if (item && item->data(IdRole).toString() == id)
        {
            delete m_conversationList->takeItem(i);
            break;
        }
    }
}

void ChatList::clearConversations()
{
    m_conversationList->clear();
    allConversationIds.clear();
}

void ChatList::setCurrentConversation(const QString& id)
{
    for (int i = 0; i < m_conversationList->count(); ++i)
    {
        QListWidgetItem* item = m_conversationList->item(i);
        if (item && item->data(IdRole).toString() == id)
        {
            m_conversationList->setCurrentItem(item);
            break;
        }
    }
}

QString ChatList::getCurrentConversationId() const
{
    QListWidgetItem* current = m_conversationList->currentItem();
    if (current)
    {
        return current->data(IdRole).toString();
    }
    return QString();
}

QListWidgetItem* ChatList::getCurrentItem() const
{
    return m_conversationList->currentItem();
}

void ChatList::setCurrentItemText(const QString& text)
{
    QListWidgetItem* current = m_conversationList->currentItem();
    if (current)
    {
        current->setText(text);
    }
}

void ChatList::onNewConversationClicked()
{
    emit newConversationRequested();
}

void ChatList::onConversationSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    emit conversationChanged(current, previous);

    if (current)
    {
        QString conversationId = current->data(IdRole).toString();
        emit conversationSelected(conversationId);
    }
}

void ChatList::showContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = m_conversationList->itemAt(pos);
    if (!item) return;

    const QString conversationId = item->data(IdRole).toString();

    QMenu contextMenu(this);
    contextMenu.setStyleSheet(R"(
        QMenu {
            background: rgba(255, 255, 255, 0.98);
            border: 1px solid rgba(203, 213, 225, 0.8);
            border-radius: 8px;
            padding: 6px;
            font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;
            font-size: 14px;
        }
        QMenu::item {
            padding: 10px 20px;
            border-radius: 6px;
            color: #1e293b;
            min-width: 160px;
        }
        QMenu::item:selected {
            background: rgba(59, 130, 246, 0.15);
            color: #3b82f6;
        }
        QMenu::item:disabled {
            color: #94a3b8;
        }
        QMenu::separator {
            height: 1px;
            background: rgba(226, 232, 240, 0.8);
            margin: 4px 8px;
        }
    )");

    QAction* renameAction = contextMenu.addAction(tr("Rename"));
    contextMenu.addSeparator();

    QMenu* exportMenu = contextMenu.addMenu(tr("Export Conversation"));
    QAction* exportMarkdownAction = exportMenu->addAction(tr("Export As Markdown..."));
    QAction* exportHtmlAction = exportMenu->addAction(tr("Export As HTML..."));
    QAction* exportTextAction = exportMenu->addAction(tr("Export As Text..."));

    contextMenu.addSeparator();
    QAction* showDetailsAction = contextMenu.addAction(tr("Show Details"));
    contextMenu.addSeparator();
    QAction* deleteAction = contextMenu.addAction(tr("Delete Chat"));

    QAction* selectedAction = contextMenu.exec(m_conversationList->mapToGlobal(pos));
    if (!selectedAction)
    {
        return;
    }

    if (selectedAction == renameAction)
    {
        emit renameRequested();
    }
    else if (selectedAction == exportMarkdownAction)
    {
        emit exportConversationRequested(conversationId, QStringLiteral("markdown"));
    }
    else if (selectedAction == exportHtmlAction)
    {
        emit exportConversationRequested(conversationId, QStringLiteral("html"));
    }
    else if (selectedAction == exportTextAction)
    {
        emit exportConversationRequested(conversationId, QStringLiteral("text"));
    }
    else if (selectedAction == showDetailsAction)
    {
        emit showDetailsRequested(conversationId);
    }
    else if (selectedAction == deleteAction)
    {
        emit deleteRequested();
    }

    emit contextMenuRequested(pos);
}

QListWidgetItem* ChatList::findItemById(const QString& id) const
{
    for (int i = 0; i < m_conversationList->count(); ++i)
    {
        QListWidgetItem* item = m_conversationList->item(i);
        if (item && item->data(IdRole).toString() == id)
        {
            return item;
        }
    }
    return nullptr;
}

void ChatList::setSearchCallback(std::function<QString(const QString&)> callback)
{
    searchCallback = callback;
}

void ChatList::onSearchTextChanged(const QString& text)
{
    // 重置定时器，实现防抖
    searchTimer->stop();
    if (!text.isEmpty())
    {
        searchTimer->start();
    }
    else
    {
        // 如果搜索框为空，显示所有对话
        performSearch();
    }
}

void ChatList::performSearch()
{
    QString searchText = searchEdit->text().trimmed();
    
    if (searchText.isEmpty())
    {
        // 显示所有对话
        for (int i = 0; i < m_conversationList->count(); ++i)
        {
            QListWidgetItem* item = m_conversationList->item(i);
            if (item)
            {
                setItemVisible(item, true);
            }
        }
        return;
    }
    
    // 执行搜索
    searchText = searchText.toLower();
    int matchCount = 0;
    
    for (int i = 0; i < m_conversationList->count(); ++i)
    {
        QListWidgetItem* item = m_conversationList->item(i);
        if (!item)
        {
            continue;
        }
        
        QString conversationId = item->data(IdRole).toString();
        QString title = item->text().toLower();
        
        // 搜索标题
        bool titleMatch = title.contains(searchText);
        
        // 搜索对话内容
        bool contentMatch = false;
        if (searchCallback && !titleMatch)
        {
            contentMatch = searchInConversation(conversationId, searchText);
        }
        
        bool match = titleMatch || contentMatch;
        setItemVisible(item, match);
        
        if (match)
        {
            matchCount++;
        }
    }
}

bool ChatList::searchInConversation(const QString& conversationId, const QString& searchText) const
{
    if (!searchCallback)
    {
        return false;
    }
    
    // 获取对话内容
    QString content = searchCallback(conversationId);
    if (content.isEmpty())
    {
        return false;
    }
    
    // 移除 HTML 标签，只搜索纯文本
    QTextDocument doc;
    doc.setHtml(content);
    QString plainText = doc.toPlainText().toLower();
    
    // 搜索文本
    return plainText.contains(searchText);
}

void ChatList::setItemVisible(QListWidgetItem* item, bool visible)
{
    if (!item)
    {
        return;
    }
    
    item->setData(SearchMatchRole, visible);
    item->setHidden(!visible);
}

void ChatList::onParamSettingClicked()
{
    emit paramSettingRequested();
}

void ChatList::setConversationTimestamp(const QString& id, const QString& timestamp)
{
    if (auto* item = findItemById(id))
    {
        item->setData(TimestampRole, timestamp);
        m_conversationList->update(m_conversationList->visualItemRect(item));
    }
}

void ChatList::applyStyles()
{
    static const QString kStyleSheet = QStringLiteral(R"(ChatList {
    background: rgba(255, 255, 255, 0.82);
    border: 1px solid rgba(148, 163, 184, 0.28);
    border-radius: 14px;
    padding: 10px 6px;
}

ChatList QPushButton#btnNewConversation {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
        stop:0 #2563eb, stop:1 #1d4ed8);
    color: #f8fafc;
    border: none;
    border-radius: 12px;
    font-size: 15px;
    padding: 14px 20px;
    margin: 12px 10px 10px 10px;
    min-height: 52px;
    text-align: left;
}

ChatList QPushButton#btnNewConversation:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
        stop:0 #3b82f6, stop:1 #2563eb);
}

ChatList QPushButton#btnNewConversation:pressed {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
        stop:0 #1d4ed8, stop:1 #1e40af);
}

QListWidget#m_conversationList {
    border: none;
    background: transparent;
    outline: 0;
    padding: 6px 4px 12px 4px;
    font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;
}

QListWidget#m_conversationList::item {
    padding: 14px 18px;
    margin: 6px 8px;
    border-radius: 12px;
    color: #1e293b;
    font-weight: 500;
    font-size: 14px;
    background: rgba(248, 250, 252, 0.95);
    border: 1px solid rgba(203, 213, 225, 0.6);
    min-height: 26px;
}

QListWidget#m_conversationList::item:hover {
    background: rgba(59, 130, 246, 0.12);
    border-color: rgba(59, 130, 246, 0.45);
}

QListWidget#m_conversationList::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
        stop:0 rgba(37, 99, 235, 0.18), stop:1 rgba(37, 99, 235, 0.28));
    color: #1d4ed8;
    font-weight: 600;
    border: 2px solid rgba(37, 99, 235, 0.35);
}

QListWidget#m_conversationList::item:selected:hover {
    border-color: rgba(37, 99, 235, 0.58);
}

QListWidget#m_conversationList QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 4px;
}

QListWidget#m_conversationList QScrollBar::handle:vertical {
    background: rgba(100, 116, 139, 0.35);
    border-radius: 4px;
    min-height: 30px;
}

QListWidget#m_conversationList QScrollBar::handle:vertical:hover {
    background: rgba(59, 130, 246, 0.6);
}

QListWidget#m_conversationList QScrollBar::add-line:vertical,
QListWidget#m_conversationList QScrollBar::sub-line:vertical,
QListWidget#m_conversationList QScrollBar::add-page:vertical,
QListWidget#m_conversationList QScrollBar::sub-page:vertical {
    height: 0;
    width: 0;
}

ChatList QLabel {
    color: #0f172a;
    font-weight: 600;
    font-size: 16px;
    padding: 12px 16px 6px 16px;
    background: transparent;
}

ChatList QLabel[objectName="sectionTitle"] {
    border-bottom: 1px solid rgba(203, 213, 225, 0.5);
    margin-bottom: 8px;
}

ChatList QFrame#footerDivider {
    background: rgba(203, 213, 225, 0.8);
    border: none;
    margin: 8px 12px 8px 12px;
}

ChatList QWidget#footerWidget {
    background: transparent;
    border: none;
    margin-top: 0;
    padding: 12px 8px 4px 8px;
}

ChatList QPushButton#btnParamSetting {
    background-color: rgba(37, 99, 235, 0.12);
    border: 1px solid rgba(37, 99, 235, 0.35);
    border-radius: 12px;
    font-size: 13px;
    font-weight: 600;
    color: #1d4ed8;
    padding: 10px 18px;
    min-height: 44px;
}

ChatList QPushButton#btnParamSetting:hover {
    background-color: rgba(37, 99, 235, 0.2);
    border-color: rgba(37, 99, 235, 0.5);
}

ChatList QPushButton#btnParamSetting:pressed {
    background-color: rgba(37, 99, 235, 0.32);
    border-color: rgba(37, 99, 235, 0.6);
})");

    setStyleSheet(kStyleSheet);
}
