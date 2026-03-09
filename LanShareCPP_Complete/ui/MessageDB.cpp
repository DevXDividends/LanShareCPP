#include "MessageDB.h"
#include <sqlite3.h>
#include <QDebug>
#include <QByteArray>

MessageDB::MessageDB() : db_(nullptr) {}

MessageDB::~MessageDB() { close(); }

bool MessageDB::open(const QString &path)
{
    int rc = sqlite3_open(path.toUtf8().constData(), &db_);
    if (rc != SQLITE_OK)
    {
        qWarning() << "Cannot open DB:" << sqlite3_errmsg(db_);
        db_ = nullptr;
        return false;
    }
    return createTables();
}

void MessageDB::close()
{
    if (db_)
    {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool MessageDB::isOpen() const { return db_ != nullptr; }

bool MessageDB::createTables()
{
    const char *sql = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            contact         TEXT    NOT NULL,
            sender          TEXT    NOT NULL,
            content         TEXT    NOT NULL DEFAULT '',
            encrypted_blob  BLOB,
            timestamp       INTEGER NOT NULL,
            is_own          INTEGER NOT NULL DEFAULT 0,
            is_read         INTEGER NOT NULL DEFAULT 0,
            is_group        INTEGER NOT NULL DEFAULT 0
        );
        CREATE INDEX IF NOT EXISTS idx_contact ON messages(contact);
        CREATE INDEX IF NOT EXISTS idx_unread  ON messages(contact, is_read);
    )";

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        qWarning() << "Create table failed:" << errMsg;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool MessageDB::saveMessage(const StoredMessage &msg)
{
    if (!db_)
        return false;

    const char *sql = R"(
        INSERT INTO messages
            (contact, sender, content, encrypted_blob, timestamp, is_own, is_read, is_group)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, msg.contact.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, msg.sender.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, msg.content.toUtf8().constData(), -1, SQLITE_TRANSIENT);

    if (!msg.encryptedBlob.isEmpty())
        sqlite3_bind_blob(stmt, 4, msg.encryptedBlob.constData(),
                          msg.encryptedBlob.size(), SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, 4);

    sqlite3_bind_int64(stmt, 5, static_cast<sqlite3_int64>(msg.timestamp));
    sqlite3_bind_int(stmt, 6, msg.isOwn ? 1 : 0);
    sqlite3_bind_int(stmt, 7, msg.isRead ? 1 : 0);
    sqlite3_bind_int(stmt, 8, msg.isGroup ? 1 : 0);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

QList<StoredMessage> MessageDB::loadMessages(const QString &contact)
{
    QList<StoredMessage> result;
    if (!db_)
        return result;

    const char *sql = R"(
        SELECT id, contact, sender, content, encrypted_blob,
               timestamp, is_own, is_read, is_group
        FROM messages
        WHERE contact = ?
        ORDER BY id ASC;
    )";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return result;

    sqlite3_bind_text(stmt, 1, contact.toUtf8().constData(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        StoredMessage m;
        m.id = sqlite3_column_int64(stmt, 0);
        m.contact = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
        m.sender = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)));
        m.content = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3)));

        // encrypted blob (may be NULL)
        const void *blob = sqlite3_column_blob(stmt, 4);
        int blobSize = sqlite3_column_bytes(stmt, 4);
        if (blob && blobSize > 0)
            m.encryptedBlob = QByteArray(static_cast<const char *>(blob), blobSize);

        m.timestamp = static_cast<uint64_t>(sqlite3_column_int64(stmt, 5));
        m.isOwn = sqlite3_column_int(stmt, 6) != 0;
        m.isRead = sqlite3_column_int(stmt, 7) != 0;
        m.isGroup = sqlite3_column_int(stmt, 8) != 0;

        result.append(m);
    }

    sqlite3_finalize(stmt);
    return result;
}

int MessageDB::getUnreadCount(const QString &contact)
{
    if (!db_)
        return 0;

    const char *sql = R"(
        SELECT COUNT(*) FROM messages
        WHERE contact = ? AND is_read = 0 AND is_own = 0;
    )";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return 0;

    sqlite3_bind_text(stmt, 1, contact.toUtf8().constData(), -1, SQLITE_TRANSIENT);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return count;
}

QMap<QString, int> MessageDB::getAllUnreadCounts()
{
    QMap<QString, int> result;
    if (!db_)
        return result;

    const char *sql = R"(
        SELECT contact, COUNT(*) FROM messages
        WHERE is_read = 0 AND is_own = 0
        GROUP BY contact;
    )";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return result;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        QString contact = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
        int count = sqlite3_column_int(stmt, 1);
        result[contact] = count;
    }

    sqlite3_finalize(stmt);
    return result;
}

void MessageDB::markAsRead(const QString &contact)
{
    if (!db_)
        return;

    const char *sql = "UPDATE messages SET is_read = 1 WHERE contact = ? AND is_own = 0;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return;

    sqlite3_bind_text(stmt, 1, contact.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void MessageDB::clearAll()
{
    if (!db_)
        return;
    sqlite3_exec(db_, "DELETE FROM messages;", nullptr, nullptr, nullptr);
}
void MessageDB::clearAllMessages()
{
    const char *sql = "DELETE FROM messages;";
    char *errMsg = nullptr;
    sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
    if (errMsg)
        sqlite3_free(errMsg);
}