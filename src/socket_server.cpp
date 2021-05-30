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

void send_via_socket(std::string msg, int sock_fd, char* buf, int buflen){
    int s = 0;  // number of bytes sent
    int l = msg.size();  // the length of the message
    memset(buf, 0, buflen);
    strcpy(buf, (std::to_string(l) + " ").c_str());  // let's write that at the beginning of the message. If msg = "Hello world!", here we write "12 " in the buffer
    int cl = strlen(buf);  // we've reached the 3-rd char in example
    while (s < l)  // while we haven't sent the whole message
    {
        // write the next batch in the buffer, starting from cl ('cause first cl bytes are already written)
        // we've sent s bytes, so we need to send the message starting from s
        strcpy(buf + cl, msg.substr(s, buflen - cl).c_str());
        send(sock_fd, buf, strlen(buf), 0);
        memset(buf, 0, buflen); // clear the buffer after sending the next batch
        s += buflen - cl;  // we have sent new (buflen - cl) bytes from the message
        cl = 0;  // we haven't written anything in the buffer
    }
    // e.g. if we are sending "Hello world!!" and the buflen is 5
    // First, we are going to send "13 He" (cl = 3 and buflen - cl = 2), "He" from the "Hello world!!"
    // Then, "llo w" (cl = 0, buflen - cl = 5)
    // Then, "orld!"
    // Then, "!"
}

std::string processSocketMessage(std::string message, SocketParser::LoginMessageParser::LoginMessage loginMessage)
{
    // If the command is empty, then some error occured
    if (message.size() == 0)
    {
        return "";
    } 

    // check if the user wants to get chat history
    SocketParser::GetMessageParser getMessageParser;
    
    if (getMessageParser.checkMessage(message))
    {
        SocketParser::GetMessageParser::GetMessage getMessage = getMessageParser.parse(message);
        // Check if the user inputted correct credentials here
        if (loginMessage.username != getMessage.username)
            return "";
        if (loginMessage.guid != getMessage.guid)
            return "";
        // get the chat history
        getMessage.messages = getMessages(getMessage.username, getMessage.usernameFrom);
        // parser to a string
        return getMessageParser.parse(getMessage);
    }

    SocketParser::GetNotificationMessageParser getNotificationsMessageParser;

    if (getNotificationsMessageParser.checkMessage(message))
    {
        // same as above
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
        // Get notifications for the user (also delete those, so won't show second time)
        getNotificationsMessage.messages = MessageQueue::messageQueue->getNotifications(loginMessage.username);
        // std::cerr << "resp: " << getNotificationsMessageParser.parse(getNotificationsMessage) << std::endl;
        return getNotificationsMessageParser.parse(getNotificationsMessage);
    }
    // otherwise, add to the message queue and respond "OK"
    MessageQueue::messageQueue->addMessage(message, loginMessage);
    return "OK";
}

void listener(int client_sock_fd) //amen angam nor kpneluc asum a es clienty kpel a
{
    const int buflen = 2048;
    char buf[buflen];
    memset(buf, 0, buflen);
    // Recieve the login message
    if (recv(client_sock_fd, buf, buflen, 0) == -1)
    {
        ERROR("Cannot receive");
        return;
    }

    LOG(buf);
    SocketParser::LoginMessageParser loginMessageParser;
    std::string message = buf;
    if (!loginMessageParser.checkMessage(message)){
        // If not the login message, then something wrong is sent
        ERROR("Wrong format");
        memset(buf, 0, buflen);
        strcpy(buf, "Wrong format");
        send(client_sock_fd, buf, strlen(buf), 0);
        shutdown(client_sock_fd, SHUT_RDWR);
        close(client_sock_fd);
        return;
    }
    // Get the username and guid from a single string (e.g. "Login: myUsername\myGUID")
    SocketParser::LoginMessageParser::LoginMessage loginMessage = loginMessageParser.parse(message);
    std::cerr << "Logged in " << loginMessage.username << " With guid: " << loginMessage.guid << std::endl;
    if (!exist_or_create_user(loginMessage.username, loginMessage.guid))
    {
        // If the user exists and has other guid, then authentification failed
        ERROR("Wrong guid");
        memset(buf, 0, buflen);
        strcpy(buf, "Wrong guid");
        send(client_sock_fd, buf, strlen(buf), 0);
        shutdown(client_sock_fd, SHUT_RDWR);
        close(client_sock_fd);
        return;
    }

    // Send OK, so the client knows login was successful
    memset(buf, 0, buflen);
    strcpy(buf, "OK");
    send(client_sock_fd, buf, strlen(buf), 0);
    // Change user status to "online"
    changeUserStatus(loginMessage.username, true);
    // Change all groups statuses that the user belongs to, to "online"
    for (std::string groupId: getGroupsOfUser(loginMessage.username))
        changeGroupStatus(groupId, true);
    // get commands from client
    while (1)
    {
        memset(buf, 0, buflen);
        // get the client command
        if (recv(client_sock_fd, buf, buflen, 0) == -1)
        {
            ERROR("Cannot receive");
            break;
        }
        // get the response for the command got above
        message = processSocketMessage(buf, loginMessage);
        // if the response is empty, then something wrong happened
        if (message.size() == 0)
        {
            break; 
        }
        // send the response
        memset(buf, 0, buflen);
        send_via_socket(message, client_sock_fd, buf, buflen);
    }
    std::cerr << "closing: " << client_sock_fd << std::endl;

    // check if the group statuses will be changed back
    for (std::string groupId: getGroupsOfUser(loginMessage.username))
        changeGroupStatus(groupId, isGroupActive(groupId));

    // change user status to offline
    changeUserStatus(loginMessage.username, false);
    // shut down the socket
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
        // accept a new connection from a new client
        if(client_sock_fd == -1)
        {
            ERROR("Cannot accept connection.");
            break;
        }
        LOG("Connection accepted"); 
        LOG(client_sock_fd);
        threads.push_back(std::thread(listener, client_sock_fd));   // start listening to the client
    }
    for (size_t i = 0; i < threads.size(); i++)
    {
        threads[i].join();  // wait until all the threads finished their work
    }
    close(socket_fd);
    return 0;
}