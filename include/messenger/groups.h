#pragma once
#include <string>
#include <vector>

void addUserToGroup(std::string username, std::string groupId);

std::string createGroup(std::string groupName); 

std::vector<std::string> getGroupsOfUser(std::string username);  

std::vector<std::string> getUsersFromGroup(std::string groupName);  

bool isGroupActive(std::string groupId);

void changeGroupStatus(std::string groupId, bool isActive);

std::string getGroupIdFromGroupName(std::string groupName);