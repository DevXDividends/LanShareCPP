#ifndef MESSAGEDB_H
#define MESSAGEDB_H

#include <QString>
#include <QList>
#include <QMap>
#include <cstdint>

// Forward declare sqlite3 so we don't need sqlite3.h in header
struct sqlite3;

struct StoredMessage {
    int64_t  id;
    QString  contact;    // userID or groupName
    QString  sender;     // who sent it
    QString  content;    // plaintext if own, "[encrypted]" if received
    QByteArray encryptedBlob; // raw encrypted bytes (empty for own messages)
    uint64_t timestamp;
    bool     isOwn;
    bool     isRead;
    bool     isGroup;
};

class MessageDB
{
public:
    MessageDB();
    ~MessageDB();

    // Opens (or creates) the database at given path
    bool open(const QString& path);
    void close();
    bool isOpen() const;

    // Save a message
    bool saveMessage(const StoredMessage& msg);

    // Load all messages for a contact
    QList<StoredMessage> loadMessages(const QString& contact);

    // Unread count for a contact
    int getUnreadCount(const QString& contact);

    // Get all contacts that have unread messages
    QMap<QString, int> getAllUnreadCounts();

    // Mark all messages from a contact as read
    void markAsRead(const QString& contact);

    // Clear all history (on logout)
    void clearAll();

private:
    bool createTables();

    sqlite3* db_ = nullptr;
};

#endif // MESSAGEDB_H