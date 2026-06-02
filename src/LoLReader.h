#pragma once
#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <tuple>
#include <queue>
#include "../external/json.hpp"
#include "LoLEventHandler.h"
#include "LoLTypes.h"

const std::string url ="https://127.0.0.1:2999/liveclientdata/allgamedata";


class LoLReader {
    public: 
        LoLEventHandler &lolEventHandler;
        
        void initializeLoop();
        void closeLoop();
        void process();
        
        LoLReader(LoLEventHandler& _lolEventHandler);
        bool isIdle; 
        bool isInGame;
    private:
        std::atomic<bool> running;  
        std::thread workerThread;

        void stopCoreLoop();
        void liveClientEventLoop();
        void coreLoop();
        bool queryForGame();
        bool isLoadingOrInGame();

        std::tuple<bool, LoLPlayersInfo> getPlayersInfo();

};