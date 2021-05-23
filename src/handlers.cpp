#include <messenger/handlers.h>
#include <messenger/user.h>
#include <messenger/message.h>
#include <messenger/groups.h>

void sendNotification(SocketParser::Message message)
{
    std::vector<std::string> vec;
    if (user_exists(message.reciever))
        vec.push_back(message.reciever);
    else
        vec = getUsersFromGroup(message.reciever);
    for (std::string reciever : vec)
    {
        std::cerr << "Notifying user: " << reciever << std::endl;
        if (user_online(reciever))
        {
            std::cerr << "Notification: " << reciever << "  " << message.sender << " " << message.content << std::endl;
            MessageQueue::messageQueue->addNotification(message, reciever);
        }
    }
}

void storeMessage(SocketParser::Message message)
{
    sendMessage(message.content, message.reciever, message.sender, message.ts);
} 