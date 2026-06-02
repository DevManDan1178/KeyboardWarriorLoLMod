
#include "HotkeyManager.h"
#include "imgui.h"
#include <string>
#include <format>
#include <optional>

const float CATEGORY_INDENT = 10.0f;
const ImVec4 TEXT_COLOR = ImVec4(.95f, .95f, .95f, 1.0f);
const ImVec4 FRAME_BG_COLOR = ImVec4(0.1f, 0.25f, 0.35f, 1.0f);


namespace HotkeysUI {
    
    bool defaultsToggled = false;
    bool eventsToggled = false;
    
    void hotkeysUI(HotkeyManager& hotkeyManager) {
        ImGui::Text("-----  [Hotkeys]  -----");
        ImGui::Dummy(ImVec2(0, 4));
        //Defaults
        ImGui::Text("Default Hotkeys");
        ImGui::Dummy(ImVec2(0, 2));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.5f, .5f, defaultsToggled ? 1.0f : .5f, 1.0f));
            
        if (ImGui::Button(std::format("{} ##HotkeyDefaults", defaultsToggled ? "Hide" : "Show").c_str())) {
            defaultsToggled = !defaultsToggled;
        }
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::PopStyleColor();

        if (defaultsToggled) {
            ImGui::Indent(CATEGORY_INDENT);
            std::vector<Hotkey> defaultHotkeys = hotkeyManager.defaultHotkeys;
            static std::vector<std::optional<Hotkey>> failedChanges;
            if (failedChanges.size() != defaultHotkeys.size()) {
                failedChanges.resize(defaultHotkeys.size());
            }

            for (int idx = 0; idx < defaultHotkeys.size(); idx ++) {
                Hotkey hotkey = defaultHotkeys[idx];
                std::string hotkeyStr = hotkeyManager.hotkeyToString(hotkey);
                
                ImGui::Dummy(ImVec2(0, 2));
                ImGui::Text(std::format("Default Hotkey [{}]", idx + 1).c_str());
                ImGui::Dummy(ImVec2(0, 2));
                
                

                std::optional<Hotkey> &failedChange = failedChanges[idx];
                if (ImGui::Button(std::format("{} ##DefaultsHotkey{}", failedChange.has_value() ? hotkeyStr + " [cannot change to " + hotkeyManager.hotkeyToString(failedChange.value()) + "]"  : hotkeyStr, idx + 1).c_str())) {
                    Hotkey queriedHotkey = hotkeyManager.queryHotkey();                  
                    bool success = hotkeyManager.setHotkey(queriedHotkey, false, idx);
                    
                    if (!success) {
                        failedChange = queriedHotkey;
                    } else {
                        failedChange.reset();
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button(std::format("Delete Hotkey {}##{}{}", idx + 1, "Defaults", idx).c_str())) {
                    hotkeyManager.removeHotkey(false, idx);
                    failedChange.reset();
                }
            }

            ImGui::Dummy(ImVec2(0, 4));
            if (ImGui::Button("Create New Hotkey ##Defaults")) {
                hotkeyManager.addHotkey(Hotkey(), false);
            }

            ImGui::Unindent(CATEGORY_INDENT);
        }

        //Events
        ImGui::Text("Event Hotkeys");
        ImGui::Dummy(ImVec2(0, 2));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.5f, .5f, eventsToggled ? 1.0f : .5f, 1.0f));
            
        if (ImGui::Button(std::format("{} ##HotkeyEvents", eventsToggled ? "Hide" : "Show").c_str())) {
            eventsToggled = !eventsToggled;
        }
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::PopStyleColor();

        
        if (eventsToggled) {
            ImGui::Indent(CATEGORY_INDENT);
            std::vector<Hotkey> eventHotkeys = hotkeyManager.eventHotkeys;

            static std::vector<std::optional<Hotkey>> failedChanges;
            if (failedChanges.size() != eventHotkeys.size()) {
                failedChanges.resize(eventHotkeys.size());
            }
            for (int idx = 0; idx < eventHotkeys.size(); idx ++) {
                Hotkey hotkey =eventHotkeys[idx];
                std::string hotkeyStr = hotkeyManager.hotkeyToString(hotkey);
                
                ImGui::Dummy(ImVec2(0, 2));
                ImGui::Text(std::format("Event Hotkey [{}]", idx + 1).c_str());
                ImGui::Dummy(ImVec2(0, 2));
        
                std::optional<Hotkey> &failedChange = failedChanges[idx];
                if (ImGui::Button(std::format("{} ##EventsHotkey{}", failedChange.has_value() ? hotkeyStr + " [cannot not change to " + hotkeyManager.hotkeyToString(failedChange.value()) + "]"  : hotkeyStr, idx + 1).c_str())) {
                    Hotkey queriedHotkey = hotkeyManager.queryHotkey();
                    
                    bool success = hotkeyManager.setHotkey(queriedHotkey, true, idx);
                    
                    if (!success) {
                        failedChange = queriedHotkey;
                    } else {
                        failedChange.reset();
                    }
                }
                
                ImGui::SameLine();

                if (ImGui::Button(std::format("Delete Hotkey {}##{}{}", idx + 1, "Events", idx).c_str())) {
                    hotkeyManager.removeHotkey(true, idx);
                    failedChange.reset();
                }
            }

            ImGui::Dummy(ImVec2(0, 4));
            if (ImGui::Button("Create New Hotkey ##Events")) {
                hotkeyManager.addHotkey(Hotkey(), true);
            }

            //SkipEvent hotkey
            ImGui::Dummy(ImVec2(0, 2));
            ImGui::Text("Skip Event Hotkey");
            ImGui::Dummy(ImVec2(0, 2));
            {
                std::string hotkeyStr = hotkeyManager.hotkeyToString(hotkeyManager.skipEventHotkey);
                static std::optional<Hotkey> failedChange;
                
                if (ImGui::Button(std::format("{} ##SkipEventHotkey", failedChange.has_value() ? hotkeyStr + " [cannot not change to " + hotkeyManager.hotkeyToString(failedChange.value()) + "]"  : hotkeyStr).c_str())) {
                    Hotkey queriedHotkey = hotkeyManager.queryHotkey();
                    
                    bool success = hotkeyManager.setSkipEventHotkey(queriedHotkey);
   
                    if (!success) {
                        failedChange = queriedHotkey;
                    } else {
                        failedChange.reset();
                    }
                }
            }

            ImGui::Unindent(CATEGORY_INDENT);
        }

        ImGui::Text("Toggle UI Interactable (In Game)");
        ImGui::Dummy(ImVec2(0, 2));
        static std::optional<Hotkey> failedChange;
        std::string hotkeyStr = hotkeyManager.hotkeyToString(hotkeyManager.toggleInGameInteractableHotkey);
        if (ImGui::Button(std::format("{} ##ToggleInGameInteractable", failedChange.has_value() ? hotkeyStr + " [cannot not change to " + hotkeyManager.hotkeyToString(failedChange.value()) + "]"  : hotkeyStr.c_str()).c_str())) {
            Hotkey queriedHotkey = hotkeyManager.queryHotkey();
            
            bool success = hotkeyManager.setToggleInGameInteractableHotkey(queriedHotkey);
            
            if (!success) {
                failedChange = queriedHotkey;
            } else {
                failedChange.reset();
            }
        }
        ImGui::Dummy(ImVec2(0, 2));
        ImGui::Text("Shift is treated as a hotkey modifier (and not as an individual hotkey)");
        ImGui::Text("!! Alt and Ctrl are not supported as hotkey modifiers");
    }
}