#include "ChatWindow.h"
#include "MessageBubble.h"
#include "FileTransferWidget.h"
#include "ui_ChatWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <iostream>

ChatWindow::ChatWindow(LanShare::ClientCore* client, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatWindow)
    , client_(client)
    , currentIsGroup_(false)
{
    ui->setupUi(this);
    
    setWindowTitle("🎮 LanShare Chat");
    resize(1200, 700);
    
    setupUI();
    applyStyles();
    
    // Set up callbacks
    client_->setMessageCallback([this](const std::string& from, const std::vector<uint8_t>& encrypted, uint64_t timestamp) {
        QMetaObject::invokeMethod(this, [this, from, encrypted, timestamp]() {
            onMessageReceived(from, encrypted, timestamp);
        });
    });
    
    client_->setGroupMessageCallback([this](const std::string& group, const std::string& from, 
                                            const std::vector<uint8_t>& encrypted, uint64_t timestamp) {
        QMetaObject::invokeMethod(this, [this, group, from, encrypted, timestamp]() {
            onGroupMessageReceived(group, from, encrypted, timestamp);
        });
    });
    
    client_->setUserListCallback([this](const std::vector<std::string>& users) {
        QMetaObject::invokeMethod(this, [this, users]() {
            onUserListReceived(users);
        });
    });
    
    // Connect signals
    connect(ui->sendButton, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(ui->attachButton, &QPushButton::clicked, this, &ChatWindow::onAttachClicked);
    connect(ui->contactList, &QListWidget::itemClicked, this, &ChatWindow::onContactClicked);
    connect(ui->createGroupButton, &QPushButton::clicked, this, &ChatWindow::onCreateGroupClicked);
    connect(ui->joinGroupButton, &QPushButton::clicked, this, &ChatWindow::onJoinGroupClicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &ChatWindow::onRefreshUsersClicked);
    
    // Load initial data
    loadOnlineUsers();
    loadGroups();
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::setupUI()
{
    // Create chat area
    chatArea_ = new QWidget();
    chatLayout_ = new QVBoxLayout(chatArea_);
    chatLayout_->addStretch();
    chatLayout_->setSpacing(10);
    chatLayout_->setContentsMargins(10, 10, 10, 10);
    
    scrollArea_ = new QScrollArea();
    scrollArea_->setWidget(chatArea_);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // Set chat area to central widget
    ui->chatContainer->layout()->addWidget(scrollArea_);
    
    // Set user info
    QString userID = QString::fromStdString(client_->getUserID());
    ui->userLabel->setText(QString("👤 %1").arg(userID));
}

void ChatWindow::applyStyles()
{
    QString style = R"(
        QMainWindow {
            background-color: #36393F;
        }
        
        QWidget#sidebar {
            background-color: #2F3136;
        }
        
        QLabel#userLabel {
            color: #FFFFFF;
            font-size: 14px;
            font-weight: bold;
            padding: 10px;
            background-color: #202225;
        }
        
        QLabel#chatTitleLabel {
            color: #FFFFFF;
            font-size: 16px;
            font-weight: bold;
            padding: 10px;
            background-color: #202225;
        }
        
        QListWidget {
            background-color: #2F3136;
            border: none;
            color: #DCDDDE;
            font-size: 14px;
            padding: 5px;
        }
        
        QListWidget::item {
            padding: 10px;
            border-radius: 5px;
            margin: 2px;
        }
        
        QListWidget::item:selected {
            background-color: #5865F2;
            color: white;
        }
        
        QListWidget::item:hover {
            background-color: #40444B;
        }
        
        QTextEdit {
            background-color: #40444B;
            border: 2px solid #202225;
            border-radius: 8px;
            color: #DCDDDE;
            padding: 10px;
            font-size: 14px;
        }
        
        QPushButton {
            background-color: #5865F2;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 20px;
            font-size: 14px;
            font-weight: bold;
        }
        
        QPushButton:hover {
            background-color: #4752C4;
        }
        
        QPushButton:pressed {
            background-color: #3C45A5;
        }
        
        QPushButton#attachButton {
            background-color: #3BA55D;
        }
        
        QPushButton#attachButton:hover {
            background-color: #2D7D46;
        }
        
        QScrollArea {
            background-color: #36393F;
            border: none;
        }
        
        QLabel#sectionLabel {
            color: #8E9297;
            font-size: 12px;
            font-weight: bold;
            padding: 15px 10px 5px 10px;
        }
    )";
    
    setStyleSheet(style);
}

void ChatWindow::loadOnlineUsers()
{
    client_->requestUserList();
}

void ChatWindow::loadGroups()
{
    // Add some sample groups (in real app, get from server)
    ui->contactList->addItem("👥 general");
    ui->contactList->addItem("👥 team");
}

void ChatWindow::onContactClicked(QListWidgetItem* item)
{
    QString text = item->text();
    
    // Check if group or user
    bool isGroup = text.startsWith("👥");
    QString contactId = isGroup ? text.mid(3) : text.mid(3); // Remove emoji
    
    switchToContact(contactId, isGroup);
}

void ChatWindow::switchToContact(const QString& contactId, bool isGroup)
{
    currentContact_ = contactId;
    currentIsGroup_ = isGroup;
    
    // Update chat title
    QString icon = isGroup ? "👥" : "💬";
    QString encrypted = "🔒";
    ui->chatTitleLabel->setText(QString("%1 %2 %3").arg(icon, contactId, encrypted));
    
    // Clear current chat
    clearChat();
    
    // Load message history if exists
    if (messageHistory_.contains(contactId)) {
        currentMessages_ = messageHistory_[contactId];
        for (auto* bubble : currentMessages_) {
            chatLayout_->insertWidget(chatLayout_->count() - 1, bubble);
        }
    }
    
    scrollToBottom();
}

void ChatWindow::onSendClicked()
{
    QString message = ui->messageInput->toPlainText().trimmed();
    if (message.isEmpty() || currentContact_.isEmpty()) return;
    
    try {
        // Use a shared key derived from both usernames (same result on both sides)
        auto sharedKey = getSharedKey(currentContact_);
        
        auto encrypted = client_->getCrypto().encrypt(
            message.toStdString(), sharedKey
        );
        auto blob = encrypted.serialize();
        
        if (currentIsGroup_) {
            client_->sendGroupMessage(currentContact_.toStdString(), blob);
        } else {
            client_->sendPrivateMessage(currentContact_.toStdString(), blob);
        }
        
        addMessageToChat("You", message, true, false);
        ui->messageInput->clear();
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString("Failed to send: %1").arg(e.what()));
    }
}
void ChatWindow::onAttachClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select File", "", "All Files (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Show file transfer widget
    FileTransferWidget* ftWidget = new FileTransferWidget(fileName, true);
    chatLayout_->insertWidget(chatLayout_->count() - 1, ftWidget);
    
    // TODO: Actually send file
    QMessageBox::information(this, "File Transfer", 
        "File transfer functionality will be completed in next update!");
}

void ChatWindow::onMessageReceived(const std::string& from, const std::vector<uint8_t>& encrypted, uint64_t timestamp)
{
    QString fromUser = QString::fromStdString(from);
    
    // Only show if this is the active chat
    if (!currentIsGroup_ && fromUser == currentContact_) {
        addEncryptedMessage(fromUser, encrypted, false);
    }
}

void ChatWindow::onGroupMessageReceived(const std::string& group, const std::string& from, 
                                        const std::vector<uint8_t>& encrypted, uint64_t timestamp)
{
    QString groupName = QString::fromStdString(group);
    QString fromUser = QString::fromStdString(from);
    
    // Only show if this is the active chat
    if (currentIsGroup_ && groupName == currentContact_) {
        addEncryptedMessage(fromUser, encrypted, false);
    }
}

void ChatWindow::onUserListReceived(const std::vector<std::string>& users)
{
    // Clear users section
    ui->contactList->clear();
    
    // Add section label
    QListWidgetItem* userHeader = new QListWidgetItem("USERS");
    userHeader->setFlags(Qt::NoItemFlags);
    ui->contactList->addItem(userHeader);
    
    // Add users
    for (const auto& user : users) {
        QString userID = QString::fromStdString(user);
        if (userID != QString::fromStdString(client_->getUserID())) {
            ui->contactList->addItem("🟢 " + userID);
        }
    }
    
    // Add groups
    QListWidgetItem* groupHeader = new QListWidgetItem("GROUPS");
    groupHeader->setFlags(Qt::NoItemFlags);
    ui->contactList->addItem(groupHeader);
    
    ui->contactList->addItem("👥 general");
    ui->contactList->addItem("👥 team");
}

void ChatWindow::onDecryptClicked(MessageBubble* bubble)
{
    try {
        auto encrypted = LanShare::AESGCM::EncryptedData::deserialize(bubble->getEncryptedData());
        
        // Use same shared key derivation — will match sender's key
        auto sharedKey = getSharedKey(currentContact_);
        std::string plaintext = client_->getCrypto().decryptToString(encrypted, sharedKey);
        
        bubble->setDecryptedText(QString::fromStdString(plaintext));
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Decryption Failed", e.what());
    }
}

void ChatWindow::addMessageToChat(const QString& sender, const QString& message, bool isOwn, bool isEncrypted)
{
    MessageBubble* bubble = new MessageBubble(sender, message, isOwn, isEncrypted);
    
    if (isEncrypted) {
        connect(bubble, &MessageBubble::decryptClicked, this, &ChatWindow::onDecryptClicked);
    }
    
    chatLayout_->insertWidget(chatLayout_->count() - 1, bubble);
    currentMessages_.append(bubble);
    messageHistory_[currentContact_] = currentMessages_;
    
    scrollToBottom();
}

void ChatWindow::addEncryptedMessage(const QString& sender, const std::vector<uint8_t>& encrypted, bool isOwn)
{
    MessageBubble* bubble = new MessageBubble(sender, encrypted, isOwn);
    connect(bubble, &MessageBubble::decryptClicked, this, &ChatWindow::onDecryptClicked);
    
    chatLayout_->insertWidget(chatLayout_->count() - 1, bubble);
    currentMessages_.append(bubble);
    messageHistory_[currentContact_] = currentMessages_;
    
    scrollToBottom();
}

void ChatWindow::onCreateGroupClicked()
{
    bool ok;
    QString groupName = QInputDialog::getText(this, "Create Group", 
        "Group name:", QLineEdit::Normal, "", &ok);
    
    if (ok && !groupName.isEmpty()) {
        client_->createGroup(groupName.toStdString());
        QMessageBox::information(this, "Success", "Group created: " + groupName);
        
        // Add to list
        ui->contactList->addItem("👥 " + groupName);
    }
}

void ChatWindow::onJoinGroupClicked()
{
    bool ok;
    QString groupName = QInputDialog::getText(this, "Join Group", 
        "Group name:", QLineEdit::Normal, "", &ok);
    
    if (ok && !groupName.isEmpty()) {
        client_->joinGroup(groupName.toStdString());
        QMessageBox::information(this, "Success", "Joined group: " + groupName);
        
        // Add to list if not already there
        ui->contactList->addItem("👥 " + groupName);
    }
}

void ChatWindow::onRefreshUsersClicked()
{
    loadOnlineUsers();
}

void ChatWindow::clearChat()
{
    // Remove all message widgets
    for (auto* bubble : currentMessages_) {
        chatLayout_->removeWidget(bubble);
        bubble->hide();
    }
    currentMessages_.clear();
}

void ChatWindow::scrollToBottom()
{
    QTimer::singleShot(100, this, [this]() {
        scrollArea_->verticalScrollBar()->setValue(
            scrollArea_->verticalScrollBar()->maximum()
        );
    });
}

QString ChatWindow::getCurrentContact() const
{
    return currentContact_;
}

bool ChatWindow::isCurrentContactGroup() const
{
    return currentIsGroup_;
}
LanShare::AESGCM::Key ChatWindow::getSharedKey(const QString& contact)
{
    QString myID = QString::fromStdString(client_->getUserID());
    QStringList parts = {myID, contact};
    parts.sort();  // Sort so A→B and B→A give same key
    
    std::string sharedSecret = parts[0].toStdString() + ":" + 
                               parts[1].toStdString() + ":lanshare-v1";
    
    return LanShare::AESGCM::deriveKeyFromPassword(sharedSecret, "lanshare-salt-2024");
}