#include "ui/MessagesUI.h"
#include "Messages.h"
#include <vector>
#include <string>
#include <format>
#include <map>
#include "imgui.h"

const float MESSAGE_TEXT_DISPLAY_MAX_WIDTH = 450.0f;
const ImVec4 TEXT_COLOR = ImVec4(.95f, .95f, .95f, 1.0f);
const ImVec4 FRAME_BG_COLOR = ImVec4(0.1f, 0.25f, 0.35f, 1.0f);

namespace MessagesUI {
    int categoryToggleStates = 0;

    void MessagesMenu(Messages &messages) {
        std::map<std::string, std::vector<Message>> &messagesMap = messages.messages;
        int idx = -1; //start from -1 and increments at start of every element [== starting from 0]
        
        for (const auto& pair : messagesMap) {
            idx++;
            std::string category = pair.first;
            bool toggled = MessagesUI::categoryToggleStates & (1 << idx);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.5f, .5f, toggled ? 1.0f : .5f, 1.0f));
            if (ImGui::Button(category.c_str())) {
                categoryToggleStates ^= (1 << idx);
            }
            ImGui::PopStyleColor();
            
            if (!toggled) {
                continue;
            }
            std::vector<Message> messageList = pair.second;

            static std::vector<MessageBuffer> messageBuffers; 

            if (messageBuffers.size() != messageList.size()) {
                messageBuffers.resize(messageList.size());


                for (size_t i = 0; i < messageList.size(); i++) {
                    MessageBuffer& messageBuffer = messageBuffers[i];
                    strncpy_s(messageBuffer.title, messageList[i].messageTitle.c_str(), sizeof(messageBuffer.title));
                    strncpy_s(messageBuffer.content, messageList[i].messageContent.c_str(), sizeof(messageBuffer.content));
                }
            }

            static std::vector<bool> editingMessage;
            if (editingMessage.size() != messageList.size())
            {
                editingMessage.assign(messageList.size(), false);
            }
           
            ImGui::BeginGroup();
            ImGui::PushStyleColor(ImGuiCol_Text, TEXT_COLOR);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, FRAME_BG_COLOR); 
            

            for (int i = 0; i < messageList.size(); i++) {
                ImGui::Dummy(ImVec2(0, 2));
                ImGui::BeginGroup();
                MessageBuffer& messageBuffer = messageBuffers[i];
                ImGui::Text(std::format("Hotkey Number [{}]", i + 1).c_str());

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Title: ");
                ImGui::SameLine();
                
                
                if (ImGui::InputText(std::format("##{}/{}", i + 1, idx, i).c_str(), messageBuffer.title, IM_ARRAYSIZE(messageBuffer.title), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::IsItemDeactivated()) {
                    messages.setMessageTitle(category, i, messageBuffer.title);
                }

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Message: ");
                ImGui::SameLine();
                if (editingMessage[i])
                {
                    ImGui::SetKeyboardFocusHere();
                    
                    if (ImGui::InputText( std::format("##msg_{}/{}", idx, i).c_str(), messageBuffer.content, IM_ARRAYSIZE(messageBuffer.content), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::IsItemDeactivated()) {
                        messages.setMessageContent(category, i, messageBuffer.content);
                        editingMessage[i] = false;
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_Escape)){
                        editingMessage[i] = false;
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
                        editingMessage[i] = true;
                    }
                }
                                
                ImGui::Dummy(ImVec2(0, 4));
                if (ImGui::Button(std::format("Delete Message {}##{}/{}", i + 1, idx, i).c_str())) {
                    messages.deleteMessage(category, i);
                }

                ImGui::EndGroup();
                ImGui::Dummy(ImVec2(0, 8));
            }
            ImGui::Dummy(ImVec2(0, 8));
            
            if (ImGui::Button(std::format("Create New Message##{}", idx).c_str())) {
                messages.createNewMessage(category, Message("I am rank 1 Yuumi player", "True Message"));
            }

            ImGui::PopStyleColor(2);
            ImGui::EndGroup();
        }
        
    }
}