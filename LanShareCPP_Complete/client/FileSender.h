#ifndef FILESENDER_H
#define FILESENDER_H

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace LanShare {

class ClientCore;

using FileProgressCallback = std::function<void(uint64_t bytesSent, uint64_t totalBytes, int percentage)>;
using FileCompleteCallback = std::function<void(bool success, const std::string& message)>;

class FileSender {
public:
    explicit FileSender(ClientCore& client);
    ~FileSender();
    
    // Send file to user or group
    void sendFile(const std::string& filepath, const std::string& targetID, bool isGroup = false);
    
    // Send folder (will be compressed first)
    void sendFolder(const std::string& folderPath, const std::string& targetID, bool isGroup = false);
    
    // Cancel ongoing transfer
    void cancelTransfer();
    
    // Status
    bool isTransferring() const;
    std::string getCurrentFile() const;
    uint64_t getBytesTransferred() const;
    uint64_t getTotalBytes() const;
    int getProgress() const;
    
    // Callbacks
    void setProgressCallback(FileProgressCallback callback);
    void setCompleteCallback(FileCompleteCallback callback);
    
private:
    void sendFileInternal(const std::string& filepath, const std::string& targetID, bool isGroup);
    std::vector<uint8_t> compressFolder(const std::string& folderPath);
    
    ClientCore& client_;
    
    bool transferring_;
    std::string currentFile_;
    uint64_t bytesTransferred_;
    uint64_t totalBytes_;
    
    FileProgressCallback progressCallback_;
    FileCompleteCallback completeCallback_;
};

} // namespace LanShare

#endif // FILESENDER_H
