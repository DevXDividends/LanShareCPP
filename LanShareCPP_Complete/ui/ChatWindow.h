#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QString>
#include <QMap>
#include <vector>

#include "../client/ClientCore.h"

namespace Ui { class ChatWindow; }

class MessageBubble;
class FileTransferWidget;

class ChatWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatWindow(LanShare::ClientCore* client, QWidget *parent = nullptr);
    ~ChatWindow();

private slots:
    void onSendClicked();
    void onAttachClicked();
    void onContactClicked(QListWidgetItem* item);
    void onCreateGroupClicked();
    void onJoinGroupClicked();
    void onRefreshUsersClicked();

    // Message callbacks (invoked on UI thread)
    void onMessageReceived(const std::string& from,
                           const std::vector<uint8_t>& encrypted,
                           uint64_t timestamp);
    void onGroupMessageReceived(const std::string& group,
                                const std::string& from,
                                const std::vector<uint8_t>& encrypted,
                                uint64_t timestamp);
    void onUserListReceived(const std::vector<std::string>& users);
    void onDecryptClicked(MessageBubble* bubble);

    // File transfer callbacks (invoked on UI thread)
    void onFileMetaReceived(const std::string& from,
                            const std::string& filename,
                            uint64_t filesize);
    void onFileProgress(const std::string& userID, int percent);
    void onFileComplete(const std::string& from, const std::string& filename);
    void onFileSent(const std::string& toUserID, const std::string& filename);
    void onFileError(const std::string& userID, const std::string& reason);

private:
    void setupUI();
    void applyStyles();
    void loadOnlineUsers();
    void loadGroups();
    void addMessageToChat(const QString& sender, const QString& message,
                          bool isOwn, bool isEncrypted);
    void addEncryptedMessage(const QString& sender,
                             const std::vector<uint8_t>& encrypted,
                             bool isOwn);
    void switchToContact(const QString& contactId, bool isGroup);
    void clearChat();
    void scrollToBottom();
    QString getCurrentContact() const;
    bool isCurrentContactGroup() const;
    LanShare::AESGCM::Key getSharedKey(const QString& contact);

    Ui::ChatWindow* ui;
    LanShare::ClientCore* client_;

    QString currentContact_;
    bool currentIsGroup_;

    QMap<QString, QList<MessageBubble*>> messageHistory_;
    QList<MessageBubble*> currentMessages_;

    // Active file transfer widgets keyed by contact/userID
    QMap<QString, FileTransferWidget*> activeTransfers_;

    QWidget*     chatArea_;
    QVBoxLayout* chatLayout_;
    QScrollArea* scrollArea_;
};

#endif // CHATWINDOW_H