#pragma once
#include <string>
#include "sqlpp11/sqlpp11.h"
#include "models.h"
#include "connection.h"
#include "helpers.h"
#include <vector>

void sendMessage(std::string content, std::string reciever, std::string sender, int ts)
{
    messenger::Message m;
    auto &db = *(Connection().get_db());
    db(insert_into(m).set(m.messageGUID = generate_guid(), 
                          m.recieverUsername = reciever,
                          m.senderUsername = sender, 
                          m.timestamp = ts,
                          m.content = content)
      );                     
}

std::vector<std::pair<std::string, std::string> > getMessages(std::string username, unsigned int limit = 5)
{
    messenger::Message m;
    auto &db = *(Connection().get_db());   
    auto query = select(m.senderUsername, m.content).from(m).where(m.recieverUsername == username)
                .order_by(m.timestamp.desc()).limit(limit);
    std::vector<std::pair<std::string, std::string> > ret_val;
    for (const auto& row :  db(query))
    {
        ret_val.emplace_back(row.senderUsername, row.content); //emplace_back-ը զույգ ա սարքում ու փուշ ա անում
    }
    return ret_val;
} 