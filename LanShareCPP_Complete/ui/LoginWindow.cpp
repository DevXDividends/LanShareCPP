#include "LoginWindow.h"
#include "ChatWindow.h"
#include "ui_LoginWindow.h"
#include <QMessageBox>
#include <QTimer>
#include <iostream>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWindow)
    , chatWindow_(nullptr)
    , isConnecting_(false)
    , isRegistering_(false)
{
    ui->setupUi(this);
    ui->usernameInput->setMinimumHeight(45);
ui->passwordInput->setMinimumHeight(45);
ui->serverInput->setMinimumHeight(45);
ui->portInput->setMinimumHeight(45);

ui->loginButton->setMinimumHeight(50);
ui->registerButton->setMinimumHeight(50);
    
    setWindowTitle("🎮 LanShare - Login");
    setFixedSize(450, 600);
    
    applyStyles();
    
    // Connect buttons
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    
    // Initialize client
    client_ = std::make_unique<LanShare::ClientCore>();
    
    // Set up callbacks
    client_->setAuthCallback([this](bool success, const std::string& userID, const std::string& msg) {
        QMetaObject::invokeMethod(this, [this, success, userID, msg]() {
            onAuthResult(success, userID, msg);
        });
    });
    
    client_->setConnectionCallback([this](bool connected, const std::string& reason) {
        QMetaObject::invokeMethod(this, [this, connected, reason]() {
            onConnectionResult(connected, reason);
        });
    });
    
    // Default server
    ui->serverInput->setText("127.0.0.1");
    ui->portInput->setText("5555");
    
    // Start async
    client_->startAsync();
}

LoginWindow::~LoginWindow()
{
    if (client_) {
        client_->disconnect();
    }
    delete ui;
}

void LoginWindow::applyStyles()
{
    // Discord-inspired color scheme
    QString mainStyle = R"(
        QWidget {
            background-color: #36393F;
            color: #DCDDDE;
            font-family: 'Segoe UI', Arial;
        }
        
        QLabel#titleLabel {
            color: #5865F2;
            font-size: 28px;
            font-weight: bold;
        }
        
        QLabel#subtitleLabel {
            color: #B9BBBE;
            font-size: 14px;
        }
        
        QLineEdit {
            background-color: #40444B;
            border: 2px solid #202225;
            border-radius: 5px;
            padding: 10px;
            color: #DCDDDE;
            font-size: 14px;
        }
        
        QLineEdit:focus {
            border: 2px solid #5865F2;
        }
        
        QPushButton {
            background-color: #5865F2;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 12px;
            font-size: 14px;
            font-weight: bold;
        }
        
        QPushButton:hover {
            background-color: #4752C4;
        }
        
        QPushButton:pressed {
            background-color: #3C45A5;
        }
        
        QPushButton#registerButton {
            background-color: #3BA55D;
        }
        
        QPushButton#registerButton:hover {
            background-color: #2D7D46;
        }
        
        QLabel#errorLabel {
            color: #ED4245;
            font-size: 12px;
        }
        
        QLabel#successLabel {
            color: #57F287;
            font-size: 12px;
        }
        
        QCheckBox {
            color: #B9BBBE;
        }
        
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border-radius: 3px;
            border: 2px solid #202225;
            background-color: #40444B;
        }
        
        QCheckBox::indicator:checked {
            background-color: #5865F2;
            border-color: #5865F2;
        }
    )";
    
    setStyleSheet(mainStyle);
}

void LoginWindow::connectToServer()
{
    if (isConnecting_) return;
    
    isConnecting_ = true;
    ui->statusLabel->setText("🔌 Connecting to server...");
    ui->statusLabel->setStyleSheet("color: #FEE75C;");
    
    QString serverIP = ui->serverInput->text().trimmed();
    QString portStr = ui->portInput->text().trimmed();
    
    if (serverIP.isEmpty()) {
        serverIP = "127.0.0.1";
    }
    
    bool ok;
    unsigned short port = portStr.toUShort(&ok);
    if (!ok) {
        port = 5555;
    }
    
    client_->connect(serverIP.toStdString(), port);
}

void LoginWindow::onConnectionResult(bool connected, const std::string& reason)
{
    isConnecting_ = false;
    
    if (connected) {
        ui->statusLabel->setText("✅ Connected!");
        ui->statusLabel->setStyleSheet("color: #57F287;");
    } else {
        showError(QString("Connection failed: %1").arg(QString::fromStdString(reason)));
    }
}

void LoginWindow::onLoginClicked()
{
    QString username = ui->usernameInput->text().trimmed();
    QString password = ui->passwordInput->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        showError("Please enter username and password");
        return;
    }
    
    if (!client_->isConnected()) {
        connectToServer();
        
        // Retry login after connection
        QTimer::singleShot(1500, this, [this, username, password]() {
            if (client_->isConnected()) {
                client_->login(username.toStdString(), password.toStdString());
                ui->statusLabel->setText("🔐 Logging in...");
                ui->statusLabel->setStyleSheet("color: #FEE75C;");
            }
        });
    } else {
        client_->login(username.toStdString(), password.toStdString());
        ui->statusLabel->setText("🔐 Logging in...");
        ui->statusLabel->setStyleSheet("color: #FEE75C;");
    }
}

void LoginWindow::onRegisterClicked()
{
    QString username = ui->usernameInput->text().trimmed();
    QString password = ui->passwordInput->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        showError("Please enter username and password");
        return;
    }
    
    if (password.length() < 4) {
        showError("Password must be at least 4 characters");
        return;
    }
    
    isRegistering_ = true;
    
    if (!client_->isConnected()) {
        connectToServer();
        
        // Retry register after connection
        QTimer::singleShot(1500, this, [this, username, password]() {
            if (client_->isConnected()) {
                client_->registerUser(username.toStdString(), password.toStdString());
                ui->statusLabel->setText("📝 Registering...");
                ui->statusLabel->setStyleSheet("color: #FEE75C;");
            }
        });
    } else {
        client_->registerUser(username.toStdString(), password.toStdString());
        ui->statusLabel->setText("📝 Registering...");
        ui->statusLabel->setStyleSheet("color: #FEE75C;");
    }
}

void LoginWindow::onAuthResult(bool success, const std::string& userID, const std::string& message)
{
    if (success) {
        showSuccess(QString("✨ Success! Welcome %1").arg(QString::fromStdString(userID)));
        
        // Open chat window after a short delay
        QTimer::singleShot(500, this, &LoginWindow::openChatWindow);
    } else {
        showError(QString::fromStdString(message));
        isRegistering_ = false;
    }
}

void LoginWindow::showError(const QString& message)
{
    ui->statusLabel->setText("❌ " + message);
    ui->statusLabel->setStyleSheet("color: #ED4245;");
}

void LoginWindow::showSuccess(const QString& message)
{
    ui->statusLabel->setText(message);
    ui->statusLabel->setStyleSheet("color: #57F287;");
}

void LoginWindow::openChatWindow()
{
    if (!chatWindow_) {
        chatWindow_ = new ChatWindow(client_.get());
    }
    
    chatWindow_->show();
    this->hide();
}

void LoginWindow::onTabChanged(int index)
{
    // Future: For switching between login/register tabs
}
