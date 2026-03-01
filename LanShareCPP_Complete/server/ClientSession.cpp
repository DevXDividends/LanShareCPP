#include "ClientSession.h"
#include "ServerCore.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include "AuthManager.h"
#include "GroupManager.h"
#include "MessageRouter.h"

namespace LanShare {

ClientSession::ClientSession(boost::asio::ip::tcp::socket socket, ServerCore& server)
    : socket_(std::move(socket))
    , server_(server)
    , authenticated_(false)
    , writing_(false)
{
    std::memset(&readHeader_, 0, sizeof(readHeader_));
}

ClientSession::~ClientSession() {
    if (authenticated_) {
        server_.unregisterClient(userID_);
    }
}

void ClientSession::start() {
    readHeader();
}

void ClientSession::stop() {
    boost::system::error_code ec;
    socket_.close(ec);
}

void ClientSession::sendMessage(MessageType type, const std::vector<uint8_t>& payload) {
    OutgoingMessage msg;
    msg.header.length = static_cast<uint32_t>(payload.size());
    msg.header.type = type;
    msg.data = payload;
    
    std::lock_guard<std::mutex> lock(writeMutex_);
    writeQueue_.push_back(msg);
    
    if (!writing_) {
        writeNext();
    }
}

void ClientSession::sendMessage(MessageType type, const std::string& payload) {
    std::vector<uint8_t> data(payload.begin(), payload.end());
    sendMessage(type, data);
}

std::string ClientSession::getUserID() const {
    return userID_;
}

void ClientSession::setUserID(const std::string& userID) {
    userID_ = userID;
}

bool ClientSession::isAuthenticated() const {
    return authenticated_;
}

std::string ClientSession::getRemoteAddress() const {
    try {
        return socket_.remote_endpoint().address().to_string();
    } catch (...) {
        return "unknown";
    }
}

void ClientSession::readHeader() {
    auto self = shared_from_this();
    boost::asio::async_read(socket_,
        boost::asio::buffer(&readHeader_, sizeof(MessageHeader)),
        [this, self](const boost::system::error_code& error, std::size_t) {
            if (!error) {
                if (readHeader_.length <= MAX_MESSAGE_SIZE) {
                    readPayload(readHeader_.length, readHeader_.type);
                } else {
                    std::cerr << "Message too large: " << readHeader_.length << "\n";
                    stop();
                }
            } else {
                handleError("readHeader", error);
            }
        });
}

void ClientSession::readPayload(uint32_t payloadLength, MessageType type) {
    readBuffer_.resize(payloadLength);
    
    auto self = shared_from_this();
    boost::asio::async_read(socket_,
        boost::asio::buffer(readBuffer_),
        [this, self, type](const boost::system::error_code& error, std::size_t) {
            if (!error) {
                handleMessage(type, readBuffer_);
                readHeader();
            } else {
                handleError("readPayload", error);
            }
        });
}

void ClientSession::handleMessage(MessageType type, const std::vector<uint8_t>& payload) {
    switch (type) {
        case MessageType::AUTH_REGISTER:
            handleAuthRegister(payload);
            break;
        case MessageType::AUTH_LOGIN:
            handleAuthLogin(payload);
            break;
        case MessageType::AUTH_LOGOUT:
            handleAuthLogout();
            break;
        case MessageType::MSG_PRIVATE:
            handlePrivateMessage(payload);
            break;
        case MessageType::MSG_GROUP:
            handleGroupMessage(payload);
            break;
        case MessageType::GROUP_CREATE:
            handleGroupCreate(payload);
            break;
        case MessageType::GROUP_JOIN:
            handleGroupJoin(payload);
            break;
        case MessageType::GROUP_LEAVE:
            handleGroupLeave(payload);
            break;
        case MessageType::USER_LIST:
            handleUserListRequest();
            break;
        case MessageType::PING:
            handlePing();
            break;
        default:
            std::cerr << "Unknown message type: " << static_cast<int>(type) << "\n";
    }
}

void ClientSession::handleAuthRegister(const std::vector<uint8_t>& payload) {
    std::string data(payload.begin(), payload.end());
    std::istringstream ss(data);
    std::string username, password;
    
    std::getline(ss, username, ':');
    std::getline(ss, password);
    
    std::string userID;
    if (server_.getAuthManager().registerUser(username, password, userID)) {
        userID_ = userID;
        authenticated_ = true;
        server_.registerClient(userID_, shared_from_this());
        
        sendMessage(MessageType::AUTH_SUCCESS, userID);
        std::cout << "User registered: " << username << " (" << userID << ")\n";
    } else {
        sendMessage(MessageType::AUTH_FAIL, "Username already exists");
    }
}

void ClientSession::handleAuthLogin(const std::vector<uint8_t>& payload) {
    std::string data(payload.begin(), payload.end());
    std::istringstream ss(data);
    std::string username, password;
    
    std::getline(ss, username, ':');
    std::getline(ss, password);
    
    std::string userID;
    if (server_.getAuthManager().authenticateUser(username, password, userID)) {
        userID_ = userID;
        authenticated_ = true;
        server_.registerClient(userID_, shared_from_this());
        
        sendMessage(MessageType::AUTH_SUCCESS, userID);
        std::cout << "User logged in: " << username << " (" << userID << ")\n";
        
        // Deliver offline messages
        server_.getMessageRouter().deliverOfflineMessages(userID);
    } else {
        sendMessage(MessageType::AUTH_FAIL, "Invalid credentials");
    }
}

void ClientSession::handleAuthLogout() {
    if (authenticated_) {
        server_.unregisterClient(userID_);
        authenticated_ = false;
        std::cout << "User logged out: " << userID_ << "\n";
    }
    stop();
}

void ClientSession::handlePrivateMessage(const std::vector<uint8_t>& payload) {
    if (!authenticated_) {
        sendMessage(MessageType::ERROR_MSG, "Not authenticated");
        return;
    }
    
    // Parse: targetUserID + encrypted data
    size_t nullPos = 0;
    while (nullPos < payload.size() && payload[nullPos] != 0) {
        ++nullPos;
    }
    
    if (nullPos >= payload.size()) {
        sendMessage(MessageType::ERROR_MSG, "Invalid message format");
        return;
    }
    
    std::string targetUserID(payload.begin(), payload.begin() + nullPos);
    std::vector<uint8_t> encryptedData(payload.begin() + nullPos + 1, payload.end());
    
    server_.routePrivateMessage(userID_, targetUserID, encryptedData);
}

void ClientSession::handleGroupMessage(const std::vector<uint8_t>& payload) {
    if (!authenticated_) {
        sendMessage(MessageType::ERROR_MSG, "Not authenticated");
        return;
    }
    
    // Parse: groupName + encrypted data
    size_t nullPos = 0;
    while (nullPos < payload.size() && payload[nullPos] != 0) {
        ++nullPos;
    }
    
    if (nullPos >= payload.size()) {
        sendMessage(MessageType::ERROR_MSG, "Invalid message format");
        return;
    }
    
    std::string groupName(payload.begin(), payload.begin() + nullPos);
    std::vector<uint8_t> encryptedData(payload.begin() + nullPos + 1, payload.end());
    
    // Check if user is member
    if (!server_.getGroupManager().isMember(groupName, userID_)) {
        sendMessage(MessageType::ERROR_MSG, "Not a member of group");
        return;
    }
    
    server_.routeGroupMessage(userID_, groupName, encryptedData);
}

void ClientSession::handleGroupCreate(const std::vector<uint8_t>& payload) {
    if (!authenticated_) {
        sendMessage(MessageType::ERROR_MSG, "Not authenticated");
        return;
    }
    
    std::string groupName(payload.begin(), payload.end());
    
    if (server_.getGroupManager().createGroup(groupName, userID_)) {
        sendMessage(MessageType::GROUP_INFO, "Group created: " + groupName);
        std::cout << "Group created: " << groupName << " by " << userID_ << "\n";
    } else {
        sendMessage(MessageType::ERROR_MSG, "Group already exists");
    }
}

void ClientSession::handleGroupJoin(const std::vector<uint8_t>& payload) {
    if (!authenticated_) {
        sendMessage(MessageType::ERROR_MSG, "Not authenticated");
        return;
    }
    
    std::string groupName(payload.begin(), payload.end());
    
    if (server_.getGroupManager().joinGroup(groupName, userID_)) {
        sendMessage(MessageType::GROUP_INFO, "Joined group: " + groupName);
        std::cout << userID_ << " joined group: " << groupName << "\n";
    } else {
        sendMessage(MessageType::ERROR_MSG, "Group does not exist");
    }
}

void ClientSession::handleGroupLeave(const std::vector<uint8_t>& payload) {
    if (!authenticated_) {
        sendMessage(MessageType::ERROR_MSG, "Not authenticated");
        return;
    }
    
    std::string groupName(payload.begin(), payload.end());
    
    if (server_.getGroupManager().leaveGroup(groupName, userID_)) {
        sendMessage(MessageType::GROUP_INFO, "Left group: " + groupName);
        std::cout << userID_ << " left group: " << groupName << "\n";
    } else {
        sendMessage(MessageType::ERROR_MSG, "Not a member of group");
    }
}

void ClientSession::handleUserListRequest() {
    if (!authenticated_) {
        sendMessage(MessageType::ERROR_MSG, "Not authenticated");
        return;
    }
    
    auto users = server_.getOnlineUsers();
    std::ostringstream ss;
    for (size_t i = 0; i < users.size(); ++i) {
        ss << users[i];
        if (i < users.size() - 1) {
            ss << ",";
        }
    }
    
    sendMessage(MessageType::USER_LIST, ss.str());
}

void ClientSession::handlePing() {
    sendMessage(MessageType::PONG, "");
}

void ClientSession::writeNext() {
    if (writeQueue_.empty()) {
        writing_ = false;
        return;
    }
    
    writing_ = true;
    OutgoingMessage& msg = writeQueue_.front();
    
    // Prepare buffers
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(&msg.header, sizeof(MessageHeader)));
    buffers.push_back(boost::asio::buffer(msg.data));
    
    auto self = shared_from_this();
    boost::asio::async_write(socket_, buffers,
        [this, self](const boost::system::error_code& error, std::size_t) {
            std::lock_guard<std::mutex> lock(writeMutex_);
            if (!error) {
                writeQueue_.pop_front();
                writeNext();
            } else {
                handleError("write", error);
            }
        });
}

void ClientSession::handleError(const std::string& context, const boost::system::error_code& error) {
    if (error != boost::asio::error::eof &&
        error != boost::asio::error::connection_reset) {
        std::cerr << "Error in " << context << ": " << error.message() << "\n";
    }
    stop();
}

void ClientSession::handleFilePrivateMeta(const std::vector<uint8_t>& payload) {
    // TODO: Implement file transfer
    sendMessage(MessageType::ERROR_MSG, "File transfer not yet implemented");
}

void ClientSession::handleFilePrivateChunk(const std::vector<uint8_t>& payload) {
    // TODO: Implement file transfer
}

void ClientSession::handleFilePrivateComplete() {
    // TODO: Implement file transfer
}

} // namespace LanShare
