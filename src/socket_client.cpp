#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <ctime>
#include <queue>
#include <condition_variable>

#include <socket_parser/message_parser.h>

using namespace std::chrono_literals;

#define ERROR(msg) std::cerr << "Client Error: " << msg << ". "<< strerror(errno) << std::endl;
#define LOG(msg) std::cout << "Client Log: " << msg << std::endl;

std::queue<std::pair<SocketParser::MessageType, std::vector<std::string> > > qi;
std::mutex m;
std::condition_variable cv;

void listener(){
    while(1)
    {
        /*
            e.g.    reciever_username
                    1689465135
                    barev :D
        */
        std::string command = "", line, send_msg;
        std::cin >> command;
        SocketParser::MessageType messageType;
        std::vector<std::string> v;
        std::string inpt;
        if (command == "send") {
            messageType = SocketParser::MessageType::SEND;
            std::cout << "Reciever: ";
            std::cin >> inpt;
            v.push_back(inpt);
            std::cout << "Message: ";
            std::getline(std::cin, inpt);
            if (inpt.empty())
                std::getline(std::cin, inpt);
            v.push_back(inpt);
            int ts = std::time(nullptr);
            v.push_back(std::to_string(ts));
        } else if (command == "get") {
            std::cout << "Username: ";
            std::cin >> inpt;
            v.push_back(inpt);
            messageType = SocketParser::MessageType::GET;
        } else if (command == "get_notification"){
            messageType = SocketParser::MessageType::GET_NOTIFICATION;
        } else if (command == "add") {
            messageType = SocketParser::MessageType::ADD_USER_TO_GROUP;
            std::cout << "Group Name: ";
            std::cin >> inpt;
            v.push_back(inpt);
            std::cout << "Username: ";
            std::cin >> inpt;
            v.push_back(inpt);
        } else if (command == "create") {
            messageType = SocketParser::MessageType::CREATE_GROUP;
            std::cout << "Group Name: ";
            std::cin >> inpt;
            v.push_back(inpt);
        } else {
            std::cerr << "Invalid " << command << std::endl;
            continue;
        } 
        {
            std::lock_guard<std::mutex> lk(m);
            qi.push(std::make_pair(messageType, v));
        }
        std::cerr << "notify" << std::endl;
        cv.notify_all();
    }
}

int main()
{
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
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //hascen petq a chisht grel arden
    addr.sin_port = htons(1500);    //es 1500y sin_porti hamarn a, uzum em servery ed porti vra lini

    if (connect(socket_fd, (sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
    {
        ERROR("Cannot connect");
        return -1;
    }
    LOG("Connected successfully");

    //arajin namaki tesqy` username guid(password)
    //if username chka, create ed guid-ov user

    const int buflen = 2048;
    char buf[buflen];

    SocketParser::AddUserToGroupMessageParser addUserToGroupMessageParser;
    SocketParser::CreateGroupMessageParser createGroupMessageParser;
    SocketParser::GetMessageParser getMessageParser;
    SocketParser::GetNotificationMessageParser getNotificationMessageParser;
    SocketParser::LoginMessageParser loginMessageParser;
    SocketParser::SendMessageParser sendMessageParser;

    std::cout << "Username and password: ";
    SocketParser::LoginMessageParser::LoginMessage loginMessage;
    std::cin >> loginMessage.username >> loginMessage.guid;
    
    memset(buf, 0, buflen);
    strcpy(buf, loginMessageParser.parse(loginMessage).c_str());
    send(socket_fd, buf, strlen(buf), 0);
    memset(buf, 0, buflen);

    ssize_t bytes = recv(socket_fd, buf, buflen, 0); 
    std::string response = buf;
    LOG("From server:");
    LOG(buf);
    if (response != "OK")
    {
        ERROR(response);
        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);
        return 0;
    }
    
    SocketParser::GetNotificationMessageParser::GetNotificationMessage getNotificationMessage;
    SocketParser::GetMessageParser::GetMessage getMessage;
    SocketParser::SendMessageParser::SendMessage sendMessage;
    SocketParser::CreateGroupMessageParser::CreateGroupMessage createGroupMessage;
    SocketParser::AddUserToGroupMessageParser::AddUserToGroupMessage addUserToGroupMessage;

    getNotificationMessage.username = loginMessage.username;
    getNotificationMessage.guid = loginMessage.guid;
    getMessage.username = loginMessage.username;
    getMessage.guid = loginMessage.guid;
    sendMessage.username = loginMessage.username;
    sendMessage.guid = loginMessage.guid;
    createGroupMessage.username = loginMessage.username;
    createGroupMessage.guid = loginMessage.guid;
    addUserToGroupMessage.username = loginMessage.username;
    addUserToGroupMessage.guid = loginMessage.guid;

    std::thread t(listener);
    while (1)
    {
        std::pair<SocketParser::MessageType, std::vector<std::string> > s;
        {
            std::lock_guard<std::mutex> lk(m);
            if (!qi.empty())
            {
                s = qi.front();
                qi.pop();
            }
        }
        if (s.second.empty()) 
        {
            std::unique_lock<std::mutex> ulk(m);
            cv.wait_for(ulk, 2s); // wait 2 seconds
            if (!qi.empty())
            {
                s = qi.front();
                qi.pop();
            }
            if (s.second.empty())
            {
                s.first = SocketParser::MessageType::GET_NOTIFICATION;
                s.second.push_back(getNotificationMessageParser.parse(getNotificationMessage));
            }
            ulk.unlock();
        }

        std::string send_message;
        switch (s.first)
        {
        case SocketParser::MessageType::GET:
            getMessage.usernameFrom = s.second[0];
            send_message = getMessageParser.parse(getMessage);
            break;
        case SocketParser::MessageType::GET_NOTIFICATION:
            send_message = getNotificationMessageParser.parse(getNotificationMessage);
            break;
        case SocketParser::MessageType::SEND:
            sendMessage.recieverUsername = s.second[0];
            sendMessage.content = s.second[1];
            sendMessage.ts = std::stoi(s.second[2]);
            send_message = sendMessageParser.parse(sendMessage);
            break;
        case SocketParser::MessageType::CREATE_GROUP:
            createGroupMessage.groupName = s.second[0];
            send_message = createGroupMessageParser.parse(createGroupMessage);
            break;
        case SocketParser::MessageType::ADD_USER_TO_GROUP:
            addUserToGroupMessage.groupName = s.second[0];
            addUserToGroupMessage.newUser = s.second[1];
            send_message = addUserToGroupMessageParser.parse(addUserToGroupMessage);
            break;
        
        default:
            throw "Invalid Message type";
        }
        memset(buf, 0, buflen);
        strcpy(buf, send_message.c_str());
        send(socket_fd, buf, send_message.size(), 0);
        memset(buf, 0, buflen);
        ssize_t bytes = recv(socket_fd, buf, buflen, 0); 
        switch (s.first)
        {
        case SocketParser::MessageType::SEND:
        case SocketParser::MessageType::CREATE_GROUP:
        case SocketParser::MessageType::ADD_USER_TO_GROUP:
            if (std::string(buf) != "OK")
            {
                ERROR(buf);
                break;
            }
            LOG(buf);
            break;
        case SocketParser::MessageType::GET:
            getMessage = getMessageParser.parse(std::string(buf));
            for (SocketParser::Message message: getMessage.messages)
            {
                LOG(message.parse());
            }
            break;
        case SocketParser::MessageType::GET_NOTIFICATION:
            getNotificationMessage = getNotificationMessageParser.parse(std::string(buf));
            for (SocketParser::Message message: getNotificationMessage.messages)
            {
                LOG(message.parse());
            }
            break;
        default:
            throw "Invalid Message type";
        }        
    }
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    return 0;
}