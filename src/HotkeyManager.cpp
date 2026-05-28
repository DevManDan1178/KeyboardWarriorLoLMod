#include "HotkeyManager.h"
#include "ChatSender.h"
#include <unordered_map>
#include <windows.h>
#include <SDL.h>
#include <iostream>
#define SDL_MAIN_HANDLED
#include "HotkeyManager.h"
#include <SDL.h>
#include <iostream>
/*
Hotkeys for dynamic events, with different messageIds to configure messages according to dynamic events.
Ex:
Game over:
message 1 (Id = 1): GG (Hotkey 1)
message 2 (Id = 2): GGEZ (Hotkey 2)

*/



struct Hotkey {
    int key;
    int messageId;
};




HotkeyManager::HotkeyManager(Config& _config)
    : config(_config) {}

void HotkeyManager::setup() {
    // TODO
}

void HotkeyManager::handleHotkey(int id) {
    // TODO ChatSender usage
}


int HotkeyManager::getKeybind() {
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN)
                return event.key.keysym.sym;

            if (event.type == SDL_MOUSEBUTTONDOWN)
                return event.button.button;

            if (event.type == SDL_QUIT)
                return -1;
        }

        SDL_Delay(1);
    }
}