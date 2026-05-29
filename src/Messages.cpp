#include "../external/json.hpp"
#include <fstream>
#include <unordered_map>
using namespace std;

using json = nlohmann::json;

class Messages {
    bool load(const std::string& path) {
        ifstream file(path);
        if (!file.is_open()) return false;
        
        //TODO: Read from the file to assign corresponding values to messages and hotkeys vectors

        return true;
    }

    string getMessage(const string& key) const {
        //TODO Get message from the hotkey
        return "";
    }
};

