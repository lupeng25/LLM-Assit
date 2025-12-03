/********************************************************************************
** Form generated from reading UI file 'Frm_AIAssit.ui'
**
** Created by: Qt User Interface Compiler version 5.12.9
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FRM_AIASSIT_H
#define UI_FRM_AIASSIT_H

#include <AIAssit\ChatList.h>
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

    void setupUi(QWidget *Frm_AIAssit)
    {
        if (Frm_AIAssit->objectName().isEmpty())
            Frm_AIAssit->setObjectName(QString::fromUtf8("Frm_AIAssit"));
        Frm_AIAssit->resize(1225, 800);
        Frm_AIAssit->setMinimumSize(QSize(900, 600));
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


        retranslateUi(Frm_AIAssit);

        QMetaObject::connectSlotsByName(Frm_AIAssit);
    } // setupUi

    void retranslateUi(QWidget *Frm_AIAssit)
    {
        Frm_AIAssit->setWindowTitle(QApplication::translate("Frm_AIAssit", "GKG AI\345\212\251\346\211\213", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Frm_AIAssit: public Ui_Frm_AIAssit {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FRM_AIASSIT_H
