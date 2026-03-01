#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include "../common/Protocol.h"
#include <string>
#include <unordered_map>
#include <mutex>

namespace LanShare {

struct UserCredentials {
    std::string username;
    std::string passwordHash;
    std::string userID;
    uint64_t registrationTime;
    uint64_t lastLoginTime;
};

class AuthManager {
public:
    AuthManager();
    ~AuthManager();
    
    // User registration
    bool registerUser(const std::string& username, const std::string& password, std::string& outUserID);
    
    // User login
    bool authenticateUser(const std::string& username, const std::string& password, std::string& outUserID);
    
    // User queries
    bool userExists(const std::string& username) const;
    bool isUserIDValid(const std::string& userID) const;
    std::string getUsernameFromUserID(const std::string& userID) const;
    
    // Password management
    bool changePassword(const std::string& userID, const std::string& oldPassword, 
                       const std::string& newPassword);
    
    // Persistence
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);
    
private:
    std::string hashPassword(const std::string& password, const std::string& salt) const;
    std::string generateSalt() const;
    std::string generateUserID(const std::string& username, const std::string& passwordHash) const;
    
    std::unordered_map<std::string, UserCredentials> users_; // username -> credentials
    std::unordered_map<std::string, std::string> userIDMap_; // userID -> username
    mutable std::mutex mutex_;
};

} // namespace LanShare

#endif // AUTHMANAGER_H
