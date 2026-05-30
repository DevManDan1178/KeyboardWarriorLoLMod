#include "Messages.h"
#include "HotkeyManager.h"

#include "ui/MessagesUI.h"

#include <windows.h>
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


int main() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Setup OpenGL context attributes (required for ImGui rendering)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create SDL window with OpenGL
    SDL_Window* window = SDL_CreateWindow(
        "KeyboardWarriorLoLMod",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_OPENGL
    );

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    //Logic
    Messages& messages = *(new Messages());
    bool success = messages.load();
    if (!success) {
        return 0;
    }
    HotkeyManager hotkeyManager(messages);
    
    success = hotkeyManager.load();
    if (!success) {
        return 0;
    }

    bool running = true;
    bool windowHidden = false;
    
    
    // While the application is running
    int categoryToggleStates = 0; //Bitmask for the toggle of every category

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT)
                running = false;
        }
        if (windowHidden) continue;

        // New frame setup
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        // ImGui UI
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        ImGui::Begin(
            "Overlay",
            nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_AlwaysVerticalScrollbar
        );
        MessagesUI::MessagesMenu(messages);

        //Input detection (makes app uncloseable)
        //Keybind kb = hotkeyManager.getKeybind();
        //ImGui::Text("Detected key code: %d", kb.keyCode);


        //UI END
        ImGui::End();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, 800, 600);

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    std::cout << "Window size: " << w << "x" << h << std::endl;
    // Cleanup / Shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}