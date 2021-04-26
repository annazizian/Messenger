#pragma once
#include <string>
#include "sqlpp11/sqlpp11.h"
#include "models.h"
#include "connection.h"
#include "helpers.h"
#include <vector>

void sendMessage(std::string content, std::string reciever, std::string sender, int ts);

std::vector<std::pair<std::string, std::string> > getMessages(std::string username, unsigned int limit = 5);