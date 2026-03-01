#ifndef FILERECEIVER_H
#define FILERECEIVER_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include <fstream>

namespace LanShare {

using FileReceiveProgressCallback = std::function<void(const std::string& filename, uint64_t bytesReceived, uint64_t totalBytes, int percentage)>;
using FileReceiveCompleteCallback = std::function<void(const std::string& filename, const std::string& savePath, bool success)>;

struct IncomingFile {
    std::string filename;
    std::string fromUserID;
    uint64_t totalSize;
    uint64_t receivedSize;
    std::string savePath;
    std::ofstream fileStream;
    bool isComplete;
};

class FileReceiver {
public:
    FileReceiver();
    ~FileReceiver();
    
    // Handle incoming file metadata
    void handleFileMetadata(const std::string& fromUserID, const std::string& filename, uint64_t filesize);
    
    // Handle incoming file chunk
    void handleFileChunk(const std::string& fromUserID, const std::vector<uint8_t>& chunk);
    
    // Handle file transfer complete
    void handleFileComplete(const std::string& fromUserID, const std::string& filename);
    
    // Set save directory
    void setSaveDirectory(const std::string& directory);
    std::string getSaveDirectory() const;
    
    // Transfer management
    bool hasIncomingFile(const std::string& fromUserID) const;
    IncomingFile* getIncomingFile(const std::string& fromUserID);
    void cancelTransfer(const std::string& fromUserID);
    
    // Callbacks
    void setProgressCallback(FileReceiveProgressCallback callback);
    void setCompleteCallback(FileReceiveCompleteCallback callback);
    
private:
    std::string generateSavePath(const std::string& filename);
    void extractIfCompressed(const std::string& filepath);
    
    std::string saveDirectory_;
    std::unordered_map<std::string, IncomingFile> incomingFiles_;
    
    FileReceiveProgressCallback progressCallback_;
    FileReceiveCompleteCallback completeCallback_;
};

} // namespace LanShare

#endif // FILERECEIVER_H
