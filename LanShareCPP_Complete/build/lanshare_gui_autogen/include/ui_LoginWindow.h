/********************************************************************************
** Form generated from reading UI file 'LoginWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINWINDOW_H
#define UI_LOGINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginWindow
{
public:
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QSpacerItem *verticalSpacer_2;
    QLabel *label;
    QLineEdit *usernameInput;
    QLabel *label_2;
    QLineEdit *passwordInput;
    QLabel *label_3;
    QLineEdit *serverInput;
    QLabel *label_4;
    QLineEdit *portInput;
    QSpacerItem *verticalSpacer_3;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QLabel *statusLabel;
    QSpacerItem *verticalSpacer_4;

    void setupUi(QWidget *LoginWindow)
    {
        if (LoginWindow->objectName().isEmpty())
            LoginWindow->setObjectName("LoginWindow");
        LoginWindow->resize(400, 550);
        verticalLayout = new QVBoxLayout(LoginWindow);
        verticalLayout->setSpacing(20);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(40, 40, 40, 40);
        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        titleLabel = new QLabel(LoginWindow);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(titleLabel);

        subtitleLabel = new QLabel(LoginWindow);
        subtitleLabel->setObjectName("subtitleLabel");
        subtitleLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(subtitleLabel);

        verticalSpacer_2 = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        label = new QLabel(LoginWindow);
        label->setObjectName("label");

        verticalLayout->addWidget(label);

        usernameInput = new QLineEdit(LoginWindow);
        usernameInput->setObjectName("usernameInput");

        verticalLayout->addWidget(usernameInput);

        label_2 = new QLabel(LoginWindow);
        label_2->setObjectName("label_2");

        verticalLayout->addWidget(label_2);

        passwordInput = new QLineEdit(LoginWindow);
        passwordInput->setObjectName("passwordInput");
        passwordInput->setEchoMode(QLineEdit::Password);

        verticalLayout->addWidget(passwordInput);

        label_3 = new QLabel(LoginWindow);
        label_3->setObjectName("label_3");

        verticalLayout->addWidget(label_3);

        serverInput = new QLineEdit(LoginWindow);
        serverInput->setObjectName("serverInput");

        verticalLayout->addWidget(serverInput);

        label_4 = new QLabel(LoginWindow);
        label_4->setObjectName("label_4");

        verticalLayout->addWidget(label_4);

        portInput = new QLineEdit(LoginWindow);
        portInput->setObjectName("portInput");

        verticalLayout->addWidget(portInput);

        verticalSpacer_3 = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);

        loginButton = new QPushButton(LoginWindow);
        loginButton->setObjectName("loginButton");

        verticalLayout->addWidget(loginButton);

        registerButton = new QPushButton(LoginWindow);
        registerButton->setObjectName("registerButton");

        verticalLayout->addWidget(registerButton);

        statusLabel = new QLabel(LoginWindow);
        statusLabel->setObjectName("statusLabel");
        statusLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(statusLabel);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_4);


        retranslateUi(LoginWindow);

        QMetaObject::connectSlotsByName(LoginWindow);
    } // setupUi

    void retranslateUi(QWidget *LoginWindow)
    {
        LoginWindow->setWindowTitle(QCoreApplication::translate("LoginWindow", "LanShare - Login", nullptr));
        titleLabel->setText(QCoreApplication::translate("LoginWindow", "\360\237\216\256 LanShare", nullptr));
        subtitleLabel->setText(QCoreApplication::translate("LoginWindow", "Secure LAN Messaging", nullptr));
        label->setText(QCoreApplication::translate("LoginWindow", "Username", nullptr));
        usernameInput->setPlaceholderText(QCoreApplication::translate("LoginWindow", "Enter your username", nullptr));
        label_2->setText(QCoreApplication::translate("LoginWindow", "Password", nullptr));
        passwordInput->setPlaceholderText(QCoreApplication::translate("LoginWindow", "Enter your password", nullptr));
        label_3->setText(QCoreApplication::translate("LoginWindow", "Server IP", nullptr));
        serverInput->setText(QCoreApplication::translate("LoginWindow", "127.0.0.1", nullptr));
        serverInput->setPlaceholderText(QCoreApplication::translate("LoginWindow", "Server IP address", nullptr));
        label_4->setText(QCoreApplication::translate("LoginWindow", "Port", nullptr));
        portInput->setText(QCoreApplication::translate("LoginWindow", "5555", nullptr));
        portInput->setPlaceholderText(QCoreApplication::translate("LoginWindow", "Port number", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginWindow", "Login", nullptr));
        registerButton->setText(QCoreApplication::translate("LoginWindow", "Register New Account", nullptr));
        statusLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class LoginWindow: public Ui_LoginWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINWINDOW_H
