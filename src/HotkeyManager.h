#pragma once
#include "Config.h"

struct Hotkey {
    int key;
    int messageId;
};

enum class BindType {
    Keyboard, Mouse, None
};

enum Modifiers {
    None = 0,
    Ctrl = 1 << 0,
    Shift = 1 << 1,
    Alt = 1 << 2,
};

struct Keybind {
    BindType bindType;
    int keyCode;
    uint8_t modifiers; //Bitmask from enum Modifiers
};


class HotkeyManager {
public:
    Config& config;
    HotkeyManager(Config& config);
    void setup();
    Keybind getKeybind();

    private:
    
    void handleHotkey(int id);
};

