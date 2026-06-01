#pragma once
#include <string>
#include <vector>

struct LoLPlayersInfo
{
    std::string localPlayer;
    std::vector<std::string> teammates;
    std::vector<std::string> enemies;

    LoLPlayersInfo() = default;
};

struct PlayerInfo {
    std::string puuid;
    std::string riotId;
    std::string team;
    std::string champion;
};

