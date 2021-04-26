#include <messenger/MessageQueue.h>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>

void callHandlers(MessageQueue* mq) 
{
    std::unique_lock<std::mutex> m(mq->m);
    while (true)
    {
        mq->cv.wait(m);
        std::string plain_msg = mq->messages.front();
        SocketMessage msg;
        try
        {
            msg = mq->parse(plain_msg);
        }
        catch(const std::exception& e)
        {
            m.unlock();
            continue;
        }
        
        mq->messages.pop();
        m.unlock();
		for (MessageHandler handler : mq->handlers) 
        {
			handler(msg);
		}
    }
}

MessageQueue::MessageQueue() : t(callHandlers, this) 
{
}

void MessageQueue::AddMessage(std::string message) 
{
    std::unique_lock<std::mutex> m(this->m);
    m.lock();
    messages.push(message);
    cv.notify_one();
    m.unlock();
}

void MessageQueue::RegisterHandler(MessageHandler handler) 
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
    int zZzZz = message.find('\n');
    socketMessage.reciever = message.substr(ind + 1, zZzZz - ind - 1); 
    if (message.find('\n', zZzZz + 1) == std::string::npos)
    {
        throw "Wrong format";
    }
    int ind2 = message.find("\n", zZzZz + 1);
    try
    {
        socketMessage.timestamp = std::stoi(message.substr(zZzZz + 1, ind2 - zZzZz - 1)); // stoi - string to int   
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
        throw "Wrong format";
    }
    
    socketMessage.content = message.substr(ind2 + 1);
    return socketMessage;
}