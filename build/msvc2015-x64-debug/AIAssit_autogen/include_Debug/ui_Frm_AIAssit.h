/********************************************************************************
** Form generated from reading UI file 'Frm_AIAssit.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FRM_AIASSIT_H
#define UI_FRM_AIASSIT_H

#include <AIAssit\ChatList.h>
#include <AIAssit\aiparamwidget.h>
#include <AIAssit\chatinputwidget.h>
#include <AIAssit\chatshowwidget.h>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Frm_AIAssit
{
public:
    QHBoxLayout *mainHorizontalLayout;
    ChatList *ChatListWidget;
    QVBoxLayout *conversationLayout;
    QWidget *widget_3;
    QVBoxLayout *verticalLayout;
    ChatShowWidget *ChatShow;
    ChatInputWidget *ChatInput;
    AIParamWidget *AIParams;
    QVBoxLayout *parameterLayout;

    void setupUi(QWidget *Frm_AIAssit)
    {
        if (Frm_AIAssit->objectName().isEmpty())
            Frm_AIAssit->setObjectName(QString::fromUtf8("Frm_AIAssit"));
        Frm_AIAssit->resize(1225, 800);
        Frm_AIAssit->setMinimumSize(QSize(900, 600));
        Frm_AIAssit->setStyleSheet(QString::fromUtf8("QWidget#Frm_AIAssit {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 #eef2ff, stop:1 #e0f2fe);\n"
"    border-radius: 18px;\n"
"    font-family: \"Microsoft YaHei UI\", \"Segoe UI\", sans-serif;\n"
"    font-size: 14px;\n"
"    color: #0f172a;\n"
"}\n"
"\n"
"QWidget#Frm_AIAssit QPushButton {\n"
"    font-family: \"Microsoft YaHei UI\", \"Segoe UI\", sans-serif;\n"
"    font-weight: 600;\n"
"}\n"
"\n"
"QWidget#Frm_AIAssit QFrame,\n"
"QWidget#Frm_AIAssit QListWidget,\n"
"QWidget#Frm_AIAssit QScrollArea {\n"
"    background: transparent;\n"
"}\n"
"\n"
"QToolTip {\n"
"    background: rgba(15, 23, 42, 0.92);\n"
"    color: #f8fafc;\n"
"    border: 1px solid rgba(148, 163, 184, 0.35);\n"
"    padding: 6px 10px;\n"
"    border-radius: 6px;\n"
"}\n"
"\n"
"/* \345\205\250\345\261\200\346\273\232\345\212\250\346\235\241\347\276\216\345\214\226 */\n"
"QScrollBar:vertical {\n"
"    background: transparent;\n"
"    width: 12px;\n"
"    margin: 4px;\n"
"}\n"
"\n"
"QScrollBar::handle:vertical "
                        "{\n"
"    background: rgba(100, 116, 139, 0.35);\n"
"    border-radius: 6px;\n"
"    min-height: 40px;\n"
"}\n"
"\n"
"QScrollBar::handle:vertical:hover {\n"
"    background: rgba(59, 130, 246, 0.55);\n"
"}\n"
"\n"
"QScrollBar::add-line:vertical,\n"
"QScrollBar::sub-line:vertical,\n"
"QScrollBar::add-page:vertical,\n"
"QScrollBar::sub-page:vertical {\n"
"    height: 0;\n"
"    width: 0;\n"
"}\n"
"\n"
"QScrollBar:horizontal {\n"
"    background: transparent;\n"
"    height: 10px;\n"
"    margin: 4px;\n"
"}\n"
"\n"
"QScrollBar::handle:horizontal {\n"
"    background: rgba(100, 116, 139, 0.35);\n"
"    border-radius: 6px;\n"
"    min-width: 40px;\n"
"}\n"
"\n"
"QScrollBar::handle:horizontal:hover {\n"
"    background: rgba(59, 130, 246, 0.55);\n"
"}\n"
"\n"
"QScrollBar::add-line:horizontal,\n"
"QScrollBar::sub-line:horizontal,\n"
"QScrollBar::add-page:horizontal,\n"
"QScrollBar::sub-page:horizontal {\n"
"    height: 0;\n"
"    width: 0;\n"
"}"));
        mainHorizontalLayout = new QHBoxLayout(Frm_AIAssit);
        mainHorizontalLayout->setSpacing(10);
        mainHorizontalLayout->setContentsMargins(11, 11, 11, 11);
        mainHorizontalLayout->setObjectName(QString::fromUtf8("mainHorizontalLayout"));
        mainHorizontalLayout->setContentsMargins(10, 10, 10, 10);
        ChatListWidget = new ChatList(Frm_AIAssit);
        ChatListWidget->setObjectName(QString::fromUtf8("ChatListWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ChatListWidget->sizePolicy().hasHeightForWidth());
        ChatListWidget->setSizePolicy(sizePolicy);
        ChatListWidget->setMinimumSize(QSize(200, 0));
        ChatListWidget->setMaximumSize(QSize(280, 16777215));
        ChatListWidget->setStyleSheet(QString::fromUtf8("ChatList {\n"
"    background: rgba(255, 255, 255, 0.82);\n"
"    border: 1px solid rgba(148, 163, 184, 0.28);\n"
"    border-radius: 14px;\n"
"    padding: 10px 6px;\n"
"    backdrop-filter: blur(14px);\n"
"}\n"
"\n"
"ChatList QPushButton#btnNewConversation {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 #2563eb, stop:1 #1d4ed8);\n"
"    color: #f8fafc;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    font-size: 15px;\n"
"    padding: 14px 20px;\n"
"    margin: 12px 10px 10px 10px;\n"
"    min-height: 52px;\n"
"    text-align: left;\n"
"    letter-spacing: 0.2px;\n"
"}\n"
"\n"
"ChatList QPushButton#btnNewConversation:hover {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 #3b82f6, stop:1 #2563eb);\n"
"}\n"
"\n"
"ChatList QPushButton#btnNewConversation:pressed {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 #1d4ed8, stop:1 #1e40af);\n"
"}\n"
"\n"
"QListWidget#m_conversationList {\n"
"    border: none;\n"
""
                        "    background: transparent;\n"
"    outline: 0;\n"
"    padding: 6px 4px 12px 4px;\n"
"    font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;\n"
"}\n"
"\n"
"QListWidget#m_conversationList::item {\n"
"    padding: 14px 18px;\n"
"    margin: 6px 8px;\n"
"    border-radius: 12px;\n"
"    color: #1e293b;\n"
"    font-weight: 500;\n"
"    font-size: 14px;\n"
"    background: rgba(248, 250, 252, 0.95);\n"
"    border: 1px solid rgba(203, 213, 225, 0.6);\n"
"    min-height: 26px;\n"
"}\n"
"\n"
"QListWidget#m_conversationList::item:hover {\n"
"    background: rgba(59, 130, 246, 0.12);\n"
"    border-color: rgba(59, 130, 246, 0.45);\n"
"}\n"
"\n"
"QListWidget#m_conversationList::item:selected {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 rgba(37, 99, 235, 0.18), stop:1 rgba(37, 99, 235, 0.28));\n"
"    color: #1d4ed8;\n"
"    font-weight: 600;\n"
"    border: 2px solid rgba(37, 99, 235, 0.35);\n"
"}\n"
"\n"
"QListWidget#m_conversationList::item:selected:hover {\n"
"    border"
                        "-color: rgba(37, 99, 235, 0.58);\n"
"}\n"
"\n"
"QListWidget#m_conversationList QScrollBar:vertical {\n"
"    background: transparent;\n"
"    width: 8px;\n"
"    margin: 4px;\n"
"}\n"
"\n"
"QListWidget#m_conversationList QScrollBar::handle:vertical {\n"
"    background: rgba(100, 116, 139, 0.35);\n"
"    border-radius: 4px;\n"
"    min-height: 30px;\n"
"}\n"
"\n"
"QListWidget#m_conversationList QScrollBar::handle:vertical:hover {\n"
"    background: rgba(59, 130, 246, 0.6);\n"
"}\n"
"\n"
"QListWidget#m_conversationList QScrollBar::add-line:vertical,\n"
"QListWidget#m_conversationList QScrollBar::sub-line:vertical,\n"
"QListWidget#m_conversationList QScrollBar::add-page:vertical,\n"
"QListWidget#m_conversationList QScrollBar::sub-page:vertical {\n"
"    height: 0;\n"
"    width: 0;\n"
"}\n"
"\n"
"ChatList QLabel {\n"
"    color: #0f172a;\n"
"    font-weight: 600;\n"
"    font-size: 16px;\n"
"    padding: 12px 16px 6px 16px;\n"
"    background: transparent;\n"
"}\n"
"\n"
"ChatList QLabel[objectName=\"sectionTitl"
                        "e\"] {\n"
"    border-bottom: 1px solid rgba(203, 213, 225, 0.5);\n"
"    margin-bottom: 8px;\n"
"}"));
        conversationLayout = new QVBoxLayout(ChatListWidget);
        conversationLayout->setSpacing(8);
        conversationLayout->setContentsMargins(11, 11, 11, 11);
        conversationLayout->setObjectName(QString::fromUtf8("conversationLayout"));
        conversationLayout->setContentsMargins(12, 12, 12, 12);

        mainHorizontalLayout->addWidget(ChatListWidget);

        widget_3 = new QWidget(Frm_AIAssit);
        widget_3->setObjectName(QString::fromUtf8("widget_3"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(1);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(widget_3->sizePolicy().hasHeightForWidth());
        widget_3->setSizePolicy(sizePolicy1);
        verticalLayout = new QVBoxLayout(widget_3);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        ChatShow = new ChatShowWidget(widget_3);
        ChatShow->setObjectName(QString::fromUtf8("ChatShow"));
        ChatShow->setStyleSheet(QString::fromUtf8("/* =================== \345\244\264\351\203\250\345\214\272\345\237\237\346\240\267\345\274\217 =================== */\n"
"QWidget#chatHeader {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"        stop:0 rgba(255, 255, 255, 0.96), stop:1 rgba(241, 245, 249, 0.94));\n"
"    border: none;\n"
"    border-radius: 14px 14px 0 0;\n"
"    border-bottom: 1px solid rgba(226, 232, 240, 0.75);\n"
"    padding: 0;\n"
"}\n"
"\n"
"QLabel#chatTitle {\n"
"    color: #0f172a;\n"
"    font-weight: 700;\n"
"    font-size: 19px;\n"
"    letter-spacing: 0.3px;\n"
"    padding: 0 18px;\n"
"    background: transparent;\n"
"}\n"
"\n"
"QPushButton#toggleButton {\n"
"    background-color: rgba(37, 99, 235, 0.1);\n"
"    border: 1px solid rgba(37, 99, 235, 0.35);\n"
"    border-radius: 12px;\n"
"    padding: 10px;\n"
"    min-width: 46px;\n"
"    min-height: 46px;\n"
"    max-width: 46px;\n"
"    max-height: 46px;\n"
"    color: #2563eb;\n"
"}\n"
"\n"
"QPushButton#toggleButton:hover {\n"
"    background-color: rgba(37"
                        ", 99, 235, 0.18);\n"
"    border-color: rgba(37, 99, 235, 0.55);\n"
"}\n"
"\n"
"QPushButton#toggleButton:pressed {\n"
"    background-color: rgba(37, 99, 235, 0.28);\n"
"    border-color: rgba(37, 99, 235, 0.65);\n"
"}\n"
"\n"
"QPushButton#btnParamSet {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 #f97316, stop:1 #ea580c);\n"
"    color: #fff9f0;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    font-size: 14px;\n"
"    padding: 10px 22px;\n"
"}\n"
"\n"
"QPushButton#btnParamSet:hover {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 #fb923c, stop:1 #f97316);\n"
"}\n"
"\n"
"QPushButton#btnParamSet:pressed {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 #ea580c, stop:1 #c2410c);\n"
"}\n"
"\n"
"QFrame#frame {\n"
"    background: rgba(255, 255, 255, 0.92);\n"
"    border-radius: 0 0 14px 14px;\n"
"    border: 1px solid rgba(203, 213, 225, 0.6);\n"
"}\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"QWidget#emptyStateWidget,\n"
""
                        "QLabel#emptyIconLabel {\n"
"    background: transparent;\n"
"    border: none;\n"
"}\n"
"\n"
"QLabel#emptyTextLabel {\n"
"    color: #475569;\n"
"    background: transparent;\n"
"    border: none;\n"
"    font-size: 20px;\n"
"    font-weight: 600;\n"
"    letter-spacing: 0.2px;\n"
"}\n"
"\n"
"\n"
"\n"
"QPushButton#UpButton,\n"
"QPushButton#DownButton {\n"
"    background-color: rgba(37, 99, 235, 0.12);\n"
"    border: none;\n"
"    border-radius: 16px;\n"
"    margin: 3px;\n"
"    min-width: 38px;\n"
"    min-height: 38px;\n"
"    max-width: 38px;\n"
"    max-height: 38px;\n"
"    color: #1d4ed8;\n"
"}\n"
"\n"
"QPushButton#UpButton:hover,\n"
"QPushButton#DownButton:hover {\n"
"    background-color: rgba(37, 99, 235, 0.24);\n"
"}\n"
"\n"
"QPushButton#UpButton:pressed,\n"
"QPushButton#DownButton:pressed {\n"
"    background-color: rgba(37, 99, 235, 0.35);\n"
"}\n"
"\n"
"QPushButton#UpButton:focus,\n"
"QPushButton#DownButton:focus {\n"
"    outline: none;\n"
"}"));

        verticalLayout->addWidget(ChatShow);

        ChatInput = new ChatInputWidget(widget_3);
        ChatInput->setObjectName(QString::fromUtf8("ChatInput"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(ChatInput->sizePolicy().hasHeightForWidth());
        ChatInput->setSizePolicy(sizePolicy2);
        ChatInput->setMinimumSize(QSize(0, 120));

        verticalLayout->addWidget(ChatInput);


        mainHorizontalLayout->addWidget(widget_3);

        AIParams = new AIParamWidget(Frm_AIAssit);
        AIParams->setObjectName(QString::fromUtf8("AIParams"));
        sizePolicy.setHeightForWidth(AIParams->sizePolicy().hasHeightForWidth());
        AIParams->setSizePolicy(sizePolicy);
        AIParams->setMinimumSize(QSize(280, 0));
        AIParams->setMaximumSize(QSize(320, 16777215));
        AIParams->setStyleSheet(QString::fromUtf8("AIParamWidget {\n"
"    background: rgba(255, 255, 255, 0.95);\n"
"    border: 1px solid rgba(226, 232, 240, 0.8);\n"
"    border-radius: 12px;\n"
"    padding: 16px;\n"
"    backdrop-filter: blur(10px);\n"
"}\n"
"\n"
"QLabel {\n"
"    font-weight: 600;\n"
"    color: #334155;\n"
"    font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;\n"
"    font-size: 14px;\n"
"    background: transparent;\n"
"}\n"
"\n"
"QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {\n"
"    border: 1px solid rgba(203, 213, 225, 0.8);\n"
"    border-radius: 8px;\n"
"    padding: 8px 12px;\n"
"    background: rgba(248, 250, 252, 0.8);\n"
"    selection-background-color: #3b82f6;\n"
"    selection-color: white;\n"
"    font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;\n"
"    font-size: 14px;\n"
"    color: #1e293b;\n"
"    transition: all 0.3s ease;\n"
"}\n"
"\n"
"QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {\n"
"    border-color: #3b82f6;\n"
"    background: rgba(255, 255, 255, 0.95);\n"
"    box"
                        "-shadow: 0 0 0 3px rgba(59, 130, 246, 0.1);\n"
"}\n"
"\n"
"QLineEdit:hover, QComboBox:hover, QSpinBox:hover, QDoubleSpinBox:hover {\n"
"    border-color: rgba(148, 163, 184, 0.8);\n"
"    background: rgba(241, 245, 249, 0.9);\n"
"}\n"
"\n"
"QCheckBox {\n"
"    spacing: 10px;\n"
"    color: #475569;\n"
"    font-weight: 500;\n"
"    font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;\n"
"    font-size: 14px;\n"
"    background: transparent;\n"
"}\n"
"\n"
"QCheckBox::indicator {\n"
"    width: 18px;\n"
"    height: 18px;\n"
"    border: 2px solid rgba(148, 163, 184, 0.8);\n"
"    border-radius: 4px;\n"
"    background: rgba(248, 250, 252, 0.8);\n"
"}\n"
"\n"
"QCheckBox::indicator:hover {\n"
"    border-color: #3b82f6;\n"
"}\n"
"\n"
"QCheckBox::indicator:checked {\n"
"    background: #3b82f6;\n"
"    border-color: #3b82f6;\n"
"}\n"
"\n"
"QCheckBox::indicator:checked:hover {\n"
"    background: #60a5fa;\n"
"    border-color: #60a5fa;\n"
"}\n"
"\n"
"QPushButton {\n"
"    background: qlineargradient(x1:0, y1:"
                        "0, x2:0, y2:1, \n"
"        stop:0 #3b82f6, stop:1 #1d4ed8);\n"
"    border: none;\n"
"    color: white;\n"
"    padding: 10px 20px;\n"
"    border-radius: 8px;\n"
"    font-weight: 600;\n"
"    font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;\n"
"    font-size: 14px;\n"
"    min-width: 90px;\n"
"    min-height: 36px;\n"
"    transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);\n"
"    box-shadow: 0 4px 6px -1px rgba(59, 130, 246, 0.2);\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, \n"
"        stop:0 #60a5fa, stop:1 #3b82f6);\n"
"    transform: translateY(-1px);\n"
"    box-shadow: 0 10px 15px -3px rgba(59, 130, 246, 0.3);\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, \n"
"        stop:0 #1d4ed8, stop:1 #1e40af);\n"
"    transform: translateY(0px);\n"
"    box-shadow: 0 2px 4px -1px rgba(59, 130, 246, 0.2);\n"
"}\n"
"\n"
"QPushButton:disabled {\n"
"    background: #cbd5e1;\n"
"    color: #94a3b8;\n"
"  "
                        "  box-shadow: none;\n"
"    transform: none;\n"
"}\n"
"\n"
"/* \345\217\202\346\225\260\351\235\242\346\235\277\346\240\207\351\242\230\346\240\267\345\274\217 */\n"
"QLabel[objectName^=\"label\"] {\n"
"    font-size: 16px;\n"
"    font-weight: 700;\n"
"    color: #1e293b;\n"
"    margin-bottom: 8px;\n"
"    padding-bottom: 8px;\n"
"    border-bottom: 1px solid rgba(226, 232, 240, 0.8);\n"
"}"));
        parameterLayout = new QVBoxLayout(AIParams);
        parameterLayout->setSpacing(16);
        parameterLayout->setContentsMargins(11, 11, 11, 11);
        parameterLayout->setObjectName(QString::fromUtf8("parameterLayout"));
        parameterLayout->setContentsMargins(16, 16, 16, 16);

        mainHorizontalLayout->addWidget(AIParams);


        retranslateUi(Frm_AIAssit);

        QMetaObject::connectSlotsByName(Frm_AIAssit);
    } // setupUi

    void retranslateUi(QWidget *Frm_AIAssit)
    {
        Frm_AIAssit->setWindowTitle(QCoreApplication::translate("Frm_AIAssit", "GKG AI\345\212\251\346\211\213", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Frm_AIAssit: public Ui_Frm_AIAssit {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FRM_AIASSIT_H
