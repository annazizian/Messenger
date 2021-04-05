#pragma once
#include "models.h"
#include "connection.h" 
#include <string>

bool exist_or_create_user(std::string username, std::string guid) //ete ka, stugi guidy chishta or not
{
    messenger::User u;
    auto &db = *(Connection().get_db());
    for (const auto& row : db(select(u.privateGuid).from(u).where(u.username==username)))
        return row.privateGuid == guid;  
    db(insert_into(u).set(u.username = username, u.status = false, u.privateGuid = guid));
    return true; 
}

bool user_exists(std::string username)
{
    messenger::User u;
    auto &db = *(Connection().get_db());
    for (const auto& row :  db(select(u.username).from(u).where(u.username == username)))
        return true;
    return false;
}