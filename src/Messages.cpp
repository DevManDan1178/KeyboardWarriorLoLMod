#include "../external/json.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Messages.h"

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
    //std::cout << "Successfully retrieved messages\n" << data.dump(4) << std::endl;
    
    for (auto& [category, messages] : data.items()) {
        
        if (!messages.is_array()) {
            std::cout << "Skipping non-array category in messages.json: " << category << std::endl;
            continue;
        }

        std::vector<Message> messageList;
        for (auto& messageInfo : messages) {
            messageList.push_back(Message(messageInfo.value("Message", ""), messageInfo.value("Title", "")));
            //std::cout << "Message with title: " << msg.messageTitle << " and str: " << msg.messageContent << std::endl;
        }
        Messages::messages[category] = messageList;
    }
    return true;
}

bool Messages::writeToJSON() {
    json data;
    for (const auto& [category, messageList] : messages) {
        json array = json::array();

        for (const auto& msg : messageList) {
            json obj;
            obj["Title"] = msg.messageTitle;
            obj["Message"] = msg.messageContent;

            array.push_back(obj);
        }

        data[category] = array;
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open file for writing: " << path << std::endl;
        return false;
    }

    file << data.dump(4); 
    return true;
}

bool Messages::setMessageContent(std::string category, int index, std::string content) {
    if (!(Messages::messages.contains(category) && index < Messages::messages[category].size())) {
        std::cout << "attempt to set non existing message under category " << category << " and index " << index << std::endl;
        return false;
    }
    std::string prev = Messages::messages[category][index].messageContent;
    if (content == prev) {
        return true;
    }
    Messages::messages[category][index].messageContent = content;
    Messages::attemptWriteToJSON();
    return true;
}

bool Messages::setMessageTitle(std::string category, int index, std::string title) {
    if (!(Messages::messages.contains(category) &&  index < Messages::messages[category].size())) {
        std::cout << "attempt to set non existing message under category " << category << " and index " << index << std::endl;
        return false;
    }
    std::string prev = Messages::messages[category][index].messageTitle;
    if (title == prev) {
        return true;
    }
    Messages::messages[category][index].messageTitle = title;
    Messages::attemptWriteToJSON();
    return true;
}

void Messages::attemptWriteToJSON() {
    bool success = Messages::writeToJSON();
    if (!success) {
        std::cout << "unable to write messages to JSON" << std::endl;
    }
}

bool Messages::createNewMessage(std::string category, Message message) {
    if (!Messages::messages.contains(category)) {
        std::cout << "attempt to create message under non existing category " << category << std::endl;
        return false;
    }
    Messages::messages[category].push_back(message);
    Messages::attemptWriteToJSON();
    return true;
}

bool Messages::deleteMessage(std::string category, int index) {
     if (!(Messages::messages.contains(category) && index < Messages::messages[category].size())) {
        std::cout << "attempt to delete non existing message under category " << category << " and index " << index << std::endl;
        return false;
    }
    Messages::messages[category].erase(Messages::messages[category].begin() + index);
    Messages::attemptWriteToJSON();
    return true;
}