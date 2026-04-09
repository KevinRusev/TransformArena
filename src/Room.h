#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Enemy.h"
#include "Projectile.h"
#include "Item.h"

enum class RoomType { Normal, Boss, Shop, Start };

struct RoomDoor
{
    int direction; // 0=up, 1=right, 2=down, 3=left
    bool exists;
};

class Room
{
public:
    Room();

    void generate(int floor, RoomType type);
    void update(float dt, sf::Vector2f playerPos, std::vector<Projectile>& projectiles);
    void draw(sf::RenderWindow& window);
    void drawWalls(sf::RenderWindow& window);
    void drawFloor(sf::RenderWindow& window);
    void drawDoors(sf::RenderWindow& window);
    void drawShop(sf::RenderWindow& window, const sf::Font& font);

    bool isCleared() const;
    bool isBossRoom() const;
    bool isShopRoom() const;
    RoomType getType() const;
    bool isVisited() const;
    void markVisited();

    std::vector<Enemy>& getEnemies();
    void setDoor(int direction, bool exists);
    bool hasDoor(int direction) const;
    bool doorOpen(int direction) const;

    std::vector<Item>& getShopItems();

    int gridX, gridY;

private:
    std::vector<Enemy> enemies;
    std::vector<Item> shopItems;
    RoomType roomType;
    bool cleared;
    bool visited;
    bool spawned;
    int floorNum;
    int floorPattern;

    bool doors[4];

    void spawnEnemies();
    void generateShopItems();
};
