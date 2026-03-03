#ifndef CLIENTCORE_H
#define CLIENTCORE_H

#include "../common/Protocol.h"
#include "../common/AESGCM.h"
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <functional>
#include <deque>
#include <mutex>
#include <atomic>
#include <optional>

namespace LanShare {

// Callback types
using MessageCallback = std::function<void(const std::string& fromUserID, const std::vector<uint8_t>& encryptedData, uint64_t timestamp)>;
using GroupMessageCallback = std::function<void(const std::string& groupName, const std::string& fromUserID, const std::vector<uint8_t>& encryptedData, uint64_t timestamp)>;
using FileMetadataCallback = std::function<void(const std::string& fromUserID, const std::string& filename, uint64_t filesize)>;
using FileChunkCallback = std::function<void(const std::string& fromUserID, const std::vector<uint8_t>& chunk, uint64_t offset)>;
using FileCompleteCallback = std::function<void(const std::string& fromUserID, const std::string& filename)>;
using UserListCallback = std::function<void(const std::vector<std::string>& users)>;
using ConnectionCallback = std::function<void(bool connected, const std::string& reason)>;
using AuthCallback = std::function<void(bool success, const std::string& userID, const std::string& message)>;

class ClientCore {
public:
    explicit ClientCore();
    ~ClientCore();
    
    // Connection management
    void connect(const std::string& serverIP, unsigned short port = 5555);
    void disconnect();
    bool isConnected() const;
    
    // Authentication
    void registerUser(const std::string& username, const std::string& password);
    void login(const std::string& username, const std::string& password);
    void logout();
    
    // User info
    std::string getUserID() const;
    std::string getUsername() const;
    bool isAuthenticated() const;
    
    // Messaging
    void sendPrivateMessage(const std::string& toUserID, const std::vector<uint8_t>& encryptedData);
    void sendGroupMessage(const std::string& groupName, const std::vector<uint8_t>& encryptedData);
    
    // File transfer
    void sendFile(const std::string& toUserID, const std::string& filepath, bool isGroup = false);
    void sendFileChunk(const std::string& toUserID, const std::vector<uint8_t>& chunk, bool isGroup = false);
    void sendFileComplete(const std::string& toUserID, bool isGroup = false);
    
    // Group management
    void createGroup(const std::string& groupName);
    void joinGroup(const std::string& groupName);
    void leaveGroup(const std::string& groupName);
    void requestGroupList();
    
    // User management
    void requestUserList();
    
    // Cryptography
    AESGCM& getCrypto();
    void setEncryptionKey(const AESGCM::Key& key);
    AESGCM::Key getEncryptionKey() const;
    
    // Callbacks
    void setMessageCallback(MessageCallback callback);
    void setGroupMessageCallback(GroupMessageCallback callback);
    void setFileMetadataCallback(FileMetadataCallback callback);
    void setFileChunkCallback(FileChunkCallback callback);
    void setFileCompleteCallback(FileCompleteCallback callback);
    void setUserListCallback(UserListCallback callback);
    void setConnectionCallback(ConnectionCallback callback);
    void setAuthCallback(AuthCallback callback);
    
    // Run the client (blocking)
    void run();
    
    // Run in background thread
    void startAsync();
    void stopAsync();
    
private:
    void readHeader();
    void readPayload(uint32_t payloadLength, MessageType type);
    void handleMessage(MessageType type, const std::vector<uint8_t>& payload);
    
    void sendMessage(MessageType type, const std::vector<uint8_t>& payload);
    void sendMessage(MessageType type, const std::string& payload);
    void writeNext();
    
    void handleError(const std::string& context, const boost::system::error_code& error);
    
    // Message handlers
    void handleAuthSuccess(const std::vector<uint8_t>& payload);
    void handleAuthFail(const std::vector<uint8_t>& payload);
    void handlePrivateMessage(const std::vector<uint8_t>& payload);
    void handleGroupMessage(const std::vector<uint8_t>& payload);
    void handleFileMetadata(const std::vector<uint8_t>& payload);
    void handleFileChunk(const std::vector<uint8_t>& payload);
    void handleFileComplete(const std::vector<uint8_t>& payload);
    void handleUserList(const std::vector<uint8_t>& payload);
    void handlePong();
    void handleError(const std::vector<uint8_t>& payload);
    
    boost::asio::io_context ioContext_;
    boost::asio::ip::tcp::socket socket_;
    std::unique_ptr<std::thread> ioThread_;

      // ADD THIS LINE RIGHT HERE ↓
std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> workGuard_;


    std::string userID_;
    std::string username_;
    std::atomic<bool> authenticated_;
    std::atomic<bool> connected_;
    
    // Cryptography
    AESGCM crypto_;
    AESGCM::Key encryptionKey_;
    mutable std::mutex keyMutex_;
    
    // Read buffer
    MessageHeader readHeader_;
    std::vector<uint8_t> readBuffer_;
    
    // Write queue
    struct OutgoingMessage {
        MessageHeader header;
        std::vector<uint8_t> data;
    };
    std::deque<OutgoingMessage> writeQueue_;
    std::mutex writeMutex_;
    bool writing_;
    
    // Callbacks
    MessageCallback messageCallback_;
    GroupMessageCallback groupMessageCallback_;
    FileMetadataCallback fileMetadataCallback_;
    FileChunkCallback fileChunkCallback_;
    FileCompleteCallback fileCompleteCallback_;
    UserListCallback userListCallback_;
    ConnectionCallback connectionCallback_;
    AuthCallback authCallback_;
    std::mutex callbackMutex_;
};

} // namespace LanShare

#endif // CLIENTCORE_H
