#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <memory>

#include "../client/ClientCore.h"

namespace Ui {
class LoginWindow;
}

class ChatWindow;

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onTabChanged(int index);
    void onAuthResult(bool success, const std::string& userID, const std::string& message);
    void onConnectionResult(bool connected, const std::string& reason);

private:
    void setupUI();
    void applyStyles();
    void connectToServer();
    void showError(const QString& message);
    void showSuccess(const QString& message);
    void openChatWindow();

    Ui::LoginWindow *ui;
    std::unique_ptr<LanShare::ClientCore> client_;
    ChatWindow* chatWindow_;
    
    bool isConnecting_;
    bool isRegistering_;
};

#endif // LOGINWINDOW_H
