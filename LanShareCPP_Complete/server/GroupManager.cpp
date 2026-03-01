#include "GroupManager.h"
#include <algorithm>
#include <fstream>
#include <sstream>

#include <ctime>

namespace LanShare {

GroupManager::GroupManager() {}

GroupManager::~GroupManager() {}

bool GroupManager::createGroup(const std::string& groupName, const std::string& creatorUserID) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if group already exists
    if (groups_.find(groupName) != groups_.end()) {
        return false;
    }
    
    // Validate group name
    if (groupName.empty() || groupName.length() > 64) {
        return false;
    }
    
    // Create group
    GroupInfo info;
    info.groupName = groupName;
    info.creatorUserID = creatorUserID;
    info.members.insert(creatorUserID);
    info.creationTime = std::time(nullptr);
    
    groups_[groupName] = info;
    return true;
}

bool GroupManager::deleteGroup(const std::string& groupName, const std::string& userID) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = groups_.find(groupName);
    if (it == groups_.end()) {
        return false;
    }
    
    // Only creator can delete group
    if (it->second.creatorUserID != userID) {
        return false;
    }
    
    groups_.erase(it);
    return true;
}

bool GroupManager::joinGroup(const std::string& groupName, const std::string& userID) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = groups_.find(groupName);
    if (it == groups_.end()) {
        return false;
    }
    
    // Add user to group
    it->second.members.insert(userID);
    return true;
}

bool GroupManager::leaveGroup(const std::string& groupName, const std::string& userID) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = groups_.find(groupName);
    if (it == groups_.end()) {
        return false;
    }
    
    // Remove user from group
    it->second.members.erase(userID);
    
    // If group is empty, delete it
    if (it->second.members.empty()) {
        groups_.erase(it);
    }
    
    return true;
}

bool GroupManager::isMember(const std::string& groupName, const std::string& userID) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = groups_.find(groupName);
    if (it == groups_.end()) {
        return false;
    }
    
    return it->second.members.find(userID) != it->second.members.end();
}

bool GroupManager::groupExists(const std::string& groupName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return groups_.find(groupName) != groups_.end();
}

std::vector<std::string> GroupManager::getGroupMembers(const std::string& groupName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = groups_.find(groupName);
    if (it == groups_.end()) {
        return {};
    }
    
    return std::vector<std::string>(it->second.members.begin(), it->second.members.end());
}

std::vector<std::string> GroupManager::getUserGroups(const std::string& userID) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> result;
    for (const auto& pair : groups_) {
        if (pair.second.members.find(userID) != pair.second.members.end()) {
            result.push_back(pair.first);
        }
    }
    return result;
}

std::vector<std::string> GroupManager::getAllGroups() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> result;
    result.reserve(groups_.size());
    for (const auto& pair : groups_) {
        result.push_back(pair.first);
    }
    return result;
}

GroupInfo GroupManager::getGroupInfo(const std::string& groupName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = groups_.find(groupName);
    if (it != groups_.end()) {
        return it->second;
    }
    return GroupInfo();
}

void GroupManager::saveToFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    
    for (const auto& pair : groups_) {
        const GroupInfo& info = pair.second;
        file << info.groupName << "|"
             << info.creatorUserID << "|"
             << info.creationTime << "|";
        
        // Write members
        bool first = true;
        for (const auto& member : info.members) {
            if (!first) file << ",";
            file << member;
            first = false;
        }
        file << "\n";
    }
}

void GroupManager::loadFromFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        // File doesn't exist yet
        return;
    }
    
    groups_.clear();
    
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string groupName, creatorUserID, membersStr;
        uint64_t creationTime;
        
        std::getline(ss, groupName, '|');
        std::getline(ss, creatorUserID, '|');
        ss >> creationTime;
        ss.ignore(1); // Skip '|'
        std::getline(ss, membersStr);
        
        GroupInfo info;
        info.groupName = groupName;
        info.creatorUserID = creatorUserID;
        info.creationTime = creationTime;
        
        // Parse members
        std::stringstream memberSS(membersStr);
        std::string member;
        while (std::getline(memberSS, member, ',')) {
            if (!member.empty()) {
                info.members.insert(member);
            }
        }
        
        groups_[groupName] = info;
    }
}

} // namespace LanShare
