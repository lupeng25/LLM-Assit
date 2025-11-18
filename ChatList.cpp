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
#include <QIcon>

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
            const int baseHeight = 48; // Chatbox风格：更紧凑
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

            const QRectF itemRect = opt.rect;
            const bool isSelected = opt.state & QStyle::State_Selected;
            const bool isHovered = opt.state & QStyle::State_MouseOver;

            // Chatbox style: clean and flat, no shadow, no border
            QColor bgColor = Qt::transparent;
            if (isSelected)
            {
                bgColor = QColor(59, 130, 246, 25);
            }
            else if (isHovered)
            {
                bgColor = QColor(241, 245, 249);
            }

            // Draw background
            if (bgColor.alpha() > 0)
            {
                painter->fillRect(itemRect, bgColor);
            }

            const QString title = index.data(Qt::DisplayRole).toString();
            const QString timestamp = index.data(ChatList::TimestampRole).toString();
            const bool hasTimestamp = !timestamp.trimmed().isEmpty();

            // Text color: use dark color for both selected and unselected
            const QColor titleColor = QColor(30, 41, 59);
            const QColor timeColor = QColor(148, 163, 184);

            QRectF textRect = itemRect.adjusted(16, 0, -16, 0);
            const int timeWidth = hasTimestamp ? opt.fontMetrics.horizontalAdvance(timestamp) + 8 : 0;
            const QRectF timeRect = hasTimestamp
                ? QRectF(textRect.right() - timeWidth, textRect.top(), timeWidth, textRect.height())
                : QRectF();

            QFont titleFont = opt.font;
            titleFont.setPointSize(10);
            titleFont.setWeight(QFont::Medium);

            // Draw timestamp
            if (hasTimestamp)
            {
                painter->setPen(timeColor);
                painter->setFont(opt.font);
                painter->drawText(timeRect, Qt::AlignRight | Qt::AlignVCenter, timestamp);
            }

            // Draw title
            QRectF titleRect = textRect;
            if (hasTimestamp)
            {
                titleRect.setRight(timeRect.left() - 8);
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
    , topHeaderWidget(nullptr)
    , appIconLabel(nullptr)
    , appTitleLabel(nullptr)
    , conversationHeaderWidget(nullptr)
    , conversationTitleLabel(nullptr)
    , listViewButton(nullptr)
    , clearAllButton(nullptr)
    , m_conversationList(nullptr)
    , btnNewConversation(nullptr)
    , footerDivider(nullptr)
    , footerWidget(nullptr)
    , footerLayout(nullptr)
    , btnAbout(nullptr)
    , btnParamSetting(nullptr)
    , searchEdit(nullptr)
    , searchTimer(nullptr)
    , searchCallback(nullptr)
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
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 顶部header：图标+标题
    topHeaderWidget = new QWidget(this);
    topHeaderWidget->setObjectName("topHeaderWidget");
    QHBoxLayout* topHeaderLayout = new QHBoxLayout(topHeaderWidget);
    topHeaderLayout->setContentsMargins(16, 16, 16, 12);
    topHeaderLayout->setSpacing(10);

    appIconLabel = new QLabel(topHeaderWidget);
    appIconLabel->setObjectName("appIconLabel");
    appIconLabel->setFixedSize(32, 32);
    appIconLabel->setPixmap(QIcon(":/QtWidgetsApp/ICONs/CustomerService.png").pixmap(32, 32));

    appTitleLabel = new QLabel(tr("Chatbox"), topHeaderWidget);
    appTitleLabel->setObjectName("appTitleLabel");
    QFont titleFont;
    titleFont.setFamily("Microsoft YaHei UI");
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    appTitleLabel->setFont(titleFont);

    topHeaderLayout->addWidget(appIconLabel);
    topHeaderLayout->addWidget(appTitleLabel);
    topHeaderLayout->addStretch();

    // Conversation header
    conversationHeaderWidget = new QWidget(this);
    conversationHeaderWidget->setObjectName("conversationHeaderWidget");
    QHBoxLayout* conversationHeaderLayout = new QHBoxLayout(conversationHeaderWidget);
    conversationHeaderLayout->setContentsMargins(16, 8, 16, 8);
    conversationHeaderLayout->setSpacing(8);

    conversationTitleLabel = new QLabel(tr("Conversation"), conversationHeaderWidget);
    conversationTitleLabel->setObjectName("conversationTitleLabel");
    conversationTitleLabel->hide(); // Hide the label
    QFont conversationTitleFont;
    conversationTitleFont.setFamily("Microsoft YaHei UI");
    conversationTitleFont.setPointSize(14);
    conversationTitleFont.setBold(true);
    conversationTitleLabel->setFont(conversationTitleFont);

    listViewButton = new QPushButton(conversationHeaderWidget);
    listViewButton->setObjectName("listViewButton");
    listViewButton->setFixedSize(24, 24);
    listViewButton->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_open.png"));
    listViewButton->setIconSize(QSize(20, 20));
    listViewButton->setCursor(Qt::PointingHandCursor);

    clearAllButton = new QPushButton(conversationHeaderWidget);
    clearAllButton->setObjectName("clearAllButton");
    clearAllButton->setFixedSize(24, 24);
    clearAllButton->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_up.png")); // Temporary icon, can be replaced with trash icon later
    clearAllButton->setIconSize(QSize(20, 20));
    clearAllButton->setCursor(Qt::PointingHandCursor);

    conversationHeaderLayout->addStretch();
    conversationHeaderLayout->addWidget(listViewButton);
    conversationHeaderLayout->addWidget(clearAllButton);

    // Conversation list
    m_conversationList = new QListWidget(this);
    m_conversationList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_conversationList->setObjectName("m_conversationList");
    m_conversationList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_conversationList->setMouseTracking(true);
    m_conversationList->setSpacing(8);
    m_conversationList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_conversationList->setFrameShape(QFrame::NoFrame);
    m_conversationList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_conversationList->setItemDelegate(new ChatListDelegate(m_conversationList));

    // New conversation button
    btnNewConversation = new QPushButton(this);
    btnNewConversation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    btnNewConversation->setMinimumHeight(32);
    btnNewConversation->setText(tr("New Conversation"));
    btnNewConversation->setObjectName("btnNewConversation");
    btnNewConversation->setCursor(Qt::PointingHandCursor);
    btnNewConversation->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_open.png"));
    btnNewConversation->setIconSize(QSize(14, 14));

    // Create search box
    QWidget* searchWidget = new QWidget(this);
    QHBoxLayout* searchLayout = new QHBoxLayout(searchWidget);
    searchLayout->setContentsMargins(16, 8, 16, 8);
    searchLayout->setSpacing(8);
    
    searchEdit = new QLineEdit(searchWidget);
    searchEdit->setPlaceholderText(tr("Search conversations..."));
    searchEdit->setObjectName("searchEdit");
    
    searchLayout->addWidget(searchEdit);
    searchWidget->setLayout(searchLayout);

    // Create search debounce timer
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(300);

    // Add to layout
    mainLayout->addWidget(topHeaderWidget);
    // Hide conversation header widget (only keep buttons if needed, or remove entirely)
    conversationHeaderWidget->hide();
    // mainLayout->addWidget(conversationHeaderWidget); // Commented out to hide the entire header
    mainLayout->addWidget(searchWidget);
    mainLayout->addWidget(m_conversationList);
    mainLayout->addWidget(btnNewConversation);

    footerDivider = new QFrame(this);
    footerDivider->setObjectName("footerDivider");
    footerDivider->setFixedHeight(1);
    footerDivider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mainLayout->addWidget(footerDivider);

    footerWidget = new QWidget(this);
    footerWidget->setObjectName("footerWidget");
    footerLayout = new QVBoxLayout(footerWidget);
    footerLayout->setContentsMargins(16, 8, 16, 16);
    footerLayout->setSpacing(8);

    btnParamSetting = new QPushButton(tr("Param Setting"), footerWidget);
    btnParamSetting->setObjectName("btnParamSetting");
    btnParamSetting->setCursor(Qt::PointingHandCursor);
    btnParamSetting->setMinimumHeight(32);
    btnParamSetting->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_open.png")); // Temporary icon
    btnParamSetting->setIconSize(QSize(16, 16));

    btnAbout = new QPushButton(tr("About (1.17.1)"), footerWidget);
    btnAbout->setObjectName("btnAbout");
    btnAbout->setCursor(Qt::PointingHandCursor);
    btnAbout->setMinimumHeight(32);
    btnAbout->setIcon(QIcon(":/QtWidgetsApp/ICONs/icon_open.png")); // Temporary icon
    btnAbout->setIconSize(QSize(16, 16));

    footerLayout->addWidget(btnParamSetting);
    footerLayout->addWidget(btnAbout);

    mainLayout->addWidget(footerWidget);

    applyStyles();
}

void ChatList::connectSignals()
{
    connect(btnNewConversation, &QPushButton::clicked,
        this, &ChatList::onNewConversationClicked);

    // Connect list selection change signal
    connect(m_conversationList, &QListWidget::currentItemChanged,
        this, &ChatList::onConversationSelectionChanged);

    connect(m_conversationList, &QListWidget::customContextMenuRequested,
        this, &ChatList::showContextMenu);
    
    // Connect search box signals
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
    if (btnAbout)
    {
        connect(btnAbout, &QPushButton::clicked,
            this, &ChatList::onAboutClicked);
    }
    if (clearAllButton)
    {
        connect(clearAllButton, &QPushButton::clicked,
            this, &ChatList::onClearAllClicked);
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

void ChatList::onAboutClicked()
{
    emit aboutClicked();
}

void ChatList::onClearAllClicked()
{
    emit clearAllRequested();
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
    background: #ffffff;
    border: none;
}

ChatList QWidget#topHeaderWidget {
    background: transparent;
    border: none;
}

ChatList QLabel#appIconLabel {
    background: transparent;
    border: none;
}

ChatList QLabel#appTitleLabel {
    color: #1e293b;
    background: transparent;
    border: none;
}

ChatList QWidget#conversationHeaderWidget {
    background: transparent;
    border: none;
}

ChatList QLabel#conversationTitleLabel {
    color: #1e293b;
    background: transparent;
    border: none;
}

ChatList QPushButton#listViewButton,
ChatList QPushButton#clearAllButton {
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 4px;
}

ChatList QPushButton#listViewButton:hover,
ChatList QPushButton#clearAllButton:hover {
    background: rgba(148, 163, 184, 0.15);
}

ChatList QPushButton#listViewButton:pressed,
ChatList QPushButton#clearAllButton:pressed {
    background: rgba(148, 163, 184, 0.25);
}

ChatList QPushButton#btnNewConversation {
    background: #3b82f6;
    color: #ffffff;
    border: none;
    border-radius: 8px;
    font-size: 12px;
    font-weight: 600;
    padding: 6px 12px;
    margin: 6px 16px;
    min-height: 32px;
    text-align: left;
}

ChatList QPushButton#btnNewConversation:hover {
    background: #2563eb;
}

ChatList QPushButton#btnNewConversation:pressed {
    background: #1d4ed8;
}

QListWidget#m_conversationList {
    border: none;
    background: transparent;
    outline: 0;
    padding: 8px 16px;
    font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;
}

QListWidget#m_conversationList::item {
    padding: 12px 16px;
    margin: 2px 0;
    border-radius: 8px;
    color: #1e293b;
    font-weight: 500;
    font-size: 10px;
    background: transparent;
    border: none;
    min-height: 44px;
}

QListWidget#m_conversationList::item:hover {
    background: rgba(241, 245, 249, 0.8);
}

QListWidget#m_conversationList::item:selected {
    background: rgba(59, 130, 246, 0.1);
    color: #1e293b;
    font-weight: 500;
    border: none;
}

QListWidget#m_conversationList::item:selected:hover {
    background: rgba(59, 130, 246, 0.15);
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

ChatList QFrame#footerDivider {
    background: rgba(226, 232, 240, 0.8);
    border: none;
    margin: 8px 16px;
}

/* Footer */
ChatList QWidget#footerWidget {
    background: transparent;
    border: none;
    margin-top: 0;
    padding: 0;
}

ChatList QPushButton#btnAbout,
ChatList QPushButton#btnParamSetting {
    background: transparent;
    border: none;
    border-radius: 6px;
    font-size: 12px;
    font-weight: 500;
    color: #64748b;
    padding: 6px 12px;
    min-height: 32px;
    text-align: left;
}

ChatList QPushButton#btnAbout:hover,
ChatList QPushButton#btnParamSetting:hover {
    background: rgba(241, 245, 249, 0.8);
    color: #1e293b;
}

ChatList QPushButton#btnAbout:pressed,
ChatList QPushButton#btnParamSetting:pressed {
    background: rgba(226, 232, 240, 0.8);
}

ChatList QLineEdit#searchEdit {
    background: rgba(241, 245, 249, 0.8);
    border: 1px solid rgba(203, 213, 225, 0.6);
    border-radius: 8px;
    padding: 6px 12px;
    font-size: 13px;
    color: #1e293b;
}

ChatList QLineEdit#searchEdit:focus {
    border-color: #3b82f6;
    background: rgba(255, 255, 255, 1.0);
}

ChatList QLineEdit#searchEdit::placeholder {
    color: #94a3b8;
})");

    setStyleSheet(kStyleSheet);
}
