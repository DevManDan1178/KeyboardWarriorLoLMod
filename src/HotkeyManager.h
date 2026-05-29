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
};



class HotkeyManager {
    public:
        Messages& messages;
        HotkeyManager(Messages& messages);
        bool load();
        Hotkey getHotkey();

    private:
        std::vector<Hotkey> eventHotkeys;
        std::vector<Hotkey> defaultHotkeys;
        void handleHotkey(Hotkey keybind);
};

