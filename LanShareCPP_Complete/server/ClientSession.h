#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include "../common/Protocol.h"
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <deque>
#include <mutex>

namespace LanShare {

class ServerCore;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    explicit ClientSession(boost::asio::ip::tcp::socket socket, ServerCore& server);
    ~ClientSession();

    void start();
    void stop();

    void sendMessage(MessageType type, const std::vector<uint8_t>& payload);
    void sendMessage(MessageType type, const std::string& payload);

    std::string getUserID() const;
    void setUserID(const std::string& userID);
    bool isAuthenticated() const;
    std::string getRemoteAddress() const;

private:
    void readHeader();
    void readPayload(uint32_t payloadLength, MessageType type);
    void handleMessage(MessageType type, const std::vector<uint8_t>& payload);
    void writeNext();
    void handleError(const std::string& context, const boost::system::error_code& error);

    // Message handlers
    void handleAuthRegister(const std::vector<uint8_t>& payload);
    void handleAuthLogin(const std::vector<uint8_t>& payload);
    void handleAuthLogout();
    void handlePrivateMessage(const std::vector<uint8_t>& payload);
    void handleGroupMessage(const std::vector<uint8_t>& payload);
    void handleGroupCreate(const std::vector<uint8_t>& payload);
    void handleGroupJoin(const std::vector<uint8_t>& payload);
    void handleGroupLeave(const std::vector<uint8_t>& payload);
    void handleUserListRequest();
    void handlePing();

    // ★ NEW: single handler for all file transfer message types
    void handleFileTransfer(MessageType type, const std::vector<uint8_t>& payload);

    // Legacy stubs — kept so old .cpp references still compile
    void handleFilePrivateMeta(const std::vector<uint8_t>& payload);
    void handleFilePrivateChunk(const std::vector<uint8_t>& payload);
    void handleFilePrivateComplete();

    boost::asio::ip::tcp::socket socket_;
    ServerCore& server_;

    std::string userID_;
    bool authenticated_;

    MessageHeader readHeader_;
    std::vector<uint8_t> readBuffer_;

    struct OutgoingMessage {
        MessageHeader header;
        std::vector<uint8_t> data;
    };
    std::deque<OutgoingMessage> writeQueue_;
    std::mutex writeMutex_;
    bool writing_;
};

} // namespace LanShare

#endif // CLIENTSESSION_H