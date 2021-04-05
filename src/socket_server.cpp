#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <vector>
#include "connection.h"
#include "user.h"
#include "message.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ERROR(msg) std::cerr << "Server Error: " << msg << ". "<< strerror(errno) << std::endl;
#define LOG(msg) std::cout << "Server Log: " << msg << std::endl;

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

    std::string username = "", guid = "", message = buf;

    if (message.find(' ') == std::string::npos) //aysinqn username chenq stacel
    {
        ERROR("Wrong format");
        memset(buf, 0, buflen);
        strcpy(buf, "Wrong format");
        send(client_sock_fd, buf, strlen(buf), 0);
        shutdown(client_sock_fd, SHUT_RDWR);
        close(client_sock_fd);
        return;
    }
    //stacel enq username
    int ind = message.find(' ');
    username = message.substr(0, ind);
    guid = message.substr(ind + 1);
    if (!exist_or_create_user(username, guid))
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

    while (1)
    {
        memset(buf, 0, buflen);
        if (recv(client_sock_fd, buf, buflen, 0) == -1)
        {
            ERROR("Cannot receive");
            return;
        }
        message = buf;
        if (message == "get")
        {
            std::string response = "";
            for(auto pair: getMessages(username))
            {
                response += pair.first;
                response += ": ";
                response += pair.second;
                response += "\n";
            }
            memset(buf, 0, buflen);
            strcpy(buf, response.c_str());
            send(client_sock_fd, buf, buflen, 0);
            continue;
        }
        /*
            e.g.    reciever_username
                    1689465135
                    barev :D
        */
        if (message.find('\n') == std::string::npos)
        {
            ERROR("Wrong format");
            memset(buf, 0, buflen);
            strcpy(buf, "Wrong format");
            send(client_sock_fd, buf, strlen(buf), 0);
            shutdown(client_sock_fd, SHUT_RDWR);
            close(client_sock_fd);
            return;
        }
        ind = message.find('\n');
        std::string reciever = message.substr(0, ind); 
        if (message.find('\n', ind + 1) == std::string::npos)
        {
            ERROR("Wrong format");
            memset(buf, 0, buflen);
            strcpy(buf, "Wrong format");
            send(client_sock_fd, buf, strlen(buf), 0);
            shutdown(client_sock_fd, SHUT_RDWR);
            close(client_sock_fd);
            return;
        }
        int ind2 = message.find("\n", ind + 1);
        // TODO: Exception handling
        int ts = std::stoi(message.substr(ind + 1, ind2 - ind - 1)); // stoi - string to int
        std::string content = message.substr(ind2 + 1);
        if (!user_exists(reciever))
        {
            ERROR("Wrong reciever");
            memset(buf, 0, buflen);
            strcpy(buf, "Wrong reciever");
            send(client_sock_fd, buf, strlen(buf), 0);
            shutdown(client_sock_fd, SHUT_RDWR);
            close(client_sock_fd);
            return;
        }
        sendMessage(content, reciever, username, ts);
        memset(buf, 0, buflen);
        strcpy(buf, "OK");
        send(client_sock_fd, buf, strlen(buf), 0);
    }
    
    shutdown(client_sock_fd, SHUT_RDWR);
    close(client_sock_fd);
}

int main()
{ 
    Connection("db.sqlite", true);
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