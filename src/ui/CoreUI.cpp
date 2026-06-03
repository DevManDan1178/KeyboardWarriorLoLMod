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

const ImVec4 DEFAULT_TITLE_BAR_DRAG_COLOR = ImVec4(0.2f, 0.2f, 0.2f, 0.5f);
const ImVec4 DRAGGABLE_TITLE_BAR_DRAG_COLOR = ImVec4(0.4f, 0.4f, 0.4f, 0.75f);
const ImVec4 DEFAULT_TITLE_BAR_HOVERED_COLOR = ImVec4(0.5f, 0.5f, 0.5f, 0.85f);

namespace CoreUI {

    int getEventOverlayUIFrameHeight(size_t defaultMessageListCount, size_t eventMessageListCount) {
        return 115
        + (defaultMessageListCount > 0 ? 20 : 0) 
        + 20 * (int) (defaultMessageListCount + eventMessageListCount);
    }

    static void drawTitleBar(SDL_Window* window, HWND hwnd, bool draggable, bool minimizeAndCloseEnabled) {
    
        ImGui::PushStyleColor(ImGuiCol_Button, draggable ? DRAGGABLE_TITLE_BAR_DRAG_COLOR :  DEFAULT_TITLE_BAR_DRAG_COLOR);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, draggable ? DEFAULT_TITLE_BAR_HOVERED_COLOR : DEFAULT_TITLE_BAR_DRAG_COLOR);

        ImGui::Button("KeyboardWarriorLoL", ImVec2(ImGui::GetWindowWidth() - (minimizeAndCloseEnabled ? 76 : 20), 20));
        if (draggable && ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
            ReleaseCapture();
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }

        if (!minimizeAndCloseEnabled) {
            ImGui::PopStyleColor(2);
            ImGui::Separator();
            return;
        }

        ImGui::SameLine(0, 4);
        if (ImGui::Button("-", ImVec2(26, 20))) {
            SDL_MinimizeWindow(window);
        }

        ImGui::SameLine(0, 4);
        if (ImGui::Button("X", ImVec2(26, 20))) {
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
            ImGuiWindowFlags_NoMove     
        );

        //drawTitleBar(window, hwnd, true, true); not borderless anymore, so already has the default one

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

    void eventsOverlayUIFrame(SDL_Window* window, HWND hwnd, LoLEventHandler& lolEventHandler, Messages& messages, HotkeyManager& hotkeyManager, bool interactable, bool& alwaysVisible) {
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


        drawTitleBar(window, hwnd, interactable, false);

        ImGui::Text(std::format("{} - Toggle: [{}]", interactable ? "Draggable" : "Anchored", hotkeyManager.hotkeyToString(hotkeyManager.toggleInGameInteractableHotkey)).c_str());
        ImGui::Text(std::format("{} - Toggle: [{}]", alwaysVisible ? "Always Visible" : "Visible On Event", hotkeyManager.hotkeyToString(hotkeyManager.toggleInGameAlwaysVisibleHotkey)).c_str());
        
        //Defaults
        
        std::vector<Message> defaultMessages = messages.defaultMessages;
        if (defaultMessages.size() > 0) {
            std::vector<Hotkey> defaultHotkeys = hotkeyManager.defaultHotkeys;
            ImGui::Dummy(ImVec2(0, 4));
            ImGui::Text("Default Messages");
            for (int i = 0; i < defaultMessages.size(); i++) {
                std::string messageTitle = defaultMessages[i].messageTitle;
                std::string hotkeyStr = defaultHotkeys.size() > i ? hotkeyManager.hotkeyToString(defaultHotkeys[i]) : "undefined";
                ImGui::Text(std::format("Message {}: \"{}\" - [{}]", i + 1, messageTitle, hotkeyStr).c_str());
            }
        }

        //Events
        ImGui::Dummy(ImVec2(0, 4));
        std::vector<Hotkey> eventHotkeys = hotkeyManager.eventHotkeys;
        if (!(eventCategory.empty() || eventName.empty()) && messages.eventMessages[eventCategory][eventName].size() > 0) {
            ImGui::Text(std::format("Event: \"{}\" - Skip: [{}]", eventName, hotkeyManager.hotkeyToString(hotkeyManager.skipEventHotkey)).c_str());
            std::vector<Message> eventMessages = messages.eventMessages[eventCategory][eventName];
            for (int i = 0; i < eventMessages.size(); i++) {
                std::string messageTitle = eventMessages[i].messageTitle;
                std::string hotkeyStr = eventHotkeys.size() > i ? hotkeyManager.hotkeyToString(eventHotkeys[i]) : "undefined";
                ImGui::Text(std::format("Message {}: \"{}\" - [{}]", i + 1, messageTitle, hotkeyStr).c_str());
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