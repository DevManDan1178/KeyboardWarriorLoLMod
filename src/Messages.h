#pragma once
#include <string>
#include <unordered_map>

class Messages {
public:
    bool load(const std::string& path);

    std::string getMessage(const std::string& key) const;
    int getHotkeyVK(const std::string& key) const;

private:
    std::unordered_map<std::string, std::string> messages;
    std::unordered_map<std::string, int> hotkeys;
};