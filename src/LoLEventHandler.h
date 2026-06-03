#pragma once

#include "../external/json.hpp"
#include "Messages.h"
#include "HotkeyManager.h"
#include <string>
#include "unordered_map"
#include <variant>
#include "LoLTypes.h"
#include "ChatSender.h"
#include <functional>
#include <queue>
#include <tuple>
#include <chrono>

using json = nlohmann::json;


//Event Categories
class LoLEventHandler {
    public:
        std::queue<std::tuple<std::string, std::string>> eventQueue; //{Category, eventName}
        Messages &messages;
        HotkeyManager &hotkeyManager;
        LoLPlayersInfo playersInfo;
        ChatSender &chatSender;
        std::function<void(std::string, std::string)> &onCurrentEventChanged;

        bool timerRunning;
        std::chrono::steady_clock::time_point currentEventStartTime;


        LoLEventHandler(Messages &_messages, HotkeyManager &_hotkeyManager, ChatSender &_chatSender, std::function<void(std::string, std::string)> &_onCurrentEventChanged);
        void processLoLEvent(json LoLEvent);
        
        void closeCurrentEvent();
        void process();

        void processHotkeyPressed(int hotkeyIndex, bool isEvent);

        std::tuple<std::string, std::string> getCurrentEvent();
    private:   
        void queueLoLEvent(std::string eventCategory, std::string eventName);
        void printPlayersInfo();
};