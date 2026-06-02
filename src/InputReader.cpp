#include "InputReader.h"
#include <iostream>
#include <thread>
#include <bit>
#include <windows.h>

// uiohook raw keycodes for modifier keys
#define KEY_LEFT_CTRL   0x001D
#define KEY_RIGHT_CTRL  0xE01D
#define KEY_LEFT_SHIFT  0x002A
#define KEY_RIGHT_SHIFT 0x0036
#define KEY_LEFT_ALT    0x0038
#define KEY_RIGHT_ALT   0xE038

// Static state
std::vector<InputReader::HotkeyBinding> InputReader::s_bindings;
std::vector<InputReader::HotkeyBinding> InputReader::s_releaseBindings;
std::unordered_map<int, bool> InputReader::s_keyStates;

bool InputReader::s_ctrl  = false;
bool InputReader::s_shift = false;
bool InputReader::s_alt   = false;

static bool isModifierKey(int keycode)
{
    return keycode == KEY_LEFT_CTRL   || keycode == KEY_RIGHT_CTRL ||
           keycode == KEY_LEFT_SHIFT  || keycode == KEY_RIGHT_SHIFT ||
           keycode == KEY_LEFT_ALT    || keycode == KEY_RIGHT_ALT;
}

static void updateModifier(int keycode, bool pressed)
{
    if (keycode == KEY_LEFT_CTRL || keycode == KEY_RIGHT_CTRL)
        InputReader::s_ctrl = pressed;
    else if (keycode == KEY_LEFT_SHIFT || keycode == KEY_RIGHT_SHIFT)
        InputReader::s_shift = pressed;
    else if (keycode == KEY_LEFT_ALT || keycode == KEY_RIGHT_ALT)
        InputReader::s_alt = pressed;
}

// ---------------- Modifier state ----------------

uint8_t InputReader::currentModifiers()
{
    uint8_t mods = Modifiers::None;

    bool ctrl  = s_ctrl  || (GetAsyncKeyState(VK_CONTROL) & 0x8000);
    bool shift = s_shift || (GetAsyncKeyState(VK_SHIFT) & 0x8000);
    bool alt   = s_alt   || (GetAsyncKeyState(VK_MENU) & 0x8000);

    if (ctrl)  mods |= Modifiers::Ctrl;
    if (shift) mods |= Modifiers::Shift;
    if (alt)   mods |= Modifiers::Alt;

    /*
    if (DEBUG_INPUT)
    {
        std::cout << "[MOD STATE] Ctrl=" << ctrl
                  << " Shift=" << shift
                  << " Alt=" << alt
                  << " (raw ctrl=" << s_ctrl
                  << ", shift=" << s_shift
                  << ", alt=" << s_alt << ")\n";
    }
    */
    return mods;
}

bool InputReader::matchesModifiers(uint8_t required)
{
    uint8_t current = currentModifiers();
    bool match = (current & required) == required;

    return match;
}

// ---------------- Dispatch ----------------

void InputReader::dispatch(uiohook_event* event) {
    switch (event->type)
    {
        // ---------------- KEY PRESS ----------------
        case EVENT_KEY_PRESSED:
        {
            int code = event->data.keyboard.keycode;

            updateModifier(code, true);
            s_keyStates[code] = true;

            if (!isModifierKey(code))
            {
                for (auto& binding : s_bindings)
                {
                    if (binding.hotkey.bindType == BindType::Keyboard &&
                        binding.hotkey.keyCode == code)
                    {
                        if (matchesModifiers(binding.hotkey.modifiers))
                        {
                            binding.callback();
                        }
                    }
                }
            }
            break;
        }

        // ---------------- KEY RELEASE ----------------
        case EVENT_KEY_RELEASED:
        {
            int code = event->data.keyboard.keycode;
            updateModifier(code, false);
            s_keyStates[code] = false;

            if (!isModifierKey(code))
            {
                for (auto& binding : s_releaseBindings)
                {
                    if (binding.hotkey.bindType == BindType::Keyboard &&
                        binding.hotkey.keyCode == code)
                    {
                        if (matchesModifiers(binding.hotkey.modifiers))
                        {
                            binding.callback();
                        }
                    }
                }
            }
            break;
        }

        // ---------------- MOUSE PRESS ----------------




        case EVENT_MOUSE_PRESSED:
        {
            int btn = event->data.mouse.button;

            s_keyStates[-btn] = true;

            uint8_t mods = currentModifiers();

            InputReader::HotkeyBinding* bestMatch = nullptr;
            int bestScore = -1;

            for (auto& binding : s_bindings)
            {
                if (binding.hotkey.bindType != BindType::Mouse)
                    continue;

                if (binding.hotkey.keyCode != btn)
                    continue;

                if (!matchesModifiers(binding.hotkey.modifiers))
                    continue;

                // score = number of modifier bits (more specific wins)
                int score = std::popcount(binding.hotkey.modifiers);

                if (score > bestScore)
                {
                    bestScore = score;
                    bestMatch = &binding;
                }
            }

            if (bestMatch)
                bestMatch->callback();

            break;
        }

        // ---------------- MOUSE RELEASE ----------------
       case EVENT_MOUSE_RELEASED:
        {
            int btn = event->data.mouse.button;

            s_keyStates[-btn] = false;

            InputReader::HotkeyBinding* bestMatch = nullptr;
            int bestScore = -1;

            for (auto& binding : s_releaseBindings)
            {
                if (binding.hotkey.bindType != BindType::Mouse)
                    continue;

                if (binding.hotkey.keyCode != btn)
                    continue;

                if (!matchesModifiers(binding.hotkey.modifiers))
                    continue;

                int score = std::popcount(binding.hotkey.modifiers);

                if (score > bestScore)
                {
                    bestScore = score;
                    bestMatch = &binding;
                }
            }

            if (bestMatch)  
                bestMatch->callback();

            break;
        }

        default:
            break;
    }
}

// ---------------- Hook lifecycle ----------------

bool InputReader::start()
{
    hook_set_dispatch_proc(dispatch);

    std::thread([]()
    {
        int result = hook_run();
        if (result != UIOHOOK_SUCCESS)
            std::cerr << "[InputReader] hook_run failed: " << result << "\n";
    }).detach();

    return true;
}

void InputReader::stop()
{
    hook_stop();
}

// ---------------- Hotkeys ----------------

void InputReader::onHotkey(const Hotkey& hotkey, HotkeyCallback callback)
{
    std::cout << "[REGISTER] key=" << hotkey.keyCode
              << " modifiers=" << (int)hotkey.modifiers << "\n";

    s_bindings.push_back({ hotkey, std::move(callback) });
}

void InputReader::onHotkeyRelease(const Hotkey& hotkey, HotkeyCallback callback)
{
    s_releaseBindings.push_back({ hotkey, std::move(callback) });
}

void InputReader::clearHotkeys()
{
    s_bindings.clear();
    s_releaseBindings.clear();
}
                
// ---------------- Query ----------------

bool InputReader::isKeyDown(int keycode)
{
    bool down = s_keyStates[keycode];

    return down;
}