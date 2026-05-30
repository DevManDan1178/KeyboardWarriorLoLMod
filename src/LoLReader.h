#pragma once
#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <queue>
#include "../external/json.hpp"


const std::string url ="https://127.0.0.1:2999/liveclientdata/allgamedata";


class LoLReader {
    public: 
       
        void start();
        void stop();
        void processNewEvents();
        
    private:
        std::queue<nlohmann::json> eventQueue;
        std::atomic<bool> running;  
        std::mutex eventMutex;
        std::thread workerThread;

        void liveClientEventLoop();

};