#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

namespace LanShare {

struct GroupInfo {
    std::string groupName;
    std::string creatorUserID;
    std::unordered_set<std::string> members;
    uint64_t creationTime;
};

class GroupManager {
public:
    GroupManager();
    ~GroupManager();
    
    // Group creation and deletion
    bool createGroup(const std::string& groupName, const std::string& creatorUserID);
    bool deleteGroup(const std::string& groupName, const std::string& userID);
    
    // Group membership
    bool joinGroup(const std::string& groupName, const std::string& userID);
    bool leaveGroup(const std::string& groupName, const std::string& userID);
    bool isMember(const std::string& groupName, const std::string& userID) const;
    
    // Group queries
    bool groupExists(const std::string& groupName) const;
    std::vector<std::string> getGroupMembers(const std::string& groupName) const;
    std::vector<std::string> getUserGroups(const std::string& userID) const;
    std::vector<std::string> getAllGroups() const;
    GroupInfo getGroupInfo(const std::string& groupName) const;
    
    // Persistence
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);
    
private:
    std::unordered_map<std::string, GroupInfo> groups_;
    mutable std::mutex mutex_;
};

} // namespace LanShare

#endif // GROUPMANAGER_H
