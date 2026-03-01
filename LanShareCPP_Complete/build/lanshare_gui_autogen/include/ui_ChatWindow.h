/********************************************************************************
** Form generated from reading UI file 'ChatWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHATWINDOW_H
#define UI_CHATWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChatWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QWidget *sidebar;
    QVBoxLayout *verticalLayout;
    QLabel *userLabel;
    QListWidget *contactList;
    QVBoxLayout *verticalLayout_3;
    QPushButton *refreshButton;
    QPushButton *createGroupButton;
    QPushButton *joinGroupButton;
    QWidget *chatWidget;
    QVBoxLayout *verticalLayout_2;
    QLabel *chatTitleLabel;
    QWidget *chatContainer;
    QVBoxLayout *verticalLayout_4;
    QWidget *inputWidget;
    QHBoxLayout *horizontalLayout_2;
    QTextEdit *messageInput;
    QVBoxLayout *verticalLayout_5;
    QPushButton *sendButton;
    QPushButton *attachButton;

    void setupUi(QMainWindow *ChatWindow)
    {
        if (ChatWindow->objectName().isEmpty())
            ChatWindow->setObjectName("ChatWindow");
        ChatWindow->resize(1200, 700);
        centralwidget = new QWidget(ChatWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        sidebar = new QWidget(centralwidget);
        sidebar->setObjectName("sidebar");
        sidebar->setMinimumSize(QSize(250, 0));
        sidebar->setMaximumSize(QSize(250, 16777215));
        verticalLayout = new QVBoxLayout(sidebar);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        userLabel = new QLabel(sidebar);
        userLabel->setObjectName("userLabel");

        verticalLayout->addWidget(userLabel);

        contactList = new QListWidget(sidebar);
        contactList->setObjectName("contactList");

        verticalLayout->addWidget(contactList);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(5);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(10, 10, 10, 10);
        refreshButton = new QPushButton(sidebar);
        refreshButton->setObjectName("refreshButton");

        verticalLayout_3->addWidget(refreshButton);

        createGroupButton = new QPushButton(sidebar);
        createGroupButton->setObjectName("createGroupButton");

        verticalLayout_3->addWidget(createGroupButton);

        joinGroupButton = new QPushButton(sidebar);
        joinGroupButton->setObjectName("joinGroupButton");

        verticalLayout_3->addWidget(joinGroupButton);


        verticalLayout->addLayout(verticalLayout_3);


        horizontalLayout->addWidget(sidebar);

        chatWidget = new QWidget(centralwidget);
        chatWidget->setObjectName("chatWidget");
        verticalLayout_2 = new QVBoxLayout(chatWidget);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        chatTitleLabel = new QLabel(chatWidget);
        chatTitleLabel->setObjectName("chatTitleLabel");

        verticalLayout_2->addWidget(chatTitleLabel);

        chatContainer = new QWidget(chatWidget);
        chatContainer->setObjectName("chatContainer");
        verticalLayout_4 = new QVBoxLayout(chatContainer);
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);

        verticalLayout_2->addWidget(chatContainer);

        inputWidget = new QWidget(chatWidget);
        inputWidget->setObjectName("inputWidget");
        inputWidget->setMinimumSize(QSize(0, 100));
        inputWidget->setMaximumSize(QSize(16777215, 100));
        horizontalLayout_2 = new QHBoxLayout(inputWidget);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        messageInput = new QTextEdit(inputWidget);
        messageInput->setObjectName("messageInput");
        messageInput->setMaximumSize(QSize(16777215, 80));

        horizontalLayout_2->addWidget(messageInput);

        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName("verticalLayout_5");
        sendButton = new QPushButton(inputWidget);
        sendButton->setObjectName("sendButton");

        verticalLayout_5->addWidget(sendButton);

        attachButton = new QPushButton(inputWidget);
        attachButton->setObjectName("attachButton");

        verticalLayout_5->addWidget(attachButton);


        horizontalLayout_2->addLayout(verticalLayout_5);


        verticalLayout_2->addWidget(inputWidget);


        horizontalLayout->addWidget(chatWidget);

        ChatWindow->setCentralWidget(centralwidget);

        retranslateUi(ChatWindow);

        QMetaObject::connectSlotsByName(ChatWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ChatWindow)
    {
        ChatWindow->setWindowTitle(QCoreApplication::translate("ChatWindow", "LanShare Chat", nullptr));
        userLabel->setText(QCoreApplication::translate("ChatWindow", "\360\237\221\244 User", nullptr));
        refreshButton->setText(QCoreApplication::translate("ChatWindow", "\360\237\224\204 Refresh", nullptr));
        createGroupButton->setText(QCoreApplication::translate("ChatWindow", "\342\236\225 Create Group", nullptr));
        joinGroupButton->setText(QCoreApplication::translate("ChatWindow", "\360\237\221\245 Join Group", nullptr));
        chatTitleLabel->setText(QCoreApplication::translate("ChatWindow", "\360\237\222\254 Select a contact to start chatting", nullptr));
        messageInput->setPlaceholderText(QCoreApplication::translate("ChatWindow", "Type a message...", nullptr));
        sendButton->setText(QCoreApplication::translate("ChatWindow", "Send \360\237\223\244", nullptr));
        attachButton->setText(QCoreApplication::translate("ChatWindow", "\360\237\223\216 File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChatWindow: public Ui_ChatWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHATWINDOW_H
