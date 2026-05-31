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
    Ctrl = 1 << 0,
    Shift = 1 << 1,
    Alt = 1 << 2,
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
        std::vector<Hotkey> eventHotkeys;
        std::vector<Hotkey> defaultHotkeys;
        Messages& messages;
        HotkeyManager(Messages& messages);
        
        bool load();
        Hotkey queryHotkey();

        bool addHotkey(Hotkey hotkey, bool isEventHotkey);
        bool setHotkey(Hotkey hotkey, bool isEventHotkey, int index);
        bool removeHotkey(bool isEventHotkey, int index);
        static std::string hotkeyToString(const Hotkey& hotkey);

    private:
        void handleHotkey(Hotkey keybind);
        void attemptWriteToJSON();
        bool writeToJSON();
        static bool checkHotkeyIsInList(Hotkey hotkey, const std::vector<Hotkey> hotkeyList, int exceptForIndex = -1);
};

