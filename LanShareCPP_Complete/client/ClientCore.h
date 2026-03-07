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
#include <map>
#include <thread>

namespace LanShare
{

    // Callback types
    using MessageCallback = std::function<void(const std::string &, const std::vector<uint8_t> &, uint64_t)>;
    using GroupMessageCallback = std::function<void(const std::string &, const std::string &, const std::vector<uint8_t> &, uint64_t)>;
    using FileMetadataCallback = std::function<void(const std::string &fromUserID, const std::string &filename, uint64_t filesize)>;
    using FileChunkCallback = std::function<void(const std::string &fromUserID, const std::vector<uint8_t> &chunk, uint64_t offset)>;
    using FileCompleteCallback = std::function<void(const std::string &fromUserID, const std::string &filename)>;
    using FileProgressCallback = std::function<void(const std::string &userID, int percent)>;
    using FileSentCallback = std::function<void(const std::string &toUserID, const std::string &filename)>;
    using FileErrorCallback = std::function<void(const std::string &userID, const std::string &reason)>;
    using UserListCallback = std::function<void(const std::vector<std::string> &)>;
    using ConnectionCallback = std::function<void(bool, const std::string &)>;
    using AuthCallback = std::function<void(bool, const std::string &, const std::string &)>;
    using GroupCodeCallback = std::function<void(const std::string &groupName, const std::string &code)>;


    class ClientCore
    {
    public:
        explicit ClientCore();
        ~ClientCore();

        // Connection
        void connect(const std::string &serverIP, unsigned short port = 5555);
        void disconnect();
        bool isConnected() const;

        // Auth
        void registerUser(const std::string &username, const std::string &password);
        void login(const std::string &username, const std::string &password);
        void logout();
        std::string getUserID() const;
        std::string getUsername() const;
        bool isAuthenticated() const;

        // Messaging
        void sendPrivateMessage(const std::string &toUserID, const std::vector<uint8_t> &encryptedData);
        void sendGroupMessage(const std::string &groupName, const std::vector<uint8_t> &encryptedData);

        // File transfer (high-level — call sendFile, the rest is automatic)
        void sendFile(const std::string &toUserID, const std::string &filepath, bool isGroup = false);
        void sendFileChunk(const std::string &toUserID, const std::vector<uint8_t> &chunk, bool isGroup = false);
        void sendFileComplete(const std::string &toUserID, bool isGroup = false);

        // Groups
        void createGroup(const std::string &groupName);
        void joinGroup(const std::string &groupName, const std::string &joinCode);        void leaveGroup(const std::string &groupName);
        void requestGroupList();
        void requestUserList();

        // Crypto
        AESGCM &getCrypto();
        void setEncryptionKey(const AESGCM::Key &key);
        AESGCM::Key getEncryptionKey() const;

        // Callbacks
        void setMessageCallback(MessageCallback callback);
        void setGroupMessageCallback(GroupMessageCallback callback);
        void setFileMetadataCallback(FileMetadataCallback callback);
        void setFileChunkCallback(FileChunkCallback callback);
        void setFileCompleteCallback(FileCompleteCallback callback);
        void setFileProgressCallback(FileProgressCallback callback);
        void setFileSentCallback(FileSentCallback callback);
        void setFileErrorCallback(FileErrorCallback callback);
        void setUserListCallback(UserListCallback callback);
        void setConnectionCallback(ConnectionCallback callback);
        void setAuthCallback(AuthCallback callback);
        void setGroupCodeCallback(GroupCodeCallback callback);

        void run();
        void startAsync();
        void stopAsync();

    private:
        void readHeader();
        void readPayload(uint32_t payloadLength, MessageType type);
        void handleMessage(MessageType type, const std::vector<uint8_t> &payload);
        void sendMessage(MessageType type, const std::vector<uint8_t> &payload);
        void sendMessage(MessageType type, const std::string &payload);
        void writeNext();
        void handleError(const std::string &context, const boost::system::error_code &error);

        // Message handlers
        void handleAuthSuccess(const std::vector<uint8_t> &payload);
        void handleAuthFail(const std::vector<uint8_t> &payload);
        void handlePrivateMessage(const std::vector<uint8_t> &payload);
        void handleGroupMessage(const std::vector<uint8_t> &payload);
        void handleFileMetadata(const std::vector<uint8_t> &payload);
        void handleFileChunk(const std::vector<uint8_t> &payload);
        void handleFileComplete(const std::vector<uint8_t> &payload);
        void handleUserList(const std::vector<uint8_t> &payload);
        void handlePong();
        void handleError(const std::vector<uint8_t> &payload);
        void handleGroupCode(const std::vector<uint8_t> &payload);
        // Incoming file assembly buffer
        struct IncomingFile
        {
            std::string filename;
            std::string fromUserID;
            uint64_t totalSize = 0;
            uint64_t receivedSize = 0;
            std::map<uint32_t, std::vector<uint8_t>> chunks; // chunkIndex → data
        };

        boost::asio::io_context ioContext_;
        boost::asio::ip::tcp::socket socket_;
        std::unique_ptr<std::thread> ioThread_;
        std::unique_ptr<boost::asio::executor_work_guard<
            boost::asio::io_context::executor_type>>
            workGuard_;

        std::string userID_;
        std::string username_;
        std::atomic<bool> authenticated_;
        std::atomic<bool> connected_;

        AESGCM crypto_;
        AESGCM::Key encryptionKey_;
        mutable std::mutex keyMutex_;

        MessageHeader readHeader_;
        std::vector<uint8_t> readBuffer_;

        struct OutgoingMessage
        {
            MessageHeader header;
            std::vector<uint8_t> data;
        };
        std::deque<OutgoingMessage> writeQueue_;
        std::mutex writeMutex_;
        std::atomic<bool> writing_;
        // Incoming files
        std::map<std::string, IncomingFile> incomingFiles_;
        std::mutex filesMutex_;

        // Callbacks
        MessageCallback messageCallback_;
        GroupMessageCallback groupMessageCallback_;
        FileMetadataCallback fileMetadataCallback_;
        FileChunkCallback fileChunkCallback_;
        FileCompleteCallback fileCompleteCallback_;
        FileProgressCallback fileProgressCallback_;
        FileSentCallback fileSentCallback_;
        FileErrorCallback fileErrorCallback_;
        UserListCallback userListCallback_;
        ConnectionCallback connectionCallback_;
        AuthCallback authCallback_;
        GroupCodeCallback groupCodeCallback_;
        std::mutex callbackMutex_;
    };

} // namespace LanShare

#endif // CLIENTCORE_H