#include "ui/MessagesUI.h"
#include "Messages.h"
#include "HotkeyManager.h"
#include <vector>
#include <string>
#include <format>
#include <map>
#include <iostream>
#include <unordered_map>
#include <tuple>
#include "imgui.h"

const float SECTION_INDENT = 5.0f;
const float CATEGORY_SELECTION_INDENT = 5.0f;
const float CATEGORY_INDENT = 10.0f;
const float EVENT_INDENT = 10.0f;
const float MESSAGE_TEXT_DISPLAY_MAX_WIDTH = 450.0f;
const ImVec4 TEXT_COLOR = ImVec4(.95f, .95f, .95f, 1.0f);
const ImVec4 FRAME_BG_COLOR = ImVec4(0.1f, 0.25f, 0.35f, 1.0f);

const std::string DEFAULT_NEW_MESSAGE_CONTENT = "Good Game";
const std::string DEFAULT_NEW_MESSAGE_TITLE = "GG";

namespace MessagesUI {

    int categoryToggleStates = 0;
    bool defaultsToggled = false;
    std::map<int, int> eventToggleStatesMap;
    
    std::string getHotkeyStr(HotkeyManager &hotkeyManager, int index, bool isEvent) {
        std::vector<Hotkey> hotkeyList = isEvent? hotkeyManager.eventHotkeys : hotkeyManager.defaultHotkeys;
        if (index >= hotkeyList.size()) {
            return "undefined";
        }
        return hotkeyManager.hotkeyToString(hotkeyList[index]);
    }

    void messagesMenu(Messages &messages, HotkeyManager &hotkeyManager) {
        std::vector<Message> &defaultMessages = messages.defaultMessages;
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Message>>> &eventMessages = messages.eventMessages;

        int categoryIdx = -1;
       

        static std::tuple<int, int> editedObject(-1, -1);
        static std::map<int, std::vector<MessageBuffer>> messageBuffersMap;

        ImGui::Text("-  [Message Hotkeys]  -");
        ImGui::Indent(SECTION_INDENT);
        std::vector<Message> defaultMessagesList = messages.defaultMessages;
        
        //Default Messages
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::Text("Default Messages");
        ImGui::Indent(CATEGORY_SELECTION_INDENT);
        ImGui::Dummy(ImVec2(0, 2));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.5f, .5f, defaultsToggled ? 1.0f : .5f, 1.0f));
            
        if (ImGui::Button(std::format("{} ##categoryToggleDefaults", defaultsToggled ? "Hide" : "Show").c_str())) {
            defaultsToggled = !defaultsToggled;
        }
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::PopStyleColor();
        if (defaultsToggled) {
            std::vector<MessageBuffer> defaultMessageBuffers = messageBuffersMap[-1]; 

            if (defaultMessageBuffers.size() != defaultMessagesList.size()) {
                defaultMessageBuffers.resize(defaultMessagesList.size());

                for (size_t i = 0; i < defaultMessagesList.size(); i++) {
                    MessageBuffer& messageBuffer = defaultMessageBuffers[i];
                    strncpy_s(messageBuffer.title, defaultMessagesList[i].messageTitle.c_str(), sizeof(messageBuffer.title));
                    strncpy_s(messageBuffer.content, defaultMessagesList[i].messageContent.c_str(), sizeof(messageBuffer.content));
                }
            }
            ImGui::Indent(CATEGORY_INDENT);
            for (int messageIdx = 0; messageIdx < defaultMessagesList.size(); messageIdx++) {
                
                ImGui::Dummy(ImVec2(0, 2));
                ImGui::BeginGroup();
                MessageBuffer& messageBuffer = defaultMessageBuffers[messageIdx];
                
                ImGui::Text(std::format("Default Hotkey Number {} - [{}]", messageIdx + 1, getHotkeyStr(hotkeyManager, messageIdx, false)).c_str());

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Title: ");
                ImGui::SameLine();
                
                
                if (ImGui::InputText(std::format("##{}{}", messageIdx, "Defaults").c_str(), messageBuffer.title, IM_ARRAYSIZE(messageBuffer.title), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::IsItemDeactivated()) {
                    messages.setDefaultMessageTitle(messageIdx, messageBuffer.title);
                }

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Message: ");
                ImGui::SameLine();
                if (std::get<0>(editedObject) == categoryIdx && std::get<1>(editedObject) == messageIdx)
                {
                    ImGui::SetKeyboardFocusHere();
                    
                    if (ImGui::InputText(std::format("##msg_{}{}", "Defaults", messageIdx).c_str(), messageBuffer.content, IM_ARRAYSIZE(messageBuffer.content), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::IsItemDeactivated()) {
                        messages.setDefaultMessageContent(messageIdx, messageBuffer.content);
                        std::get<1>(editedObject) = -1;
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_Escape)){
                        std::get<1>(editedObject) = -1;
                    }
                }
                else {
                    ImVec2 padding(6.0f, 4.0f);

                    float availWidth = ImGui::GetContentRegionAvail().x;
                    float boxWidth = std::min(availWidth, MESSAGE_TEXT_DISPLAY_MAX_WIDTH);

                    float wrapWidth = boxWidth - padding.x * 2.0f;

                    // Measure wrapped text height properly ----
                    ImGui::PushTextWrapPos(ImGui::GetCursorScreenPos().x + wrapWidth);

                    float textHeight = ImGui::CalcTextSize(
                        messageBuffer.content,
                        nullptr,
                        true,
                        wrapWidth
                    ).y;

                    ImGui::PopTextWrapPos();

                    ImVec2 boxSize(
                        boxWidth,
                        textHeight + padding.y * 2.0f
                    );

                    ImVec2 pos = ImGui::GetCursorScreenPos();
                    ImDrawList* draw = ImGui::GetWindowDrawList();

                    // Background
                    draw->AddRectFilled(
                        pos,
                        ImVec2(pos.x + boxSize.x, pos.y + boxSize.y),
                        ImGui::GetColorU32(FRAME_BG_COLOR),
                        4.0f
                    );

                    // Reserve space in layout
                    ImGui::Dummy(boxSize);

                    // Format
                    ImGui::SetCursorScreenPos(ImVec2(pos.x + padding.x, pos.y + padding.y));
                    ImGui::PushTextWrapPos(pos.x + boxWidth - padding.x);
                    ImGui::TextUnformatted(messageBuffer.content);
                    ImGui::PopTextWrapPos();

                    //Click detection 
                    ImVec2 min = pos;
                    ImVec2 max = ImVec2(pos.x + boxSize.x, pos.y + boxSize.y);

                    if (ImGui::IsMouseHoveringRect(min, max) &&
                        ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    {
                        std::get<0>(editedObject) = categoryIdx;
                        std::get<1>(editedObject) = messageIdx;
                    }
                }
                                
                ImGui::Dummy(ImVec2(0, 2));
                if (ImGui::Button(std::format("Delete Message {}##{}{}", messageIdx + 1, "Defaults", messageIdx).c_str())) {
                    messages.deleteDefaultMessage(messageIdx);
                }

                ImGui::EndGroup();
                ImGui::Dummy(ImVec2(0, 2));


            }
             
            if (ImGui::Button("Create New Message##Defaults")) {
                messages.createNewDefaultMessage(Message(DEFAULT_NEW_MESSAGE_CONTENT, DEFAULT_NEW_MESSAGE_TITLE));
            }
            ImGui::Unindent(CATEGORY_INDENT);
        }

        ImGui::Unindent(CATEGORY_SELECTION_INDENT);
        //EVENT MESSAGES

        ImGui::Dummy(ImVec2(0, 4));
        ImGui::Text("Event Messages");
        ImGui::Dummy(ImVec2(0, 2));
        ImGui::Indent(CATEGORY_SELECTION_INDENT);

        for (const auto& pairCategoryEventMessages : eventMessages) {  
            categoryIdx++; 
            std::string category = pairCategoryEventMessages.first;
            
            bool categoryVisibilityToggled = categoryToggleStates & (1 << categoryIdx);
                    
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.5f, .5f, categoryVisibilityToggled ? 1.0f : .5f, 1.0f));
            
            if (ImGui::Button(std::format("{}##categoryToggle{}", category.c_str(), category).c_str())) {
                categoryToggleStates^= (1 << categoryIdx);
            }

            ImGui::PopStyleColor();
            

            if (!categoryVisibilityToggled) {
                continue;
            }

            
            
            ImGui::Indent(CATEGORY_INDENT);
            int& eventToggleStates = eventToggleStatesMap[categoryIdx];
            
            std::vector<std::string> eventKeyOrder = messages.eventKeyOrders.at(category);

            for (int eventIdx = 0; eventIdx < eventKeyOrder.size(); eventIdx++) {         
                ImGui::Dummy(ImVec2(0, 4));
                std::string event = eventKeyOrder[eventIdx];
                bool eventVisibilityToggled = eventToggleStates & (1 << eventIdx);
  
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.5f, .5f, eventVisibilityToggled ? 1.0f : .5f, 1.0f));
                if (ImGui::Button(std::format("{}##eventToggle{}{}", event.c_str(), category, eventIdx).c_str())) {
                    eventToggleStates ^= (1 << eventIdx);
                }
                ImGui::PopStyleColor();
                
                if (!eventVisibilityToggled) {
                    continue;
                }

                ImGui::Indent(EVENT_INDENT);
                
                ImGui::Dummy(ImVec2(0, 4));
                std::vector<Message> messageList = eventMessages[category][event];

                std::vector<MessageBuffer> eventMessageBuffers = messageBuffersMap[eventIdx]; 

                if (eventMessageBuffers.size() != messageList.size()) {
                    eventMessageBuffers.resize(messageList.size());

                    for (size_t i = 0; i < messageList.size(); i++) {
                        MessageBuffer& messageBuffer = eventMessageBuffers[i];
                        strncpy_s(messageBuffer.title, messageList[i].messageTitle.c_str(), sizeof(messageBuffer.title));
                        strncpy_s(messageBuffer.content, messageList[i].messageContent.c_str(), sizeof(messageBuffer.content));
                    }
                }

                ImGui::BeginGroup();
                ImGui::PushStyleColor(ImGuiCol_Text, TEXT_COLOR);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, FRAME_BG_COLOR); 
                

                for (int i = 0; i < messageList.size(); i++) {
                    ImGui::Dummy(ImVec2(0, 2));
                    ImGui::BeginGroup();
                    MessageBuffer& messageBuffer = eventMessageBuffers[i];

                    ImGui::Text(std::format("Event Hotkey Number {} - [{}]", i + 1, getHotkeyStr(hotkeyManager, i, true)).c_str());

                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Title: ");
                    ImGui::SameLine();
                    
                    
                    if (ImGui::InputText(std::format("##{}{}/{}", i + 1, category, eventIdx, i).c_str(), messageBuffer.title, IM_ARRAYSIZE(messageBuffer.title), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::IsItemDeactivated()) {
                        messages.setEventMessageTitle(category, event, i, messageBuffer.title);
                    }

                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Message: ");
                    ImGui::SameLine();
                    if (std::get<0>(editedObject) == categoryIdx && std::get<1>(editedObject) == eventIdx)
                    {
                        ImGui::SetKeyboardFocusHere();
                        
                        if (ImGui::InputText( std::format("##msg_{}{}/{}", category, eventIdx, i).c_str(), messageBuffer.content, IM_ARRAYSIZE(messageBuffer.content), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::IsItemDeactivated()) {
                            messages.setEventMessageContent(category, event, i, messageBuffer.content);
                            std::get<1>(editedObject) = -1;
                        }

                        if (ImGui::IsKeyPressed(ImGuiKey_Escape)){
                            std::get<1>(editedObject) = -1;
                        }
                    }
                    else {
                        ImVec2 padding(6.0f, 4.0f);

                        float availWidth = ImGui::GetContentRegionAvail().x;
                        float boxWidth = std::min(availWidth, MESSAGE_TEXT_DISPLAY_MAX_WIDTH);

                        float wrapWidth = boxWidth - padding.x * 2.0f;

                        // Measure wrapped text height properly ----
                        ImGui::PushTextWrapPos(ImGui::GetCursorScreenPos().x + wrapWidth);

                        float textHeight = ImGui::CalcTextSize(
                            messageBuffer.content,
                            nullptr,
                            true,
                            wrapWidth
                        ).y;

                        ImGui::PopTextWrapPos();

                        ImVec2 boxSize(
                            boxWidth,
                            textHeight + padding.y * 2.0f
                        );

                        ImVec2 pos = ImGui::GetCursorScreenPos();
                        ImDrawList* draw = ImGui::GetWindowDrawList();

                        // Background
                        draw->AddRectFilled(
                            pos,
                            ImVec2(pos.x + boxSize.x, pos.y + boxSize.y),
                            ImGui::GetColorU32(FRAME_BG_COLOR),
                            4.0f
                        );

                        // Reserve space in layout
                        ImGui::Dummy(boxSize);

                        // Format
                        ImGui::SetCursorScreenPos(ImVec2(pos.x + padding.x, pos.y + padding.y));
                        ImGui::PushTextWrapPos(pos.x + boxWidth - padding.x);
                        ImGui::TextUnformatted(messageBuffer.content);
                        ImGui::PopTextWrapPos();

                        //Click detection 
                        ImVec2 min = pos;
                        ImVec2 max = ImVec2(pos.x + boxSize.x, pos.y + boxSize.y);

                        if (ImGui::IsMouseHoveringRect(min, max) &&
                            ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                        {
                            std::get<0>(editedObject) = categoryIdx;
                            std::get<1>(editedObject) = eventIdx;
                        }
                    }
                                    
                    ImGui::Dummy(ImVec2(0, 2));
                    if (ImGui::Button(std::format("Delete Message {}##{}{}/{}", i + 1, category, eventIdx, i).c_str())) {
                        messages.deleteEventMessage(category, event, i);
                    }

                    ImGui::EndGroup();
                    ImGui::Dummy(ImVec2(0, 2));
                }
                
                
                if (ImGui::Button(std::format("Create New Message##{}{}", category, eventIdx).c_str())) {
                    messages.createNewEventMessage(category, event, Message("I am rank 1 Yuumi player", "True Message"));
                }

                ImGui::PopStyleColor(2);
                ImGui::EndGroup();
                ImGui::Unindent(EVENT_INDENT);
                ImGui::Dummy(ImVec2(0, 4));
            }
            ImGui::Unindent(CATEGORY_INDENT);       
            ImGui::Dummy(ImVec2(0, 4));
        }
        ImGui::Unindent(SECTION_INDENT);
        ImGui::Unindent(CATEGORY_SELECTION_INDENT);
    }
}