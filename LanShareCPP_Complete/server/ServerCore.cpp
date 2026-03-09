#include "ServerCore.h"
#include "ClientSession.h"
#include "AuthManager.h"
#include "GroupManager.h"
#include "MessageRouter.h"
#include <iostream>
#include <cstdio>

namespace LanShare {

ServerCore::ServerCore(unsigned short port)
    : port_(port)
    , ioContext_()
    , acceptor_(ioContext_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    , running_(false)
{
    authManager_ = std::make_unique<AuthManager>();
    groupManager_ = std::make_unique<GroupManager>();
    messageRouter_ = std::make_unique<MessageRouter>(*this);
    
    // Wipe groups on every server start — fresh session every time
    std::remove("groups.db");
    std::cout << "Groups cleared (fresh session)\n";

    // Load only users — accounts persist across restarts
    try {
        authManager_->loadFromFile("users.db");
        std::cout << "Loaded user database\n";
    } catch (const std::exception& e) {
        std::cout << "Note: " << e.what() << " (this is OK for first run)\n";
    }
}

ServerCore::~ServerCore() {
    stop();
}

void ServerCore::start() {
    std::cout << "Starting server on port " << port_ << "...\n";
    running_ = true;
    acceptConnections();
    ioContext_.run();
}

void ServerCore::stop() {
    if (!running_) return;
    
    std::cout << "Stopping server...\n";
    running_ = false;
    
    // Save only users — groups are intentionally not saved
    try {
        authManager_->saveToFile("users.db");
        std::cout << "Saved user database\n";
    } catch (const std::exception& e) {
        std::cerr << "Error saving data: " << e.what() << "\n";
    }
    
    ioContext_.stop();
}

bool ServerCore::isRunning() const {
    return running_;
}

void ServerCore::acceptConnections() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioContext_);
    
    acceptor_.async_accept(*socket,
        [this, socket](const boost::system::error_code& error) {
            if (!error) {
                auto session = std::make_shared<ClientSession>(std::move(*socket), *this);
                session->start();
            }
            
            if (running_) {
                acceptConnections();
            }
        });
}

void ServerCore::registerClient(const std::string& userID, std::shared_ptr<ClientSession> session) {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    clients_[userID] = session;
    std::cout << "Client registered: " << userID << " (Total: " << clients_.size() << ")\n";
}

void ServerCore::unregisterClient(const std::string& userID) {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    clients_.erase(userID);
    std::cout << "Client unregistered: " << userID << " (Total: " << clients_.size() << ")\n";
}

std::shared_ptr<ClientSession> ServerCore::getClient(const std::string& userID) {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    auto it = clients_.find(userID);
    return (it != clients_.end()) ? it->second : nullptr;
}

std::vector<std::string> ServerCore::getOnlineUsers() const {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    std::vector<std::string> users;
    users.reserve(clients_.size());
    for (const auto& pair : clients_) {
        users.push_back(pair.first);
    }
    return users;
}

void ServerCore::routePrivateMessage(const std::string& fromUserID, const std::string& toUserID,
                                     const std::vector<uint8_t>& encryptedData) {
    messageRouter_->routePrivateMessage(fromUserID, toUserID, encryptedData);
}

void ServerCore::routeGroupMessage(const std::string& fromUserID, const std::string& groupName,
                                   const std::vector<uint8_t>& encryptedData) {
    messageRouter_->routeGroupMessage(fromUserID, groupName, encryptedData);
}

AuthManager& ServerCore::getAuthManager() {
    return *authManager_;
}

GroupManager& ServerCore::getGroupManager() {
    return *groupManager_;
}

MessageRouter& ServerCore::getMessageRouter() {
    return *messageRouter_;
}

boost::asio::io_context& ServerCore::getIOContext() {
    return ioContext_;
}

} // namespace LanShare