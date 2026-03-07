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
    std::string joinCode;          // ★ e.g. "SHARK-42"
    std::unordered_set<std::string> members;
    uint64_t creationTime;
};

class GroupManager {
public:
    GroupManager();
    ~GroupManager();

    // Returns generated join code on success, empty string on failure
    std::string createGroup(const std::string& groupName,
                            const std::string& creatorUserID);

    bool deleteGroup(const std::string& groupName, const std::string& userID);

    // joinGroup now requires the code
    bool joinGroup(const std::string& groupName,
                   const std::string& userID,
                   const std::string& joinCode);

    bool leaveGroup(const std::string& groupName, const std::string& userID);
    bool isMember(const std::string& groupName, const std::string& userID) const;

    bool groupExists(const std::string& groupName) const;
    std::vector<std::string> getGroupMembers(const std::string& groupName) const;
    std::vector<std::string> getUserGroups(const std::string& userID) const;
    std::vector<std::string> getAllGroups() const;
    GroupInfo getGroupInfo(const std::string& groupName) const;

    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);

private:
    std::string generateCode();   // generates e.g. "TIGER-57"

    std::unordered_map<std::string, GroupInfo> groups_;
    mutable std::mutex mutex_;
};

} // namespace LanShare

#endif // GROUPMANAGER_H