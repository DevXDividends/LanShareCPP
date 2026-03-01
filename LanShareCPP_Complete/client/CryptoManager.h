#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include "../common/AESGCM.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace LanShare {

class CryptoManager {
public:
    CryptoManager();
    ~CryptoManager();
    
    // Key management
    void setUserKey(const AESGCM::Key& key);
    AESGCM::Key getUserKey() const;
    
    // Per-user/group keys (for future enhancement)
    void setConversationKey(const std::string& conversationID, const AESGCM::Key& key);
    AESGCM::Key getConversationKey(const std::string& conversationID) const;
    bool hasConversationKey(const std::string& conversationID) const;
    void removeConversationKey(const std::string& conversationID);
    
    // Message encryption/decryption
    std::vector<uint8_t> encryptMessage(const std::string& message);
    std::vector<uint8_t> encryptMessage(const std::string& message, const std::string& conversationID);
    std::string decryptMessage(const std::vector<uint8_t>& encryptedData);
    std::string decryptMessage(const std::vector<uint8_t>& encryptedData, const std::string& conversationID);
    
    // File encryption/decryption
    std::vector<uint8_t> encryptFile(const std::vector<uint8_t>& fileData);
    std::vector<uint8_t> decryptFile(const std::vector<uint8_t>& encryptedData);
    
    // Key derivation
    static AESGCM::Key deriveKeyFromPassword(const std::string& password, const std::string& salt);
    
    // Utility
    std::string encryptedDataToBase64(const std::vector<uint8_t>& encryptedData);
    std::vector<uint8_t> base64ToEncryptedData(const std::string& base64);
    
private:
    AESGCM crypto_;
    AESGCM::Key userKey_;
    std::unordered_map<std::string, AESGCM::Key> conversationKeys_;
    mutable std::mutex mutex_;
};

} // namespace LanShare

#endif // CRYPTOMANAGER_H
