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
        std::unique_lock<std::mutex> m(mq->m);
        mq->cv.wait(m);
        // get commands from the queue
        std::string plain_msg = mq->messages.front().first;
        SocketParser::LoginMessageParser::LoginMessage loginMessage = mq->messages.front().second;
        mq->messages.pop();
        m.unlock();
        // if the user wanted to create a group
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
            // create the group and add the creator to the group
            std::string id = createGroup(createGroupMessage.groupName); 
            addUserToGroup(loginMessage.username, id);
            continue;
        }
        
        // if the user wants to add a new member to the group
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
            // check if the user belongs to the group
            std::string groupId = getGroupIdFromGroupName(addUserToGroupMessage.groupName);
            for (std::string curGroupId: getGroupsOfUser(loginMessage.username))
            {
                if (curGroupId == groupId)
                {
                    // if he dose: add the new user to the group
                    addUserToGroup(addUserToGroupMessage.newUser, groupId);
                    break;
                }
            }
            continue;
        }

        // if the user didn't send a message either, then he did something else
        // something we don't expect. It's an error
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
            // if the message is sent to the group
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
            // check if user belongs to the group
            if (!ok)
            {
                std::cerr << "You don't belong here!" << std::endl;
                // if he doesn't, then skip this command
                continue;
            }
        }
        // create the message and call handlers
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