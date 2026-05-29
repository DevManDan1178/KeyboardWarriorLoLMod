#include "Messages.h"
#include "HotkeyManager.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>
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
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
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
    std::cout << "loading hotkeys" << std::endl;
    success = hotkeyManager.load();
    if (!success) {
        return 0;
    }

    bool running = true;
    bool windowHidden = false;
    
    std::map<std::string, std::vector<Message>> &messagesMap = messages.messages;
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
        ImGui::Begin("Panel");
        {
            int idx = -1;
            for (const auto& pair : messagesMap) {
                idx++;
                std::string category = pair.first;
                bool toggled = categoryToggleStates & (1 << idx);

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.5f, .5f, toggled ? 1.0f : .5f, 1.0f));
                if (ImGui::Button(category.c_str())) {
                    categoryToggleStates ^= (1 << idx);
                }
                ImGui::PopStyleColor();
                
                if (!toggled) {
                    continue;
                }
                std::vector<Message> messageList = pair.second;
                ImGui::BeginGroup();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.95f, .95f, .95f, 1.0f));
                
                static std::vector<MessageBuffer> messageBuffers; 

                if (messageBuffers.size() != messageList.size()) {
                    messageBuffers.resize(messageList.size());


                    for (size_t i = 0; i < messageList.size(); i++) {
                        MessageBuffer& messageBuffer = messageBuffers[i];
                        strncpy_s(messageBuffer.title, messageList[i].messageTitle.c_str(), sizeof(messageBuffer.title));
                        strncpy_s(messageBuffer.content, messageList[i].messageContent.c_str(), sizeof(messageBuffer.content));
                    }
                }

                for (int i = 0; i < messageList.size(); i++) {
                    ImGui::BeginGroup();
                    MessageBuffer& messageBuffer = messageBuffers[i];
                    if (ImGui::InputText(std::format("[Title (Message {})]##{}/{}", i + 1, idx, i).c_str(), messageBuffer.title, IM_ARRAYSIZE(messageBuffer.title), ImGuiInputTextFlags_EnterReturnsTrue)) {
                        messages.setMessageTitle(category, i, messageBuffer.title);
                    }

                    if (ImGui::InputText(std::format("[Message]##{}/{}", idx, i).c_str(), messageBuffer.content,IM_ARRAYSIZE(messageBuffer.content), ImGuiInputTextFlags_EnterReturnsTrue)){
                        messages.setMessageContent(category, i, messageBuffer.content);
                    }

                    if (ImGui::Button(std::format("Delete Message {}##{}/{}", i + 1, idx, i).c_str())) {
                        messages.deleteMessage(category, i);
                    }

                    ImGui::EndGroup();
                }
                if (ImGui::Button(std::format("Create New Message##{}", idx).c_str())) {
                    messages.createNewMessage(category, Message("I am rank 1 Yuumi player", "True Message"));
                }
                ImGui::PopStyleColor();
                ImGui::EndGroup();
            }
        }
        

        //Input detection (makes app uncloseable)
        //Keybind kb = hotkeyManager.getKeybind();
        //ImGui::Text("Detected key code: %d", kb.keyCode);


        //UI END
        ImGui::End();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, 800, 600);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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