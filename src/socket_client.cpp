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

using namespace std::chrono_literals;

#define ERROR(msg) std::cerr << "Client Error: " << msg << ". "<< strerror(errno) << std::endl;
#define LOG(msg) std::cout << "Client Log: " << msg << std::endl;

std::queue<std::string> q;
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
        std::string command = "", send_msg;
        std::cin >> command;
        if (command == "send")
        {
            send_msg = "";
            std::string reciever;
            std::cout << "Reciever: ";
            std::cin >> reciever; 
            send_msg += reciever;
            send_msg += "\n";
            int ts = std::time(nullptr);
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
        {
            std::lock_guard<std::mutex> lk(m);
            q.push(send_msg);
        }
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
    std::string username, guid;
    std::cout << "Username: ";
    std::cin >> username >> guid; 
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

    std::thread t(listener);
    while (1)
    {
        std::string s = "";
        {
            std::lock_guard<std::mutex> lk(m);
            if (!q.empty())
            {
                s = q.front();
                q.pop();
            }
        }
        if (s == "")
        {
            std::unique_lock<std::mutex> ulk(m);
            cv.wait_for(ulk, 2s); // wait 2 seconds
            if (!q.empty())
            {
                s = q.front();
                q.pop();
            }
            else
            {
                s = "get_notifications";
            }
            ulk.unlock();
        }
        memset(buf, 0, buflen);
        strcpy(buf, s.c_str());
        send(socket_fd, buf, s.size(), 0);
        memset(buf, 0, buflen);
        ssize_t bytes = recv(socket_fd, buf, buflen, 0); 
        if (s != "get" && s != "get_notifications")
        {
            LOG(buf);
            if (std::string(buf) != "OK")
            {
                ERROR(buf);
                break;
            }
        }
        LOG(buf);
    }
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    return 0;
}