#include "ClientCore.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <thread>
#include <chrono>
#include <algorithm>

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
                    if (connectionCallback_) connectionCallback_(true, "Connected successfully");
                    readHeader();
                } else {
                    connected_ = false;
                    std::cerr << "Connection failed: " << error.message() << "\n";
                    if (connectionCallback_) connectionCallback_(false, error.message());
                }
            });
        
    } catch (const std::exception& e) {
        std::cerr << "Connect error: " << e.what() << "\n";
        if (connectionCallback_) connectionCallback_(false, e.what());
    }
}

void ClientCore::disconnect() {
    if (!connected_) return;
    if (authenticated_) sendMessage(MessageType::AUTH_LOGOUT, "");
    boost::system::error_code ec;
    socket_.close(ec);
    connected_ = false;
    authenticated_ = false;
    std::cout << "Disconnected from server\n";
}

bool ClientCore::isConnected() const { return connected_; }

void ClientCore::registerUser(const std::string& username, const std::string& password) {
    sendMessage(MessageType::AUTH_REGISTER, username + ":" + password);
}

void ClientCore::login(const std::string& username, const std::string& password) {
    username_ = username;
    sendMessage(MessageType::AUTH_LOGIN, username + ":" + password);
}

void ClientCore::logout() {
    sendMessage(MessageType::AUTH_LOGOUT, "");
    authenticated_ = false;
}

std::string ClientCore::getUserID() const { return userID_; }
std::string ClientCore::getUsername() const { return username_; }
bool ClientCore::isAuthenticated() const { return authenticated_; }

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

// ─────────────────────────────────────────────
//  FILE TRANSFER - SENDER SIDE
// ─────────────────────────────────────────────

void ClientCore::sendFile(const std::string& toUserID, const std::string& filepath, bool isGroup)
{
    // Run in background so UI stays responsive
    std::thread([this, toUserID, filepath, isGroup]() {
        try {
            std::ifstream file(std::filesystem::u8path(filepath), std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                std::cerr << "Cannot open file: " << filepath << "\n";
                if (fileErrorCallback_) fileErrorCallback_(toUserID, "Cannot open file");
                return;
            }

            uint64_t filesize = static_cast<uint64_t>(file.tellg());
            file.seekg(0);

            // Extract just the filename (not full path)
            std::filesystem::path p(filepath);
            std::string filename = p.filename().string();

            // 1. Send FILE_META: toUserID\0filename\0filesize(8 bytes)
            {
                std::vector<uint8_t> meta;
                meta.insert(meta.end(), toUserID.begin(), toUserID.end());
                meta.push_back(0);
                meta.insert(meta.end(), filename.begin(), filename.end());
                meta.push_back(0);
                // filesize as 8 little-endian bytes
                for (int i = 0; i < 8; i++)
                    meta.push_back((filesize >> (i * 8)) & 0xFF);

                sendMessage(isGroup ? MessageType::FILE_GROUP_META
                                    : MessageType::FILE_PRIVATE_META, meta);
            }

            // 2. Send FILE_CHUNKs
            std::vector<uint8_t> chunkBuf(CHUNK_SIZE);
            uint64_t bytesSent = 0;
            uint32_t chunkIndex = 0;

            while (bytesSent < filesize) {
                uint64_t remaining = filesize - bytesSent;
                uint32_t toRead = static_cast<uint32_t>(
                    remaining < CHUNK_SIZE ? remaining : CHUNK_SIZE);

                file.read(reinterpret_cast<char*>(chunkBuf.data()), toRead);
                std::vector<uint8_t> rawChunk(chunkBuf.begin(), chunkBuf.begin() + toRead);

                // Encrypt chunk with shared key
               std::vector<std::string> ids = {userID_, toUserID};
std::sort(ids.begin(), ids.end());
AESGCM::Key key = AESGCM::deriveKeyFromPassword(
    "file:" + ids[0] + ":" + ids[1] + ":lanshare-v1", "lanshare-salt-2024");
                auto encChunk = crypto_.encrypt(rawChunk, key);
                auto blob = encChunk.serialize();

                // Payload: toUserID\0 + chunkIndex(4 bytes) + encrypted blob
                std::vector<uint8_t> payload;
                payload.insert(payload.end(), toUserID.begin(), toUserID.end());
                payload.push_back(0);
                for (int i = 0; i < 4; i++)
                    payload.push_back((chunkIndex >> (i * 8)) & 0xFF);
                payload.insert(payload.end(), blob.begin(), blob.end());

                sendMessage(isGroup ? MessageType::FILE_GROUP_CHUNK
                                    : MessageType::FILE_PRIVATE_CHUNK, payload);

                bytesSent += toRead;
                chunkIndex++;
                                std::this_thread::sleep_for(std::chrono::milliseconds(10));


                // Progress callback
                int pct = static_cast<int>((bytesSent * 100) / filesize);
                if (fileProgressCallback_) fileProgressCallback_(toUserID, pct);
            }

            // 3. Send FILE_COMPLETE: toUserID\0filename\0
            {
                std::vector<uint8_t> done;
                done.insert(done.end(), toUserID.begin(), toUserID.end());
                done.push_back(0);
                done.insert(done.end(), filename.begin(), filename.end());
                done.push_back(0);
                sendMessage(isGroup ? MessageType::FILE_GROUP_COMPLETE
                                    : MessageType::FILE_PRIVATE_COMPLETE, done);
            }

            if (fileSentCallback_) fileSentCallback_(toUserID, filename);
            std::cout << "File sent: " << filename << " (" << filesize << " bytes)\n";

        } catch (const std::exception& e) {
            std::cerr << "File send error: " << e.what() << "\n";
            if (fileErrorCallback_) fileErrorCallback_(toUserID, e.what());
        }
    }).detach();
}

void ClientCore::sendFileChunk(const std::string& toUserID,
                               const std::vector<uint8_t>& chunk, bool isGroup) {
    // Used internally by sendFile — kept for API compatibility
}

void ClientCore::sendFileComplete(const std::string& toUserID, bool isGroup) {
    // Used internally by sendFile — kept for API compatibility
}

// ─────────────────────────────────────────────
//  FILE TRANSFER - RECEIVER SIDE
// ─────────────────────────────────────────────

void ClientCore::handleFileMetadata(const std::vector<uint8_t>& payload)
{
    size_t p = 0;
    size_t n1 = p;
    while (n1 < payload.size() && payload[n1] != 0) n1++;
    std::string fromUserID(payload.begin() + p, payload.begin() + n1);

    size_t n2 = n1 + 1, n3 = n2;
    while (n3 < payload.size() && payload[n3] != 0) n3++;
    std::string filename(payload.begin() + n2, payload.begin() + n3);

    uint64_t filesize = 0;
    if (n3 + 1 + 8 <= payload.size())
        for (int i = 0; i < 8; i++)
            filesize |= (uint64_t(payload[n3 + 1 + i]) << (i * 8));

    std::cout << "Incoming file: " << filename
              << " from " << fromUserID
              << " (" << filesize << " bytes)\n";

    {
        std::lock_guard<std::mutex> lock(filesMutex_);
        incomingFiles_.erase(fromUserID);  // fully remove old entry
        IncomingFile f;
        f.filename     = filename;
        f.fromUserID   = fromUserID;
        f.totalSize    = filesize;
        f.receivedSize = 0;
        incomingFiles_[fromUserID] = std::move(f);
    }

    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (fileMetadataCallback_)
        fileMetadataCallback_(fromUserID, filename, filesize);
}

void ClientCore::handleFileChunk(const std::vector<uint8_t>& payload)
{
    size_t n1 = 0;
    while (n1 < payload.size() && payload[n1] != 0) n1++;
    std::string fromUserID(payload.begin(), payload.begin() + n1);

    if (n1 + 1 + 4 > payload.size()) return;

    uint32_t chunkIndex = 0;
    for (int i = 0; i < 4; i++)
        chunkIndex |= (uint32_t(payload[n1 + 1 + i]) << (i * 8));

    std::vector<uint8_t> blob(payload.begin() + n1 + 1 + 4, payload.end());

    // Decrypt
   std::vector<std::string> ids = {userID_, fromUserID};
std::sort(ids.begin(), ids.end());
AESGCM::Key key = AESGCM::deriveKeyFromPassword(
    "file:" + ids[0] + ":" + ids[1] + ":lanshare-v1", "lanshare-salt-2024");

    std::vector<uint8_t> rawChunk;
    try {
        auto encData = AESGCM::EncryptedData::deserialize(blob);
        rawChunk = crypto_.decrypt(encData, key);
    } catch (const std::exception& e) {
        std::cerr << "Chunk decrypt error: " << e.what() << "\n";
        return;  // ← don't crash, just skip bad chunk
    }

    {
        std::lock_guard<std::mutex> lock(filesMutex_);
        // Key is fromUserID — matches handleFileMetadata
        auto it = incomingFiles_.find(fromUserID);
        if (it == incomingFiles_.end()) {
            std::cerr << "No incoming file for: " << fromUserID << "\n";
            return;
        }

        IncomingFile& f = it->second;
        f.chunks[chunkIndex] = rawChunk;
        f.receivedSize += rawChunk.size();

        int pct = f.totalSize > 0
            ? static_cast<int>((f.receivedSize * 100) / f.totalSize)
            : 0;

        if (fileProgressCallback_) fileProgressCallback_(fromUserID, pct);
        if (fileChunkCallback_)
            fileChunkCallback_(fromUserID, rawChunk,
                static_cast<uint64_t>(chunkIndex) * CHUNK_SIZE);
    }
}

void ClientCore::handleFileComplete(const std::vector<uint8_t>& payload)
{
    size_t n1 = 0;
    while (n1 < payload.size() && payload[n1] != 0) n1++;
    std::string fromUserID(payload.begin(), payload.begin() + n1);

    size_t n2 = n1 + 1, n3 = n2;
    while (n3 < payload.size() && payload[n3] != 0) n3++;
    std::string filename(payload.begin() + n2, payload.begin() + n3);

    std::cout << "File complete: " << filename << " from " << fromUserID << "\n";

    {
        std::lock_guard<std::mutex> lock(filesMutex_);
        // Key is fromUserID — matches handleFileMetadata
        auto it = incomingFiles_.find(fromUserID);
        if (it == incomingFiles_.end()) {
            std::cerr << "No file to complete for: " << fromUserID << "\n";
            return;
        }

        IncomingFile& f = it->second;

        // Build Downloads path
        std::string downloadsPath;
#ifdef _WIN32
        const char* home = getenv("USERPROFILE");
        downloadsPath = home ? std::string(home) + "\\Downloads\\" : ".\\";
#else
        const char* home = getenv("HOME");
        downloadsPath = home ? std::string(home) + "/Downloads/" : "./";
#endif
        std::string outPath = downloadsPath + f.filename;

        std::ofstream outFile(std::filesystem::u8path(outPath), std::ios::binary);
        if (outFile.is_open()) {
            for (uint32_t i = 0; i < (uint32_t)f.chunks.size(); i++) {
                auto cit = f.chunks.find(i);
                if (cit != f.chunks.end())
                    outFile.write(reinterpret_cast<const char*>(cit->second.data()),
                                  cit->second.size());
            }
            outFile.close();
            std::cout << "Saved: " << outPath << "\n";
        } else {
            std::cerr << "Cannot write file: " << outPath << "\n";
        }

        incomingFiles_.erase(it);
    }

    std::lock_guard<std::mutex> cbLock(callbackMutex_);
    if (fileCompleteCallback_) fileCompleteCallback_(fromUserID, filename);
}

// ─────────────────────────────────────────────
//  GROUP MANAGEMENT
// ─────────────────────────────────────────────

void ClientCore::createGroup(const std::string& groupName) {
    sendMessage(MessageType::GROUP_CREATE, groupName);
}

void ClientCore::joinGroup(const std::string& groupName, const std::string& joinCode) {
    sendMessage(MessageType::GROUP_JOIN, groupName + "|" + joinCode);
}

void ClientCore::leaveGroup(const std::string& groupName) {
    sendMessage(MessageType::GROUP_LEAVE, groupName);
}

void ClientCore::requestUserList() {
    sendMessage(MessageType::USER_LIST, "");
}

void ClientCore::requestGroupList() {
    sendMessage(MessageType::GROUP_LIST, "");
}

// ─────────────────────────────────────────────
//  CRYPTO
// ─────────────────────────────────────────────

AESGCM& ClientCore::getCrypto() { return crypto_; }

void ClientCore::setEncryptionKey(const AESGCM::Key& key) {
    std::lock_guard<std::mutex> lock(keyMutex_);
    encryptionKey_ = key;
}

AESGCM::Key ClientCore::getEncryptionKey() const {
    std::lock_guard<std::mutex> lock(keyMutex_);
    return encryptionKey_;
}

// ─────────────────────────────────────────────
//  CALLBACKS
// ─────────────────────────────────────────────

void ClientCore::setMessageCallback(MessageCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_); messageCallback_ = cb; }
void ClientCore::setGroupMessageCallback(GroupMessageCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_); groupMessageCallback_ = cb; }
void ClientCore::setFileMetadataCallback(FileMetadataCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_); fileMetadataCallback_ = cb; }
void ClientCore::setFileChunkCallback(FileChunkCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_); fileChunkCallback_ = cb; }
void ClientCore::setFileCompleteCallback(FileCompleteCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_); fileCompleteCallback_ = cb; }
void ClientCore::setFileProgressCallback(FileProgressCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_); fileProgressCallback_ = cb; }
void ClientCore::setFileSentCallback(FileSentCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_); fileSentCallback_ = cb; }
void ClientCore::setFileErrorCallback(FileErrorCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_); fileErrorCallback_ = cb; }
void ClientCore::setUserListCallback(UserListCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_); userListCallback_ = cb; }
void ClientCore::setConnectionCallback(ConnectionCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_); connectionCallback_ = cb; }
void ClientCore::setAuthCallback(AuthCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_); authCallback_ = cb; }
    void ClientCore::setGroupCodeCallback(GroupCodeCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_); groupCodeCallback_ = cb; }

// ─────────────────────────────────────────────
//  IO / ASYNC
// ─────────────────────────────────────────────

void ClientCore::run() { ioContext_.run(); }

void ClientCore::startAsync() {
    workGuard_ = std::make_unique<boost::asio::executor_work_guard<
        boost::asio::io_context::executor_type>>(
        boost::asio::make_work_guard(ioContext_));
    ioThread_ = std::make_unique<std::thread>([this]() { ioContext_.run(); });
}

void ClientCore::stopAsync() {
    if (workGuard_) { workGuard_->reset(); workGuard_.reset(); }
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
                if (readHeader_.length <= MAX_MESSAGE_SIZE)
                    readPayload(readHeader_.length, readHeader_.type);
                else { std::cerr << "Message too large\n"; disconnect(); }
            } else { handleError("readHeader", error); }
        });
}

void ClientCore::readPayload(uint32_t payloadLength, MessageType type) {
    readBuffer_.resize(payloadLength);
    boost::asio::async_read(socket_,
        boost::asio::buffer(readBuffer_),
        [this, type](const boost::system::error_code& error, std::size_t) {
            if (!error) { handleMessage(type, readBuffer_); readHeader(); }
            else { handleError("readPayload", error); }
        });
}

void ClientCore::handleMessage(MessageType type, const std::vector<uint8_t>& payload) {
    switch (type) {
        case MessageType::AUTH_SUCCESS:       handleAuthSuccess(payload); break;
        case MessageType::AUTH_FAIL:          handleAuthFail(payload); break;
        case MessageType::MSG_PRIVATE:        handlePrivateMessage(payload); break;
        case MessageType::MSG_GROUP:          handleGroupMessage(payload); break;
        case MessageType::FILE_PRIVATE_META:
        case MessageType::FILE_GROUP_META:    handleFileMetadata(payload); break;
        case MessageType::FILE_PRIVATE_CHUNK:
        case MessageType::FILE_GROUP_CHUNK:   handleFileChunk(payload); break;
        case MessageType::FILE_PRIVATE_COMPLETE:
        case MessageType::FILE_GROUP_COMPLETE: handleFileComplete(payload); break;
        case MessageType::USER_LIST:          handleUserList(payload); break;
        case MessageType::PONG:               handlePong(); break;
        case MessageType::ERROR_MSG:          handleError(payload); break;
        case MessageType::GROUP_CODE:         handleGroupCode(payload); break;
        default: std::cerr << "Unknown message type\n";
    }
}

void ClientCore::handleAuthSuccess(const std::vector<uint8_t>& payload) {
    userID_ = std::string(payload.begin(), payload.end());
    authenticated_ = true;
    std::cout << "Authenticated: " << userID_ << "\n";
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (authCallback_) authCallback_(true, userID_, "Authentication successful");
}

void ClientCore::handleAuthFail(const std::vector<uint8_t>& payload) {
    std::string reason(payload.begin(), payload.end());
    authenticated_ = false;
    std::cerr << "Auth failed: " << reason << "\n";
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (authCallback_) authCallback_(false, "", reason);
}

void ClientCore::handlePrivateMessage(const std::vector<uint8_t>& payload) {
    size_t nullPos = 0;
    while (nullPos < payload.size() && payload[nullPos] != 0) ++nullPos;
    if (nullPos >= payload.size()) return;
    std::string fromUserID(payload.begin(), payload.begin() + nullPos);
    std::vector<uint8_t> encryptedData(payload.begin() + nullPos + 1, payload.end());
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (messageCallback_) messageCallback_(fromUserID, encryptedData, std::time(nullptr));
}

void ClientCore::handleGroupMessage(const std::vector<uint8_t>& payload) {
    size_t n1 = 0;
    while (n1 < payload.size() && payload[n1] != 0) ++n1;
    if (n1 >= payload.size()) return;
    size_t n2 = n1 + 1;
    while (n2 < payload.size() && payload[n2] != 0) ++n2;
    if (n2 >= payload.size()) return;
    std::string groupName(payload.begin(), payload.begin() + n1);
    std::string fromUserID(payload.begin() + n1 + 1, payload.begin() + n2);
    std::vector<uint8_t> encryptedData(payload.begin() + n2 + 1, payload.end());
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (groupMessageCallback_)
        groupMessageCallback_(groupName, fromUserID, encryptedData, std::time(nullptr));
}

void ClientCore::handleUserList(const std::vector<uint8_t>& payload) {
    std::string data(payload.begin(), payload.end());
    std::vector<std::string> users;
    size_t start = 0, end = data.find(',');
    while (end != std::string::npos) {
        users.push_back(data.substr(start, end - start));
        start = end + 1; end = data.find(',', start);
    }
    if (start < data.length()) users.push_back(data.substr(start));
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (userListCallback_) userListCallback_(users);
}

void ClientCore::handlePong() {}
void ClientCore::handleGroupCode(const std::vector<uint8_t>& payload) {
    std::string data(payload.begin(), payload.end());
    size_t sep = data.find('|');
    if (sep == std::string::npos) return;
    std::string groupName = data.substr(0, sep);
    std::string code      = data.substr(sep + 1);
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (groupCodeCallback_) groupCodeCallback_(groupName, code);
}

void ClientCore::handleError(const std::vector<uint8_t>& payload) {
    std::cerr << "Server error: " << std::string(payload.begin(), payload.end()) << "\n";
}

void ClientCore::sendMessage(MessageType type, const std::vector<uint8_t>& payload) {
    OutgoingMessage msg;
    msg.header.length = static_cast<uint32_t>(payload.size());
    msg.header.type = type;
    msg.data = payload;

    bool needsStart = false;
    {
        std::lock_guard<std::mutex> lock(writeMutex_);
        writeQueue_.push_back(msg);
        if (!writing_.exchange(true))   // atomically set true, returns old value
            needsStart = true;
    }
    if (needsStart)
        boost::asio::post(ioContext_, [this]() { writeNext(); });
}

void ClientCore::sendMessage(MessageType type, const std::string& payload) {
    sendMessage(type, std::vector<uint8_t>(payload.begin(), payload.end()));
}

void ClientCore::writeNext() {
    std::unique_lock<std::mutex> lock(writeMutex_);
    if (writeQueue_.empty()) {
        writing_ = false;
        return;
    }

    // Copy the front message so the buffer stays alive during async_write
    OutgoingMessage msg = writeQueue_.front();
    lock.unlock();

    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(&msg.header, sizeof(MessageHeader)));
    buffers.push_back(boost::asio::buffer(msg.data));

    boost::asio::async_write(socket_, buffers,
        [this, msg](const boost::system::error_code& error, std::size_t) {
            if (!error) {
                std::lock_guard<std::mutex> lock(writeMutex_);
                writeQueue_.pop_front();
                if (!writeQueue_.empty())
                    boost::asio::post(ioContext_, [this]() { writeNext(); });
                else
                    writing_ = false;
            } else {
                handleError("write", error);
            }
        });
}

void ClientCore::handleError(const std::string& context, const boost::system::error_code& error) {
    if (error != boost::asio::error::eof &&
        error != boost::asio::error::connection_reset)
        std::cerr << "Error in " << context << ": " << error.message() << "\n";
    disconnect();
}

} // namespace LanShare