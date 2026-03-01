#ifndef AESGCM_H
#define AESGCM_H

#include <string>
#include <cstdint>
#include <vector>
#include <array>
#include <memory>
#include <stdexcept>

namespace LanShare {

class CryptoException : public std::runtime_error {
public:
    explicit CryptoException(const std::string& msg) : std::runtime_error(msg) {}
};

class AESGCM {
public:
    // Constants
    static constexpr size_t KEY_SIZE = 32;      // 256 bits
    static constexpr size_t NONCE_SIZE = 12;    // 96 bits (recommended for GCM)
    static constexpr size_t TAG_SIZE = 16;      // 128 bits
    
    using Key = std::array<uint8_t, KEY_SIZE>;
    using Nonce = std::array<uint8_t, NONCE_SIZE>;
    using Tag = std::array<uint8_t, TAG_SIZE>;
    
    // Encrypted data structure: [nonce][ciphertext][tag]
    struct EncryptedData {
        Nonce nonce;
        std::vector<uint8_t> ciphertext;
        Tag tag;
        
        // Serialize to single blob
        std::vector<uint8_t> serialize() const;
        
        // Deserialize from blob
        static EncryptedData deserialize(const std::vector<uint8_t>& blob);
        static EncryptedData deserialize(const uint8_t* data, size_t size);
    };
    
    // Constructor
    AESGCM();
    ~AESGCM();
    
    // Key Management
    static Key generateKey();
    static Key deriveKeyFromPassword(const std::string& password, const std::string& salt);
    
    // Encryption
    EncryptedData encrypt(const std::vector<uint8_t>& plaintext, const Key& key);
    EncryptedData encrypt(const std::string& plaintext, const Key& key);
    EncryptedData encrypt(const uint8_t* data, size_t size, const Key& key);
    
    // Decryption
    std::vector<uint8_t> decrypt(const EncryptedData& encrypted, const Key& key);
    std::string decryptToString(const EncryptedData& encrypted, const Key& key);
    
    // Nonce generation
    static Nonce generateNonce();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
    
    // Internal encryption/decryption
    EncryptedData encryptInternal(const uint8_t* plaintext, size_t size, const Key& key);
    std::vector<uint8_t> decryptInternal(const EncryptedData& encrypted, const Key& key);
};

// Utility functions
std::string base64Encode(const std::vector<uint8_t>& data);
std::vector<uint8_t> base64Decode(const std::string& encoded);
std::string hexEncode(const std::vector<uint8_t>& data);
std::vector<uint8_t> hexDecode(const std::string& hex);

} // namespace LanShare

#endif // AESGCM_H
