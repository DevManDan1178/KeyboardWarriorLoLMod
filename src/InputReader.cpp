#include "InputReader.h"
#include <iostream>
#include <thread>

// uiohook raw keycodes for modifier keys
#define KEY_LEFT_CTRL   0x001D
#define KEY_RIGHT_CTRL  0xE01D
#define KEY_LEFT_SHIFT  0x002A
#define KEY_RIGHT_SHIFT 0x0036
#define KEY_LEFT_ALT    0x0038
#define KEY_RIGHT_ALT   0xE038

// Static member definitions
std::vector<InputReader::HotkeyBinding> InputReader::s_bindings;
std::vector<InputReader::HotkeyBinding> InputReader::s_releaseBindings;
std::unordered_map<int, bool>           InputReader::s_keyStates;
bool InputReader::s_ctrl  = false;
bool InputReader::s_shift = false;
bool InputReader::s_alt   = false;


static bool isModifierKey(int keycode) {
    return keycode == KEY_LEFT_CTRL   || keycode == KEY_RIGHT_CTRL  ||
           keycode == KEY_LEFT_SHIFT  || keycode == KEY_RIGHT_SHIFT ||
           keycode == KEY_LEFT_ALT    || keycode == KEY_RIGHT_ALT;
}

static void updateModifier(int keycode, bool pressed) {
    if      (keycode == KEY_LEFT_CTRL  || keycode == KEY_RIGHT_CTRL)
        InputReader::s_ctrl  = pressed;
    else if (keycode == KEY_LEFT_SHIFT || keycode == KEY_RIGHT_SHIFT)
        InputReader::s_shift = pressed;
    else if (keycode == KEY_LEFT_ALT   || keycode == KEY_RIGHT_ALT)
        InputReader::s_alt   = pressed;
}

uint8_t InputReader::currentModifiers() {
    uint8_t mods = Modifiers::None;
    if (s_ctrl)  mods |= Modifiers::Ctrl;
    if (s_shift) mods |= Modifiers::Shift;
    if (s_alt)   mods |= Modifiers::Alt;
    return mods;
}

bool InputReader::matchesModifiers(uint8_t required) {
    return currentModifiers() == required;
}

void InputReader::dispatch(uiohook_event* event) {
    switch (event->type) {

        case EVENT_KEY_PRESSED: {
            int code = event->data.keyboard.keycode;
            if (s_keyStates[code]) {
                break; 
            }
            s_keyStates[code] = true;
            updateModifier(code, true);                                     
            
            if (!isModifierKey(code)) {
                for (auto& binding : s_bindings) {
                    if (binding.hotkey.bindType == BindType::Keyboard)
                    {
                        std::cout
                            << "pressed=" << code
                            << " expected=" << binding.hotkey.keyCode
                            << " match=" << (binding.hotkey.keyCode == code)
                            << '\n';
                    }
                    if (binding.hotkey.bindType == BindType::Keyboard &&
                        binding.hotkey.keyCode  == code               &&
                        matchesModifiers(binding.hotkey.modifiers))
                    {
                        binding.callback();
                    }
                }
            }
            break;
        }

        case EVENT_KEY_RELEASED: {
            int code = event->data.keyboard.keycode;

            s_keyStates[code] = false;
            updateModifier(code, false);

            if (!isModifierKey(code)) {
                for (auto& binding : s_releaseBindings) {
                    if (binding.hotkey.bindType == BindType::Keyboard)
                    {
                        std::cout
                            << "released=" << code
                            << " expected=" << binding.hotkey.keyCode
                            << " match=" << (binding.hotkey.keyCode == code)
                            << '\n';
                    }
                    if (binding.hotkey.bindType == BindType::Keyboard &&
                        binding.hotkey.keyCode  == code               &&
                        matchesModifiers(binding.hotkey.modifiers))
                    {
                        binding.callback();
                    }
                }
            }
            break;
        }

        case EVENT_MOUSE_PRESSED: {
            int btn = event->data.mouse.button;
            s_keyStates[-btn] = true;

            for (auto& binding : s_bindings) {
                if (binding.hotkey.bindType == BindType::Mouse &&
                    binding.hotkey.keyCode  == btn             &&
                    matchesModifiers(binding.hotkey.modifiers))
                {
                    binding.callback();
                }
            }
            break;
        }

        case EVENT_MOUSE_RELEASED: {
            int btn = event->data.mouse.button;
            s_keyStates[-btn] = false;

            for (auto& binding : s_releaseBindings) {
                if (binding.hotkey.bindType == BindType::Mouse &&
                    binding.hotkey.keyCode  == btn             &&
                    matchesModifiers(binding.hotkey.modifiers))
                {
                    binding.callback();
                }
            }
            break;
        }

        default:
            break;
    }
}

bool InputReader::start() {
    hook_set_dispatch_proc(dispatch);

    std::thread([]() {
        int result = hook_run();
        if (result != UIOHOOK_SUCCESS)
            std::cerr << "[InputReader] hook_run failed: " << result << "\n";
    }).detach();

    return true;
}

void InputReader::stop() {
    hook_stop();
}

void InputReader::onHotkey(const Hotkey& hotkey, HotkeyCallback callback) {
    std::cout << "Hotkey registered - key: " << hotkey.keyCode << std::endl;
    s_bindings.push_back({ hotkey, std::move(callback) });
}

void InputReader::onHotkeyRelease(const Hotkey& hotkey, HotkeyCallback callback) {
    s_releaseBindings.push_back({ hotkey, std::move(callback) });
}

void InputReader::clearHotkeys() {
    s_bindings.clear();
    s_releaseBindings.clear();
}

bool InputReader::isKeyDown(int keycode) {
    auto it = s_keyStates.find(keycode);
    return it != s_keyStates.end() && it->second;
}