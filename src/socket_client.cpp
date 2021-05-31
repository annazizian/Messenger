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

std::string recieve_from_socket(int sock_fd, char* buf, int buflen)
{
    // recieves the whole message from the socket, even if it's very long
    // Will recieve message of the following format
    // "12 Hello world!"
    // where 12 is the length of "Hello world!"
    // Note, that there is a space after 12
    int s = 0;
    int l;  // length to the message (12 in the example)
    bool end = false; // if we finished reading
    int llen;  // number of digits of l (2 for the example)
    std::string res;
    while (!end) {
        memset(buf, 0, buflen);
        int x;  // the number of chars received
        if ((x = recv(sock_fd, buf, buflen, 0)) == -1)
        {
            break;
        }
        if (s == 0) {  // if reading the first batch
            try {
                l = std::stoi(buf);  // try to read the length of the message
            }
            catch (std::runtime_error& e) {
                return "";  // if a wrong message was sent
            }
            llen = std::to_string(l).size();  // the number of digits of l
            if (buf[llen] != ' ')  // there needs to be a space after l
                return "";
            res = std::string(buf).substr(llen + 1);  // let's cut the first part
            s = x - llen - 1;  // We've already read x bytes including l and an extra space
        }
        else {
            s += x;  // New x bytes read, number of bytes read is increased by x
            res += std::string(buf);  // Adding the next batch to the result
        }
        if (s >= l)  // If we have read the whole l bytes
            end = true;  // then finish the cycle
    }
    return res;  // in the example "Hello world!" was sent, so it will be returned
}

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
        // Get the command: one of send, get, get_notification, add, create
        // for each command get corresponding arguments and push to the vector v
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
            // Push the command with its arguments to the queue (while locked)
        }
        std::cerr << "notify" << std::endl;
        cv.notify_all();
        // notify main thread
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

    // Here we initialise parsers
    SocketParser::AddUserToGroupMessageParser addUserToGroupMessageParser;
    SocketParser::CreateGroupMessageParser createGroupMessageParser;
    SocketParser::GetMessageParser getMessageParser;
    SocketParser::GetNotificationMessageParser getNotificationMessageParser;
    SocketParser::LoginMessageParser loginMessageParser;
    SocketParser::SendMessageParser sendMessageParser;

    // Here we input the username and the password
    std::cout << "Username and password: ";
    SocketParser::LoginMessageParser::LoginMessage loginMessage;
    std::cin >> loginMessage.username >> loginMessage.guid;
    
    // Here we send the login message via socket
    memset(buf, 0, buflen);
    strcpy(buf, loginMessageParser.parse(loginMessage).c_str());
    send(socket_fd, buf, strlen(buf), 0);
    memset(buf, 0, buflen);

    std::string response = receieve_from_socket(socket_fd, buf, buflen);
    LOG("From server:");
    LOG(buf);
    if (response != "OK")   // check, if the usename and password were correct
    {
        ERROR(response);
        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);
        return 0;
    }
    
    // Here we initialise possible messages we will send/recieve via socket 
    SocketParser::GetNotificationMessageParser::GetNotificationMessage getNotificationMessage;
    SocketParser::GetMessageParser::GetMessage getMessage;
    SocketParser::SendMessageParser::SendMessage sendMessage;
    SocketParser::CreateGroupMessageParser::CreateGroupMessage createGroupMessage;
    SocketParser::AddUserToGroupMessageParser::AddUserToGroupMessage addUserToGroupMessage;

    // Here we set username and guid to not do this on every messages send
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

    std::thread t(listener);    // start the user input listener
    while (1)
    {
        std::pair<SocketParser::MessageType, std::vector<std::string> > s;
        {
            std::lock_guard<std::mutex> lk(m);
            if (!qi.empty())    // If the user has inputed a message, get that
            {
                s = qi.front();
                qi.pop();
            }
        }
        if (s.second.empty()) 
        {
            std::unique_lock<std::mutex> ulk(m);
            cv.wait_for(ulk, 2s); // wait 2 seconds
            if (!qi.empty()) // If user has inputted a command
            {
                s = qi.front(); // get the command
                qi.pop();
            }
            if (s.second.empty()) // if not command inputted, then get notifications
            {
                s.first = SocketParser::MessageType::GET_NOTIFICATION;
                s.second.push_back(getNotificationMessageParser.parse(getNotificationMessage));
            }
            ulk.unlock();
        }

        std::string send_message;
        // Here we check each command, and for each one parse the command into a string
        // to be sent via socket using parsers
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
            throw "Invalid Message type";   // If an invalid command is pushed (not inputted), the app will crash
        }
        memset(buf, 0, buflen);
        strcpy(buf, send_message.c_str());
        send(socket_fd, buf, send_message.size(), 0);
        memset(buf, 0, buflen);
        // Send the command and recieve the response
        ssize_t bytes = recv(socket_fd, buf, buflen, 0); 
        switch (s.first)
        {
        case SocketParser::MessageType::SEND:
        case SocketParser::MessageType::CREATE_GROUP:
        case SocketParser::MessageType::ADD_USER_TO_GROUP:
        // In this three cases any response other than "OK" means ERROR
            if (std::string(buf) != "OK")
            {
                ERROR(buf);
                break;
            }
            LOG(buf);
            break;
        // In other two cases, parse the message to get the message list
        case SocketParser::MessageType::GET:
            getMessage = getMessageParser.parse(std::string(buf));
            for (SocketParser::Message message: getMessage.messages)
            {
                LOG(message.parse());
                // Log each message in the terminal
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