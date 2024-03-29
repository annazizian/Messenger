#include <string>
#include <messenger/user.h>
#include <messenger/models.h>
#include <messenger/connection.h>

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

bool user_online(std::string username)
{
    messenger::User u;
    auto &db = *(Connection().get_db());
    for (const auto& row :  db(select(u.status).from(u).where(u.username == username)))
        return row.status;
    return false;
}

void changeUserStatus(std::string username, bool status) 
{
    messenger::User u;
    auto &db = *(Connection().get_db());
    db(update(u).set(u.status = status).where(u.username == username));
}