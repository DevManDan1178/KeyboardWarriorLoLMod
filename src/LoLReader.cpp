#include <LoLReader.h>
#include "helpers/HttpHelper.h"
#include <queue>
#include <thread>
#include <tuple>
#include <iostream>
#include "../external/json.hpp"

using json = nlohmann::json;

const int IDLE_TIME_BETWEEN_CHECKS = 2000; //Milliseconds
const int LOADING_TIME_BETWEEN_CHECKS = 500; //Milliseconds
const int TIME_BETWEEN_EVENT_LOOP = 1000; //Milliseconds [150]


void LoLReader::stopCoreLoop(){
    if (workerThread.joinable())
    {
        workerThread.join();
    }
}

void LoLReader::process(){
    lolEventHandler.process();
}

LoLReader::LoLReader(LoLEventHandler& _lolEventHandler)
    : lolEventHandler(_lolEventHandler) {}

std::tuple<bool, LoLPlayersInfo> LoLReader::getPlayersInfo()
{
    LoLPlayersInfo result;

    try
    {
        // local player 
        std::tuple<bool, std::string> httpQueryResultLocal = HTTPHelper::HttpGet("https://127.0.0.1:2999/liveclientdata/activeplayername");
        auto [localSuccess, localBody] = httpQueryResultLocal;
        if (!localBody.empty())
        {
            json localJson = json::parse(localBody);
            result.localPlayer = localJson.get<std::string>();
        }

        // all players
        std::tuple<bool, std::string> httpQueryResult = HTTPHelper::HttpGet("https://127.0.0.1:2999/liveclientdata/playerlist");
        auto [success, body] = httpQueryResult;
        if (body.empty())
            return {false, result};

        json players = json::parse(body);
        if (!players.is_array()) {
            return {false, result};
        }
        
        std::string localTeam;

        auto getPlayerName = [](json p) {
            return p.value("riotIdGameName", p.value("summonerName", "")) + "#" + p.value("riotIdTagLine", "");
        };

        // find local team
        for (const auto& p : players)
        {
            if (getPlayerName(p) == result.localPlayer)
            {
                localTeam = p.value("team", "");
                break;
            }
        }

        // split teammates/enemies
        for (const auto& p : players)
        {
            std::string name = getPlayerName(p);

            std::string team = p.value("team", "");

            if (name == result.localPlayer) {
                result.teammates.push_back(name);
                continue;
            }
            if (team == localTeam){
                result.teammates.push_back(name);
            } else {
                result.enemies.push_back(name);
            }
                
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "getTeams error: " << e.what() << std::endl;
        return {false, result};
    }

    return {true, result};
}

void LoLReader::liveClientEventLoop()
{
    int nextEventID = 0;

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_BETWEEN_EVENT_LOOP));
        
        //Gets all event with id >= lastEventId
        std::string queryAdress = "https://127.0.0.1:2999/liveclientdata/eventdata?eventID=" + std::to_string(nextEventID);
        std::tuple<bool, std::string> httpQueryResult = HTTPHelper::HttpGet(queryAdress);
        
        auto [success, body] = httpQueryResult;
        if (!success) {
            return;
        }
        
        auto pushEvent = [&](const json& event)
        {
            std::lock_guard<std::mutex> lock(eventMutex);
            lolEventHandler.processLoLEvent(event);
        };
        
        if (!body.empty()) {
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
                    if (id < nextEventID) {
                        continue;
                    }
                    
                    pushEvent(event);
                    
                    nextEventID = std::max(nextEventID + 1, id);
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

bool LoLReader::isLoadingOrInGame() {
    auto [success, body] = HTTPHelper::HttpGet("https://127.0.0.1:2999/liveclientdata/activeplayername");
    return success;
}

bool LoLReader::queryForGame()
{
    auto [success, body] = HTTPHelper::HttpGet("https://127.0.0.1:2999/liveclientdata/allgamedata");

    if (!success) {
        return false;
    }
       

    try {
        json data = json::parse(body);

        // Basic validation: key that always exists in live game
        return data.contains("activePlayer") &&
               data.contains("allPlayers") &&
               data["allPlayers"].is_array();
    } catch (...)
    {
        return false;
    }
}


void LoLReader::initializeLoop() {
     if (running) {
        return;
    }
    running = true;
    workerThread = std::thread(&LoLReader::coreLoop, this);
}

void LoLReader::coreLoop() {
    std::cout << "Starting to run Core Loop" << std::endl;
    while (running) {
        isInGame = false;
        if (isLoadingOrInGame()) {
            isIdle = false;
           
            if (queryForGame()) { 
                auto [success, playersInfo] = getPlayersInfo();
                if (!success) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_BETWEEN_EVENT_LOOP));
                    continue;
                }
                lolEventHandler.playersInfo = playersInfo;
                std::cout << "Game Detected: Starting Live Client Event Loop" << std::endl;
                
                isInGame = true;
                liveClientEventLoop();
            } else {
                //std::cout << "Is in loading screen: tick rate is lowered" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(LOADING_TIME_BETWEEN_CHECKS));
            }
        } else {
            std::cout << "is idle" << std::endl;
            isIdle = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(IDLE_TIME_BETWEEN_CHECKS));
    }
    std::cout << "Core Loop over" << std::endl;
}

void LoLReader::closeLoop() {
    running = false;
    stopCoreLoop();
}

