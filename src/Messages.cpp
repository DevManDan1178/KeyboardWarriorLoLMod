#include "../external/json.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Messages.h"
#include <unordered_map>

using json = nlohmann::json;

const std::filesystem::path path = std::filesystem::current_path() / "config/messages.json";

bool Messages::load() {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open file at path " << path << std::endl;
        return false;
    }
    
    json data;
    file >> data;
    //TODO: Read from the file to assign corresponding values to messages and hotkeys vectors
    std::cout << "Successfully retrieved messages\n" << data.dump(4) << std::endl;
    
    for (auto& [category, messages] : data.items()) {
        
        if (!messages.is_array()) {
            std::cout << "Skipping non-array category in messages.json: " << category << std::endl;
            continue;
        }

        std::vector<Message> messageList;
        for (auto& messageInfo : messages) {
            Message msg;
            msg.messageTitle = messageInfo.value("Title", "");
            msg.messageStr = messageInfo.value("Message", "");
            messageList.push_back(msg);
            //std::cout << "Message with title: " << msg.messageTitle << " and str: " << msg.messageStr << std::endl;
        }
        Messages::messages[category] = messageList;
    }
    return true;
}


std::string Messages::getMessage(const std::string& key) const {
    //TODO Get message from the hotkey
    return "";
}


