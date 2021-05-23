#pragma once
#include <string>
#include "sqlpp11/sqlpp11.h"
#include "models.h"
#include "connection.h"
#include "helpers.h"
#include <vector>
#include <socket_parser/message_parser.h>

void sendMessage(std::string content, std::string reciever, std::string sender, int ts);

std::vector<SocketParser::Message> getMessages(std::string username, std::string usernameFrom, unsigned int limit = 5);