#pragma once
#include "Messages.h"
#include <string>
#include <unordered_map>
#include <vector>

enum class BindType {
    Keyboard, Mouse, None
};

enum Modifiers {
    None = 0,
    Shift = 1 << 0,
    // Ctrl = 1 << 1,
    // Alt = 1 << 2,
};

struct Hotkey {
    int keyCode;
    BindType bindType;
    uint8_t modifiers; //Bitmask from enum Modifiers
    Hotkey()
        : keyCode(0), bindType(BindType::None), modifiers(0) {}
        
    Hotkey(int k, BindType b, uint8_t m)
        : keyCode(k), bindType(b), modifiers(m) {}
};


 
class HotkeyManager {
    public:       
        const float maxEventHotkeyDuration = 15.0f;
        const float minEventHotkeyDuration = 3.0f; 
        std::vector<Hotkey> eventHotkeys;
        std::vector<Hotkey> defaultHotkeys;
       
        Hotkey skipEventHotkey;
        float eventHotkeyDuration;
        Hotkey toggleInGameInteractableHotkey;
        Hotkey toggleInGameAlwaysVisibleHotkey;

        Messages& messages;
        
        HotkeyManager(Messages& messages);
        
        bool load();
        Hotkey queryHotkey();

        bool addHotkey(Hotkey hotkey, bool isEventHotkey);
        bool setHotkey(Hotkey hotkey, bool isEventHotkey, int index);
        bool setSkipEventHotkey(Hotkey hotkey);
        bool setToggleInGameInteractableHotkey(Hotkey hotkey);
        bool setToggleInGameAlwaysVisibleHotkey(Hotkey hotkey);
        bool removeHotkey(bool isEventHotkey, int index);
        void setEventHotkeyDuration(float duration);
        static std::string hotkeyToString(const Hotkey& hotkey);

    private:
        void attemptWriteToJSON();
        bool writeToJSON();
};

