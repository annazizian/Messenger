#pragma once

#include <stdio.h>
#include <queue>
#include <set>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <unordered_map>

struct SocketMessage
{
	std::string sender;
	std::string reciever;
	std::string content;
	int timestamp;
    std::string toString();
};

typedef void (*MessageHandler)(SocketMessage message);

class MessageQueue 
{
public:
    MessageQueue();
    void addMessage(std::string message);
    void registerHandler(MessageHandler handler); 
    void addNotification(SocketMessage socketMessage);
    std::queue<SocketMessage> getNotifications(std::string username); 
    static MessageQueue* messageQueue;
private:
    std::queue<std::string> messages;
    std::set<MessageHandler> handlers;
    std::mutex m; 
    std::mutex notificationMutex;
    std::thread t;
    std::condition_variable cv;
    friend void callHandlers(MessageQueue* mq);
	SocketMessage parse(std::string message);
    // notifications for each person - key:username, value:queue of socket-messages
    std::unordered_map<std::string, std::queue<SocketMessage> > userNotifications;
};