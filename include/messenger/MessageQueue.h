#pragma once

#include <stdio.h>
#include <queue>
#include <set>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <unordered_map>

#include <socket_parser/message_parser.h>

typedef void (*MessageHandler)(SocketParser::Message message);

class MessageQueue 
{
public:
    MessageQueue();
    void addMessage(std::string message, SocketParser::LoginMessageParser::LoginMessage loginMessage);
    void registerHandler(MessageHandler handler); 
    void addNotification(SocketParser::Message socketMessage, std::string reciever);
    std::vector<SocketParser::Message> getNotifications(std::string username); 
    static MessageQueue* messageQueue;
private:
    std::queue<std::pair<std::string, SocketParser::LoginMessageParser::LoginMessage> > messages;
    std::set<MessageHandler> handlers;
    std::mutex m; 
    std::mutex notificationMutex;
    std::thread t;
    std::condition_variable cv;
    friend void callHandlers(MessageQueue* mq);
    // notifications for each person - key:username, value:queue of socket-messages
    std::unordered_map<std::string, std::queue<SocketParser::Message> > userNotifications;
};