#include <messenger/message.h>
#include <messenger/groups.h>
#include <messenger/user.h>

void sendMessage(std::string content, std::string reciever, std::string sender, int ts) // reciever can be either username or groupId
{
    messenger::Message m;
    auto &db = *(Connection().get_db());
    db(insert_into(m).set(m.messageGUID = generate_guid(), 
                          m.recieverUsername = reciever,
                          m.senderUsername = sender, 
                          m.timestamp = ts,
                          m.content = content,
                          m.isGroup = !user_exists(reciever))
      );                     
}

std::vector<SocketParser::Message> getMessages(std::string username, std::string usernameFrom, unsigned int limit)
{
    messenger::Message m;
    auto &db = *(Connection().get_db());
    
    std::vector<SocketParser::Message> ret_val;
    
    if (user_exists(usernameFrom))
    {
        auto condition = (m.recieverUsername == usernameFrom and m.senderUsername == username)
                    or (m.senderUsername == usernameFrom and m.recieverUsername == username);

        auto query = select(m.senderUsername, m.content, m.recieverUsername, m.timestamp).from(m).where(condition)
                    .order_by(m.timestamp.desc()).limit(limit);
        for (const auto& row : db(query))
        {
            ret_val.emplace_back(row.senderUsername, row.recieverUsername, row.content, row.timestamp);
        }
    }
    else
    {
        bool ok = false;
        std::string groupId = getGroupIdFromGroupName(usernameFrom);
        for (std::string curGroupId: getGroupsOfUser(username))
        {
            if (curGroupId == groupId)
            {
                ok = true;
                break;
            }
        }
        if (ok)
        {
            auto condition = m.recieverUsername == usernameFrom;

            auto query = select(m.senderUsername, m.content, m.recieverUsername, m.timestamp).from(m).where(condition)
                        .order_by(m.timestamp.desc()).limit(limit);
            for (const auto& row : db(query))
            {
                ret_val.emplace_back(row.senderUsername, row.recieverUsername, row.content, row.timestamp);
            }
        }
    }
    return ret_val;
} 