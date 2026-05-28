#include "ChatSender.h"
#include <iostream>
#include <windows.h>

void ChatSender::sendMessage(const std::string& msg) {
    //TODO  uses SendInput() to send chat message to LoL client
    std::cout << "[ChatSender] Would send: " << msg << "\n";
}