#include <messenger/MessageQueue.h>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>


MessageQueue* MessageQueue::messageQueue = new MessageQueue();

void callHandlers(MessageQueue* mq) 
{
    while (true)
    {
        std::cerr << "Before unique lock" << std::endl;
        std::unique_lock<std::mutex> m(mq->m);
        mq->cv.wait(m);
        std::cerr << "Got message wait()" << std::endl;
        std::string plain_msg = mq->messages.front();
        SocketMessage msg;
        try
        {
            msg = mq->parse(plain_msg);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << " Parse" << std::endl;
            m.unlock();
            continue;
        }
        catch(char const *e)
        {
            std::cerr << e << " Parse" << std::endl;
            m.unlock();
            continue;
        }
        
        mq->messages.pop();
        m.unlock();
		for (MessageHandler handler : mq->handlers) 
        {
            try
            {
                handler(msg);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << "handler \n";
            }
	        catch(char const *e)
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

void MessageQueue::addMessage(std::string message) 
{
    std::cerr << "Add Message" << std::endl;
    {
        std::lock_guard<std::mutex> m(this->m);
        messages.push(message);
    }
    std::cerr << "Notify" << std::endl;
    cv.notify_one();
}

void MessageQueue::registerHandler(MessageHandler handler) 
{
    handlers.insert(handler);
}

SocketMessage MessageQueue::parse(std::string message)
{
    SocketMessage socketMessage;
    /*
        e.g.    sender_username
                reciever_username
                1689465135
                barev :D
    */
    std::cerr << "Message to parse: " << message << std::endl;
    if (message.find('\n') == std::string::npos)
    {
        throw "Wrong format";
    }
    int ind = message.find('\n');
    socketMessage.sender = message.substr(0, ind); 
    if (message.find('\n', ind + 1) == std::string::npos)
    {
        throw "Wrong format";
    }
    int zZzZz = message.find('\n', ind + 1);
    socketMessage.reciever = message.substr(ind + 1, zZzZz - ind - 1); 
    if (message.find('\n', zZzZz + 1) == std::string::npos)
    {
        throw "Wrong format";
    }
    int ind2 = message.find("\n", zZzZz + 1);
    std::cerr << "Timestamp:" << message.substr(zZzZz + 1, ind2 - zZzZz - 1) << std::endl;
    try
    {
        socketMessage.timestamp = std::stoi(message.substr(zZzZz + 1, ind2 - zZzZz - 1)); // stoi - string to int   
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << " timestamp\n";
        throw "Wrong format";
    }
    
    socketMessage.content = message.substr(ind2 + 1);
    return socketMessage;
}

void MessageQueue::addNotification(SocketMessage socketMessage)
{
    std::lock_guard<std::mutex> m(this->notificationMutex);
    userNotifications[socketMessage.reciever].push(socketMessage);
}

std::queue<SocketMessage> MessageQueue::getNotifications(std::string username)
{
    std::lock_guard<std::mutex> m(this->notificationMutex);
    std::queue<SocketMessage> ret;
    while (!userNotifications[username].empty())
    {
        ret.push(userNotifications[username].front());
        userNotifications[username].pop();
    }
    return ret;
} 

std::string SocketMessage::toString()
{
    /*
        e.g.    sender_username
                reciever_username
                1689465135
                barev :D
    */
    return this->sender + "\n" + this->reciever + "\n" + std::to_string(this->timestamp) + "\n" + this->content;
}