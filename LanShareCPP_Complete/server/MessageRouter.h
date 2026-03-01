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

// Offline message storage
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
    
    // File transfer routing
    bool routeFileMetadata(const std::string& fromUserID, const std::string& toUserID,
                          const std::string& filename, uint64_t filesize, bool isGroup);
    bool routeFileChunk(const std::string& fromUserID, const std::string& toUserID,
                       const std::vector<uint8_t>& chunk, bool isGroup);
    bool routeFileComplete(const std::string& fromUserID, const std::string& toUserID, bool isGroup);
    
    // Offline message handling
    void storeOfflineMessage(const std::string& toUserID, const OfflineMessage& msg);
    std::vector<OfflineMessage> retrieveOfflineMessages(const std::string& userID);
    void deliverOfflineMessages(const std::string& userID);
    
    // Statistics
    uint64_t getTotalMessagesRouted() const;
    uint64_t getOfflineMessageCount(const std::string& userID) const;
    
private:
    ServerCore& server_;
    
    // Offline message storage
    std::unordered_map<std::string, std::deque<OfflineMessage>> offlineMessages_;
    mutable std::mutex offlineMutex_;
    
    // Statistics
    uint64_t totalMessagesRouted_;
    mutable std::mutex statsMutex_;
};

} // namespace LanShare

#endif // MESSAGEROUTER_H
