#pragma once

#include "LoLEventHandler.h"
#include "Messages.h"
#include "HotkeyManager.h"
#include <SDL_syswm.h>

namespace CoreUI {
    void eventsOverlayUIFrame(SDL_Window* window, HWND hwnd, LoLEventHandler& lolEventHandler, Messages& messages, HotkeyManager& hotkeyManager, bool interactable);
    void mainUIFrame(SDL_Window* window, HWND hwnd, Messages& messages, HotkeyManager& hotkeyManager);
}