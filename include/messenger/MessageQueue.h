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
    MessageQueue(); // also creates a thread for "callHandlers" function
    // Add a command from loginMessage.username to the message queue
    void addMessage(std::string message, SocketParser::LoginMessageParser::LoginMessage loginMessage);
    // register a new handler
    void registerHandler(MessageHandler handler);
    // add a notification to the notification queue
    void addNotification(SocketParser::Message socketMessage, std::string reciever);
    // get notifications for the given user
    std::vector<SocketParser::Message> getNotifications(std::string username); 
    // The one and only message queue you would want to user
    static MessageQueue* messageQueue;
private:
    // The actual queue
    std::queue<std::pair<std::string, SocketParser::LoginMessageParser::LoginMessage> > messages;
    // Message handlers
    std::set<MessageHandler> handlers;
    // Message queue mutex
    std::mutex m;
    // Notification queue mutex
    std::mutex notificationMutex;
    // The thread, that calls handlers
    std::thread t;
    // Notifier for the thread, that new message was added to the message queue
    // WARNING: not to be confused with messenger's notifications
    std::condition_variable cv;
    // The function which calls handlers of message queue
    friend void callHandlers(MessageQueue* mq);
    // notifications for each person - key:username, value:queue of socket-messages
    std::unordered_map<std::string, std::queue<SocketParser::Message> > userNotifications;
};