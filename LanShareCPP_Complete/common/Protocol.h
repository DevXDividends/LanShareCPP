#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <cstdint>

namespace LanShare {

constexpr uint32_t PROTOCOL_VERSION  = 1;
constexpr uint32_t MAX_MESSAGE_SIZE  = 100 * 1024 * 1024; // 100MB
constexpr uint32_t CHUNK_SIZE        = 64 * 1024;         // 64KB

enum class MessageType : uint8_t {
    // Authentication
    AUTH_REGISTER   = 0x01,
    AUTH_LOGIN      = 0x02,
    AUTH_SUCCESS    = 0x03,
    AUTH_FAIL       = 0x04,
    AUTH_LOGOUT     = 0x05,

    // Private Messaging
    MSG_PRIVATE     = 0x10,
    MSG_PRIVATE_ACK = 0x11,

    // Group Messaging
    MSG_GROUP       = 0x20,
    MSG_GROUP_ACK   = 0x21,

    // File Transfer
    FILE_PRIVATE_META     = 0x30,
    FILE_PRIVATE_CHUNK    = 0x31,
    FILE_PRIVATE_COMPLETE = 0x32,
    FILE_GROUP_META       = 0x33,
    FILE_GROUP_CHUNK      = 0x34,
    FILE_GROUP_COMPLETE   = 0x35,

    // Group Management
    GROUP_CREATE    = 0x40,
    GROUP_JOIN      = 0x41,
    GROUP_LEAVE     = 0x42,
    GROUP_LIST      = 0x43,
    GROUP_MEMBERS   = 0x44,
    GROUP_INFO      = 0x45,
    GROUP_CODE      = 0x46,  // ★ Server → admin: "SHARK-42"

    // User Management
    USER_LIST       = 0x50,
    USER_STATUS     = 0x51,
    USER_INFO       = 0x52,

    // System
    PING            = 0x60,
    PONG            = 0x61,
    ERROR_MSG       = 0x62,
    DISCONNECT      = 0x63
};

#pragma pack(push, 1)
struct MessageHeader {
    uint32_t    length;
    MessageType type;
};
#pragma pack(pop)

inline std::string createUserID(const std::string& username, const std::string& hash) {
    return "LS-" + username + "-" + hash.substr(0, 4);
}

inline bool isValidUserID(const std::string& userID) {
    if (userID.length() < 8)            return false;
    if (userID.substr(0, 3) != "LS-")   return false;
    size_t lastDash = userID.rfind('-');
    if (lastDash == std::string::npos || lastDash < 3) return false;
    return true;
}

constexpr char FIELD_SEPARATOR = ':';
constexpr char MSG_TERMINATOR  = '\n';

enum class ErrorCode : uint8_t {
    SUCCESS                  = 0,
    INVALID_CREDENTIALS      = 1,
    USER_ALREADY_EXISTS      = 2,
    USER_NOT_FOUND           = 3,
    INVALID_MESSAGE_FORMAT   = 4,
    ENCRYPTION_ERROR         = 5,
    DECRYPTION_ERROR         = 6,
    FILE_TRANSFER_ERROR      = 7,
    GROUP_NOT_FOUND          = 8,
    GROUP_ALREADY_EXISTS     = 9,
    PERMISSION_DENIED        = 10,
    SERVER_ERROR             = 11,
    UNKNOWN_ERROR            = 255
};

} // namespace LanShare

#endif // PROTOCOL_H