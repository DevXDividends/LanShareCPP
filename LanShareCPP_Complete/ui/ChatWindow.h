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

namespace Ui {
class ChatWindow;
}

class MessageBubble;

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
    void onMessageReceived(const std::string& from, const std::vector<uint8_t>& encrypted, uint64_t timestamp);
    void onGroupMessageReceived(const std::string& group, const std::string& from, 
                                const std::vector<uint8_t>& encrypted, uint64_t timestamp);
    void onUserListReceived(const std::vector<std::string>& users);
    void onDecryptClicked(MessageBubble* bubble);

private:
    void setupUI();
    void applyStyles();
    void loadOnlineUsers();
    void loadGroups();
    void addMessageToChat(const QString& sender, const QString& message, bool isOwn, bool isEncrypted);
    void addEncryptedMessage(const QString& sender, const std::vector<uint8_t>& encrypted, bool isOwn);
    void switchToContact(const QString& contactId, bool isGroup);
    void clearChat();
    void scrollToBottom();
    QString getCurrentContact() const;
    bool isCurrentContactGroup() const;

    Ui::ChatWindow *ui;
    LanShare::ClientCore* client_;
    
    QString currentContact_;
    bool currentIsGroup_;
    
    QMap<QString, QList<MessageBubble*>> messageHistory_;
    QList<MessageBubble*> currentMessages_;
    
    // UI elements (created in code, not .ui file)
    QWidget* chatArea_;
    QVBoxLayout* chatLayout_;
    QScrollArea* scrollArea_;
};

#endif // CHATWINDOW_H
