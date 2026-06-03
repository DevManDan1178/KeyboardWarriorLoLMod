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

std::tuple<std::string, std::string> LoLEventHandler::getNextEvent() {
	if (eventQueue.size() <= 1) {
		return {"", ""};
	}
	std::queue<std::tuple<std::string, std::string>> queueClone = eventQueue;
	queueClone.pop();
	return queueClone.front();
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
	auto [eventCategory, eventName] = getCurrentEvent();
	std::vector<Message> messageList = isEvent ? messages.eventMessages[eventCategory][eventName] : messages.defaultMessages;
	if (hotkeyIndex >= messageList.size()) {
		return;
	}
	std::string messageContent = messageList[hotkeyIndex].messageContent;
	chatSender.sendMessage(messageContent);
}

void LoLEventHandler::updateCurrentEventStartTime() {
	using namespace std::chrono;
    currentEventStartTime = steady_clock::now();
}

float LoLEventHandler::getEventHotkeyDuration() {
	//If the current event is a quadra kill, can wait for a pentakill (30s)
	auto [eventCategory, eventName] = getCurrentEvent();
	if (eventName == "Quadra Kill") {
		return 30.0f;
	}
	return hotkeyManager.eventHotkeyDuration;
}

void LoLEventHandler::process() {
	using namespace std::chrono;

    auto [eventCategory, eventName] = getCurrentEvent();

    if (eventCategory.empty() && eventName.empty()) {
        return;
    }

    auto elapsed = duration_cast<milliseconds>(steady_clock::now() - currentEventStartTime).count();
    if (elapsed >= getEventHotkeyDuration() * 1000.0f) {
        closeCurrentEvent();
    }
}

float LoLEventHandler::getHotkeyExpirationProgress() {
	using namespace std::chrono;
	float elapsed = duration_cast<milliseconds>(steady_clock::now() - currentEventStartTime).count() * 1.0f;
	float total = getEventHotkeyDuration() * 1000.0f;

	return std::clamp(elapsed/total, 0.0f, 1.0f);
}

void LoLEventHandler::closeCurrentEvent() {
	if (eventQueue.size() > 0) {
		eventQueue.pop();
		auto [eventCategory, eventName] = getCurrentEvent();
		displayEventChange(eventCategory, eventName);
		updateCurrentEventStartTime();
	}
}


void LoLEventHandler::queueLoLEvent(std::string eventCategory, std::string eventName, bool forceAsCurrent) {
	if (forceAsCurrent) {
		updateCurrentEventStartTime();
		std::queue<std::tuple<std::string, std::string>>().swap(eventQueue);
		eventQueue.push({eventCategory, eventName});
		displayEventChange(eventCategory, eventName);
		return;
	}
	if (eventQueue.size() == 0) {
		updateCurrentEventStartTime();
	}

	eventQueue.push({eventCategory, eventName});
	if (eventQueue.size() <= 2) {
		displayEventChange(eventCategory, eventName);
	}
}

LoLEventHandler::LoLEventHandler(Messages& _messages, HotkeyManager& _hotkeyManager, ChatSender& _chatSender, std::function<void(std::string, std::string)> &_displayEventChange) 
	: messages(_messages), hotkeyManager(_hotkeyManager), chatSender(_chatSender), displayEventChange(_displayEventChange) {}


double lastPlayerKillTime = -1000.0;
int currentMultikillStreak = 0;
int gameKillCount = 0;

void LoLEventHandler::processLoLEvent(json lolEvent) {
	std::string eventName = lolEvent["EventName"];
	std::string localSummonerName = playersInfo.localPlayer;
	auto isAmongAssisters = [&]() -> bool {
		std::vector<std::string> assisters = lolEvent["Assisters"].get<std::vector<std::string>>();
		return std::find(assisters.begin(), assisters.end(), localSummonerName) != assisters.end();
	};			

	auto isKiller = [&]() -> bool {
		return lolEvent["KillerName"].get<std::string>() == localSummonerName;
	};


	//GameStates
	if (eventName == "GameStart") {
		queueLoLEvent("Game State", "GameStart");
		return;
	} 
    //Kills
	if (eventName == "ChampionKill") {
		gameKillCount++;
		if (lolEvent["VictimName"].get<std::string>() == localSummonerName) {
			queueLoLEvent("Kills", "Death");
			return;
		}
		// Also avoid sending kill when it's first blood
		if (!isKiller() || gameKillCount <= 1) {
			return;
		}

		double eventTime = lolEvent["EventTime"];
		double timeSinceLastKill = eventTime - lastPlayerKillTime;
    	bool extendsMultikill = (currentMultikillStreak < 4 && timeSinceLastKill <= 10.0) || (currentMultikillStreak == 4 && timeSinceLastKill <= 30.0);
		
		std::vector<std::string> assisters = lolEvent["Assisters"].get<std::vector<std::string>>();
		
		if (!extendsMultikill) {
			currentMultikillStreak = 1;
		}

		if (assisters.size() > 0) {
			queueLoLEvent("Kills", "Assisted Kill", true);
		} else if (!extendsMultikill) {
			queueLoLEvent("Kills", "Solo Kill", true);	
		}

		lastPlayerKillTime = eventTime;
		return;
	} 
	if (eventName == "FirstBlood") {
		if (lolEvent["Recipient"].get<std::string>() != localSummonerName) {
			return;
		}
		queueLoLEvent("Kills", "First Blood", true);
		return;
	}
	if (eventName == "Multikill") {
		if (!isKiller()) {
			return;
		}
		int killStreak = lolEvent["KillStreak"];
		currentMultikillStreak = killStreak;
		switch(killStreak) {
			case 2:
				queueLoLEvent("Kills", "Double Kill", true);
				return;
			case 3:
				queueLoLEvent("Kills", "Triple Kill", true);
				return;
			case 4:
				queueLoLEvent("Kills", "Quadra Kill", true);
				return;
			case 5:
				queueLoLEvent("Kills", "Pentakill", true);
				return;
			default:
				break;
		}
	}
	if (eventName == "Ace" ) {
		std::string acingTeam = lolEvent["AcingTeam"].get<std::string>();
		if (acingTeam != playersInfo.localPlayerTeam) {
			return;
		}
		queueLoLEvent("Kills", "Ace");
		return;
	}
    //Objectives
	if (eventName == "DragonKill") {
		if (!(isKiller() || isAmongAssisters())) {
			return;
		}
		queueLoLEvent("Objectives", "Dragon");
		return;
	}
	if (eventName == "BaronKill") {
		if (!(isKiller() || isAmongAssisters())){
			return;
		}
 		queueLoLEvent("Objectives", "Baron");
		return;
	}
	if (eventName == "HordeKill") { //Grubs
		if (!(isKiller() || isAmongAssisters())) {
			return;
		}
		queueLoLEvent("Objectives", "Void Grubs");
		return;	
	}
	if (eventName == "HeraldKill") {
		if (!(isKiller() || isAmongAssisters())) {
			return;
		}
		queueLoLEvent("Objectives", "Rift Herald");
		return;
	}
	if (eventName == "AtakhanKill") {
		if (!(isKiller() || isAmongAssisters())) {
			return;
		}
		queueLoLEvent("Objectives", "Atakhan");
		return;
	}
    //Structures
	if (eventName == "InhibKilled") {
		if (!(isKiller() || isAmongAssisters())) {
			return;
		}
		queueLoLEvent("Structures", "Inhibitor");
		return;
	}
	if (eventName == "TurretKilled") {
		if (!(isKiller())) {
			return;
		}
		queueLoLEvent("Structures", "Turret");
		return;
	}
	if (eventName == "FirstBrick") { //First turret
		if (!(isKiller())) {
			return;
		}
		queueLoLEvent("Structures", "First Turret");
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