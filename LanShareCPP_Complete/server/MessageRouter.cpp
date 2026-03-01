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

bool MessageRouter::routePrivateMessage(const std::string& fromUserID, const std::string& toUserID,
                                       const std::vector<uint8_t>& encryptedData) {
    // Get target client
    auto targetSession = server_.getClient(toUserID);
    
    if (targetSession) {
        // User is online - deliver immediately
        std::vector<uint8_t> payload;
        payload.insert(payload.end(), fromUserID.begin(), fromUserID.end());
        payload.push_back(0); // Null terminator
        payload.insert(payload.end(), encryptedData.begin(), encryptedData.end());
        
        targetSession->sendMessage(MessageType::MSG_PRIVATE, payload);
        
        std::lock_guard<std::mutex> lock(statsMutex_);
        totalMessagesRouted_++;
        
        std::cout << "Message routed: " << fromUserID << " -> " << toUserID << "\n";
        return true;
    } else {
        // User is offline - store message
        OfflineMessage offlineMsg;
        offlineMsg.fromUserID = fromUserID;
        offlineMsg.toUserID = toUserID;
        offlineMsg.type = MessageType::MSG_PRIVATE;
        offlineMsg.payload = encryptedData;
        offlineMsg.timestamp = std::time(nullptr);
        
        storeOfflineMessage(toUserID, offlineMsg);
        
        std::cout << "Message stored offline: " << fromUserID << " -> " << toUserID << "\n";
        return true;
    }
}

bool MessageRouter::routeGroupMessage(const std::string& fromUserID, const std::string& groupName,
                                     const std::vector<uint8_t>& encryptedData) {
    // Get group members
    auto members = server_.getGroupManager().getGroupMembers(groupName);
    
    if (members.empty()) {
        return false;
    }
    
    std::vector<uint8_t> payload;
    payload.insert(payload.end(), groupName.begin(), groupName.end());
    payload.push_back(0); // Null terminator
    payload.insert(payload.end(), fromUserID.begin(), fromUserID.end());
    payload.push_back(0); // Null terminator
    payload.insert(payload.end(), encryptedData.begin(), encryptedData.end());
    
    int deliveredCount = 0;
    int offlineCount = 0;
    
    for (const auto& memberID : members) {
        // Don't send back to sender
        if (memberID == fromUserID) {
            continue;
        }
        
        auto session = server_.getClient(memberID);
        if (session) {
            session->sendMessage(MessageType::MSG_GROUP, payload);
            deliveredCount++;
        } else {
            // Store for offline member
            OfflineMessage offlineMsg;
            offlineMsg.fromUserID = fromUserID;
            offlineMsg.toUserID = memberID;
            offlineMsg.type = MessageType::MSG_GROUP;
            offlineMsg.payload = encryptedData;
            offlineMsg.timestamp = std::time(nullptr);
            
            storeOfflineMessage(memberID, offlineMsg);
            offlineCount++;
        }
    }
    
    std::lock_guard<std::mutex> lock(statsMutex_);
    totalMessagesRouted_ += deliveredCount;
    
    std::cout << "Group message routed: " << fromUserID << " -> " << groupName 
              << " (delivered: " << deliveredCount << ", offline: " << offlineCount << ")\n";
    
    return true;
}

void MessageRouter::storeOfflineMessage(const std::string& toUserID, const OfflineMessage& msg) {
    std::lock_guard<std::mutex> lock(offlineMutex_);
    offlineMessages_[toUserID].push_back(msg);
}

std::vector<OfflineMessage> MessageRouter::retrieveOfflineMessages(const std::string& userID) {
    std::lock_guard<std::mutex> lock(offlineMutex_);
    
    auto it = offlineMessages_.find(userID);
    if (it != offlineMessages_.end()) {
        std::vector<OfflineMessage> messages;
        messages.insert(messages.end(), it->second.begin(), it->second.end());
        offlineMessages_.erase(it);
        return messages;
    }
    
    return {};
}

void MessageRouter::deliverOfflineMessages(const std::string& userID) {
    auto messages = retrieveOfflineMessages(userID);
    
    if (messages.empty()) {
        return;
    }
    
    std::cout << "Delivering " << messages.size() << " offline messages to " << userID << "\n";
    
    auto session = server_.getClient(userID);
    if (!session) {
        // User already disconnected? Store them back
        std::lock_guard<std::mutex> lock(offlineMutex_);
        for (const auto& msg : messages) {
            offlineMessages_[userID].push_back(msg);
        }
        return;
    }
    
    for (const auto& msg : messages) {
        std::vector<uint8_t> payload;
        
        if (msg.type == MessageType::MSG_PRIVATE) {
            payload.insert(payload.end(), msg.fromUserID.begin(), msg.fromUserID.end());
            payload.push_back(0);
            payload.insert(payload.end(), msg.payload.begin(), msg.payload.end());
            
            session->sendMessage(MessageType::MSG_PRIVATE, payload);
        } else if (msg.type == MessageType::MSG_GROUP) {
            // TODO: Include group name in stored message
            payload.insert(payload.end(), msg.fromUserID.begin(), msg.fromUserID.end());
            payload.push_back(0);
            payload.insert(payload.end(), msg.payload.begin(), msg.payload.end());
            
            session->sendMessage(MessageType::MSG_GROUP, payload);
        }
    }
}

uint64_t MessageRouter::getTotalMessagesRouted() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return totalMessagesRouted_;
}

uint64_t MessageRouter::getOfflineMessageCount(const std::string& userID) const {
    std::lock_guard<std::mutex> lock(offlineMutex_);
    
    auto it = offlineMessages_.find(userID);
    if (it != offlineMessages_.end()) {
        return it->second.size();
    }
    return 0;
}

bool MessageRouter::routeFileMetadata(const std::string& fromUserID, const std::string& toUserID,
                                     const std::string& filename, uint64_t filesize, bool isGroup) {
    // TODO: Implement file transfer routing
    return false;
}

bool MessageRouter::routeFileChunk(const std::string& fromUserID, const std::string& toUserID,
                                  const std::vector<uint8_t>& chunk, bool isGroup) {
    // TODO: Implement file transfer routing
    return false;
}

bool MessageRouter::routeFileComplete(const std::string& fromUserID, const std::string& toUserID, bool isGroup) {
    // TODO: Implement file transfer routing
    return false;
}

} // namespace LanShare
