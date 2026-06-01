#pragma once

#include <uiohook.h>
#include <functional>
#include <unordered_map>
#include <vector>
#include "HotkeyManager.h" 

using HotkeyCallback = std::function<void()>;

class InputReader {
public:
    static bool start();
    static void stop();

    static void onHotkey(const Hotkey& hotkey, HotkeyCallback callback);
    static void onHotkeyRelease(const Hotkey& hotkey, HotkeyCallback callback);
    static void clearHotkeys();

    static uint8_t currentModifiers();

    static bool isKeyDown(int keycode);
    static void dispatch(uiohook_event* event);
    
    static bool s_ctrl;
    static bool s_shift;
    static bool s_alt;
    
private:
    
    static bool matchesModifiers(uint8_t required);

    struct HotkeyBinding {
        Hotkey      hotkey;
        HotkeyCallback callback;
    };

    static std::vector<HotkeyBinding>    s_bindings;
    static std::vector<InputReader::HotkeyBinding> s_releaseBindings;
    static std::unordered_map<int, bool> s_keyStates;


};