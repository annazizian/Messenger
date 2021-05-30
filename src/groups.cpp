#include <string>
#include <messenger/groups.h>
#include <messenger/helpers.h>
#include <messenger/models.h>
#include <messenger/connection.h>

void addUserToGroup(std::string username, std::string groupId) 
{
    messenger::UserGroup ug;
    auto &db = *(Connection().get_db());
    db(insert_into(ug).set(ug.groupId = groupId, ug.username = username));
}

std::string createGroup(std::string groupName)
{
    messenger::Groups g;
    auto &db = *(Connection().get_db());
    std::string id = generate_guid();
    db(insert_into(g).set(g.groupName = groupName, g.groupId = id, g.createDate = get_ts(), 
    g.isActive = false));
    return id;
}

std::string getGroupIdFromGroupName(std::string groupName)
{
    messenger::Groups g;
    auto &db = *(Connection().get_db());
    for (const auto& row : db(select(g.groupId).from(g).where(g.groupName == groupName)))
        return row.groupId;
    return "";   
}

std::vector<std::string> getGroupsOfUser(std::string username)
{
    messenger::UserGroup ug;
    auto &db = *(Connection().get_db());
    std::vector<std::string> vec;
    for (const auto& row : db(select(ug.groupId).from(ug).where(ug.username == username)))
        vec.push_back(row.groupId);
    return vec;  
}

bool isGroupActive(std::string groupId)
{
    messenger::User u;
    messenger::UserGroup ug;
    auto &db = *(Connection().get_db());
    for (const auto& row : db(select(u.status).from(u.join(ug).on(u.username == ug.username))
    .where(ug.groupId == groupId)))
        if (row.status == true)
        {
            // for each user in the group check if the user is online
            // if he is, then the group is online too
            return true;
        } 
    return false;
}

void changeGroupStatus(std::string groupId, bool isActive)
{
    messenger::Groups g;
    auto &db = *(Connection().get_db());
    db(update(g).set(g.isActive = isActive).where(g.groupId == groupId));
}

std::vector<std::string> getUsersFromGroup(std::string groupName)
{
    messenger::UserGroup ug;
    auto &db = *(Connection().get_db());
    std::vector<std::string> vec;
    for (const auto& row : db(select(ug.username).from(ug).where(ug.groupId == getGroupIdFromGroupName(groupName))))
        vec.push_back(row.username);
    return vec;  
}