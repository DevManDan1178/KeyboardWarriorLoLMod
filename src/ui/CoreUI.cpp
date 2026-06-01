#include "MessagesUI.h"
#include "HotkeysUI.h"

#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>

#include "Messages.h"
#include "HotkeyManager.h"
#include "LoLEventHandler.h"

namespace CoreUI {

    static void blankTransparentFrame(SDL_Window* window) {
        //Render blank transparent frame — nothing visible
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::Render();
        int w, h;
        SDL_GL_GetDrawableSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        return;
    }

    static void drawTitleBar(SDL_Window* window, HWND hwnd, bool enableMinimize = true, bool enableClose = true) {
    
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));

        ImGui::Button("KeyboardWarriorLoL", ImVec2(ImGui::GetWindowWidth() - 76, 20));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
            ReleaseCapture();
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }

        ImGui::SameLine(0, 4);
        if (enableMinimize && ImGui::Button("-", ImVec2(26, 20))) {
            SDL_MinimizeWindow(window);
        }

        ImGui::SameLine(0, 4);
        if (enableClose && ImGui::Button("X", ImVec2(26, 20))) {
            SDL_Event quitEvent;
            quitEvent.type = SDL_QUIT;
            SDL_PushEvent(&quitEvent);
        }

        ImGui::PopStyleColor(2);
        ImGui::Separator();
    }

    void mainUIFrame(SDL_Window* window, HWND hwnd, Messages& messages, HotkeyManager& hotkeyManager) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        ImGui::Begin(
            "Configurations",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize   |
            ImGuiWindowFlags_NoMove     |
            ImGuiWindowFlags_AlwaysVerticalScrollbar
        );

        drawTitleBar(window, hwnd);

        MessagesUI::messagesMenu(messages, hotkeyManager);
        ImGui::Dummy(ImVec2(0, 12));
        HotkeysUI::hotkeysUI(hotkeyManager);

        ImGui::End();

        // Rendering
        ImGui::Render();
        int w, h;
        SDL_GL_GetDrawableSize(window, &w, &h);
        glViewport(0, 0, w, h);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    void eventsOverlayUIFrame(SDL_Window* window, HWND hwnd, LoLEventHandler& lolEventHandler, Messages& messages, HotkeyManager& hotkeyManager, bool interactable) {
        auto [eventCategory, eventName] = lolEventHandler.getCurrentEvent();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();

        ImGui::NewFrame();
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        ImGui::Begin(
            "Overlay",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize   |
            ImGuiWindowFlags_NoMove     |
            ImGuiWindowFlags_NoScrollbar
        );

        if (interactable) {
            drawTitleBar(window, hwnd, false, false);
        }
        ImGui::Text(std::format("Toggle Interactable - Hotkey: {}", hotkeyManager.hotkeyToString(hotkeyManager.toggleInGameInteractableHotkey)).c_str());
        //Defaults
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::Text("Default Messages");
        std::vector<Message> defaultMessages = messages.defaultMessages;
        std::vector<Hotkey> defaultHotkeys = hotkeyManager.defaultHotkeys;
        for (int i = 0; i < defaultMessages.size(); i++) {
            std::string messageTitle = defaultMessages[i].messageTitle;
            std::string hotkeyStr = defaultHotkeys.size() > i ? hotkeyManager.hotkeyToString(defaultHotkeys[i]) : "undefined";
            ImGui::Text(std::format("Message {}: \"{}\" - Hotkey: {}", i + 1, messageTitle, hotkeyStr).c_str());
        }

        //Events
        ImGui::Dummy(ImVec2(0, 4));
        std::vector<Hotkey> eventHotkeys = hotkeyManager.eventHotkeys;
        if (!(eventCategory.empty() || eventName.empty())) {
            ImGui::Text(std::format("Event - [{}]", eventName).c_str());

            std::vector<Message> eventMessages = messages.eventMessages[eventCategory][eventName];
            for (int i = 0; i < eventMessages.size(); i++) {
                std::string messageTitle = eventMessages[i].messageTitle;
                std::string hotkeyStr = eventHotkeys.size() > i ? hotkeyManager.hotkeyToString(eventHotkeys[i]) : "undefined";
                ImGui::Text(std::format("Message {}: \"{}\" - Hotkey: {}", i + 1, messageTitle, hotkeyStr).c_str());
            }
        } else {
            ImGui::Text("[No current event]");
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        int w, h;
        SDL_GL_GetDrawableSize(window, &w, &h);
        glViewport(0, 0, w, h);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
}