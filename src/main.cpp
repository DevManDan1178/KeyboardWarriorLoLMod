#include "Messages.h"
#include "HotkeyManager.h"

#include "ui/MessagesUI.h"
#include "ui/HotkeysUI.h"
#include "LoLReader.h"
#include "CoreUI.h"
#include <windows.h>
#include "InputReader.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>
#include <uiohook.h>    
#include <iostream>
#include <bitset>
#include <map>
#include <string>
#include <format>

//ImGui
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

void setClickThrough(HWND hwnd, bool clickThrough) {
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (clickThrough)
        exStyle |= WS_EX_TRANSPARENT;
    else
        exStyle &= ~WS_EX_TRANSPARENT;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow(
        "KeyboardWarriorLoLMod",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        600, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS
    );

    SDL_SysWMinfo wmInfo{};
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_LAYERED;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    int display_w, display_h;
    SDL_GL_GetDrawableSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    Messages& messages = *(new Messages());
    bool success = messages.load();
    if (!success) return 0;

    HotkeyManager hotkeyManager(messages);
    success = hotkeyManager.load();
    if (!success) return 0;

    bool running          = true;
    bool isIdle           = true;
    bool clickThrough     = true; // tracks current click-through state during game
    ChatSender chatSender = ChatSender();
    LoLEventHandler lolEventHandler = LoLEventHandler(messages, hotkeyManager, chatSender);
    LoLReader lolReader = LoLReader(lolEventHandler);

    InputReader::start();
    lolReader.initializeLoop();

    auto onIdle = [&]() {
        isIdle = true;
        SDL_SetWindowSize(window, 600, 600);
        setClickThrough(hwnd, false);
        SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
        InputReader::clearHotkeys();
    };

    auto onGameStart = [&]() {
        isIdle = false;
        clickThrough = true; // start in click-through mode
        SDL_SetWindowSize(window, 400, 200);
        SetLayeredWindowAttributes(hwnd, 0, 175, LWA_ALPHA);
        setClickThrough(hwnd, true);

        InputReader::clearHotkeys();

        // Default message hotkeys
        for (int i = 0; i < (int)hotkeyManager.defaultHotkeys.size(); i++) {
            Hotkey hotkey = hotkeyManager.defaultHotkeys[i];
            InputReader::onHotkey(hotkey, [&lolEventHandler, i]() {
                std::cout << "hotkey pressed for default message " << std::endl;
                lolEventHandler.processHotkeyPressed(i, false);
            });
        }

        // Event hotkeys
        for (int i = 0; i < (int)hotkeyManager.eventHotkeys.size(); i++) {
            Hotkey hotkey = hotkeyManager.eventHotkeys[i];
            InputReader::onHotkey(hotkey, [&lolEventHandler, i]() {
                lolEventHandler.processHotkeyPressed(i, true);
            });
        }

        InputReader::onHotkey(hotkeyManager.skipEventHotkey, [&]() {
            lolEventHandler.closeCurrentEvent();
        });

        // Toggle interactable
        InputReader::onHotkey(hotkeyManager.toggleInGameInteractableHotkey, [&]() {
            if (!clickThrough) {
                return; 
            }
            clickThrough = false;
            setClickThrough(hwnd, false);
        });
        InputReader::onHotkeyRelease(hotkeyManager.toggleInGameInteractableHotkey, [&]() {
            if (clickThrough) {
                return; 
            }
            clickThrough = true;
            setClickThrough(hwnd, true);
        });
    };

    SDL_Event event;
    while (running) {
        lolReader.process();

        if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
                running = false;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.8f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (lolReader.isIdle) {
            if (!isIdle)
                onIdle();
            CoreUI::mainUIFrame(window, hwnd, messages, hotkeyManager);
            
        } else {
            if (isIdle)
                onGameStart(); 
            CoreUI::eventsOverlayUIFrame(window, hwnd, lolEventHandler, messages, hotkeyManager, !clickThrough);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    lolReader.closeLoop();
    return 0;
}