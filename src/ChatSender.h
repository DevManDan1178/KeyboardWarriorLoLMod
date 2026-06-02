#pragma once
#define NOMINMAX
#include <windows.h>
#include <string>

class ChatSender {
    public:
        void sendMessage(const std::string& msg);

    private:
        void sendKey(WORD vk);
        void sendUnicodeChar(wchar_t ch);
};