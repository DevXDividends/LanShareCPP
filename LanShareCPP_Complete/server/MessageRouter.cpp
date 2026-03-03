#include "MessageRouter.h"
#include "GroupManager.h"
#include "ServerCore.h"
#include "ClientSession.h"
#include <iostream>
#include <ctime>

namespace LanShare {

MessageRouter::MessageRouter(ServerCore& server)
    : server_(server)
    , totalMessagesRouted_(0)
{
}

MessageRouter::~MessageRouter() = default;

bool MessageRouter::routePrivateMessage(const std::string& fromUserID,
                                        const std::string& toUserID,
                                        const std::vector<uint8_t>& encryptedData) {
    auto targetSession = server_.getClient(toUserID);

    if (targetSession) {
        std::vector<uint8_t> payload;
        payload.insert(payload.end(), fromUserID.begin(), fromUserID.end());
        payload.push_back(0);
        payload.insert(payload.end(), encryptedData.begin(), encryptedData.end());
        targetSession->sendMessage(MessageType::MSG_PRIVATE, payload);

        std::lock_guard<std::mutex> lock(statsMutex_);
        totalMessagesRouted_++;
        std::cout << "MSG routed: " << fromUserID << " -> " << toUserID << "\n";
        return true;
    } else {
        OfflineMessage msg;
        msg.fromUserID = fromUserID;
        msg.toUserID   = toUserID;
        msg.type       = MessageType::MSG_PRIVATE;
        msg.payload    = encryptedData;
        msg.timestamp  = std::time(nullptr);
        storeOfflineMessage(toUserID, msg);
        std::cout << "MSG stored offline: " << fromUserID << " -> " << toUserID << "\n";
        return true;
    }
}

bool MessageRouter::routeGroupMessage(const std::string& fromUserID,
                                      const std::string& groupName,
                                      const std::vector<uint8_t>& encryptedData) {
    auto members = server_.getGroupManager().getGroupMembers(groupName);
    if (members.empty()) return false;

    std::vector<uint8_t> payload;
    payload.insert(payload.end(), groupName.begin(), groupName.end());
    payload.push_back(0);
    payload.insert(payload.end(), fromUserID.begin(), fromUserID.end());
    payload.push_back(0);
    payload.insert(payload.end(), encryptedData.begin(), encryptedData.end());

    int delivered = 0, offline = 0;
    for (const auto& memberID : members) {
        if (memberID == fromUserID) continue;
        auto session = server_.getClient(memberID);
        if (session) {
            session->sendMessage(MessageType::MSG_GROUP, payload);
            delivered++;
        } else {
            OfflineMessage msg;
            msg.fromUserID = fromUserID;
            msg.toUserID   = memberID;
            msg.type       = MessageType::MSG_GROUP;
            msg.payload    = encryptedData;
            msg.timestamp  = std::time(nullptr);
            storeOfflineMessage(memberID, msg);
            offline++;
        }
    }

    std::lock_guard<std::mutex> lock(statsMutex_);
    totalMessagesRouted_ += delivered;
    std::cout << "GROUP routed: " << fromUserID << " -> " << groupName
              << " (delivered:" << delivered << " offline:" << offline << ")\n";
    return true;
}

// ─────────────────────────────────────────────────────────────────────────
//  FILE TRANSFER ROUTING
//  Strategy: the payload already contains the target userID as first field
//  (toUserID\0 ...). We just need to:
//    1. Extract toUserID from payload
//    2. Prepend fromUserID so receiver knows who sent it
//    3. Forward to target session
// ─────────────────────────────────────────────────────────────────────────

// Helper: extract first null-terminated string from payload
static std::string extractFirst(const std::vector<uint8_t>& payload, size_t& afterPos) {
    size_t i = 0;
    while (i < payload.size() && payload[i] != 0) i++;
    std::string s(payload.begin(), payload.begin() + i);
    afterPos = i + 1; // position after the null byte
    return s;
}

bool MessageRouter::routeFileMetadata(const std::string& fromUserID,
                                      const std::string& toUserID,
                                      const std::string& filename,
                                      uint64_t filesize,
                                      bool isGroup) {
    // Not used directly — routing is done via routeFileRaw
    return false;
}

bool MessageRouter::routeFileChunk(const std::string& fromUserID,
                                   const std::string& toUserID,
                                   const std::vector<uint8_t>& chunk,
                                   bool isGroup) {
    return false;
}

bool MessageRouter::routeFileComplete(const std::string& fromUserID,
                                      const std::string& toUserID,
                                      bool isGroup) {
    return false;
}

// ── The real routing function used by ClientSession ────────────────────────
// payload format from client: toUserID\0 + rest
// We rewrite it to:           fromUserID\0 + rest   (receiver needs sender ID)
bool MessageRouter::routeFileRaw(const std::string& fromUserID,
                                 const std::vector<uint8_t>& payload,
                                 MessageType type) {
    // Extract toUserID from start of payload
    size_t afterTo = 0;
    std::string toUserID = extractFirst(payload, afterTo);
    // rest = everything after toUserID\0
    std::vector<uint8_t> rest(payload.begin() + afterTo, payload.end());

    if (type == MessageType::FILE_GROUP_META   ||
        type == MessageType::FILE_GROUP_CHUNK  ||
        type == MessageType::FILE_GROUP_COMPLETE) {
        // toUserID is actually groupName for group file transfers
        return routeFileRawGroup(fromUserID, toUserID, rest, type);
    }

    // Private file transfer
    auto targetSession = server_.getClient(toUserID);
    if (!targetSession) {
        std::cout << "File route: target offline: " << toUserID << "\n";
        return false;
    }

    // Rebuild payload: fromUserID\0 + rest
    std::vector<uint8_t> fwdPayload;
    fwdPayload.insert(fwdPayload.end(), fromUserID.begin(), fromUserID.end());
    fwdPayload.push_back(0);
    fwdPayload.insert(fwdPayload.end(), rest.begin(), rest.end());

    targetSession->sendMessage(type, fwdPayload);

    std::cout << "File chunk routed: " << fromUserID << " -> " << toUserID
              << " type=" << static_cast<int>(type) << "\n";
    return true;
}

bool MessageRouter::routeFileRawGroup(const std::string& fromUserID,
                                      const std::string& groupName,
                                      const std::vector<uint8_t>& rest,
                                      MessageType type) {
    auto members = server_.getGroupManager().getGroupMembers(groupName);
    if (members.empty()) return false;

    // Rebuild: fromUserID\0 + rest
    std::vector<uint8_t> fwdPayload;
    fwdPayload.insert(fwdPayload.end(), fromUserID.begin(), fromUserID.end());
    fwdPayload.push_back(0);
    fwdPayload.insert(fwdPayload.end(), rest.begin(), rest.end());

    for (const auto& memberID : members) {
        if (memberID == fromUserID) continue;
        auto session = server_.getClient(memberID);
        if (session) session->sendMessage(type, fwdPayload);
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────
//  OFFLINE MESSAGES
// ─────────────────────────────────────────────────────────────────────────

void MessageRouter::storeOfflineMessage(const std::string& toUserID,
                                        const OfflineMessage& msg) {
    std::lock_guard<std::mutex> lock(offlineMutex_);
    offlineMessages_[toUserID].push_back(msg);
}

std::vector<OfflineMessage> MessageRouter::retrieveOfflineMessages(
    const std::string& userID) {
    std::lock_guard<std::mutex> lock(offlineMutex_);
    auto it = offlineMessages_.find(userID);
    if (it != offlineMessages_.end()) {
        std::vector<OfflineMessage> msgs(it->second.begin(), it->second.end());
        offlineMessages_.erase(it);
        return msgs;
    }
    return {};
}

void MessageRouter::deliverOfflineMessages(const std::string& userID) {
    auto messages = retrieveOfflineMessages(userID);
    if (messages.empty()) return;

    std::cout << "Delivering " << messages.size()
              << " offline messages to " << userID << "\n";

    auto session = server_.getClient(userID);
    if (!session) {
        std::lock_guard<std::mutex> lock(offlineMutex_);
        for (const auto& msg : messages)
            offlineMessages_[userID].push_back(msg);
        return;
    }

    for (const auto& msg : messages) {
        std::vector<uint8_t> payload;
        payload.insert(payload.end(), msg.fromUserID.begin(), msg.fromUserID.end());
        payload.push_back(0);
        payload.insert(payload.end(), msg.payload.begin(), msg.payload.end());
        session->sendMessage(msg.type, payload);
    }
}

uint64_t MessageRouter::getTotalMessagesRouted() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return totalMessagesRouted_;
}

uint64_t MessageRouter::getOfflineMessageCount(const std::string& userID) const {
    std::lock_guard<std::mutex> lock(offlineMutex_);
    auto it = offlineMessages_.find(userID);
    return (it != offlineMessages_.end()) ? it->second.size() : 0;
}

} // namespace LanShare