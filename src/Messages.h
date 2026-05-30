#pragma once
#include <string>
#include <map>
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
        std::map<std::string, std::vector<Message>> messages;

        bool setMessageContent(std::string category, int index, std::string content);
        bool setMessageTitle(std::string category, int index, std::string title);
        bool createNewMessage(std::string category, Message message);
        bool deleteMessage(std::string category, int index);


    private:
        bool writeToJSON();
        void attemptWriteToJSON();
};