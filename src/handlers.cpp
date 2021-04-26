#include <messenger/handlers.h>
#include <messenger/user.h>
#include <messenger/message.h>

void sendNotification(SocketMessage message)
{

}

void storeMessage(SocketMessage message)
{
    if (!user_exists(message.reciever))
    {
        throw "Wrong reciever";
    }
    sendMessage(message.content, message.reciever, message.sender, message.timestamp);
}