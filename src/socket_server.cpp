#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <vector>
#include <messenger/connection.h>
#include <messenger/user.h>
#include <messenger/message.h>
#include <messenger/groups.h>
#include <messenger/MessageQueue.h>
#include <messenger/handlers.h>

#include <socket_parser/message_parser.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ERROR(msg) std::cerr << "Server Error: " << msg << ". "<< strerror(errno) << std::endl;
#define LOG(msg) std::cout << "Server Log: " << msg << std::endl;

std::string processSocketMessage(std::string message, SocketParser::LoginMessageParser::LoginMessage loginMessage)
{
    // std::cout << "ban: " + message << std::endl;
    if (message.size() == 0)
    {
        return "";
    } 

    SocketParser::GetMessageParser getMessageParser;
    
    if (getMessageParser.checkMessage(message))
    {
        SocketParser::GetMessageParser::GetMessage getMessage = getMessageParser.parse(message);
        if (loginMessage.username != getMessage.username)
            return "";
        if (loginMessage.guid != getMessage.guid)
            return "";
        getMessage.messages = getMessages(getMessage.username, getMessage.usernameFrom);
        return getMessageParser.parse(getMessage);
    }

    SocketParser::GetNotificationMessageParser getNotificationsMessageParser;

    if (getNotificationsMessageParser.checkMessage(message))
    {
        SocketParser::GetNotificationMessageParser::GetNotificationMessage getNotificationsMessage
            = getNotificationsMessageParser.parse(message);
        // std::cerr << "Got notification request: " << getNotificationsMessage.username << " with guid "
        //           << getNotificationsMessage.guid << std::endl;
        if (loginMessage.username != getNotificationsMessage.username)
        {
            std::cerr << "Wrong username" << std::endl;
            return "";
        }
        if (loginMessage.guid != getNotificationsMessage.guid)
        {
            std::cerr << "Wrong guid" << std::endl;
            return "";
        }
        getNotificationsMessage.messages = MessageQueue::messageQueue->getNotifications(loginMessage.username);
        // std::cerr << "resp: " << getNotificationsMessageParser.parse(getNotificationsMessage) << std::endl;
        return getNotificationsMessageParser.parse(getNotificationsMessage);
    }

    MessageQueue::messageQueue->addMessage(message, loginMessage);
    return "OK";
}

void listener(int client_sock_fd) //amen angam nor kpneluc asum a es clienty kpel a
{
    const int buflen = 2048;
    char buf[buflen];
    memset(buf, 0, buflen);
    if (recv(client_sock_fd, buf, buflen, 0) == -1)
    {
        ERROR("Cannot receive");
        return;
    }

    LOG(buf);
    SocketParser::LoginMessageParser loginMessageParser;
    std::string message = buf;
    if (!loginMessageParser.checkMessage(message)){
        ERROR("Wrong format");
        memset(buf, 0, buflen);
        strcpy(buf, "Wrong format");
        send(client_sock_fd, buf, strlen(buf), 0);
        shutdown(client_sock_fd, SHUT_RDWR);
        close(client_sock_fd);
        return;
    }
    SocketParser::LoginMessageParser::LoginMessage loginMessage = loginMessageParser.parse(message);
    std::cerr << "Logged in " << loginMessage.username << " With guid: " << loginMessage.guid << std::endl;
    if (!exist_or_create_user(loginMessage.username, loginMessage.guid))
    {
        ERROR("Wrong guid");
        memset(buf, 0, buflen);
        strcpy(buf, "Wrong guid");
        send(client_sock_fd, buf, strlen(buf), 0);
        shutdown(client_sock_fd, SHUT_RDWR);
        close(client_sock_fd);
        return;
    }
    memset(buf, 0, buflen);
    strcpy(buf, "OK");
    send(client_sock_fd, buf, strlen(buf), 0);
    changeUserStatus(loginMessage.username, true);
    for (std::string groupId: getGroupsOfUser(loginMessage.username))
        changeGroupStatus(groupId, true);
    while (1)
    {
        memset(buf, 0, buflen);
        if (recv(client_sock_fd, buf, buflen, 0) == -1)
        {
            ERROR("Cannot receive");
            break;
        }
        message = processSocketMessage(buf, loginMessage);
        if (message.size() == 0)
        {
            break; 
        }
        memset(buf, 0, buflen);
        strcpy(buf, message.c_str());
        send(client_sock_fd, buf, strlen(buf), 0);
    }
    std::cerr << "closing: " << client_sock_fd << std::endl;

    for (std::string groupId: getGroupsOfUser(loginMessage.username))
        changeGroupStatus(groupId, isGroupActive(groupId));

    changeUserStatus(loginMessage.username, false);
    shutdown(client_sock_fd, SHUT_RDWR);
    close(client_sock_fd);
}

int main()
{ 
    srand(time(NULL));
    Connection("db.sqlite", true);
    MessageQueue::messageQueue->registerHandler(&storeMessage);
    MessageQueue::messageQueue->registerHandler(&sendNotification);
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1)
    {
        ERROR("Cannot create socket");
        return -1;
    }
    LOG("Socket created");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    //host to network short -> htons ֆունկցիայա, որին տալիս ենք 2բայթանի թիվ, օպերացիոն համակարգը նայում ա պրոցեսորը համապա-
    //տասխանում ա ցանցային մոդուլին, կամ բայթերը տեղերը փոխում ա կամ չի փոխում
    addr.sin_port = htons(1500);

    if (bind(socket_fd, (sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
    {
        ERROR("Cannot bind");
        return -1;
    }
    LOG("Bound successfully");

    //listen-y en functionn a vor berum a sockety ashkhatanayin vijaki, "miacnuma"
    if (listen(socket_fd, SOMAXCONN) == -1)
    {
        ERROR("Cannot listen.");
        return -1;
    }

    std::vector<std::thread> threads;
    while(1)
    {
        int client_sock_fd = accept(socket_fd, 0, 0);
        if(client_sock_fd == -1)
        {
            ERROR("Cannot accept connection.");
            break;
        }
        LOG("Connection accepted"); 
        LOG(client_sock_fd);
        threads.push_back(std::thread(listener, client_sock_fd));
    }
    for (size_t i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }
    close(socket_fd);
    return 0;
}