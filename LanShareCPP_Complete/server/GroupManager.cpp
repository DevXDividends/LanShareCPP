#include "GroupManager.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include<iostream>
#include <ctime>
#include <cstdlib>

namespace LanShare {

// Simple animal+number codes — easy to say out loud
static const char* WORDS[] = {
    "TIGER", "SHARK", "EAGLE", "COBRA", "PANDA",
    "WOLF",  "LION",  "BEAR",  "HAWK",  "FOX",
    "LYNX",  "VIPER", "RHINO", "BISON", "CRANE",
    "RAVEN", "STORM", "FLAME", "FROST", "BLADE"
};
static constexpr int WORD_COUNT = 20;

std::string GroupManager::generateCode()
{
    // seed once
    static bool seeded = false;
    if (!seeded) { std::srand(static_cast<unsigned>(std::time(nullptr))); seeded = true; }

    std::string word  = WORDS[std::rand() % WORD_COUNT];
    int         num   = 10 + std::rand() % 90; // 10-99
    return word + "-" + std::to_string(num);
}

GroupManager::GroupManager()  {}
GroupManager::~GroupManager() {}

// Returns generated join code, or "" on failure
std::string GroupManager::createGroup(const std::string& groupName,
                                      const std::string& creatorUserID)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (groups_.find(groupName) != groups_.end()) return "";
    if (groupName.empty() || groupName.length() > 64) return "";

    std::string code = generateCode();

    GroupInfo info;
    info.groupName     = groupName;
    info.creatorUserID = creatorUserID;
    info.joinCode      = code;
    info.creationTime  = std::time(nullptr);
    info.members.insert(creatorUserID);

    groups_[groupName] = info;
    std::cout << "Group created: " << groupName << " | code: " << code << "\n";
    return code;
}

bool GroupManager::deleteGroup(const std::string& groupName, const std::string& userID)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = groups_.find(groupName);
    if (it == groups_.end())               return false;
    if (it->second.creatorUserID != userID) return false;
    groups_.erase(it);
    return true;
}

bool GroupManager::joinGroup(const std::string& groupName,
                             const std::string& userID,
                             const std::string& joinCode)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = groups_.find(groupName);
    if (it == groups_.end()) return false;

    // Verify join code
    if (it->second.joinCode != joinCode) {
        std::cout << "Join rejected: wrong code for " << groupName << "\n";
        return false;
    }

    it->second.members.insert(userID);
    return true;
}

bool GroupManager::leaveGroup(const std::string& groupName, const std::string& userID)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = groups_.find(groupName);
    if (it == groups_.end()) return false;
    it->second.members.erase(userID);
    if (it->second.members.empty()) groups_.erase(it);
    return true;
}

bool GroupManager::isMember(const std::string& groupName,
                            const std::string& userID) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = groups_.find(groupName);
    if (it == groups_.end()) return false;
    return it->second.members.count(userID) > 0;
}

bool GroupManager::groupExists(const std::string& groupName) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return groups_.find(groupName) != groups_.end();
}

std::vector<std::string> GroupManager::getGroupMembers(
    const std::string& groupName) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = groups_.find(groupName);
    if (it == groups_.end()) return {};
    return std::vector<std::string>(
        it->second.members.begin(), it->second.members.end());
}

std::vector<std::string> GroupManager::getUserGroups(
    const std::string& userID) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> result;
    for (const auto& pair : groups_)
        if (pair.second.members.count(userID))
            result.push_back(pair.first);
    return result;
}

std::vector<std::string> GroupManager::getAllGroups() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> result;
    result.reserve(groups_.size());
    for (const auto& pair : groups_)
        result.push_back(pair.first);
    return result;
}

GroupInfo GroupManager::getGroupInfo(const std::string& groupName) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = groups_.find(groupName);
    if (it != groups_.end()) return it->second;
    return GroupInfo();
}

// ── Persistence ────────────────────────────────────────────────────────────
// Format: groupName|creatorID|joinCode|creationTime|member1,member2,...

void GroupManager::saveToFile(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::ofstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Failed to open: " + filename);

    for (const auto& pair : groups_) {
        const GroupInfo& info = pair.second;
        file << info.groupName    << "|"
             << info.creatorUserID << "|"
             << info.joinCode      << "|"
             << info.creationTime  << "|";

        bool first = true;
        for (const auto& member : info.members) {
            if (!first) file << ",";
            file << member;
            first = false;
        }
        file << "\n";
    }
}

void GroupManager::loadFromFile(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::ifstream file(filename);
    if (!file.is_open()) return; // file doesn't exist yet

    groups_.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string groupName, creatorID, joinCode, membersStr;
        uint64_t creationTime = 0;

        std::getline(ss, groupName,   '|');
        std::getline(ss, creatorID,   '|');
        std::getline(ss, joinCode,    '|');

        std::string timeStr;
        std::getline(ss, timeStr, '|');
        if (!timeStr.empty())
            creationTime = std::stoull(timeStr);

        std::getline(ss, membersStr);

        GroupInfo info;
        info.groupName     = groupName;
        info.creatorUserID = creatorID;
        info.joinCode      = joinCode;
        info.creationTime  = creationTime;

        std::stringstream memberSS(membersStr);
        std::string member;
        while (std::getline(memberSS, member, ','))
            if (!member.empty()) info.members.insert(member);

        groups_[groupName] = info;
    }
}

} // namespace LanShare