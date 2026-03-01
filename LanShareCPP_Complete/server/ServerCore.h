#ifndef SERVERCORE_H
#define SERVERCORE_H

#include "../common/Protocol.h"
#include <boost/asio.hpp>
#include <memory>
#include <unordered_map>
#include <set>
#include <mutex>
#include <string>

namespace LanShare {

class ClientSession;
class AuthManager;
class GroupManager;
class MessageRouter;

class ServerCore {
public:
    explicit ServerCore(unsigned short port = 5555);
    ~ServerCore();
    
    // Server lifecycle
    void start();
    void stop();
    bool isRunning() const;
    
    // Client management
    void registerClient(const std::string& userID, std::shared_ptr<ClientSession> session);
    void unregisterClient(const std::string& userID);
    std::shared_ptr<ClientSession> getClient(const std::string& userID);
    std::vector<std::string> getOnlineUsers() const;
    
    // Message routing
    void routePrivateMessage(const std::string& fromUserID, const std::string& toUserID, 
                            const std::vector<uint8_t>& encryptedData);
    void routeGroupMessage(const std::string& fromUserID, const std::string& groupName,
                          const std::vector<uint8_t>& encryptedData);
    
    // File transfer routing
    void routeFileMetadata(const std::string& fromUserID, const std::string& toUserID,
                          const std::string& filename, uint64_t filesize, bool isGroup = false);
    void routeFileChunk(const std::string& fromUserID, const std::string& toUserID,
                       const std::vector<uint8_t>& chunk, bool isGroup = false);
    void routeFileComplete(const std::string& fromUserID, const std::string& toUserID, bool isGroup = false);
    
    // Accessors
    AuthManager& getAuthManager();
    GroupManager& getGroupManager();
    MessageRouter& getMessageRouter();
    boost::asio::io_context& getIOContext();
    
private:
    void acceptConnections();
    void handleAccept(std::shared_ptr<ClientSession> session, const boost::system::error_code& error);
    
    unsigned short port_;
    boost::asio::io_context ioContext_;
    boost::asio::ip::tcp::acceptor acceptor_;
    
    std::unique_ptr<AuthManager> authManager_;
    std::unique_ptr<GroupManager> groupManager_;
    std::unique_ptr<MessageRouter> messageRouter_;
    
    std::unordered_map<std::string, std::shared_ptr<ClientSession>> clients_;
    mutable std::mutex clientsMutex_;
    
    bool running_;
};

} // namespace LanShare

#endif // SERVERCORE_H
