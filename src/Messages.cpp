#include "../external/json.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Messages.h"

using json = nlohmann::json;

const std::filesystem::path path = std::filesystem::current_path() / "config/messages.json";

static Message parseMessageData(json messageData) {
    return Message(messageData.value("Message", ""), messageData.value("Title", ""));
}

static json messageToJSON(Message message) {
    json msgJSON;
    msgJSON["Content"] = message.messageContent;
    msgJSON["Title"] = message.messageTitle;
    return msgJSON;
}

bool Messages::load() {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open file at path " << path << std::endl;
        return false;
    }
    
    json messagesData;
    file >> messagesData;
    //std::cout << "Successfully retrieved messages\n" << data.dump(4) << std::endl;
    
    json defaultMessages = messagesData["Defaults"];
    for (int i = 0; i < defaultMessages.size(); i++) {
        Messages::defaultMessages.push_back(parseMessageData(defaultMessages[i]));
    }
    

    json eventMessages = messagesData["Events"];
    json eventsKeyOrder = messagesData["EventsKeyOrder"];
    for (auto& [category, categoryMessages] : eventMessages.items()) {
        std::unordered_map<std::string, std::vector<Message>> messagesMap;
        
        for (auto& [event, messages] : categoryMessages.items()) {
            if (!messages.is_array()) {
                continue;
            }

            std::vector<Message> messageList;
            for (auto& messageInfo : messages) {
                messageList.push_back(parseMessageData(messageInfo));
            }
            messagesMap[event] = messageList;
        }
        Messages::eventMessages[category] = messagesMap;
        
        std::vector<std::string> keyOrder; 
        for (int i = 0; i < eventsKeyOrder[category].size(); i++) {
            std::string key = eventsKeyOrder[category][i];
            keyOrder.push_back(key);
        }
        eventKeyOrders[category] = keyOrder;
    }
    return true;
}
bool Messages::writeToJSON()
{
    json messagesData;

    // Defaults
    messagesData["Defaults"] = json::array();
    for (const auto& message : defaultMessages)
    {
        messagesData["Defaults"].push_back(messageToJSON(message));
    }

    // Events
    for (const auto& [category, categoryEventMessages] : eventMessages)
    {
        for (const auto& [eventKey, messages] : categoryEventMessages) {
            std::cout << eventKey << std::endl;
            messagesData["Events"][category][eventKey] = json::array();
            
            std::vector<Message> messages = categoryEventMessages.at(eventKey);
            for (const auto& message : messages)
            {
                messagesData["Events"][category][eventKey].push_back(
                    messageToJSON(message)
                );
            }
        }
    }
    for (const auto& [category, keyOrder] : eventKeyOrders) {
        messagesData["EventsKeyOrder"][category] = json::array();
        for (int i = 0; i < keyOrder.size(); i++) {
            messagesData["EventsKeyOrder"][category].push_back(keyOrder[i]);
        }
    }

    std::ofstream file(path);
    if (!file.is_open())
    {
        std::cout << "Failed to open file for writing at path "
                  << path << std::endl;
        return false;
    }

    file << messagesData.dump(4); // pretty-print with 4-space indentation

    if (file.fail())
    {
        std::cout << "Failed while writing JSON to "
                  << path << std::endl;
        return false;
    }
    return true;
}

bool Messages::checkValidEventMessage(std::string category, std::string event, int index) {
    return eventMessages.contains(category) && eventMessages[category].contains(event) && index < eventMessages[category][event].size();
}

bool Messages::setEventMessageContent(std::string category, std::string event, int index, std::string content) {
    if (!checkValidEventMessage(category, event, index)) {
        std::cout << "attempt to set non existing message under category " << category << " and index " << index << std::endl;
        return false;
    }
    std::string prev = eventMessages[category][event][index].messageContent;
    if (content == prev) {
        return true;
    }
    eventMessages[category][event][index].messageContent = content;
    attemptWriteToJSON();
    return true;
}

bool Messages::setEventMessageTitle(std::string category, std::string event, int index, std::string title) {
    if  (!checkValidEventMessage(category, event, index))  {
        std::cout << "attempt to set non existing message under category " << category << " and index " << index << std::endl;
        return false;
    }
    std::string prev = eventMessages[category][event][index].messageTitle;
    if (title == prev) {
        return true;
    }
    eventMessages[category][event][index].messageTitle = title;
    attemptWriteToJSON();
    return true;
}

void Messages::attemptWriteToJSON() {
    bool success = writeToJSON();
    if (!success) {
        std::cout << "unable to write messages to JSON" << std::endl;
    }
}

bool Messages::createNewEventMessage(std::string category, std::string event, Message message) {
    if (!(eventMessages.contains(category) && eventMessages[category].contains(event))) {
        std::cout << "attempt to create message under non existing category " << category << std::endl;
        return false;
    }
    eventMessages[category][event].push_back(message);
    attemptWriteToJSON();
    return true;
}

bool Messages::deleteEventMessage(std::string category, std::string event, int index) {
     if (!checkValidEventMessage(category, event, index)) {
        std::cout << "attempt to delete non existing message under category " << category << " and index " << index << std::endl;
        return false;
    }
    eventMessages[category][event].erase(eventMessages[category][event].begin() + index);
    attemptWriteToJSON();
    return true;
}

 bool Messages::setDefaultMessageContent(int index, std::string content) {
    if (index >= defaultMessages.size()) {
        std::cout << "Attempt to modify non existing default message at index " << index << std::endl;
        return false;
    }
    defaultMessages[index].messageContent = content;
    attemptWriteToJSON();
    return true;
 }
bool Messages::setDefaultMessageTitle(int index, std::string title) {
    if (index >= defaultMessages.size()) {
        std::cout << "Attempt to modify non existing default message at index " << index << std::endl;
        return false;
    }
    defaultMessages[index].messageTitle = title;
    attemptWriteToJSON();
    return true;
}
bool Messages::createNewDefaultMessage(Message message) {
    defaultMessages.push_back(message);
    attemptWriteToJSON();
    return true;
}
bool Messages::deleteDefaultMessage(int index) {
    if (index >= defaultMessages.size()) {
        std::cout << "Attempt to delete non existing default message at index " << index << std::endl;
        return false;
    }
    defaultMessages.erase(defaultMessages.begin() + index);
    attemptWriteToJSON();
    return true;
}