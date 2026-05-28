#pragma once
#include "Config.h"

class HotkeyManager {
public:
    Config& config;
    HotkeyManager(Config& config);
    void setup();
    int getKeybind();

    private:
    
    void handleHotkey(int id);
};

