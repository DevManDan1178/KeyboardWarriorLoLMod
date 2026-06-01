#include "../external/json.hpp"
#include "Messages.h"
#include "HotkeyManager.h"
#include "lolEventHandler.h"
#include <iostream>

using json = nlohmann::json;


std::tuple<std::string, std::string> LoLEventHandler::getCurrentEvent() {
	if (eventQueue.size() == 0) {
		return {"", ""};
	}
	return eventQueue.front();
}


void LoLEventHandler::printPlayersInfo() {
	std::cout << "localPlayer : " << playersInfo.localPlayer << "\nteammates : ";
	for (std::string s : playersInfo.teammates) {
		std::cout << s << " ";
	}
	std::cout << "\nenemies : ";
	for (std::string s : playersInfo.enemies) {
		std::cout << s << " ";
	}
	std::cout << std::endl;					
}

void LoLEventHandler::processHotkeyPressed(int hotkeyIndex, bool isEvent) {
	std::cout << "Hotkey pressed for index " << hotkeyIndex << " is event: " << isEvent << std::endl;
}

void LoLEventHandler::process() {
	using namespace std::chrono;

    auto [eventCategory, eventName] = getCurrentEvent();

    if (eventCategory.empty() && eventName.empty()) {
        timerRunning = false; 
        return;
    }
	
    if (!timerRunning) {
        std::cout << "Displaying event - category: " << eventCategory << " - event name: " << eventName << std::endl;
        currentEventStartTime = steady_clock::now();
        timerRunning = true;
    }

    auto elapsed = duration_cast<milliseconds>(steady_clock::now() - currentEventStartTime).count();
    if (elapsed >= hotkeyManager.eventHotkeyDuration * 1000) {
        closeCurrentEvent();
    }
}

void LoLEventHandler::closeCurrentEvent() {
	if (eventQueue.size() > 0) {
		eventQueue.pop();
	}
	std::cout << "Moving on to new event: " << std::get<1>(getCurrentEvent()) << std::endl;
	timerRunning = false;
}


void LoLEventHandler::queueLoLEvent(std::string eventCategory, std::string eventName) {
	eventQueue.push({eventCategory, eventName});
}

LoLEventHandler::LoLEventHandler(Messages& _messages, HotkeyManager& _hotkeyManager) 
	: messages(_messages), hotkeyManager(_hotkeyManager) {}

void LoLEventHandler::processLoLEvent(json LoLEvent) {
	std::string eventName = LoLEvent["EventName"];
	
	//GameStates
	if (eventName == "GameStart") {
		queueLoLEvent("GameState", "GameStart");
		return;
	} 
	if (eventName == "GameEnd") {
		queueLoLEvent("GameState", "GameEnd");
		return;
	}
    //Kills
	if (eventName == "ChampionKill") {
		queueLoLEvent("Kills", "Kill");
		return;
	} 
	if (eventName == "FirstBlood") {
		queueLoLEvent("Kills", "FirstBlood");
		return;
	}
	if (eventName == "Multikill") {
		int killStreak = LoLEvent["KillStreak"];
		switch(killStreak) {
			case 2:
				queueLoLEvent("Kills", "DoubleKill");
				return;
			case 3:
				queueLoLEvent("Kills", "TripleKill");
				return;
			case 4:
				queueLoLEvent("Kills", "QuadraKill");
				return;
			case 5:
				queueLoLEvent("Kills", "PentaKill");
				return;
			default:
				break;
		}
	}
	if (eventName == "Ace") {
		queueLoLEvent("Kills", "Ace");
		return;
	}
    //Objectives
	if (eventName == "DragonKill") {
		queueLoLEvent("Objectives", "Dragon");
		return;
	}
	if (eventName == "BaronKill") {
		queueLoLEvent("Objectives", "Baron");
		return;
	}
	if (eventName == "HordeKill") { //Grubs
		queueLoLEvent("Objectives", "VoidGrubs");
		return;
	}
	if (eventName == "AtakhanKill") {
		queueLoLEvent("Objectives", "Atakhan");
		return;
	}
    //Structures
	if (eventName == "InhibKilled") {
		queueLoLEvent("Structures", "Inhibitor");
		return;
	}
	if (eventName == "TurretKilled") {
		queueLoLEvent("Structures", "Turret");
		return;
	}
	if (eventName == "FirstBrick") { //First turret
		queueLoLEvent("Structures", "FirstTurret");
		return;
	}
}

/*

{
	"Events": [
		{
			"EventID": 0,
			"EventName": "GameStart",
			"EventTime": 0.018873900175094606
		},
		{
			"EventID": 1,
			"EventName": "MinionsSpawning",
			"EventTime": 30.137344360351564
		},
		{
			"Assisters": [],
			"EventID": 2,
			"EventName": "ChampionKill",
			"EventTime": 91.96644592285156,
			"KillerName": "Y0URBADGETG00D",
			"VictimName": "Taric Bot"
		},
		{
			"EventID": 3,
			"EventName": "FirstBlood",
			"EventTime": 91.96644592285156,
			"Recipient": "Y0URBADGETG00D"
		},

		{
			"EventID": 9,
			"EventName": "Multikill",
			"EventTime": 104.21562194824219,
			"KillStreak": 4,
			"KillerName": "Y0URBADGETG00D"
		},
		{

		{
			"Assisters": [],
			"EventID": 12,
			"EventName": "TurretKilled",
			"EventTime": 144.30613708496095,
			"KillerName": "Y0URBADGETG00D",
			"TurretKilled": "Turret_TChaos_L1_P3_2254202041_0"
		},
		{
			"EventID": 13,
			"EventName": "FirstBrick",
			"EventTime": 144.30613708496095,
			"KillerName": "Y0URBADGETG00D"
		},
		{
			"Assisters": [],
			"EventID": 14,
			"EventName": "ChampionKill",
			"EventTime": 147.8509521484375,
			"KillerName": "Y0URBADGETG00D",
			"VictimName": "Irelia Bot"
		}
	]
}*/