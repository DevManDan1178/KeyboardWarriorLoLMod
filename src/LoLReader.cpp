#include <LoLReader.h>
#include "helpers/HttpHelper.h"
#include <queue>
#include <thread>
#include <iostream>
#include "../external/json.hpp"

using json = nlohmann::json;

const int TIME_BETWEEN_EVENT_LOOP = 2000; //Milliseconds [150]

void LoLReader::start() {
    if (running) {
        return;
    }
    running = true;
    std::cout << "Starting liveClientEventLoop" << std::endl;
    workerThread = std::thread(&LoLReader::liveClientEventLoop, this);
}


void LoLReader::stop(){
    running = false;
    if (workerThread.joinable())
    {
        workerThread.join();
    }
}
void LoLReader::processNewEvents(){

}

void LoLReader::liveClientEventLoop()
{
    int lastEventID = 0;

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_BETWEEN_EVENT_LOOP));
        std::string queryAdress = "https://127.0.0.1:2999/liveclientdata/eventdata?eventID=" + std::to_string(lastEventID);
        std::string body = HTTPHelper::HttpGet(queryAdress);
        std::cout << queryAdress << std::endl;
        auto pushEvent = [&](const json& event)
        {
            std::lock_guard<std::mutex> lock(eventMutex);
            std::cout << "Event detected in LoLReader\n" << event.dump(4) << std::endl;
            LoLReader::eventQueue.push(event);
        };
        
        if (!body.empty())
        {
            try
            {
                json data = json::parse(body);
                if (!data.contains("Events") || !data["Events"].is_array()) {
                    std::cout << "No event list for LoLReader" << std::endl;
                    continue;
                }

                for (const auto& event : data["Events"])
                {
                    if (!event.contains("EventID") || !event["EventID"].is_number_integer()){
                        continue;
                    }

                    int id = event["EventID"];
                    if (id <= lastEventID) {
                        continue;
                    }
                    
                    pushEvent(event);

                    lastEventID = std::max(lastEventID, id);
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "LoLReader parse error: " << e.what() << std::endl;
            }
        } else {
            std::cout << "Empty event list for LoLReader" << std::endl;
        }
    }
}