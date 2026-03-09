#include "LoginWindow.h"
#include "ChatWindow.h"
#include "ui_LoginWindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QThread>
#include <iostream>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWindow)
    , chatWindow_(nullptr)
    , isConnecting_(false)
    , isRegistering_(false)
    , pendingAction_(PendingAction::None)
{
    ui->setupUi(this);
    ui->usernameInput->setMinimumHeight(45);
    ui->passwordInput->setMinimumHeight(45);
    ui->serverInput->setMinimumHeight(45);
    ui->portInput->setMinimumHeight(45);
    ui->loginButton->setMinimumHeight(50);
    ui->registerButton->setMinimumHeight(50);
    
    setWindowTitle("⬡ LanShare - Login");
    setFixedSize(450, 600);
    
    applyStyles();
    
    // Connect buttons
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    
    // Initialize client
    client_ = std::make_unique<LanShare::ClientCore>();
    
    // Set up callbacks - these will be called from background thread, so use invokeMethod
    client_->setAuthCallback([this](bool success, const std::string& userID, const std::string& msg) {
        QMetaObject::invokeMethod(this, [this, success, userID, msg]() {
            onAuthResult(success, userID, msg);
        }, Qt::QueuedConnection);
    });
    
    client_->setConnectionCallback([this](bool connected, const std::string& reason) {
        QMetaObject::invokeMethod(this, [this, connected, reason]() {
            onConnectionResult(connected, reason);
        }, Qt::QueuedConnection);
    });
    
    // Default server
    ui->serverInput->setText("127.0.0.1");
    ui->portInput->setText("5555");
    
    // Start async IO context
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
    QString mainStyle = R"(
        QWidget {
            background-color: #0a0e1a;
            color: #c0f0e0;
            font-family: 'Courier New', monospace;
        }

        /* ── Title ── */
        QLabel#titleLabel {
            color: #00ffe7;
            font-size: 30px;
            font-weight: bold;
            font-family: 'Courier New', monospace;
            letter-spacing: 4px;
        }
        QLabel#subtitleLabel {
            color: #00ffe780;
            font-size: 12px;
            letter-spacing: 3px;
            font-family: 'Courier New', monospace;
        }

        /* ── Input fields ── */
        QLineEdit {
            background-color: #0d1526;
            border: 1px solid #00ffe740;
            border-radius: 4px;
            padding: 10px 14px;
            color: #00ffe7;
            font-size: 13px;
            font-family: 'Courier New', monospace;
            selection-background-color: #00ffe740;
        }
        QLineEdit:focus {
            border: 1px solid #00ffe7;
            background-color: #0f1a2e;
        }
        QLineEdit:hover {
            border: 1px solid #00ffe7a0;
        }

        /* ── Login button (cyan) ── */
        QPushButton {
            background-color: transparent;
            color: #00ffe7;
            border: 1px solid #00ffe7;
            border-radius: 4px;
            padding: 12px;
            font-size: 13px;
            font-weight: bold;
            font-family: 'Courier New', monospace;
            letter-spacing: 2px;
        }
        QPushButton:hover {
            background-color: #00ffe720;
            color: #ffffff;
            border: 1px solid #00ffe7;
        }
        QPushButton:pressed {
            background-color: #00ffe740;
        }
        QPushButton:disabled {
            border: 1px solid #1a3a3a;
            color: #1a3a3a;
        }

        /* ── Register button (green) ── */
        QPushButton#registerButton {
            color: #39ff14;
            border: 1px solid #39ff14;
        }
        QPushButton#registerButton:hover {
            background-color: #39ff1420;
            color: #ffffff;
            border: 1px solid #39ff14;
        }
        QPushButton#registerButton:pressed {
            background-color: #39ff1440;
        }
        QPushButton#registerButton:disabled {
            border: 1px solid #1a3a1a;
            color: #1a3a1a;
        }

        /* ── Status labels ── */
        QLabel#errorLabel   { color: #ff3a6e; font-size: 12px; letter-spacing: 1px; }
        QLabel#successLabel { color: #39ff14; font-size: 12px; letter-spacing: 1px; }
        QLabel#statusLabel  { color: #00ffe780; font-size: 12px; letter-spacing: 1px; }

        /* ── Form field labels ── */
        QLabel {
            color: #00ffe7a0;
            font-size: 11px;
            letter-spacing: 2px;
            font-family: 'Courier New', monospace;
        }

        QCheckBox { color: #00ffe780; }
        QCheckBox::indicator {
            width: 16px; height: 16px;
            border-radius: 2px;
            border: 1px solid #00ffe740;
            background-color: #0d1526;
        }
        QCheckBox::indicator:checked {
            background-color: #00ffe740;
            border-color: #00ffe7;
        }
    )";
    setStyleSheet(mainStyle);
}

void LoginWindow::setButtonsEnabled(bool enabled)
{
    ui->loginButton->setEnabled(enabled);
    ui->registerButton->setEnabled(enabled);
    ui->usernameInput->setEnabled(enabled);
    ui->passwordInput->setEnabled(enabled);
    ui->serverInput->setEnabled(enabled);
    ui->portInput->setEnabled(enabled);
}

void LoginWindow::connectToServer()
{
    if (isConnecting_) return;
    
    isConnecting_ = true;
    setButtonsEnabled(false);
    ui->statusLabel->setText("🔌 Connecting to server...");
    ui->statusLabel->setStyleSheet("color: #FEE75C;");
    
    QString serverIP = ui->serverInput->text().trimmed();
    QString portStr = ui->portInput->text().trimmed();
    
    if (serverIP.isEmpty()) serverIP = "127.0.0.1";
    
    bool ok;
    unsigned short port = portStr.toUShort(&ok);
    if (!ok) port = 5555;
    
    // ✅ Run blocking connect() in a background thread
    //    so the Qt UI thread (event loop) stays responsive
    QThread* thread = new QThread(this);
    
    connect(thread, &QThread::started, [this, serverIP, port, thread]() {
        client_->connect(serverIP.toStdString(), port);
        thread->quit();
    });
    
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    
    thread->start();
}

void LoginWindow::onConnectionResult(bool connected, const std::string& reason)
{
    isConnecting_ = false;
    
    if (connected) {
        ui->statusLabel->setText("✅ Connected!");
        ui->statusLabel->setStyleSheet("color: #57F287;");
        
        // Now fire the pending action (login or register)
        if (pendingAction_ == PendingAction::Login) {
            client_->login(pendingUsername_, pendingPassword_);
            ui->statusLabel->setText("🔐 Logging in...");
            ui->statusLabel->setStyleSheet("color: #FEE75C;");
        } else if (pendingAction_ == PendingAction::Register) {
            client_->registerUser(pendingUsername_, pendingPassword_);
            ui->statusLabel->setText("📝 Registering...");
            ui->statusLabel->setStyleSheet("color: #FEE75C;");
        }
        pendingAction_ = PendingAction::None;
    } else {
        setButtonsEnabled(true);
        pendingAction_ = PendingAction::None;
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
    
    if (client_->isConnected()) {
        setButtonsEnabled(false);
        client_->login(username.toStdString(), password.toStdString());
        ui->statusLabel->setText("🔐 Logging in...");
        ui->statusLabel->setStyleSheet("color: #FEE75C;");
    } else {
        // Store credentials, connect first — onConnectionResult will fire login
        pendingUsername_ = username.toStdString();
        pendingPassword_ = password.toStdString();
        pendingAction_   = PendingAction::Login;
        connectToServer();
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
    
    if (client_->isConnected()) {
        setButtonsEnabled(false);
        client_->registerUser(username.toStdString(), password.toStdString());
        ui->statusLabel->setText("📝 Registering...");
        ui->statusLabel->setStyleSheet("color: #FEE75C;");
    } else {
        // Store credentials, connect first — onConnectionResult will fire register
        pendingUsername_ = username.toStdString();
        pendingPassword_ = password.toStdString();
        pendingAction_   = PendingAction::Register;
        connectToServer();
    }
}

void LoginWindow::onAuthResult(bool success, const std::string& userID, const std::string& message)
{
    setButtonsEnabled(true);
    
    if (success) {
        showSuccess(QString("✨ Welcome, %1!").arg(QString::fromStdString(userID)));
        QTimer::singleShot(500, this, &LoginWindow::openChatWindow);
    } else {
        isRegistering_ = false;
        showError(QString::fromStdString(message));
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
    // Future: tab switching
}