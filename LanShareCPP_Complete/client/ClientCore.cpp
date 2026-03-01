#include "ClientCore.h"
#include <iostream>
#include <cstring>

namespace LanShare {

ClientCore::ClientCore()
    : ioContext_()
    , socket_(ioContext_)
    , authenticated_(false)
    , connected_(false)
    , writing_(false)
{
    std::memset(&readHeader_, 0, sizeof(readHeader_));
    encryptionKey_ = AESGCM::generateKey();
}

ClientCore::~ClientCore() {
    disconnect();
    stopAsync();
}

void ClientCore::connect(const std::string& serverIP, unsigned short port) {
    try {
        boost::asio::ip::tcp::resolver resolver(ioContext_);
        auto endpoints = resolver.resolve(serverIP, std::to_string(port));
        
        boost::asio::async_connect(socket_, endpoints,
            [this](const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint&) {
                if (!error) {
                    connected_ = true;
                    std::cout << "Connected to server\n";
                    
                    if (connectionCallback_) {
                        connectionCallback_(true, "Connected successfully");
                    }
                    
                    readHeader();
                } else {
                    connected_ = false;
                    std::cerr << "Connection failed: " << error.message() << "\n";
                    
                    if (connectionCallback_) {
                        connectionCallback_(false, error.message());
                    }
                }
            });
        
    } catch (const std::exception& e) {
        std::cerr << "Connect error: " << e.what() << "\n";
        if (connectionCallback_) {
            connectionCallback_(false, e.what());
        }
    }
}

void ClientCore::disconnect() {
    if (!connected_) return;
    
    if (authenticated_) {
        sendMessage(MessageType::AUTH_LOGOUT, "");
    }
    
    boost::system::error_code ec;
    socket_.close(ec);
    connected_ = false;
    authenticated_ = false;
    
    std::cout << "Disconnected from server\n";
}

bool ClientCore::isConnected() const {
    return connected_;
}

void ClientCore::registerUser(const std::string& username, const std::string& password) {
    std::string payload = username + ":" + password;
    sendMessage(MessageType::AUTH_REGISTER, payload);
}

void ClientCore::login(const std::string& username, const std::string& password) {
    username_ = username;
    std::string payload = username + ":" + password;
    sendMessage(MessageType::AUTH_LOGIN, payload);
}

void ClientCore::logout() {
    sendMessage(MessageType::AUTH_LOGOUT, "");
    authenticated_ = false;
}

std::string ClientCore::getUserID() const {
    return userID_;
}

std::string ClientCore::getUsername() const {
    return username_;
}

bool ClientCore::isAuthenticated() const {
    return authenticated_;
}

void ClientCore::sendPrivateMessage(const std::string& toUserID, const std::vector<uint8_t>& encryptedData) {
    std::vector<uint8_t> payload;
    payload.insert(payload.end(), toUserID.begin(), toUserID.end());
    payload.push_back(0);
    payload.insert(payload.end(), encryptedData.begin(), encryptedData.end());
    
    sendMessage(MessageType::MSG_PRIVATE, payload);
}

void ClientCore::sendGroupMessage(const std::string& groupName, const std::vector<uint8_t>& encryptedData) {
    std::vector<uint8_t> payload;
    payload.insert(payload.end(), groupName.begin(), groupName.end());
    payload.push_back(0);
    payload.insert(payload.end(), encryptedData.begin(), encryptedData.end());
    
    sendMessage(MessageType::MSG_GROUP, payload);
}

void ClientCore::createGroup(const std::string& groupName) {
    sendMessage(MessageType::GROUP_CREATE, groupName);
}

void ClientCore::joinGroup(const std::string& groupName) {
    sendMessage(MessageType::GROUP_JOIN, groupName);
}

void ClientCore::leaveGroup(const std::string& groupName) {
    sendMessage(MessageType::GROUP_LEAVE, groupName);
}

void ClientCore::requestUserList() {
    sendMessage(MessageType::USER_LIST, "");
}

AESGCM& ClientCore::getCrypto() {
    return crypto_;
}

void ClientCore::setEncryptionKey(const AESGCM::Key& key) {
    std::lock_guard<std::mutex> lock(keyMutex_);
    encryptionKey_ = key;
}

AESGCM::Key ClientCore::getEncryptionKey() const {
    std::lock_guard<std::mutex> lock(keyMutex_);
    return encryptionKey_;
}

void ClientCore::setMessageCallback(MessageCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    messageCallback_ = callback;
}

void ClientCore::setGroupMessageCallback(GroupMessageCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    groupMessageCallback_ = callback;
}

void ClientCore::setUserListCallback(UserListCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    userListCallback_ = callback;
}

void ClientCore::setConnectionCallback(ConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    connectionCallback_ = callback;
}

void ClientCore::setAuthCallback(AuthCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    authCallback_ = callback;
}

void ClientCore::run() {
    ioContext_.run();
}

void ClientCore::startAsync() {
    ioThread_ = std::make_unique<std::thread>([this]() {
        ioContext_.run();
    });
}

void ClientCore::stopAsync() {
    if (ioThread_ && ioThread_->joinable()) {
        ioContext_.stop();
        ioThread_->join();
    }
}

void ClientCore::readHeader() {
    boost::asio::async_read(socket_,
        boost::asio::buffer(&readHeader_, sizeof(MessageHeader)),
        [this](const boost::system::error_code& error, std::size_t) {
            if (!error) {
                if (readHeader_.length <= MAX_MESSAGE_SIZE) {
                    readPayload(readHeader_.length, readHeader_.type);
                } else {
                    std::cerr << "Message too large\n";
                    disconnect();
                }
            } else {
                handleError("readHeader", error);
            }
        });
}

void ClientCore::readPayload(uint32_t payloadLength, MessageType type) {
    readBuffer_.resize(payloadLength);
    
    boost::asio::async_read(socket_,
        boost::asio::buffer(readBuffer_),
        [this, type](const boost::system::error_code& error, std::size_t) {
            if (!error) {
                handleMessage(type, readBuffer_);
                readHeader();
            } else {
                handleError("readPayload", error);
            }
        });
}

void ClientCore::handleMessage(MessageType type, const std::vector<uint8_t>& payload) {
    switch (type) {
        case MessageType::AUTH_SUCCESS:
            handleAuthSuccess(payload);
            break;
        case MessageType::AUTH_FAIL:
            handleAuthFail(payload);
            break;
        case MessageType::MSG_PRIVATE:
            handlePrivateMessage(payload);
            break;
        case MessageType::MSG_GROUP:
            handleGroupMessage(payload);
            break;
        case MessageType::USER_LIST:
            handleUserList(payload);
            break;
        case MessageType::PONG:
            handlePong();
            break;
        case MessageType::ERROR_MSG:
            handleError(payload);
            break;
        default:
            std::cerr << "Unknown message type\n";
    }
}

void ClientCore::handleAuthSuccess(const std::vector<uint8_t>& payload) {
    userID_ = std::string(payload.begin(), payload.end());
    authenticated_ = true;
    
    std::cout << "Authentication successful: " << userID_ << "\n";
    
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (authCallback_) {
        authCallback_(true, userID_, "Authentication successful");
    }
}

void ClientCore::handleAuthFail(const std::vector<uint8_t>& payload) {
    std::string reason(payload.begin(), payload.end());
    authenticated_ = false;
    
    std::cerr << "Authentication failed: " << reason << "\n";
    
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (authCallback_) {
        authCallback_(false, "", reason);
    }
}

void ClientCore::handlePrivateMessage(const std::vector<uint8_t>& payload) {
    // Parse: fromUserID + encrypted data
    size_t nullPos = 0;
    while (nullPos < payload.size() && payload[nullPos] != 0) {
        ++nullPos;
    }
    
    if (nullPos >= payload.size()) return;
    
    std::string fromUserID(payload.begin(), payload.begin() + nullPos);
    std::vector<uint8_t> encryptedData(payload.begin() + nullPos + 1, payload.end());
    
    std::cout << "Message from " << fromUserID << " (" << encryptedData.size() << " bytes)\n";
    
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (messageCallback_) {
        messageCallback_(fromUserID, encryptedData, std::time(nullptr));
    }
}

void ClientCore::handleGroupMessage(const std::vector<uint8_t>& payload) {
    // Parse: groupName + fromUserID + encrypted data
    size_t firstNull = 0;
    while (firstNull < payload.size() && payload[firstNull] != 0) {
        ++firstNull;
    }
    
    if (firstNull >= payload.size()) return;
    
    size_t secondNull = firstNull + 1;
    while (secondNull < payload.size() && payload[secondNull] != 0) {
        ++secondNull;
    }
    
    if (secondNull >= payload.size()) return;
    
    std::string groupName(payload.begin(), payload.begin() + firstNull);
    std::string fromUserID(payload.begin() + firstNull + 1, payload.begin() + secondNull);
    std::vector<uint8_t> encryptedData(payload.begin() + secondNull + 1, payload.end());
    
    std::cout << "Group message from " << fromUserID << " in " << groupName << "\n";
    
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (groupMessageCallback_) {
        groupMessageCallback_(groupName, fromUserID, encryptedData, std::time(nullptr));
    }
}

void ClientCore::handleUserList(const std::vector<uint8_t>& payload) {
    std::string data(payload.begin(), payload.end());
    std::vector<std::string> users;
    
    size_t start = 0;
    size_t end = data.find(',');
    
    while (end != std::string::npos) {
        users.push_back(data.substr(start, end - start));
        start = end + 1;
        end = data.find(',', start);
    }
    
    if (start < data.length()) {
        users.push_back(data.substr(start));
    }
    
    std::cout << "Online users: " << users.size() << "\n";
    
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (userListCallback_) {
        userListCallback_(users);
    }
}

void ClientCore::handlePong() {
    // Keepalive response
}

void ClientCore::handleError(const std::vector<uint8_t>& payload) {
    std::string error(payload.begin(), payload.end());
    std::cerr << "Server error: " << error << "\n";
}

void ClientCore::sendMessage(MessageType type, const std::vector<uint8_t>& payload) {
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

void ClientCore::sendMessage(MessageType type, const std::string& payload) {
    std::vector<uint8_t> data(payload.begin(), payload.end());
    sendMessage(type, data);
}

void ClientCore::writeNext() {
    if (writeQueue_.empty()) {
        writing_ = false;
        return;
    }
    
    writing_ = true;
    OutgoingMessage& msg = writeQueue_.front();
    
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(&msg.header, sizeof(MessageHeader)));
    buffers.push_back(boost::asio::buffer(msg.data));
    
    boost::asio::async_write(socket_, buffers,
        [this](const boost::system::error_code& error, std::size_t) {
            std::lock_guard<std::mutex> lock(writeMutex_);
            if (!error) {
                writeQueue_.pop_front();
                writeNext();
            } else {
                handleError("write", error);
            }
        });
}

void ClientCore::handleError(const std::string& context, const boost::system::error_code& error) {
    if (error != boost::asio::error::eof &&
        error != boost::asio::error::connection_reset) {
        std::cerr << "Error in " << context << ": " << error.message() << "\n";
    }
    disconnect();
}

// File transfer stubs
void ClientCore::sendFile(const std::string& toUserID, const std::string& filepath, bool isGroup) {
    // TODO: Implement
    std::cerr << "File transfer not yet implemented\n";
}

void ClientCore::sendFileChunk(const std::string& toUserID, const std::vector<uint8_t>& chunk, bool isGroup) {
    // TODO: Implement
}

void ClientCore::sendFileComplete(const std::string& toUserID, bool isGroup) {
    // TODO: Implement
}

void ClientCore::requestGroupList() {
    // TODO: Implement
}

void ClientCore::setFileMetadataCallback(FileMetadataCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    fileMetadataCallback_ = callback;
}

void ClientCore::setFileChunkCallback(FileChunkCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    fileChunkCallback_ = callback;
}

void ClientCore::setFileCompleteCallback(FileCompleteCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    fileCompleteCallback_ = callback;
}

} // namespace LanShare
