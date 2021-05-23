#pragma once
#include <string>
#include <vector>

// '\' is reserved symbol and will be converted to ' ' in the message

namespace SocketParser{
    enum MessageType
    {
        SEND,
        GET,    // old messages
        GET_NOTIFICATION,   //new messages
        CREATE_GROUP,
        ADD_USER_TO_GROUP,
        LOGIN
    };

    std::string split(char splitChar, std::string& message);

    struct Message
    {
        int ts;
        std::string sender;
        std::string reciever;
        std::string content;
        Message(){}
        Message(std::string sender, std::string reciever, std::string content, int ts)
            : sender(sender), reciever(reciever), content(content), ts(ts)
        {
        }
        std::string parse();
        static Message getMessage(std::string &message);
    };

    class MessageParser
    {
    public:
        bool checkMessage(std::string message);
    protected:
        std::string prefix;
    };

    class GetMessageParser: public MessageParser
    {
    public:
        struct GetMessage
        {
            std::string username;
            std::string guid;
            std::string usernameFrom;
            std::vector<Message> messages;
        };
        GetMessageParser();
        GetMessage parse(std::string message);
        std::string parse(GetMessage getMessage);
    private:
        GetMessageParser(std::string prefix);
        static const std::string _prefix;
        static const MessageType messageType;
    };

    class GetNotificationMessageParser: public MessageParser
    {
    public:
        struct GetNotificationMessage
        {
            std::string username;
            std::string guid;
            std::vector<Message> messages;
        };
        GetNotificationMessageParser();
        GetNotificationMessage parse(std::string message);
        std::string parse(GetNotificationMessage getNotificationMessage);
    private:
        GetNotificationMessageParser(std::string prefix);
        static const std::string _prefix;
        static const MessageType messageType;
    };

    class SendMessageParser: public MessageParser
    {
    public:
        struct SendMessage
        {
            int ts;
            std::string username;
            std::string guid;
            std::string recieverUsername;
            std::string content;
        };
        SendMessageParser();
        SendMessage parse(std::string message);
        std::string parse(SendMessage sendMessage);
    private:
        SendMessageParser(std::string prefix);
        static const std::string _prefix;
        static const MessageType messageType;
    };

    class CreateGroupMessageParser: public MessageParser
    {
    public:
        struct CreateGroupMessage
        {
            std::string username;
            std::string guid;
            std::string groupName;
        };
        CreateGroupMessageParser();
        CreateGroupMessage parse(std::string message);
        std::string parse(CreateGroupMessage createGroupMessage);
    private:
        CreateGroupMessageParser(std::string prefix);
        static const std::string _prefix;
        static const MessageType messageType;
    };

    class AddUserToGroupMessageParser: public MessageParser
    {
    public:
        struct AddUserToGroupMessage
        {
            std::string username;
            std::string guid;
            std::string newUser;
            std::string groupName;
        };
        AddUserToGroupMessageParser();
        AddUserToGroupMessage parse(std::string message);
        std::string parse(AddUserToGroupMessage addUserToGroupMessage);
    private:
        AddUserToGroupMessageParser(std::string prefix);
        static const std::string _prefix;
        static const MessageType messageType;
    };

    class LoginMessageParser: public MessageParser
    {
    public:
        struct LoginMessage
        {
            std::string username;
            std::string guid;
        };
        LoginMessageParser();
        LoginMessage parse(std::string message);
        std::string parse(LoginMessage addUserToGroupMessage);
    private:
        LoginMessageParser(std::string prefix);
        static const std::string _prefix;
        static const MessageType messageType;
    };
} 