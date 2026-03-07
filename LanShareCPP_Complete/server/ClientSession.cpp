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
    : socket_(std::move(socket)), server_(server)
    , authenticated_(false), writing_(false)
{
    std::memset(&readHeader_, 0, sizeof(readHeader_));
}

ClientSession::~ClientSession() {
    if (authenticated_) server_.unregisterClient(userID_);
}

void ClientSession::start() { readHeader(); }

void ClientSession::stop() {
    boost::system::error_code ec; socket_.close(ec);
}

void ClientSession::sendMessage(MessageType type, const std::vector<uint8_t>& payload) {
    OutgoingMessage msg;
    msg.header.length = static_cast<uint32_t>(payload.size());
    msg.header.type   = type;
    msg.data          = payload;
    std::lock_guard<std::mutex> lock(writeMutex_);
    writeQueue_.push_back(msg);
    if (!writing_) writeNext();
}

void ClientSession::sendMessage(MessageType type, const std::string& payload) {
    sendMessage(type, std::vector<uint8_t>(payload.begin(), payload.end()));
}

std::string ClientSession::getUserID() const { return userID_; }
void ClientSession::setUserID(const std::string& id) { userID_ = id; }
bool ClientSession::isAuthenticated() const { return authenticated_; }
std::string ClientSession::getRemoteAddress() const {
    try { return socket_.remote_endpoint().address().to_string(); }
    catch (...) { return "unknown"; }
}

void ClientSession::readHeader() {
    auto self = shared_from_this();
    boost::asio::async_read(socket_,
        boost::asio::buffer(&readHeader_, sizeof(MessageHeader)),
        [this, self](const boost::system::error_code& ec, std::size_t) {
            if (!ec) {
                if (readHeader_.length <= MAX_MESSAGE_SIZE)
                    readPayload(readHeader_.length, readHeader_.type);
                else { std::cerr << "Message too large\n"; stop(); }
            } else handleError("readHeader", ec);
        });
}

void ClientSession::readPayload(uint32_t len, MessageType type) {
    readBuffer_.resize(len);
    auto self = shared_from_this();
    boost::asio::async_read(socket_, boost::asio::buffer(readBuffer_),
        [this, self, type](const boost::system::error_code& ec, std::size_t) {
            if (!ec) { handleMessage(type, readBuffer_); readHeader(); }
            else handleError("readPayload", ec);
        });
}

void ClientSession::handleMessage(MessageType type,
                                  const std::vector<uint8_t>& payload) {
    switch (type) {
        case MessageType::AUTH_REGISTER:  handleAuthRegister(payload);  break;
        case MessageType::AUTH_LOGIN:     handleAuthLogin(payload);     break;
        case MessageType::AUTH_LOGOUT:    handleAuthLogout();           break;
        case MessageType::MSG_PRIVATE:    handlePrivateMessage(payload); break;
        case MessageType::MSG_GROUP:      handleGroupMessage(payload);   break;
        case MessageType::GROUP_CREATE:   handleGroupCreate(payload);    break;
        case MessageType::GROUP_JOIN:     handleGroupJoin(payload);      break;
        case MessageType::GROUP_LEAVE:    handleGroupLeave(payload);     break;
        case MessageType::USER_LIST:      handleUserListRequest();       break;
        case MessageType::PING:           handlePing();                  break;
        case MessageType::FILE_PRIVATE_META:
        case MessageType::FILE_PRIVATE_CHUNK:
        case MessageType::FILE_PRIVATE_COMPLETE:
        case MessageType::FILE_GROUP_META:
        case MessageType::FILE_GROUP_CHUNK:
        case MessageType::FILE_GROUP_COMPLETE:
            handleFileTransfer(type, payload); break;
        default:
            std::cerr << "Unknown type: " << static_cast<int>(type) << "\n";
    }
}

void ClientSession::handleFileTransfer(MessageType type,
                                       const std::vector<uint8_t>& payload) {
    if (!authenticated_) { sendMessage(MessageType::ERROR_MSG, "Not authenticated"); return; }
    server_.getMessageRouter().routeFileRaw(userID_, payload, type);
}

void ClientSession::handleAuthRegister(const std::vector<uint8_t>& payload) {
    std::string data(payload.begin(), payload.end());
    std::istringstream ss(data);
    std::string username, password;
    std::getline(ss, username, ':'); std::getline(ss, password);
    std::string userID;
    if (server_.getAuthManager().registerUser(username, password, userID)) {
        userID_ = userID; authenticated_ = true;
        server_.registerClient(userID_, shared_from_this());
        sendMessage(MessageType::AUTH_SUCCESS, userID);
        std::cout << "Registered: " << username << " (" << userID << ")\n";
    } else sendMessage(MessageType::AUTH_FAIL, "Username already exists");
}

void ClientSession::handleAuthLogin(const std::vector<uint8_t>& payload) {
    std::string data(payload.begin(), payload.end());
    std::istringstream ss(data);
    std::string username, password;
    std::getline(ss, username, ':'); std::getline(ss, password);
    std::string userID;
    if (server_.getAuthManager().authenticateUser(username, password, userID)) {
        userID_ = userID; authenticated_ = true;
        server_.registerClient(userID_, shared_from_this());
        sendMessage(MessageType::AUTH_SUCCESS, userID);
        std::cout << "Login: " << username << " (" << userID << ")\n";
        server_.getMessageRouter().deliverOfflineMessages(userID);
    } else sendMessage(MessageType::AUTH_FAIL, "Invalid credentials");
}

void ClientSession::handleAuthLogout() {
    if (authenticated_) { server_.unregisterClient(userID_); authenticated_ = false; }
    stop();
}

void ClientSession::handlePrivateMessage(const std::vector<uint8_t>& payload) {
    if (!authenticated_) { sendMessage(MessageType::ERROR_MSG, "Not authenticated"); return; }
    size_t n = 0;
    while (n < payload.size() && payload[n] != 0) ++n;
    if (n >= payload.size()) { sendMessage(MessageType::ERROR_MSG, "Invalid format"); return; }
    server_.routePrivateMessage(userID_,
        std::string(payload.begin(), payload.begin() + n),
        std::vector<uint8_t>(payload.begin() + n + 1, payload.end()));
}

void ClientSession::handleGroupMessage(const std::vector<uint8_t>& payload) {
    if (!authenticated_) { sendMessage(MessageType::ERROR_MSG, "Not authenticated"); return; }
    size_t n = 0;
    while (n < payload.size() && payload[n] != 0) ++n;
    if (n >= payload.size()) { sendMessage(MessageType::ERROR_MSG, "Invalid format"); return; }
    std::string groupName(payload.begin(), payload.begin() + n);
    if (!server_.getGroupManager().isMember(groupName, userID_)) {
        sendMessage(MessageType::ERROR_MSG, "Not a member"); return;
    }
    server_.routeGroupMessage(userID_, groupName,
        std::vector<uint8_t>(payload.begin() + n + 1, payload.end()));
}

// ── Group create — payload: groupName (server generates code) ─────────────
void ClientSession::handleGroupCreate(const std::vector<uint8_t>& payload) {
    if (!authenticated_) { sendMessage(MessageType::ERROR_MSG, "Not authenticated"); return; }

    std::string groupName(payload.begin(), payload.end());
    std::string code = server_.getGroupManager().createGroup(groupName, userID_);

    if (code.empty()) {
        sendMessage(MessageType::ERROR_MSG, "Group already exists or invalid name");
        return;
    }

    // Send code back: "groupName|code"
    sendMessage(MessageType::GROUP_CODE, groupName + "|" + code);
    std::cout << "Group '" << groupName << "' created | code: " << code << "\n";
}

// ── Group join — payload: groupName|joinCode ──────────────────────────────
void ClientSession::handleGroupJoin(const std::vector<uint8_t>& payload) {
    if (!authenticated_) { sendMessage(MessageType::ERROR_MSG, "Not authenticated"); return; }

    std::string data(payload.begin(), payload.end());
    size_t sep = data.find('|');
    if (sep == std::string::npos) {
        sendMessage(MessageType::ERROR_MSG, "Invalid format — expected groupName|code");
        return;
    }

    std::string groupName = data.substr(0, sep);
    std::string joinCode  = data.substr(sep + 1);

    if (server_.getGroupManager().joinGroup(groupName, userID_, joinCode)) {
        sendMessage(MessageType::GROUP_INFO, "Joined group: " + groupName);
        std::cout << userID_ << " joined: " << groupName << "\n";
    } else {
        sendMessage(MessageType::ERROR_MSG, "Wrong join code or group not found");
    }
}

void ClientSession::handleGroupLeave(const std::vector<uint8_t>& payload) {
    if (!authenticated_) { sendMessage(MessageType::ERROR_MSG, "Not authenticated"); return; }
    std::string groupName(payload.begin(), payload.end());
    if (server_.getGroupManager().leaveGroup(groupName, userID_))
        sendMessage(MessageType::GROUP_INFO, "Left: " + groupName);
    else
        sendMessage(MessageType::ERROR_MSG, "Not a member");
}

void ClientSession::handleUserListRequest() {
    if (!authenticated_) { sendMessage(MessageType::ERROR_MSG, "Not authenticated"); return; }
    auto users = server_.getOnlineUsers();
    std::ostringstream ss;
    for (size_t i = 0; i < users.size(); ++i) {
        ss << users[i];
        if (i < users.size() - 1) ss << ",";
    }
    sendMessage(MessageType::USER_LIST, ss.str());
}

void ClientSession::handlePing() { sendMessage(MessageType::PONG, ""); }

void ClientSession::writeNext() {
    if (writeQueue_.empty()) { writing_ = false; return; }
    writing_ = true;
    OutgoingMessage& msg = writeQueue_.front();
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(&msg.header, sizeof(MessageHeader)));
    buffers.push_back(boost::asio::buffer(msg.data));
    auto self = shared_from_this();
    boost::asio::async_write(socket_, buffers,
        [this, self](const boost::system::error_code& ec, std::size_t) {
            std::lock_guard<std::mutex> lock(writeMutex_);
            if (!ec) { writeQueue_.pop_front(); writeNext(); }
            else handleError("write", ec);
        });
}

void ClientSession::handleError(const std::string& ctx,
                                const boost::system::error_code& ec) {
    if (ec != boost::asio::error::eof &&
        ec != boost::asio::error::connection_reset)
        std::cerr << "Error in " << ctx << ": " << ec.message() << "\n";
    stop();
}

void ClientSession::handleFilePrivateMeta(const std::vector<uint8_t>&) {}
void ClientSession::handleFilePrivateChunk(const std::vector<uint8_t>&) {}
void ClientSession::handleFilePrivateComplete() {}

} // namespace LanShare