#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct Message {
    std::string messageContent;
    std::string messageTitle;

    Message() = default;
    Message(std::string content, std::string title)
        : messageContent(std::move(content)),
          messageTitle(std::move(title)) {}
};


struct MessageBuffer {
    char content[256]{}; //Capped at 255 characters per message, + 1 for null terminator 
    char title[32]{};
};

class Messages {
    public:
        bool load();
        std::vector<Message> defaultMessages;
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Message>>> eventMessages;
        std::unordered_map<std::string, std::vector<std::string>> eventKeyOrders;

        bool setEventMessageContent(std::string category, std::string event, int index, std::string content);
        bool setEventMessageTitle(std::string category, std::string event, int index, std::string title);
        bool createNewEventMessage(std::string category, std::string event, Message message);
        bool deleteEventMessage(std::string category, std::string event, int index);

        bool setDefaultMessageContent(int index, std::string content);
        bool setDefaultMessageTitle(int index, std::string title);
        bool createNewDefaultMessage(Message message);
        bool deleteDefaultMessage(int index);
    private:
        bool writeToJSON();
        void attemptWriteToJSON();
        bool checkValidEventMessage(std::string category, std::string event, int index);
};