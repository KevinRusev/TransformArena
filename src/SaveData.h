#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

struct SaveData
{
    bool hasRun;
    int floor;
    int score;
    int totalKills;
    float playTime;
    int health;
    int maxHealth;
    int form; // 0=circle, 1=triangle, 2=square
    int equippedItemType;
    std::vector<int> buffIds;

    int highScore;
    int highFloor;
    int highKills;

    SaveData()
        : hasRun(false), floor(1), score(0), totalKills(0), playTime(0.f)
        , health(100), maxHealth(100), form(0), equippedItemType(0)
        , highScore(0), highFloor(0), highKills(0)
    {}

    static std::string filePath()
    {
#ifdef _WIN32
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        std::string dir(path);
        size_t pos = dir.find_last_of("\\/");
        if (pos != std::string::npos)
            dir = dir.substr(0, pos + 1);
        return dir + "savegame.txt";
#else
        return "savegame.txt";
#endif
    }

    bool save() const
    {
        std::ofstream f(filePath());
        if (!f.is_open()) return false;

        f << "has_run=" << (hasRun ? 1 : 0) << "\n";
        f << "floor=" << floor << "\n";
        f << "score=" << score << "\n";
        f << "kills=" << totalKills << "\n";
        f << "time=" << playTime << "\n";
        f << "health=" << health << "\n";
        f << "max_health=" << maxHealth << "\n";
        f << "form=" << form << "\n";
        f << "item=" << equippedItemType << "\n";

        f << "buffs=";
        for (size_t i = 0; i < buffIds.size(); i++)
        {
            if (i > 0) f << ",";
            f << buffIds[i];
        }
        f << "\n";

        f << "high_score=" << highScore << "\n";
        f << "high_floor=" << highFloor << "\n";
        f << "high_kills=" << highKills << "\n";

        return true;
    }

    bool load()
    {
        std::ifstream f(filePath());
        if (!f.is_open()) return false;

        std::string line;
        while (std::getline(f, line))
        {
            size_t eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);

            if (key == "has_run") hasRun = (val == "1");
            else if (key == "floor") floor = std::stoi(val);
            else if (key == "score") score = std::stoi(val);
            else if (key == "kills") totalKills = std::stoi(val);
            else if (key == "time") playTime = std::stof(val);
            else if (key == "health") health = std::stoi(val);
            else if (key == "max_health") maxHealth = std::stoi(val);
            else if (key == "form") form = std::stoi(val);
            else if (key == "item") equippedItemType = std::stoi(val);
            else if (key == "buffs")
            {
                buffIds.clear();
                std::stringstream ss(val);
                std::string token;
                while (std::getline(ss, token, ','))
                {
                    if (!token.empty())
                        buffIds.push_back(std::stoi(token));
                }
            }
            else if (key == "high_score") highScore = std::stoi(val);
            else if (key == "high_floor") highFloor = std::stoi(val);
            else if (key == "high_kills") highKills = std::stoi(val);
        }

        return true;
    }

    void clearRun()
    {
        hasRun = false;
        floor = 1;
        score = 0;
        totalKills = 0;
        playTime = 0.f;
        health = 100;
        maxHealth = 100;
        form = 0;
        equippedItemType = 0;
        buffIds.clear();
    }

    void updateHighScores()
    {
        if (score > highScore) highScore = score;
        if (floor > highFloor) highFloor = floor;
        if (totalKills > highKills) highKills = totalKills;
    }
};
