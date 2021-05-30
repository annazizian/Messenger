#include <socket_parser/message_parser.h>
#include <algorithm>

std::string SocketParser::split(char splitChar, std::string& message)
{
    // split the message into two parts using the first occurence of the splitChar
    // (which is basically a backslash: \)
    size_t res = message.find(splitChar);   // find the index 
    std::string firstPart = "";
    if (res == std::string::npos)   // if not found
    {
        swap(firstPart, message); // then the result is (whole message, "")
        // e.g. "ASDF" will be splitted into ("ASDF", "")
    }
    else
    {
        int ind = res; // if found
        firstPart = message.substr(0, ind);
        message = message.substr(ind + 1);
        // the result is (message before splitChar, message after first splitChar)
        // e.g. "A\B\C" will be splitted into ("A", "B\C")
    }
    return firstPart;
    // The first result will be returned
    // and the remaining part will be stored in the message variable
}


std::string SocketParser::Message::parse()
{
    // This method will parse a single message into a string
    return this->sender + '\\' + this->reciever + '\\' + std::to_string(this->ts) + '\\' + this->content;
}

SocketParser::Message SocketParser::Message::getMessage(std::string& message)
{
    // this method will get the first Message from the message list
    Message msg;
    msg.sender = SocketParser::split('\\', message);
    msg.reciever = SocketParser::split('\\', message);
    msg.ts = std::stoi(SocketParser::split('\\', message));
    msg.content = SocketParser::split('\\', message);
    return msg;
}

bool SocketParser::MessageParser::checkMessage(std::string message)
{
    // This method will be called for a dervied class to check if the message is for that command
    return message.substr(0, this->prefix.size()) == this->prefix;
}

// Here the message prefixes and types are given for each command
const std::string SocketParser::GetMessageParser::_prefix = "Get Message: ";
const SocketParser::MessageType SocketParser::GetMessageParser::messageType = SocketParser::GET;

SocketParser::GetMessageParser::GetMessageParser() : SocketParser::GetMessageParser(SocketParser::GetMessageParser::_prefix){}
SocketParser::GetMessageParser::GetMessageParser(std::string prefix)
{
    this->prefix = prefix;
}

std::string SocketParser::GetMessageParser::parse(SocketParser::GetMessageParser::GetMessage getMessage)
{
    // Parse the GetMessage struct into a string to send via socket
    // e.g. Messages from "Poxos" "Hello" at 1610000000
    //      and "How are you" at 1610000001 will be parsed into the following string
    //      and your response "Hi, fine, and you?" at 1610000002
    // "Get Message: yourUsername\yourGuid\Poxos\Poxos\yourUsername\1610000000\Hello\Poxos\yourUsername\1610000001\How are you\yourUsername\Poxos\1610000002\Hi, fine, and you?"
    // The string above will be easily(not much) parsed back via parser below
    std::string ret = this->prefix + getMessage.username + "\\" + getMessage.guid + '\\' + getMessage.usernameFrom;
    for (SocketParser::Message msg: getMessage.messages)
    {
        ret += '\\' + msg.parse();
    }
    return ret;
}

SocketParser::GetMessageParser::GetMessage SocketParser::GetMessageParser::parse(std::string message)
{
    // Parse the message from the socket into a GetMessage struct
    // You'll be able to see the messages using simple cpp structs 
    SocketParser::GetMessageParser::GetMessage getMessage;
    message = message.substr(this->prefix.size());
    getMessage.username = SocketParser::split('\\', message);
    getMessage.guid = SocketParser::split('\\', message);
    getMessage.usernameFrom = SocketParser::split('\\', message);
    while(message.size())
    {
        getMessage.messages.push_back(SocketParser::Message::getMessage(message));
    }    
    return getMessage;
}

const std::string SocketParser::GetNotificationMessageParser::_prefix = "Get Notifications: ";
const SocketParser::MessageType SocketParser::GetNotificationMessageParser::messageType = SocketParser::GET_NOTIFICATION;

SocketParser::GetNotificationMessageParser::GetNotificationMessageParser():SocketParser::GetNotificationMessageParser(SocketParser::GetNotificationMessageParser::_prefix)
{

}

SocketParser::GetNotificationMessageParser::GetNotificationMessageParser(std::string prefix)
{
    this->prefix = prefix;
}

// parsers below are very similar to the two parsers above

std::string SocketParser::GetNotificationMessageParser::parse(SocketParser::GetNotificationMessageParser::GetNotificationMessage getNotificationMessage)
{
    std::string ret = this->prefix + getNotificationMessage.username + '\\' + getNotificationMessage.guid;
    for (SocketParser::Message msg : getNotificationMessage.messages)
    {
        ret += '\\' + msg.parse();
    }
    return ret;
}

SocketParser::GetNotificationMessageParser::GetNotificationMessage SocketParser::GetNotificationMessageParser::parse(std::string message)
{
    SocketParser::GetNotificationMessageParser::GetNotificationMessage getNotificationMessage;
    message = message.substr(this->prefix.size());
    getNotificationMessage.username = SocketParser::split('\\', message);
    getNotificationMessage.guid = SocketParser::split('\\', message);
    while(message.size())
    {
        getNotificationMessage.messages.push_back(SocketParser::Message::getMessage(message));
    }
    return getNotificationMessage;
}

const std::string SocketParser::SendMessageParser::_prefix = "Send: ";
const SocketParser::MessageType SocketParser::SendMessageParser::messageType = SocketParser::SEND;

SocketParser::SendMessageParser::SendMessageParser() : SocketParser::SendMessageParser(SocketParser::SendMessageParser::_prefix)
{

}

SocketParser::SendMessageParser::SendMessageParser(std::string prefix)
{
    this->prefix = prefix;
}

std::string SocketParser::SendMessageParser::parse(SocketParser::SendMessageParser::SendMessage sendMessage)
{
    return this->prefix + sendMessage.username + '\\' + sendMessage.guid + '\\' + sendMessage.recieverUsername + '\\' + std::to_string(sendMessage.ts) + '\\' + sendMessage.content;
}

SocketParser::SendMessageParser::SendMessage SocketParser::SendMessageParser::parse(std::string message)
{
    message = message.substr(this->prefix.size());
    SocketParser::SendMessageParser::SendMessage sendMessage;
    sendMessage.username = SocketParser::split('\\', message);
    sendMessage.guid = SocketParser::split('\\', message);
    sendMessage.recieverUsername = SocketParser::split('\\', message);
    sendMessage.ts = std::stoi(SocketParser::split('\\', message));
    sendMessage.content = SocketParser::split('\\', message);
    std::replace(sendMessage.content.begin(), sendMessage.content.end(), '\\', ' ');

    return sendMessage;
}

const std::string SocketParser::CreateGroupMessageParser::_prefix = "Create group: ";
const SocketParser::MessageType SocketParser::CreateGroupMessageParser::messageType = SocketParser::CREATE_GROUP;

SocketParser::CreateGroupMessageParser::CreateGroupMessageParser(): SocketParser::CreateGroupMessageParser(SocketParser::CreateGroupMessageParser::_prefix){}

SocketParser::CreateGroupMessageParser::CreateGroupMessageParser(std::string prefix)
{
    this->prefix = prefix;
}

std::string SocketParser::CreateGroupMessageParser::parse(SocketParser::CreateGroupMessageParser::CreateGroupMessage createGroupMessage)
{
    return this->prefix + createGroupMessage.username + '\\' + createGroupMessage.guid + '\\' + createGroupMessage.groupName;
}

SocketParser::CreateGroupMessageParser::CreateGroupMessage SocketParser::CreateGroupMessageParser::parse(std::string message)
{  
    message = message.substr(this->prefix.size());
    SocketParser::CreateGroupMessageParser::CreateGroupMessage createGroupMessage;
    createGroupMessage.username = SocketParser::split('\\', message);
    createGroupMessage.guid = SocketParser::split('\\', message);
    createGroupMessage.groupName = SocketParser::split('\\', message);
    return createGroupMessage;
}

const std::string SocketParser::AddUserToGroupMessageParser::_prefix = "Add user to the group: ";
const SocketParser::MessageType SocketParser::AddUserToGroupMessageParser::messageType = SocketParser::ADD_USER_TO_GROUP;

SocketParser::AddUserToGroupMessageParser::AddUserToGroupMessageParser(): SocketParser::AddUserToGroupMessageParser(SocketParser::AddUserToGroupMessageParser::_prefix){}

SocketParser::AddUserToGroupMessageParser::AddUserToGroupMessageParser(std::string prefix)
{
    this->prefix = prefix;
}

std::string SocketParser::AddUserToGroupMessageParser::parse(SocketParser::AddUserToGroupMessageParser::AddUserToGroupMessage addUserToGroupMessage)
{
    return this->prefix + addUserToGroupMessage.username + '\\' + addUserToGroupMessage.guid + '\\' + addUserToGroupMessage.groupName + '\\' + addUserToGroupMessage.newUser;
}

SocketParser::AddUserToGroupMessageParser::AddUserToGroupMessage SocketParser::AddUserToGroupMessageParser::parse(std::string message)
{
    message = message.substr(this->prefix.size());
    SocketParser::AddUserToGroupMessageParser::AddUserToGroupMessage addUserToGroupMessage;
    addUserToGroupMessage.username = SocketParser::split('\\', message);
    addUserToGroupMessage.guid = SocketParser::split('\\', message);
    addUserToGroupMessage.groupName = SocketParser::split('\\', message);
    addUserToGroupMessage.newUser = SocketParser::split('\\', message);
    return addUserToGroupMessage;
}

const std::string SocketParser::LoginMessageParser::_prefix = "Login: ";
const SocketParser::MessageType SocketParser::LoginMessageParser::messageType = SocketParser::LOGIN;

SocketParser::LoginMessageParser::LoginMessageParser(): SocketParser::LoginMessageParser::LoginMessageParser(SocketParser::LoginMessageParser::_prefix){}
SocketParser::LoginMessageParser::LoginMessageParser(std::string prefix)
{
    this->prefix = prefix;
}

std::string SocketParser::LoginMessageParser::parse(SocketParser::LoginMessageParser::LoginMessage loginMessage)
{
    return this->prefix + loginMessage.username + '\\' + loginMessage.guid;
}

SocketParser::LoginMessageParser::LoginMessage SocketParser::LoginMessageParser::parse(std::string message)
{
    message = message.substr(this->prefix.size());
    SocketParser::LoginMessageParser::LoginMessage loginMessage;
    loginMessage.username = SocketParser::split('\\', message);
    loginMessage.guid = SocketParser::split('\\', message);
    return loginMessage;
}