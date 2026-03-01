#include "AuthManager.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace LanShare {

AuthManager::AuthManager() {}

AuthManager::~AuthManager() {}

std::string AuthManager::generateSalt() const {
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("Failed to generate salt");
    }
    
    std::stringstream ss;
    for (int i = 0; i < 16; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
    }
    return ss.str();
}

std::string AuthManager::hashPassword(const std::string& password, const std::string& salt) const {
    std::string combined = password + salt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

std::string AuthManager::generateUserID(const std::string& username, const std::string& passwordHash) const {
    // Create UserID: LS-<USERNAME>-<HASH_PREFIX>
    std::string hashPrefix = passwordHash.substr(0, 4);
    std::transform(hashPrefix.begin(), hashPrefix.end(), hashPrefix.begin(), ::toupper);
    return createUserID(username, hashPrefix);
}

bool AuthManager::registerUser(const std::string& username, const std::string& password, std::string& outUserID) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if user already exists
    if (users_.find(username) != users_.end()) {
        return false;
    }
    
    // Validate username
    if (username.empty() || username.length() > 32) {
        return false;
    }
    
    // Validate password
    if (password.empty() || password.length() < 4) {
        return false;
    }
    
    // Generate salt and hash password
    std::string salt = generateSalt();
    std::string passwordHash = hashPassword(password, salt);
    
    // Generate UserID
    std::string userID = generateUserID(username, passwordHash);
    
    // Create user credentials
    UserCredentials creds;
    creds.username = username;
    creds.passwordHash = passwordHash + ":" + salt; // Store hash:salt
    creds.userID = userID;
    creds.registrationTime = std::time(nullptr);
    creds.lastLoginTime = 0;
    
    // Store credentials
    users_[username] = creds;
    userIDMap_[userID] = username;
    
    outUserID = userID;
    return true;
}

bool AuthManager::authenticateUser(const std::string& username, const std::string& password, std::string& outUserID) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }
    
    // Extract stored hash and salt
    const std::string& storedData = it->second.passwordHash;
    size_t colonPos = storedData.find(':');
    if (colonPos == std::string::npos) {
        return false;
    }
    
    std::string storedHash = storedData.substr(0, colonPos);
    std::string salt = storedData.substr(colonPos + 1);
    
    // Hash provided password with stored salt
    std::string providedHash = hashPassword(password, salt);
    
    // Compare hashes
    if (providedHash != storedHash) {
        return false;
    }
    
    // Update last login time
    it->second.lastLoginTime = std::time(nullptr);
    
    outUserID = it->second.userID;
    return true;
}

bool AuthManager::userExists(const std::string& username) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return users_.find(username) != users_.end();
}

bool AuthManager::isUserIDValid(const std::string& userID) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return userIDMap_.find(userID) != userIDMap_.end();
}

std::string AuthManager::getUsernameFromUserID(const std::string& userID) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = userIDMap_.find(userID);
    if (it != userIDMap_.end()) {
        return it->second;
    }
    return "";
}

bool AuthManager::changePassword(const std::string& userID, const std::string& oldPassword, 
                                const std::string& newPassword) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto userIDIt = userIDMap_.find(userID);
    if (userIDIt == userIDMap_.end()) {
        return false;
    }
    
    std::string username = userIDIt->second;
    auto userIt = users_.find(username);
    if (userIt == users_.end()) {
        return false;
    }
    
    // Verify old password
    const std::string& storedData = userIt->second.passwordHash;
    size_t colonPos = storedData.find(':');
    if (colonPos == std::string::npos) {
        return false;
    }
    
    std::string storedHash = storedData.substr(0, colonPos);
    std::string salt = storedData.substr(colonPos + 1);
    std::string oldHash = hashPassword(oldPassword, salt);
    
    if (oldHash != storedHash) {
        return false;
    }
    
    // Generate new salt and hash for new password
    std::string newSalt = generateSalt();
    std::string newHash = hashPassword(newPassword, newSalt);
    
    // Update password
    userIt->second.passwordHash = newHash + ":" + newSalt;
    
    return true;
}

void AuthManager::saveToFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    
    for (const auto& pair : users_) {
        const UserCredentials& creds = pair.second;
        file << creds.username << "|"
             << creds.passwordHash << "|"
             << creds.userID << "|"
             << creds.registrationTime << "|"
             << creds.lastLoginTime << "\n";
    }
}

void AuthManager::loadFromFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        // File doesn't exist yet, which is okay for first run
        return;
    }
    
    users_.clear();
    userIDMap_.clear();
    
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string username, passwordHash, userID;
        uint64_t regTime, lastLoginTime;
        
        std::getline(ss, username, '|');
        std::getline(ss, passwordHash, '|');
        std::getline(ss, userID, '|');
        ss >> regTime;
        ss.ignore(1); // Skip '|'
        ss >> lastLoginTime;
        
        UserCredentials creds;
        creds.username = username;
        creds.passwordHash = passwordHash;
        creds.userID = userID;
        creds.registrationTime = regTime;
        creds.lastLoginTime = lastLoginTime;
        
        users_[username] = creds;
        userIDMap_[userID] = username;
    }
}

} // namespace LanShare
