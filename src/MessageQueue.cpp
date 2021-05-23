#include <messenger/MessageQueue.h>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>
#include <vector>

#include <messenger/groups.h>
#include <messenger/user.h>

MessageQueue* MessageQueue::messageQueue = new MessageQueue();

void callHandlers(MessageQueue* mq) 
{
    while (true)
    {
        std::cerr << "Before unique lock" << std::endl;
        std::unique_lock<std::mutex> m(mq->m);
        mq->cv.wait(m);
        std::cerr << "Got message wait()" << std::endl;
        std::string plain_msg = mq->messages.front().first;
        SocketParser::LoginMessageParser::LoginMessage loginMessage = mq->messages.front().second;
        mq->messages.pop();
        m.unlock();
        SocketParser::CreateGroupMessageParser createGroupMessageParser;
        std::cerr << "Action plain msg: " << plain_msg << std::endl;
        if (createGroupMessageParser.checkMessage(plain_msg))
        {
            SocketParser::CreateGroupMessageParser::CreateGroupMessage createGroupMessage
                = createGroupMessageParser.parse(plain_msg);
            if (createGroupMessage.username != loginMessage.username
                || createGroupMessage.guid != loginMessage.guid)
                {
                    std::cerr << "Create group: wrong username or guid" << std::endl;
                    continue;
                }
            std::string id = createGroup(createGroupMessage.groupName); 
            addUserToGroup(loginMessage.username, id);
            continue;
        }
        
        SocketParser::AddUserToGroupMessageParser addUserToGroupMessageParser;
        if (addUserToGroupMessageParser.checkMessage(plain_msg))
        {
            SocketParser::AddUserToGroupMessageParser::AddUserToGroupMessage addUserToGroupMessage
                = addUserToGroupMessageParser.parse(plain_msg);
            if (addUserToGroupMessage.username != loginMessage.username
                || addUserToGroupMessage.guid != loginMessage.guid)
                {
                    std::cerr << "Create group: wrong username or guid" << std::endl;
                    continue;
                }
            std::string groupId = getGroupIdFromGroupName(addUserToGroupMessage.groupName);
            for (std::string curGroupId: getGroupsOfUser(loginMessage.username))
            {
                if (curGroupId == groupId)
                {
                    addUserToGroup(addUserToGroupMessage.newUser, groupId);
                    break;
                }
            }
            continue;
        }

        SocketParser::SendMessageParser sendMessageParser;
        if (!sendMessageParser.checkMessage(plain_msg))
        {
            std::cerr << "Not send :O What could it be" << std::endl;
            continue;
        }
        SocketParser::SendMessageParser::SendMessage sendMessage
            = sendMessageParser.parse(plain_msg);
        if (sendMessage.username != loginMessage.username
            || sendMessage.guid != loginMessage.guid)
            {
                std::cerr << "Send: wrong username or guid" << std::endl;
                continue;
            }
        if (!user_exists(sendMessage.recieverUsername))
        {
            std::string groupId = getGroupIdFromGroupName(sendMessage.recieverUsername);
            bool ok = false;
            for (std::string curGroupId: getGroupsOfUser(loginMessage.username))
            {
                if (curGroupId == groupId)
                {
                    ok = true;
                    break;
                }
            }
            if (!ok)
            {
                std::cerr << "You don't belong here!" << std::endl;
                continue;
            }
        }
        SocketParser::Message message(sendMessage.username, sendMessage.recieverUsername,
            sendMessage.content, sendMessage.ts);
		for (MessageHandler handler : mq->handlers) 
        {
            try
            {
                std::cerr << "Calling handler" << std::endl;
                handler(message);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << "handler \n";
            }
            catch(const char* e)
            {
                std::cerr << e << "handler \n";
            }
		}
    }
}

MessageQueue::MessageQueue() : t(callHandlers, this) 
{
    std::cerr << "Iniit message queue" << std::endl;
}

void MessageQueue::addMessage(std::string message, SocketParser::LoginMessageParser::LoginMessage loginMessage) 
{
    std::cerr << "Add Message" << std::endl;
    {
        std::lock_guard<std::mutex> m(this->m);
        messages.push(std::make_pair(message, loginMessage));
    }
    std::cerr << "Notify" << std::endl;
    cv.notify_one();
}

void MessageQueue::registerHandler(MessageHandler handler) 
{
    handlers.insert(handler);
}

void MessageQueue::addNotification(SocketParser::Message socketMessage, std::string reciever)
{
    std::lock_guard<std::mutex> m(this->notificationMutex);
    userNotifications[reciever].push(socketMessage);
}

std::vector<SocketParser::Message> MessageQueue::getNotifications(std::string username)
{
    std::lock_guard<std::mutex> m(this->notificationMutex);
    std::vector<SocketParser::Message> ret;
    while (!userNotifications[username].empty())
    {
        auto notification = userNotifications[username].front();
        ret.emplace_back(notification.sender, notification.reciever, notification.content, notification.ts);
        userNotifications[username].pop();
    }
    return ret;
}