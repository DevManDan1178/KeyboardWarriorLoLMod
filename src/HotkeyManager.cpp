#include "../external/json.hpp"
#include "HotkeyManager.h"
#include "ChatSender.h"
#include <unordered_map>
#include "HotkeyManager.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#define SDL_MAIN_HANDLED
#include <SDL.h>


using json = nlohmann::json;

const std::filesystem::path path = std::filesystem::current_path() / "config/hotkeys.json";


HotkeyManager::HotkeyManager(Messages& _messages)
: messages(_messages) {}

static Hotkey parseHotkeyData(json hotkeyData) {
    Hotkey hotkey;
    hotkey.keyCode = hotkeyData.value("Key", 0u);
    std::string bindType = hotkeyData.value("BindType", "");
    hotkey.bindType = bindType == "Keyboard" ? BindType::Keyboard : (bindType == "Mouse" ? BindType::Mouse : BindType::None);
    hotkey.modifiers = hotkeyData.value("Modifiers", 0u);
    //std::cout << "Hotkey with key, bindtype, modifier: " << hotkey.keyCode << " " << bindType << " " << hotkey.modifiers << std::endl;
    return hotkey;
}

bool HotkeyManager::load() {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open file at path " << path << std::endl;
        return false;
    }
    
    json hotkeysData;
    file >> hotkeysData;
    //std::cout << "Successfully retrieved hotkeys\n" << hotkeysData.dump(4) << std::endl;
    
    //Get default keybinds
    json defaultHotkeys = hotkeysData["Defaults"];
    for (int i = 0; i < defaultHotkeys.size(); i++) {
        Hotkey hotkey = parseHotkeyData(defaultHotkeys[i]);
        HotkeyManager::defaultHotkeys.push_back(hotkey);
    }

    //Get event keybinds
    json eventHotkeys = hotkeysData["Events"];
    for (int i = 0; i < eventHotkeys.size(); i++) {
        Hotkey hotkey = parseHotkeyData(eventHotkeys[i]);
        HotkeyManager::eventHotkeys.push_back(hotkey);
    }
    
    return true;
}


void HotkeyManager::handleHotkey(Hotkey keybind) {
    // TODO ChatSender usage
}

Hotkey HotkeyManager::getHotkey()
{
    SDL_Event event;

    while (true)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                return { event.key.keysym.sym, BindType::Keyboard, };
            }

            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                return { event.button.button, BindType::Mouse, };
            }

            if (event.type == SDL_QUIT)
            {
                return { -1, BindType::None, };
            }
        }

        SDL_Delay(1);
    }
}



