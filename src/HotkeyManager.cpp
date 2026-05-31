#include "../external/json.hpp"
#include "HotkeyManager.h"
#include "ChatSender.h"
#include <unordered_map>
#include "HotkeyManager.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>

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
     return hotkey;
}

static json hotkeyToJson(const Hotkey& hotkey) {
    return {
        {"Key", hotkey.keyCode},
        {"BindType",
            hotkey.bindType == BindType::Keyboard ? "Keyboard" :
            hotkey.bindType == BindType::Mouse ? "Mouse" :
            "None"},
        {"Modifiers", hotkey.modifiers}
    };
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

void HotkeyManager::attemptWriteToJSON() {
    bool success = writeToJSON();
    if (!success) {
        std::cout << "unable to write hotkeys to JSON" << std::endl;
    }
}

bool HotkeyManager::writeToJSON() {
    try {
        json hotkeysData;

        // Defaults
        hotkeysData["Defaults"] = json::array();
        for (const Hotkey& hotkey : defaultHotkeys) {
            hotkeysData["Defaults"].push_back(hotkeyToJson(hotkey));
        }

        // Events
        hotkeysData["Events"] = json::array();
        for (const Hotkey& hotkey : eventHotkeys) {
            hotkeysData["Events"].push_back(hotkeyToJson(hotkey));
        }

        std::ofstream file(path);
        if (!file.is_open()) {
            std::cout << "Failed to open file for writing: " << path << std::endl;
            return false;
        }

        file << hotkeysData.dump(4); // pretty-print with 4-space indentation
        return true;
    }
    catch (const std::exception& e) {
        std::cout << "Error writing hotkeys JSON: " << e.what() << std::endl;
        return false;
    }
}

bool HotkeyManager::checkHotkeyIsInList(Hotkey hotkey, const std::vector<Hotkey> hotkeyList, int exceptForIndex) {
    for (int i = 0; i < hotkeyList.size(); i++) {
        Hotkey otherHotkey = hotkeyList[i];
        if (i != exceptForIndex && hotkey.bindType == otherHotkey.bindType && hotkey.keyCode == otherHotkey.keyCode && hotkey.modifiers == otherHotkey.modifiers) {
            return true;
        }
    }
    return false;
}

bool HotkeyManager::setHotkey(Hotkey hotkey, bool isEventHotkey, int index) {
    std::vector<Hotkey> &hotkeyList = isEventHotkey ? eventHotkeys : defaultHotkeys;
    if (index >= hotkeyList.size()) {
        std::cout << "Attempt to set non existent hotkey for " << (isEventHotkey ? "events" : "defaults") << " at index " << index << std::endl;
        return false;
    }
    if (checkHotkeyIsInList(hotkey, hotkeyList, index) || checkHotkeyIsInList(hotkey, isEventHotkey ? defaultHotkeys : eventHotkeys)) {
        std::cout << "Attempt to set an already existing hotkey" << std::endl;
        return false;
    }
    hotkeyList[index] = hotkey;
    attemptWriteToJSON();
    return true;
}

bool HotkeyManager::addHotkey(Hotkey hotkey, bool isEventHotkey) {
    std::vector<Hotkey> &hotkeyList = isEventHotkey ? eventHotkeys : defaultHotkeys;
    if (checkHotkeyIsInList(hotkey, eventHotkeys) || checkHotkeyIsInList(hotkey, defaultHotkeys)) {
        std::cout << "Attempt to set an already existing hotkey" << std::endl;
        return false;
    }
    hotkeyList.push_back(hotkey);
    attemptWriteToJSON();
    return true;
}

bool HotkeyManager::removeHotkey(bool isEventHotkey, int index) {
    std::vector<Hotkey> &hotkeyList = isEventHotkey ? eventHotkeys : defaultHotkeys;
    if (index >= hotkeyList.size()) {
        std::cout << "Attempt to remove non existent hotkey for " << (isEventHotkey ? "events" : "defaults") << " at index " << index << std::endl;
        return false;
    }
    hotkeyList.erase(hotkeyList.begin() + index);
    attemptWriteToJSON();
    return true;
}

void HotkeyManager::handleHotkey(Hotkey keybind) {
    // TODO ChatSender usage
}

Hotkey HotkeyManager::queryHotkey()
{
    SDL_Event event;

    while (true)
    {
        while (SDL_PollEvent(&event))
        {
            // Handle keyboard
            if (event.type == SDL_KEYDOWN)
            {
                SDL_Keycode key = event.key.keysym.sym;

                // Ignore modifier keys
                if (key == SDLK_LCTRL  || key == SDLK_RCTRL ||
                    key == SDLK_LSHIFT || key == SDLK_RSHIFT ||
                    key == SDLK_LALT   || key == SDLK_RALT)
                {
                    continue;
                }

                uint8_t mods = 0;
                SDL_Keymod sdlMods = SDL_GetModState();

                if (sdlMods & KMOD_CTRL)  mods |= Modifiers::Ctrl;
                if (sdlMods & KMOD_SHIFT) mods |= Modifiers::Shift;
                if (sdlMods & KMOD_ALT)   mods |= Modifiers::Alt;

                return { key, BindType::Keyboard, mods };
            }

            // Handle mouse
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                uint8_t mods = 0;
                SDL_Keymod sdlMods = SDL_GetModState();
                if (sdlMods & KMOD_CTRL)  mods |= Modifiers::Ctrl;
                if (sdlMods & KMOD_SHIFT) mods |= Modifiers::Shift;
                if (sdlMods & KMOD_ALT)   mods |= Modifiers::Alt;

                return { event.button.button, BindType::Mouse, mods };
            }

            // Quit event
            if (event.type == SDL_QUIT)
            {
                return { -1, BindType::None, 0 };
            }
        }

        SDL_Delay(1); 
    }
}





std::string HotkeyManager::hotkeyToString(const Hotkey& hotkey) {
    std::stringstream ss;

    // Modifiers
    if (hotkey.modifiers & Modifiers::Ctrl) ss << "Ctrl+";
    if (hotkey.modifiers & Modifiers::Shift) ss << "Shift+";
    if (hotkey.modifiers & Modifiers::Alt) ss << "Alt+";

    // Key or mouse
    if (hotkey.bindType == BindType::Keyboard) {
       
        const char* keyName = SDL_GetKeyName(static_cast<SDL_Keycode>(hotkey.keyCode));
        if (keyName && *keyName) {
            ss << keyName;
        } else {
            ss << "Key" << hotkey.keyCode;
        }
    } else if (hotkey.bindType == BindType::Mouse) {
        switch (hotkey.keyCode) {
            case SDL_BUTTON_LEFT:   ss << "Mouse Left"; break;
            case SDL_BUTTON_RIGHT:  ss << "Mouse Right"; break;
            case SDL_BUTTON_MIDDLE: ss << "Mouse Middle"; break;
            default: ss << "Mouse" << hotkey.keyCode; break;
        }
    } else {
        ss << "[?]";
    }

    return ss.str();
}