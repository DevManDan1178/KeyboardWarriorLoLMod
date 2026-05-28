#include "Config.h"
#include "HotkeyManager.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
//"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" to initialize environment

    
int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_Window* window = SDL_CreateWindow(
        "test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_SHOWN
    );
    Config config;

    HotkeyManager hotkeyManager(config);
    std::cout << "a " << std::endl;
    int kb = hotkeyManager.getKeybind();
    std::cout << kb << " a" << std::endl;
    return 0;
}