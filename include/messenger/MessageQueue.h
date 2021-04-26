#pragma once

#include <stdio.h>
#include <queue>
#include <set>
#include <mutex>
#include <thread>
#include <condition_variable>

struct SocketMessage
{
	std::string sender;
	std::string reciever;
	std::string content;
	int timestamp;
};

typedef void (*MessageHandler)(SocketMessage message);

class MessageQueue 
{
public:
    MessageQueue();
    void AddMessage(std::string message);
    void RegisterHandler(MessageHandler handler);
private:
    std::queue<std::string> messages;
    std::set<MessageHandler> handlers;
    std::mutex m; 
    std::thread t;
    std::condition_variable cv;
    friend void callHandlers(MessageQueue* mq);
	SocketMessage parse(std::string message);
};