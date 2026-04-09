#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Enemy.h"
#include "Projectile.h"

struct RoomDoor
{
    int direction; // 0=up, 1=right, 2=down, 3=left
    bool open;
};

class Room
{
public:
    Room();

    void generate(int floor, bool isBoss);
    void update(float dt, sf::Vector2f playerPos, std::vector<Projectile>& projectiles);
    void draw(sf::RenderWindow& window);
    void drawDoors(sf::RenderWindow& window);

    bool isCleared() const;
    bool isBossRoom() const;
    bool isVisited() const;
    void markVisited();

    std::vector<Enemy>& getEnemies();
    void setDoor(int direction, bool exists);
    bool hasDoor(int direction) const;
    bool doorOpen(int direction) const;

    int gridX, gridY;

private:
    std::vector<Enemy> enemies;
    bool boss;
    bool cleared;
    bool visited;
    bool spawned;
    int floorNum;

    bool doors[4]; // up, right, down, left

    void spawnEnemies();
};
