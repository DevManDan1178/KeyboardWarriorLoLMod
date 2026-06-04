#define NOMINMAX
#include <windows.h>
#include "ChatSender.h"
#include <iostream>
#include <thread>

const int PRE_CHAT_BACKSPACE_COUNT = 5; //In case shift is used, which starts message with /All 

static void sleep_ms(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void ChatSender::sendKey(WORD vk)
{
    INPUT input = {};
    input.type = INPUT_KEYBOARD;

    input.ki.wVk = vk;
    input.ki.wScan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    input.ki.dwFlags = KEYEVENTF_SCANCODE;

    SendInput(1, &input, sizeof(INPUT));

    sleep_ms(15);

    input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

    SendInput(1, &input, sizeof(INPUT));
}

void ChatSender::sendUnicodeChar(wchar_t ch)
{
    INPUT inputs[2] = {};

    // key down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = 0;
    inputs[0].ki.wScan = ch;
    inputs[0].ki.dwFlags = KEYEVENTF_UNICODE;

    // key up
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 0;
    inputs[1].ki.wScan = ch;
    inputs[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

void ChatSender::sendMessage(const std::string& msg)
{
    
     // Open chat (Enter)
    sendKey(VK_RETURN);
    sleep_ms(1);

    // Backspace 5 times
    for (int i = 0; i < PRE_CHAT_BACKSPACE_COUNT; ++i)
    {
        sendKey(VK_BACK);
        sleep_ms(1);
    }

    int len = MultiByteToWideChar(
        CP_UTF8, 0,
        msg.c_str(), -1,
        nullptr, 0);

    if (len <= 0)
        return;

    std::wstring wmsg(len - 1, L'\0');

    MultiByteToWideChar(
        CP_UTF8, 0,
        msg.c_str(), -1,
        wmsg.data(), len);



    for (wchar_t ch : wmsg)
    {
        sendUnicodeChar(ch);
    sleep_ms(1);
    }

    sleep_ms(1);
            
    // Send message (Enter)
    sendKey(VK_RETURN);
}