#pragma once
#include <string>

bool exist_or_create_user(std::string username, std::string guid); //ete ka, stugi guidy chishta or not

bool user_exists(std::string username);

bool user_online(std::string username);

void changeUserStatus(std::string username, bool status);

