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
#include <QDebug>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
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
    initDatabase();

    // ── Message callbacks ──────────────────────────────────────────
    client_->setMessageCallback([this](const std::string& from,
                                       const std::vector<uint8_t>& encrypted,
                                       uint64_t timestamp) {
        QMetaObject::invokeMethod(this, [this, from, encrypted, timestamp]() {
            onMessageReceived(from, encrypted, timestamp);
        });
    });

    client_->setGroupMessageCallback([this](const std::string& group,
                                            const std::string& from,
                                            const std::vector<uint8_t>& encrypted,
                                            uint64_t timestamp) {
        QMetaObject::invokeMethod(this, [this, group, from, encrypted, timestamp]() {
            onGroupMessageReceived(group, from, encrypted, timestamp);
        });
    });

    client_->setUserListCallback([this](const std::vector<std::string>& users) {
        QMetaObject::invokeMethod(this, [this, users]() {
            onUserListReceived(users);
        });
    });

    // ── File transfer callbacks ────────────────────────────────────
    client_->setFileMetadataCallback([this](const std::string& from,
                                            const std::string& filename,
                                            uint64_t filesize) {
        QMetaObject::invokeMethod(this, [this, from, filename, filesize]() {
            onFileMetaReceived(from, filename, filesize);
        });
    });

    client_->setFileProgressCallback([this](const std::string& userID, int percent) {
        QMetaObject::invokeMethod(this, [this, userID, percent]() {
            onFileProgress(userID, percent);
        });
    });

    client_->setFileCompleteCallback([this](const std::string& from,
                                            const std::string& filename) {
        QMetaObject::invokeMethod(this, [this, from, filename]() {
            onFileComplete(from, filename);
        });
    });

    client_->setFileSentCallback([this](const std::string& toUserID,
                                        const std::string& filename) {
        QMetaObject::invokeMethod(this, [this, toUserID, filename]() {
            onFileSent(toUserID, filename);
        });
    });

    client_->setFileErrorCallback([this](const std::string& userID,
                                         const std::string& reason) {
        QMetaObject::invokeMethod(this, [this, userID, reason]() {
            onFileError(userID, reason);
        });
    });

    // ── Button signals ─────────────────────────────────────────────
    connect(ui->sendButton,        &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(ui->attachButton,      &QPushButton::clicked, this, &ChatWindow::onAttachClicked);
    connect(ui->contactList,       &QListWidget::itemClicked, this, &ChatWindow::onContactClicked);
    connect(ui->createGroupButton, &QPushButton::clicked, this, &ChatWindow::onCreateGroupClicked);
    connect(ui->joinGroupButton,   &QPushButton::clicked, this, &ChatWindow::onJoinGroupClicked);
    connect(ui->refreshButton,     &QPushButton::clicked, this, &ChatWindow::onRefreshUsersClicked);

    loadOnlineUsers();
    loadGroups();
}

ChatWindow::~ChatWindow() { delete ui; }

// ── Database init ──────────────────────────────────────────────────────────

void ChatWindow::initDatabase()
{
    // Store DB next to the exe
    QString dbPath = QDir::currentPath() + "/messages_" +
                     QString::fromStdString(client_->getUserID()) + ".db";

    if (!db_.open(dbPath)) {
        qWarning() << "Failed to open message database at" << dbPath;
    } else {
        qDebug() << "Message DB opened:" << dbPath;
        // Load unread counts for badge display
        unreadCounts_ = db_.getAllUnreadCounts();
    }
}

// ── Setup ──────────────────────────────────────────────────────────────────

void ChatWindow::setupUI()
{
    chatArea_   = new QWidget();
    chatLayout_ = new QVBoxLayout(chatArea_);
    chatLayout_->addStretch();
    chatLayout_->setSpacing(10);
    chatLayout_->setContentsMargins(10, 10, 10, 10);

    scrollArea_ = new QScrollArea();
    scrollArea_->setWidget(chatArea_);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->chatContainer->layout()->addWidget(scrollArea_);

    QString userID = QString::fromStdString(client_->getUserID());
    ui->userLabel->setText(QString("👤 %1").arg(userID));
}

void ChatWindow::applyStyles()
{
    QString style = R"(
        QMainWindow { background-color: #36393F; }
        QWidget#sidebar { background-color: #2F3136; }
        QLabel#userLabel {
            color: #FFFFFF; font-size: 14px; font-weight: bold;
            padding: 10px; background-color: #202225;
        }
        QLabel#chatTitleLabel {
            color: #FFFFFF; font-size: 16px; font-weight: bold;
            padding: 10px; background-color: #202225;
        }
        QListWidget {
            background-color: #2F3136; border: none;
            color: #DCDDDE; font-size: 14px; padding: 5px;
        }
        QListWidget::item { padding: 10px; border-radius: 5px; margin: 2px; }
        QListWidget::item:selected { background-color: #5865F2; color: white; }
        QListWidget::item:hover { background-color: #40444B; }
        QTextEdit {
            background-color: #40444B; border: 2px solid #202225;
            border-radius: 8px; color: #DCDDDE; padding: 10px; font-size: 14px;
        }
        QPushButton {
            background-color: #5865F2; color: white; border: none;
            border-radius: 5px; padding: 10px 20px;
            font-size: 14px; font-weight: bold;
        }
        QPushButton:hover { background-color: #4752C4; }
        QPushButton:pressed { background-color: #3C45A5; }
        QPushButton#attachButton { background-color: #3BA55D; }
        QPushButton#attachButton:hover { background-color: #2D7D46; }
        QScrollArea { background-color: #36393F; border: none; }
        QLabel#sectionLabel {
            color: #8E9297; font-size: 12px;
            font-weight: bold; padding: 15px 10px 5px 10px;
        }
    )";
    setStyleSheet(style);
}

// ── Logout ─────────────────────────────────────────────────────────────────

void ChatWindow::onLogout()
{
    db_.clearAll();
    unreadCounts_.clear();
    messageHistory_.clear();
    currentMessages_.clear();
    currentContact_.clear();
}

// ── Contact / group loading ────────────────────────────────────────────────

void ChatWindow::loadOnlineUsers() { client_->requestUserList(); }

void ChatWindow::loadGroups()
{
    ui->contactList->addItem("👥 general");
    ui->contactList->addItem("👥 team");
}

void ChatWindow::onContactClicked(QListWidgetItem* item)
{
    QString text = item->text().trimmed();
    bool isGroup = text.startsWith("👥");
    bool isUser  = text.startsWith("🟢");

    if (!isGroup && !isUser) return; // header row

    int spaceIdx = text.indexOf(' ');
    QString contactId = (spaceIdx >= 0) ? text.mid(spaceIdx + 1).trimmed() : text;

    // Strip badge suffix like " (3)" if present
    int parenIdx = contactId.lastIndexOf(" (");
    if (parenIdx >= 0) contactId = contactId.left(parenIdx).trimmed();

    switchToContact(contactId, isGroup);
}

void ChatWindow::switchToContact(const QString& contactId, bool isGroup)
{
    currentContact_ = contactId;
    currentIsGroup_ = isGroup;

    QString icon = isGroup ? "👥" : "💬";
    ui->chatTitleLabel->setText(QString("%1 %2 🔒").arg(icon, contactId));

    clearChat();

    // Mark messages as read and clear badge
    db_.markAsRead(contactId);
    unreadCounts_.remove(contactId);
    updateContactBadge(contactId);

    // Always reload fresh from DB — guarantees correct state
    // Delete old in-memory bubbles first to avoid duplicates
    if (messageHistory_.contains(contactId)) {
        for (auto* bubble : messageHistory_[contactId])
            delete bubble;
        messageHistory_.remove(contactId);
    }

    loadHistoryForContact(contactId);

    if (messageHistory_.contains(contactId)) {
        currentMessages_ = messageHistory_[contactId];
        for (auto* bubble : currentMessages_) {
            bubble->show();
            chatLayout_->insertWidget(chatLayout_->count() - 1, bubble);
        }
    }

    scrollToBottom();
}

// ── History: load from DB ──────────────────────────────────────────────────

void ChatWindow::loadHistoryForContact(const QString& contact)
{
    auto stored = db_.loadMessages(contact);
    QList<MessageBubble*> bubbles;

    for (const auto& m : stored) {
        MessageBubble* bubble = nullptr;

        if (m.isOwn) {
            // Own messages stored as plaintext
            bubble = new MessageBubble(m.sender, m.content, true, false);
        } else if (!m.encryptedBlob.isEmpty()) {
            // Received messages stored as encrypted blob
            std::vector<uint8_t> enc(
                reinterpret_cast<const uint8_t*>(m.encryptedBlob.constData()),
                reinterpret_cast<const uint8_t*>(m.encryptedBlob.constData())
                    + m.encryptedBlob.size());
            bubble = new MessageBubble(m.sender, enc, false);
            connect(bubble, &MessageBubble::decryptClicked,
                    this, &ChatWindow::onDecryptClicked);
        } else {
            // Fallback: show as plain text
            bubble = new MessageBubble(m.sender, m.content, false, false);
        }

        if (bubble) bubbles.append(bubble);
    }

    messageHistory_[contact] = bubbles;
}

// ── Save message to DB ─────────────────────────────────────────────────────

void ChatWindow::saveMessageToDB(const QString& contact,
                                 const QString& sender,
                                 const QString& content,
                                 const std::vector<uint8_t>& encryptedBlob,
                                 bool isOwn,
                                 bool isGroup)
{
    if (!db_.isOpen()) return;

    StoredMessage m;
    m.contact   = contact;
    m.sender    = sender;
    m.content   = content;
    m.timestamp = static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch());
    m.isOwn     = isOwn;
    m.isRead    = isOwn; // own messages are always "read"
    m.isGroup   = isGroup;

    if (!encryptedBlob.empty())
        m.encryptedBlob = QByteArray(
            reinterpret_cast<const char*>(encryptedBlob.data()),
            static_cast<int>(encryptedBlob.size()));

    db_.saveMessage(m);
}

// ── Unread badge helpers ───────────────────────────────────────────────────

void ChatWindow::updateContactBadge(const QString& contact)
{
    int count = unreadCounts_.value(contact, 0);

    for (int i = 0; i < ui->contactList->count(); i++) {
        QListWidgetItem* item = ui->contactList->item(i);
        QString text = item->text();

        // Extract base name (strip emoji prefix and old badge)
        QString base = text;
        if (base.startsWith("🟢 ") || base.startsWith("👥 "))
            base = base.mid(3);

        // Strip existing badge like " (3)"
        int parenIdx = base.lastIndexOf(" (");
        if (parenIdx >= 0) base = base.left(parenIdx);

        if (base.trimmed() == contact) {
            QString prefix = text.startsWith("👥") ? "👥 " : "🟢 ";
            if (count > 0)
                item->setText(QString("%1%2 (%3)").arg(prefix, contact).arg(count));
            else
                item->setText(prefix + contact);

            // Red tint for unread
            if (count > 0)
                item->setForeground(QColor("#ED4245"));
            else
                item->setForeground(QColor("#DCDDDE"));
            break;
        }
    }
}

void ChatWindow::updateAllBadges()
{
    for (auto it = unreadCounts_.begin(); it != unreadCounts_.end(); ++it)
        updateContactBadge(it.key());
}

void ChatWindow::setContactUnreadLabel(const QString& contact, int count)
{
    unreadCounts_[contact] = count;
    updateContactBadge(contact);
}

// ── Send message ───────────────────────────────────────────────────────────

void ChatWindow::onSendClicked()
{
    QString message = ui->messageInput->toPlainText().trimmed();
    if (message.isEmpty() || currentContact_.isEmpty()) return;

    try {
        auto sharedKey = getSharedKey(currentContact_);
        auto encrypted = client_->getCrypto().encrypt(message.toStdString(), sharedKey);
        auto blob = encrypted.serialize();

        if (currentIsGroup_)
            client_->sendGroupMessage(currentContact_.toStdString(), blob);
        else
            client_->sendPrivateMessage(currentContact_.toStdString(), blob);

        // Save own message as plaintext
        saveMessageToDB(currentContact_,
                        QString::fromStdString(client_->getUserID()),
                        message, {}, true, currentIsGroup_);

        addMessageToChat("You", message, true, false);
        ui->messageInput->clear();

    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString("Failed to send: %1").arg(e.what()));
    }
}

// ── File attach ────────────────────────────────────────────────────────────

void ChatWindow::onAttachClicked()
{
    if (currentContact_.isEmpty()) {
        QMessageBox::warning(this, "No Contact", "Please select a user or group first.");
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(
        this, "Select File to Send", "", "All Files (*.*)");
    if (filePath.isEmpty()) return;

    FileTransferWidget* ftWidget = new FileTransferWidget(filePath, true);
    chatLayout_->insertWidget(chatLayout_->count() - 1, ftWidget);
    currentMessages_.append(reinterpret_cast<MessageBubble*>(ftWidget));
    messageHistory_[currentContact_] = currentMessages_;

    activeTransfers_[currentContact_] = ftWidget;
    scrollToBottom();

    client_->sendFile(currentContact_.toStdString(), filePath.toStdString(), currentIsGroup_);
}

// ── File transfer callbacks ────────────────────────────────────────────────

void ChatWindow::onFileMetaReceived(const std::string& from,
                                    const std::string& filename,
                                    uint64_t filesize)
{
    QString fromQ     = QString::fromStdString(from);
    QString filenameQ = QString::fromStdString(filename);
    double  sizeMB    = filesize / (1024.0 * 1024.0);

    FileTransferWidget* ftWidget = new FileTransferWidget(filenameQ, false);

    // If sender is not current contact, still track it but don't show yet
    if (!currentIsGroup_ && fromQ != currentContact_) {
        activeTransfers_[fromQ] = ftWidget;
        return;
    }

    chatLayout_->insertWidget(chatLayout_->count() - 1, ftWidget);
    scrollToBottom();
    activeTransfers_[fromQ] = ftWidget;

    QString sizeStr = sizeMB >= 1.0
        ? QString::number(sizeMB, 'f', 1) + " MB"
        : QString::number(filesize / 1024.0, 'f', 1) + " KB";
    ftWidget->setStatusText(QString("📥 Receiving %1 (%2)...").arg(filenameQ, sizeStr));
}

void ChatWindow::onFileProgress(const std::string& userID, int percent)
{
    QString key = QString::fromStdString(userID);
    if (activeTransfers_.contains(key))
        activeTransfers_[key]->setProgress(percent);
}

void ChatWindow::onFileComplete(const std::string& from, const std::string& filename)
{
    QString key       = QString::fromStdString(from);
    QString filenameQ = QString::fromStdString(filename);

    if (activeTransfers_.contains(key)) {
        activeTransfers_[key]->setComplete();
        activeTransfers_.remove(key);
    }

    QString downloadsPath;
#ifdef _WIN32
    const char* home = getenv("USERPROFILE");
    if (home) downloadsPath = QString(home) + "\\Downloads\\" + filenameQ;
    else      downloadsPath = filenameQ;
#else
    const char* home = getenv("HOME");
    if (home) downloadsPath = QString(home) + "/Downloads/" + filenameQ;
    else      downloadsPath = filenameQ;
#endif

    QMessageBox::information(this, "✅ File Received",
        QString("Saved to:\n%1").arg(downloadsPath));
}

void ChatWindow::onFileSent(const std::string& toUserID, const std::string& filename)
{
    QString key = QString::fromStdString(toUserID);
    if (activeTransfers_.contains(key)) {
        activeTransfers_[key]->setComplete();
        activeTransfers_.remove(key);
    }
}

void ChatWindow::onFileError(const std::string& userID, const std::string& reason)
{
    QString key = QString::fromStdString(userID);
    if (activeTransfers_.contains(key)) {
        activeTransfers_[key]->setFailed(QString::fromStdString(reason));
        activeTransfers_.remove(key);
    }
    QMessageBox::warning(this, "File Transfer Failed",
        QString::fromStdString(reason));
}

// ── Message received ───────────────────────────────────────────────────────

void ChatWindow::onMessageReceived(const std::string& from,
                                   const std::vector<uint8_t>& encrypted,
                                   uint64_t timestamp)
{
    QString fromUser = QString::fromStdString(from);

    // Always save to DB
    saveMessageToDB(fromUser, fromUser, "[encrypted]",
                    encrypted, false, false);

    if (!currentIsGroup_ && fromUser == currentContact_) {
        // Chat is open — show immediately, mark read
        addEncryptedMessage(fromUser, encrypted, false);
        db_.markAsRead(fromUser);
        unreadCounts_.remove(fromUser);
        updateContactBadge(fromUser);
    } else {
        // Chat not open — increment unread badge
        int count = unreadCounts_.value(fromUser, 0) + 1;
        setContactUnreadLabel(fromUser, count);
    }
}

void ChatWindow::onGroupMessageReceived(const std::string& group,
                                        const std::string& from,
                                        const std::vector<uint8_t>& encrypted,
                                        uint64_t timestamp)
{
    QString groupName = QString::fromStdString(group);
    QString fromUser  = QString::fromStdString(from);

    // Always save to DB
    saveMessageToDB(groupName, fromUser, "[encrypted]",
                    encrypted, false, true);

    if (currentIsGroup_ && groupName == currentContact_) {
        addEncryptedMessage(fromUser, encrypted, false);
        db_.markAsRead(groupName);
        unreadCounts_.remove(groupName);
        updateContactBadge(groupName);
    } else {
        int count = unreadCounts_.value(groupName, 0) + 1;
        setContactUnreadLabel(groupName, count);
    }
}

// ── User list received ─────────────────────────────────────────────────────

void ChatWindow::onUserListReceived(const std::vector<std::string>& users)
{
    ui->contactList->clear();

    QListWidgetItem* userHeader = new QListWidgetItem("── USERS ──");
    userHeader->setFlags(Qt::NoItemFlags);
    userHeader->setForeground(QColor("#8E9297"));
    ui->contactList->addItem(userHeader);

    for (const auto& user : users) {
        QString userID = QString::fromStdString(user);
        if (userID == QString::fromStdString(client_->getUserID())) continue;

        int unread = unreadCounts_.value(userID, 0);
        QString label = unread > 0
            ? QString("🟢 %1 (%2)").arg(userID).arg(unread)
            : QString("🟢 %1").arg(userID);

        QListWidgetItem* item = new QListWidgetItem(label);
        if (unread > 0) item->setForeground(QColor("#ED4245"));
        ui->contactList->addItem(item);
    }

    QListWidgetItem* groupHeader = new QListWidgetItem("── GROUPS ──");
    groupHeader->setFlags(Qt::NoItemFlags);
    groupHeader->setForeground(QColor("#8E9297"));
    ui->contactList->addItem(groupHeader);

    ui->contactList->addItem("👥 general");
    ui->contactList->addItem("👥 team");
}

// ── Decrypt ────────────────────────────────────────────────────────────────

void ChatWindow::onDecryptClicked(MessageBubble* bubble)
{
    try {
        auto encrypted = LanShare::AESGCM::EncryptedData::deserialize(
                            bubble->getEncryptedData());
        auto sharedKey = getSharedKey(currentContact_);
        std::string plaintext = client_->getCrypto().decryptToString(encrypted, sharedKey);
        bubble->setDecryptedText(QString::fromStdString(plaintext));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Decryption Failed", e.what());
    }
}

// ── Add message to chat ────────────────────────────────────────────────────

void ChatWindow::addMessageToChat(const QString& sender, const QString& message,
                                  bool isOwn, bool isEncrypted)
{
    MessageBubble* bubble = new MessageBubble(sender, message, isOwn, isEncrypted);
    if (isEncrypted)
        connect(bubble, &MessageBubble::decryptClicked,
                this, &ChatWindow::onDecryptClicked);
    chatLayout_->insertWidget(chatLayout_->count() - 1, bubble);
    currentMessages_.append(bubble);
    messageHistory_[currentContact_] = currentMessages_;
    scrollToBottom();
}

void ChatWindow::addEncryptedMessage(const QString& sender,
                                     const std::vector<uint8_t>& encrypted,
                                     bool isOwn)
{
    MessageBubble* bubble = new MessageBubble(sender, encrypted, isOwn);
    connect(bubble, &MessageBubble::decryptClicked,
            this, &ChatWindow::onDecryptClicked);
    chatLayout_->insertWidget(chatLayout_->count() - 1, bubble);
    currentMessages_.append(bubble);
    messageHistory_[currentContact_] = currentMessages_;
    scrollToBottom();
}

// ── Group management ───────────────────────────────────────────────────────

void ChatWindow::onCreateGroupClicked()
{
    bool ok;
    QString groupName = QInputDialog::getText(this, "Create Group",
        "Group name (no + or : characters):", QLineEdit::Normal, "", &ok);
    if (!ok || groupName.isEmpty()) return;

    if (groupName.contains('+') || groupName.contains(':')) {
        QMessageBox::warning(this, "Invalid Name",
            "Group name cannot contain '+' or ':' characters.");
        return;
    }

    client_->createGroup(groupName.toStdString());
    QMessageBox::information(this, "Success", "Group created: " + groupName);
    ui->contactList->addItem("👥 " + groupName);
}

void ChatWindow::onJoinGroupClicked()
{
    bool ok;
    QString groupName = QInputDialog::getText(this, "Join Group",
        "Group name:", QLineEdit::Normal, "", &ok);
    if (ok && !groupName.isEmpty()) {
        client_->joinGroup(groupName.toStdString());
        QMessageBox::information(this, "Success", "Joined group: " + groupName);
        ui->contactList->addItem("👥 " + groupName);
    }
}

void ChatWindow::onRefreshUsersClicked() { loadOnlineUsers(); }

// ── Utilities ──────────────────────────────────────────────────────────────

void ChatWindow::clearChat()
{
    for (auto* bubble : currentMessages_) {
        chatLayout_->removeWidget(bubble);
        bubble->hide();
    }
    currentMessages_.clear();
    // Don't clear messageHistory_ — DB reload handles that
}

void ChatWindow::scrollToBottom()
{
    QTimer::singleShot(100, this, [this]() {
        scrollArea_->verticalScrollBar()->setValue(
            scrollArea_->verticalScrollBar()->maximum());
    });
}

QString ChatWindow::getCurrentContact() const { return currentContact_; }
bool ChatWindow::isCurrentContactGroup() const { return currentIsGroup_; }

LanShare::AESGCM::Key ChatWindow::getSharedKey(const QString& contact)
{
    if (currentIsGroup_) {
        std::string sharedSecret = "group:" + contact.toStdString() + ":lanshare-v1";
        return LanShare::AESGCM::deriveKeyFromPassword(sharedSecret, "lanshare-salt-2024");
    } else {
        QString myID = QString::fromStdString(client_->getUserID());
        QStringList parts = {myID, contact};
        parts.sort();
        std::string sharedSecret = parts[0].toStdString() + ":" +
                                   parts[1].toStdString() + ":lanshare-v1";
        return LanShare::AESGCM::deriveKeyFromPassword(sharedSecret, "lanshare-salt-2024");
    }
}