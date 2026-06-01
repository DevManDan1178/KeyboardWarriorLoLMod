#include "../external/json.hpp"
#include "HotkeyManager.h"
#include "ChatSender.h"
#include <uiohook.h>
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
 
    //Get default keybinds
    json _defaultHotkeys = hotkeysData["Defaults"];
    for (int i = 0; i < _defaultHotkeys.size(); i++) {
        Hotkey hotkey = parseHotkeyData(_defaultHotkeys[i]);
        defaultHotkeys.push_back(hotkey);
    }

    //Get event keybinds
    json _eventHotkeys = hotkeysData["Events"];
    for (int i = 0; i < _eventHotkeys.size(); i++) {
        Hotkey hotkey = parseHotkeyData(_eventHotkeys[i]);
        eventHotkeys.push_back(hotkey);
    }
    
    Hotkey _skipEventHotkey = parseHotkeyData(hotkeysData["SkipEvent"]);
    skipEventHotkey = _skipEventHotkey;


    float _eventHotkeyDuration = hotkeysData["EventHotkeyDuration"];
    std::cout << _eventHotkeyDuration << std::endl;

    Hotkey _toggleInGameInteractableHotkey = parseHotkeyData(hotkeysData["ToggleInGameInteractable"]);
    toggleInGameInteractableHotkey = _toggleInGameInteractableHotkey;

    eventHotkeyDuration = _eventHotkeyDuration;

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

        hotkeysData["SkipEvent"] = hotkeyToJson(skipEventHotkey);     
        hotkeysData["ToggleInGameInteractable"] = hotkeyToJson(toggleInGameInteractableHotkey);
        hotkeysData["EventHotkeyDuration"] = eventHotkeyDuration;

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

static bool checkHotkeysEqual(Hotkey hotkey, Hotkey otherHotkey) {
    return hotkey.bindType == otherHotkey.bindType && hotkey.keyCode == otherHotkey.keyCode && hotkey.modifiers == otherHotkey.modifiers;
}

bool HotkeyManager::checkHotkeyIsInList(Hotkey hotkey, const std::vector<Hotkey> hotkeyList, int exceptForIndex) {
    for (int i = 0; i < hotkeyList.size(); i++) {
        if (i != exceptForIndex && checkHotkeysEqual(hotkey, hotkeyList[i])) {
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
    if (checkHotkeyIsInList(hotkey, hotkeyList, index) || checkHotkeyIsInList(hotkey, isEventHotkey ? defaultHotkeys : eventHotkeys) || checkHotkeysEqual(hotkey, skipEventHotkey) || checkHotkeysEqual(hotkey, toggleInGameInteractableHotkey)) {
        std::cout << "Attempt to set an already existing hotkey" << std::endl;
        return false;
    }
    hotkeyList[index] = hotkey;
    attemptWriteToJSON();
    return true;
}

bool HotkeyManager::addHotkey(Hotkey hotkey, bool isEventHotkey) {
    std::vector<Hotkey> &hotkeyList = isEventHotkey ? eventHotkeys : defaultHotkeys;
    if (checkHotkeyIsInList(hotkey, eventHotkeys) || checkHotkeyIsInList(hotkey, defaultHotkeys) || checkHotkeysEqual(hotkey, skipEventHotkey) || checkHotkeysEqual(hotkey, toggleInGameInteractableHotkey)) {
        std::cout << "Attempt to add an already existing hotkey" << std::endl;
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

bool HotkeyManager::setSkipEventHotkey(Hotkey hotkey) {
    if (checkHotkeyIsInList(hotkey, eventHotkeys) || checkHotkeyIsInList(hotkey, defaultHotkeys) || checkHotkeysEqual(hotkey, toggleInGameInteractableHotkey)) {
        std::cout << "Attempt to set an already existing hotkey to SkipEvent" << std::endl;
        return false;
    }
    skipEventHotkey = hotkey;
    attemptWriteToJSON();
    return true;
}

bool HotkeyManager::setToggleInGameInteractableHotkey(Hotkey hotkey) {
    if (checkHotkeyIsInList(hotkey, eventHotkeys) || checkHotkeyIsInList(hotkey, defaultHotkeys) || checkHotkeysEqual(hotkey, skipEventHotkey)) {
        std::cout << "Attempt to set an already existing hotkey to ToggleInGameInteractable" << std::endl;
        return false;
    }
    toggleInGameInteractableHotkey = hotkey;
    attemptWriteToJSON();
    return true;
}

void HotkeyManager::handleHotkey(Hotkey keybind) {
    // TODO ChatSender usage
}


static int SDLToUiohook(SDL_Scancode sc)
{
    switch (sc)
    {
        case SDL_SCANCODE_A: return VC_A;
        case SDL_SCANCODE_B: return VC_B;
        case SDL_SCANCODE_C: return VC_C;
        case SDL_SCANCODE_D: return VC_D;
        case SDL_SCANCODE_E: return VC_E;
        case SDL_SCANCODE_F: return VC_F;
        case SDL_SCANCODE_G: return VC_G;
        case SDL_SCANCODE_H: return VC_H;
        case SDL_SCANCODE_I: return VC_I;
        case SDL_SCANCODE_J: return VC_J;
        case SDL_SCANCODE_K: return VC_K;
        case SDL_SCANCODE_L: return VC_L;
        case SDL_SCANCODE_M: return VC_M;
        case SDL_SCANCODE_N: return VC_N;
        case SDL_SCANCODE_O: return VC_O;
        case SDL_SCANCODE_P: return VC_P;
        case SDL_SCANCODE_Q: return VC_Q;
        case SDL_SCANCODE_R: return VC_R;
        case SDL_SCANCODE_S: return VC_S;
        case SDL_SCANCODE_T: return VC_T;
        case SDL_SCANCODE_U: return VC_U;
        case SDL_SCANCODE_V: return VC_V;
        case SDL_SCANCODE_W: return VC_W;
        case SDL_SCANCODE_X: return VC_X;
        case SDL_SCANCODE_Y: return VC_Y;
        case SDL_SCANCODE_Z: return VC_Z;

        case SDL_SCANCODE_0: return VC_0;
        case SDL_SCANCODE_1: return VC_1;
        case SDL_SCANCODE_2: return VC_2;
        case SDL_SCANCODE_3: return VC_3;
        case SDL_SCANCODE_4: return VC_4;
        case SDL_SCANCODE_5: return VC_5;
        case SDL_SCANCODE_6: return VC_6;
        case SDL_SCANCODE_7: return VC_7;
        case SDL_SCANCODE_8: return VC_8;
        case SDL_SCANCODE_9: return VC_9;

        case SDL_SCANCODE_TAB: return VC_TAB;
        case SDL_SCANCODE_RETURN: return VC_ENTER;
        case SDL_SCANCODE_ESCAPE: return VC_ESCAPE;
        case SDL_SCANCODE_SPACE: return VC_SPACE;

        case SDL_SCANCODE_F1: return VC_F1;
        case SDL_SCANCODE_F2: return VC_F2;
        case SDL_SCANCODE_F3: return VC_F3;
        case SDL_SCANCODE_F4: return VC_F4;
        case SDL_SCANCODE_F5: return VC_F5;
        case SDL_SCANCODE_F6: return VC_F6;
        case SDL_SCANCODE_F7: return VC_F7;
        case SDL_SCANCODE_F8: return VC_F8;
        case SDL_SCANCODE_F9: return VC_F9;
        case SDL_SCANCODE_F10: return VC_F10;
        case SDL_SCANCODE_F11: return VC_F11;
        case SDL_SCANCODE_F12: return VC_F12;

        default:
            return VC_UNDEFINED;
    }
}
Hotkey HotkeyManager::queryHotkey()
{
    SDL_Event event;

    while (true)
    {
        while (SDL_PollEvent(&event))
        {
            // ----------------------------
            // Keyboard
            // ----------------------------
            if (event.type == SDL_KEYDOWN)
            {
                SDL_Scancode sc = event.key.keysym.scancode;

                // Ignore modifier keys
                if (sc == SDL_SCANCODE_LCTRL  || sc == SDL_SCANCODE_RCTRL ||
                    sc == SDL_SCANCODE_LSHIFT || sc == SDL_SCANCODE_RSHIFT ||
                    sc == SDL_SCANCODE_LALT   || sc == SDL_SCANCODE_RALT)
                {
                    continue;
                }

                int key = SDLToUiohook(sc);
                if (key == VC_UNDEFINED)
                    continue;

                uint8_t mods = 0;
                SDL_Keymod sdlMods = SDL_GetModState();

                if (sdlMods & KMOD_CTRL)  mods |= Modifiers::Ctrl;
                if (sdlMods & KMOD_SHIFT) mods |= Modifiers::Shift;
                if (sdlMods & KMOD_ALT)   mods |= Modifiers::Alt;

                return { key, BindType::Keyboard, mods };
            }

            // ----------------------------
            // Mouse
            // ----------------------------
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                // Ignore left and right mouse buttons
                if (event.button.button == SDL_BUTTON_LEFT ||
                    event.button.button == SDL_BUTTON_RIGHT)
                {
                    continue;
                }

                uint8_t mods = 0;
                SDL_Keymod sdlMods = SDL_GetModState();

                if (sdlMods & KMOD_CTRL)  mods |= Modifiers::Ctrl;
                if (sdlMods & KMOD_SHIFT) mods |= Modifiers::Shift;
                if (sdlMods & KMOD_ALT)   mods |= Modifiers::Alt;

                return { (int)event.button.button, BindType::Mouse, mods };
            }

            // ----------------------------
            // Quit
            // ----------------------------
            if (event.type == SDL_QUIT)
            {
                return { -1, BindType::None, 0 };
            }
        }

        SDL_Delay(1);
    }
}


std::string HotkeyManager::hotkeyToString(const Hotkey& hotkey)
{
    std::stringstream ss;

    // -------------------------
    // Modifiers
    // -------------------------
    if (hotkey.modifiers & Modifiers::Ctrl)  ss << "Ctrl+";
    if (hotkey.modifiers & Modifiers::Shift) ss << "Shift+";
    if (hotkey.modifiers & Modifiers::Alt)   ss << "Alt+";

    // -------------------------
    // Keyboard (uiohook codes)
    // -------------------------
    if (hotkey.bindType == BindType::Keyboard)
    {
        switch (hotkey.keyCode)
        {
            case VC_A: ss << "A"; break;
            case VC_B: ss << "B"; break;
            case VC_C: ss << "C"; break;
            case VC_D: ss << "D"; break;
            case VC_E: ss << "E"; break;
            case VC_F: ss << "F"; break;
            case VC_G: ss << "G"; break;
            case VC_H: ss << "H"; break;
            case VC_I: ss << "I"; break;
            case VC_J: ss << "J"; break;
            case VC_K: ss << "K"; break;
            case VC_L: ss << "L"; break;
            case VC_M: ss << "M"; break;
            case VC_N: ss << "N"; break;
            case VC_O: ss << "O"; break;
            case VC_P: ss << "P"; break;
            case VC_Q: ss << "Q"; break;
            case VC_R: ss << "R"; break;
            case VC_S: ss << "S"; break;
            case VC_T: ss << "T"; break;
            case VC_U: ss << "U"; break;
            case VC_V: ss << "V"; break;
            case VC_W: ss << "W"; break;
            case VC_X: ss << "X"; break;
            case VC_Y: ss << "Y"; break;
            case VC_Z: ss << "Z"; break;

            case VC_0: ss << "0"; break;
            case VC_1: ss << "1"; break;
            case VC_2: ss << "2"; break;
            case VC_3: ss << "3"; break;
            case VC_4: ss << "4"; break;
            case VC_5: ss << "5"; break;
            case VC_6: ss << "6"; break;
            case VC_7: ss << "7"; break;
            case VC_8: ss << "8"; break;
            case VC_9: ss << "9"; break;

            case VC_TAB:        ss << "Tab"; break;
            case VC_ENTER:      ss << "Enter"; break;
            case VC_ESCAPE:     ss << "Escape"; break;
            case VC_SPACE:      ss << "Space"; break;
            case VC_BACKSPACE:   ss << "Backspace"; break;

            case VC_F1: ss << "F1"; break;
            case VC_F2: ss << "F2"; break;
            case VC_F3: ss << "F3"; break;
            case VC_F4: ss << "F4"; break;
            case VC_F5: ss << "F5"; break;
            case VC_F6: ss << "F6"; break;
            case VC_F7: ss << "F7"; break;
            case VC_F8: ss << "F8"; break;
            case VC_F9: ss << "F9"; break;
            case VC_F10: ss << "F10"; break;
            case VC_F11: ss << "F11"; break;
            case VC_F12: ss << "F12"; break;

            case VC_INSERT:     ss << "Insert"; break;
            case VC_DELETE:     ss << "Delete"; break;
            case VC_HOME:       ss << "Home"; break;
            case VC_END:        ss << "End"; break;
            case VC_PAGE_UP:    ss << "PageUp"; break;
            case VC_PAGE_DOWN:  ss << "PageDown"; break;

            case VC_UP:    ss << "Up"; break;
            case VC_DOWN:  ss << "Down"; break;
            case VC_LEFT:  ss << "Left"; break;
            case VC_RIGHT: ss << "Right"; break;

            default:
                ss << "Key" << hotkey.keyCode;
                break;
        }
    }
    // -------------------------
    // Mouse
    // -------------------------
    else if (hotkey.bindType == BindType::Mouse)
    {
        switch (hotkey.keyCode)
        {
            case 1: ss << "Mouse Left"; break;
            case 2: ss << "Mouse Right"; break;
            case 3: ss << "Mouse Middle"; break;
            default: ss << "Mouse" << hotkey.keyCode; break;
        }
    }
    else
    {
        ss << "Undefined";
    }

    return ss.str();
}