#include "AESGCM.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <cstring>
#include <sstream>
#include <iomanip>

namespace LanShare {

// PIMPL Implementation
class AESGCM::Impl {
public:
    Impl() = default;
    ~Impl() = default;
};

AESGCM::AESGCM() : pImpl(std::make_unique<Impl>()) {}

AESGCM::~AESGCM() = default;

// Generate random key
AESGCM::Key AESGCM::generateKey() {
    Key key;
    if (RAND_bytes(key.data(), KEY_SIZE) != 1) {
        throw CryptoException("Failed to generate random key");
    }
    return key;
}

// Derive key from password using SHA-256
AESGCM::Key AESGCM::deriveKeyFromPassword(const std::string& password, const std::string& salt) {
    Key key;
    std::string combined = password + salt;
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.length(), hash);
    
    std::memcpy(key.data(), hash, KEY_SIZE);
    return key;
}

// Generate random nonce
AESGCM::Nonce AESGCM::generateNonce() {
    Nonce nonce;
    if (RAND_bytes(nonce.data(), NONCE_SIZE) != 1) {
        throw CryptoException("Failed to generate random nonce");
    }
    return nonce;
}

// Internal encryption
AESGCM::EncryptedData AESGCM::encryptInternal(const uint8_t* plaintext, size_t size, const Key& key) {
    EncryptedData result;
    result.nonce = generateNonce();
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw CryptoException("Failed to create cipher context");
    }
    
    try {
        // Initialize encryption
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), result.nonce.data()) != 1) {
            throw CryptoException("Failed to initialize encryption");
        }
        
        // Allocate buffer for ciphertext (can be slightly larger than plaintext)
        result.ciphertext.resize(size + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
        
        int len = 0;
        int ciphertext_len = 0;
        
        // Encrypt the data
        if (EVP_EncryptUpdate(ctx, result.ciphertext.data(), &len, plaintext, size) != 1) {
            throw CryptoException("Encryption failed");
        }
        ciphertext_len = len;
        
        // Finalize encryption
        if (EVP_EncryptFinal_ex(ctx, result.ciphertext.data() + len, &len) != 1) {
            throw CryptoException("Failed to finalize encryption");
        }
        ciphertext_len += len;
        
        // Resize to actual size
        result.ciphertext.resize(ciphertext_len);
        
        // Get authentication tag
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, result.tag.data()) != 1) {
            throw CryptoException("Failed to get authentication tag");
        }
        
        EVP_CIPHER_CTX_free(ctx);
        
    } catch (...) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
    
    return result;
}

// Public encryption methods
AESGCM::EncryptedData AESGCM::encrypt(const std::vector<uint8_t>& plaintext, const Key& key) {
    return encryptInternal(plaintext.data(), plaintext.size(), key);
}

AESGCM::EncryptedData AESGCM::encrypt(const std::string& plaintext, const Key& key) {
    return encryptInternal(reinterpret_cast<const uint8_t*>(plaintext.data()), plaintext.size(), key);
}

AESGCM::EncryptedData AESGCM::encrypt(const uint8_t* data, size_t size, const Key& key) {
    return encryptInternal(data, size, key);
}

// Internal decryption
std::vector<uint8_t> AESGCM::decryptInternal(const EncryptedData& encrypted, const Key& key) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw CryptoException("Failed to create cipher context");
    }
    
    std::vector<uint8_t> plaintext;
    
    try {
        // Initialize decryption
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), encrypted.nonce.data()) != 1) {
            throw CryptoException("Failed to initialize decryption");
        }
        
        // Allocate buffer for plaintext
        plaintext.resize(encrypted.ciphertext.size() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
        
        int len = 0;
        int plaintext_len = 0;
        
        // Decrypt the data
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, encrypted.ciphertext.data(), encrypted.ciphertext.size()) != 1) {
            throw CryptoException("Decryption failed");
        }
        plaintext_len = len;
        
        // Set expected tag value
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, const_cast<uint8_t*>(encrypted.tag.data())) != 1) {
            throw CryptoException("Failed to set authentication tag");
        }
        
        // Finalize decryption and verify tag
        int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
        if (ret <= 0) {
            throw CryptoException("Decryption failed - authentication tag verification failed");
        }
        plaintext_len += len;
        
        // Resize to actual size
        plaintext.resize(plaintext_len);
        
        EVP_CIPHER_CTX_free(ctx);
        
    } catch (...) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
    
    return plaintext;
}

// Public decryption methods
std::vector<uint8_t> AESGCM::decrypt(const EncryptedData& encrypted, const Key& key) {
    return decryptInternal(encrypted, key);
}

std::string AESGCM::decryptToString(const EncryptedData& encrypted, const Key& key) {
    auto plaintext = decryptInternal(encrypted, key);
    return std::string(plaintext.begin(), plaintext.end());
}

// EncryptedData serialization
std::vector<uint8_t> AESGCM::EncryptedData::serialize() const {
    std::vector<uint8_t> result;
    result.reserve(NONCE_SIZE + ciphertext.size() + TAG_SIZE);
    
    // Format: [nonce][ciphertext][tag]
    result.insert(result.end(), nonce.begin(), nonce.end());
    result.insert(result.end(), ciphertext.begin(), ciphertext.end());
    result.insert(result.end(), tag.begin(), tag.end());
    
    return result;
}

AESGCM::EncryptedData AESGCM::EncryptedData::deserialize(const std::vector<uint8_t>& blob) {
    return deserialize(blob.data(), blob.size());
}

AESGCM::EncryptedData AESGCM::EncryptedData::deserialize(const uint8_t* data, size_t size) {
    if (size < NONCE_SIZE + TAG_SIZE) {
        throw CryptoException("Invalid encrypted data: too small");
    }
    
    EncryptedData result;
    
    // Extract nonce
    std::memcpy(result.nonce.data(), data, NONCE_SIZE);
    
    // Extract ciphertext
    size_t ciphertext_size = size - NONCE_SIZE - TAG_SIZE;
    result.ciphertext.resize(ciphertext_size);
    std::memcpy(result.ciphertext.data(), data + NONCE_SIZE, ciphertext_size);
    
    // Extract tag
    std::memcpy(result.tag.data(), data + NONCE_SIZE + ciphertext_size, TAG_SIZE);
    
    return result;
}

// Base64 encoding
std::string base64Encode(const std::vector<uint8_t>& data) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    
    BIO_write(b64, data.data(), data.size());
    BIO_flush(b64);
    
    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);
    
    std::string result(bptr->data, bptr->length);
    BIO_free_all(b64);
    
    return result;
}

// Base64 decoding
std::vector<uint8_t> base64Decode(const std::string& encoded) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new_mem_buf(encoded.c_str(), encoded.length());
    bmem = BIO_push(b64, bmem);
    BIO_set_flags(bmem, BIO_FLAGS_BASE64_NO_NL);
    
    std::vector<uint8_t> result(encoded.length());
    int decoded_size = BIO_read(bmem, result.data(), encoded.length());
    
    BIO_free_all(bmem);
    
    if (decoded_size < 0) {
        throw CryptoException("Base64 decode failed");
    }
    
    result.resize(decoded_size);
    return result;
}

// Hex encoding
std::string hexEncode(const std::vector<uint8_t>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

// Hex decoding
std::vector<uint8_t> hexDecode(const std::string& hex) {
    if (hex.length() % 2 != 0) {
        throw CryptoException("Invalid hex string");
    }
    
    std::vector<uint8_t> result;
    result.reserve(hex.length() / 2);
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        result.push_back(byte);
    }
    
    return result;
}

} // namespace LanShare
