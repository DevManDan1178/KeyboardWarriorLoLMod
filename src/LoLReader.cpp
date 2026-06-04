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
const int TIME_BETWEEN_EVENT_LOOP = 100; //Milliseconds [100]


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
    LoLPlayersInfo playersInfo;

    try
    {
        // local player 
        std::tuple<bool, std::string> httpQueryResultLocal = HTTPHelper::HttpGet("https://127.0.0.1:2999/liveclientdata/activeplayername");
        auto [localSuccess, localBody] = httpQueryResultLocal;
        
        std::string localPlayerFullName;
        if (!localBody.empty())
        {
            json localJson = json::parse(localBody);
            localPlayerFullName = localJson.get<std::string>();
        }

        // all players
        std::tuple<bool, std::string> httpQueryResult = HTTPHelper::HttpGet("https://127.0.0.1:2999/liveclientdata/playerlist");
        auto [success, body] = httpQueryResult;
        if (body.empty())
            return {false, playersInfo};

        json players = json::parse(body);
        if (!players.is_array()) {
            return {false, playersInfo};
        }
        

        auto getPlayerName = [](json p) -> std::tuple<std::string, std::string> {
            std::string riotBaseName = p.value("riotIdGameName", p.value("summonerName", ""));
            return {riotBaseName, riotBaseName + "#" + p.value("riotIdTagLine", "")};
        };

        // find local team and local player
        for (const auto& p : players)
        {
            auto [summonerName, playerFullName] = getPlayerName(p);
            if (playerFullName == localPlayerFullName)
            {
                playersInfo.localPlayerTeam = p.value("team", "");
                playersInfo.localPlayer = summonerName;
                break;
            }
        }

        // split teammates/enemies
        for (const auto& p : players)
        {
            auto [summonerName, playerFullName] = getPlayerName(p);

            std::string team = p.value("team", "");

            if (playerFullName == localPlayerFullName) {
                continue;
            }
            if (team == playersInfo.localPlayerTeam){
                playersInfo.teammates.push_back(summonerName);
            } else {
                playersInfo.enemies.push_back(summonerName);
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "getTeams error: " << e.what() << std::endl;
        return {false, playersInfo};
    }

    return {true, playersInfo};
}

void LoLReader::liveClientEventLoop()
{
    int currentEventId = -1;
    bool gameEndDetected = false;
    
    while (running && !gameEndDetected) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_BETWEEN_EVENT_LOOP));
        
        //Gets all event with id >= lastEventId
        std::string queryAdress = "https://127.0.0.1:2999/liveclientdata/eventdata?eventID=" + std::to_string(std::max(currentEventId, 0));
        std::tuple<bool, std::string> httpQueryResult = HTTPHelper::HttpGet(queryAdress);
        
        auto [success, body] = httpQueryResult;
        if (!success) {
            return;
        }
        
        auto pushEvent = [&](const json& event)
        {
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
                    if (!event.contains("EventID") || !event["EventID"].is_number_integer() || !event.contains("EventName") || !event["EventName"].is_string()){
                        continue;
                    }
                    if (event["EventName"].get<std::string>() == "GameEnd") {
                        gameEndDetected = true;
                        break;
                    }

                    int id = event["EventID"];
                    if (id <= currentEventId) {
                        continue;
                    }
                    
                    pushEvent(event);
                    
                    currentEventId = std::max(currentEventId, id);
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "LoLReader parse error: " << e.what() << std::endl << queryAdress << std::endl;
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

                isInGame = true;
                liveClientEventLoop();
                lolEventHandler.reset();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(LOADING_TIME_BETWEEN_CHECKS));
            }
        } else {
            isIdle = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(IDLE_TIME_BETWEEN_CHECKS));
    }
}

void LoLReader::closeLoop() {
    running = false;
    stopCoreLoop();
}

