#pragma once
#include <messenger/MessageQueue.h>
#include <socket_parser/message_parser.h>

void sendNotification(SocketParser::Message message);
void storeMessage(SocketParser::Message message);