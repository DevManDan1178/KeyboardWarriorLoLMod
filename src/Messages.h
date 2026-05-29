#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct Message {
    std::string messageStr;
    std::string messageTitle;
};

class Messages {
    public:
        bool load();
        std::string getMessage(const std::string& key) const;

        std::unordered_map<std::string, std::vector<Message>> messages;
};