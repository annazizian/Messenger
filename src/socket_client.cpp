#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include "helpers.h"

#define ERROR(msg) std::cerr << "Client Error: " << msg << ". "<< strerror(errno) << std::endl;
#define LOG(msg) std::cout << "Client Log: " << msg << std::endl;

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
    std::string username, guid;
    std::cout << "Username: ";
    std::cin >> username >> guid; 
    if (guid == "!")
    {
        guid = generate_guid();
        std::cout << guid << std::endl;
    }
    memset(buf, 0, buflen);
    std::string send_msg = "";
    send_msg += username;
    send_msg += " ";
    send_msg += guid;
    strcpy(buf, send_msg.c_str());
    send(socket_fd, buf, send_msg.size(), 0);
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

    while (1)
    {
        std::string command = "";
        std::cin >> command;
        if (command == "send")
        {
            send_msg = "";
            std::string reciever;
            std::cout << "Reciever: ";
            std::cin >> reciever; 
            send_msg += reciever;
            send_msg += "\n";
            int ts = get_ts();
            send_msg += std::to_string(ts);
            send_msg += "\n";
            std::cout << "Message: ";
            std::string content = "";
            std::getline(std::cin, content);
            std::getline(std::cin, content);
            send_msg += content;
        } else {
            send_msg = "get";
        }
        /*
            e.g.    reciever_username
                    1689465135
                    barev :D
        */
        memset(buf, 0, buflen);
        strcpy(buf, send_msg.c_str());
        send(socket_fd, buf, send_msg.size(), 0);
        memset(buf, 0, buflen);
        ssize_t bytes = recv(socket_fd, buf, buflen, 0); 
        if (send_msg == "get")
        {
            LOG(buf);
        } else 
        {
            if (std::string (buf) != "OK")
            {
                ERROR(buf);
                break;
            }
        }
    }

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    return 0;
}