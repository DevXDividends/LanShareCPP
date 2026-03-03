#ifndef MESSAGEROUTER_H
#define MESSAGEROUTER_H

#include "../common/Protocol.h"
#include <string>
#include <vector>
#include <memory>
#include <deque>
#include <unordered_map>
#include <mutex>

namespace LanShare {

class ServerCore;

struct OfflineMessage {
    std::string fromUserID;
    std::string toUserID;
    MessageType type;
    std::vector<uint8_t> payload;
    uint64_t timestamp;
};

class MessageRouter {
public:
    explicit MessageRouter(ServerCore& server);
    ~MessageRouter();

    // Message routing
    bool routePrivateMessage(const std::string& fromUserID, const std::string& toUserID,
                             const std::vector<uint8_t>& encryptedData);
    bool routeGroupMessage(const std::string& fromUserID, const std::string& groupName,
                           const std::vector<uint8_t>& encryptedData);

    // File transfer routing — old stubs kept for compatibility
    bool routeFileMetadata(const std::string& fromUserID, const std::string& toUserID,
                           const std::string& filename, uint64_t filesize, bool isGroup);
    bool routeFileChunk(const std::string& fromUserID, const std::string& toUserID,
                        const std::vector<uint8_t>& chunk, bool isGroup);
    bool routeFileComplete(const std::string& fromUserID, const std::string& toUserID,
                           bool isGroup);

    // ★ NEW: raw payload forwarding — used by ClientSession for all file types
    bool routeFileRaw(const std::string& fromUserID,
                      const std::vector<uint8_t>& payload,
                      MessageType type);

    // Offline messages
    void storeOfflineMessage(const std::string& toUserID, const OfflineMessage& msg);
    std::vector<OfflineMessage> retrieveOfflineMessages(const std::string& userID);
    void deliverOfflineMessages(const std::string& userID);

    // Stats
    uint64_t getTotalMessagesRouted() const;
    uint64_t getOfflineMessageCount(const std::string& userID) const;

private:
    bool routeFileRawGroup(const std::string& fromUserID,
                           const std::string& groupName,
                           const std::vector<uint8_t>& rest,
                           MessageType type);

    ServerCore& server_;
    std::unordered_map<std::string, std::deque<OfflineMessage>> offlineMessages_;
    mutable std::mutex offlineMutex_;
    uint64_t totalMessagesRouted_;
    mutable std::mutex statsMutex_;
};

} // namespace LanShare

#endif // MESSAGEROUTER_H