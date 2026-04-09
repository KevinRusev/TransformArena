#include "Room.h"
#include <cstdlib>
#include <cmath>

Room::Room()
    : gridX(0), gridY(0)
    , boss(false), cleared(false), visited(false), spawned(false)
    , floorNum(1)
{
    for (int i = 0; i < 4; i++)
        doors[i] = false;
}

void Room::generate(int floor, bool isBoss)
{
    floorNum = floor;
    boss = isBoss;
    cleared = false;
    spawned = false;
    enemies.clear();
}

void Room::spawnEnemies()
{
    if (spawned) return;
    spawned = true;

    if (boss)
    {
        enemies.emplace_back(400.f, 200.f, EnemyType::Brute, true);
        // add some minions too
        int minions = 2 + floorNum;
        for (int i = 0; i < minions; i++)
        {
            float ex = 150.f + (float)(std::rand() % 500);
            float ey = 100.f + (float)(std::rand() % 300);
            EnemyType mt = (std::rand() % 2 == 0) ? EnemyType::Shooter : EnemyType::Chaser;
            enemies.emplace_back(ex, ey, mt);
        }
        return;
    }

    int baseCount = 3 + floorNum * 2;
    int count = baseCount + std::rand() % 3;

    for (int i = 0; i < count; i++)
    {
        float margin = 80.f;
        float x = margin + (float)(std::rand() % (int)(800.f - margin * 2));
        float y = margin + (float)(std::rand() % (int)(600.f - margin * 2));

        // don't spawn too close to center where player appears
        float dx = x - 400.f, dy = y - 300.f;
        if (std::sqrt(dx * dx + dy * dy) < 100.f)
        {
            x += 150.f;
            if (x > 720.f) x = 720.f;
        }

        EnemyType type;
        int roll = std::rand() % 100;

        if (floorNum == 1)
        {
            type = (roll < 70) ? EnemyType::Chaser : EnemyType::Shooter;
        }
        else if (floorNum == 2)
        {
            if (roll < 35) type = EnemyType::Chaser;
            else if (roll < 65) type = EnemyType::Shooter;
            else type = EnemyType::Brute;
        }
        else
        {
            if (roll < 25) type = EnemyType::Chaser;
            else if (roll < 55) type = EnemyType::Shooter;
            else type = EnemyType::Brute;
        }

        enemies.emplace_back(x, y, type);
    }
}

void Room::update(float dt, sf::Vector2f playerPos, std::vector<Projectile>& projectiles)
{
    if (!spawned && visited && !cleared)
        spawnEnemies();

    for (auto& e : enemies)
        e.update(dt, playerPos, projectiles);

    // push enemies apart
    for (size_t i = 0; i < enemies.size(); i++)
    {
        for (size_t j = i + 1; j < enemies.size(); j++)
        {
            if (!enemies[i].isAlive() || !enemies[j].isAlive()) continue;
            float minDist = enemies[i].getRadius() + enemies[j].getRadius();
            sf::Vector2f ai = enemies[i].getPosition(), aj = enemies[j].getPosition();
            float dx = ai.x - aj.x, dy = ai.y - aj.y;
            float d = std::sqrt(dx * dx + dy * dy);
            if (d < minDist && d > 0.1f)
            {
                float push = (minDist - d) * 0.5f;
                enemies[i].pushAway(aj, push);
                enemies[j].pushAway(ai, push);
            }
        }
    }

    if (spawned && !cleared)
    {
        bool allDead = true;
        for (auto& e : enemies)
        {
            if (e.isAlive()) { allDead = false; break; }
        }
        if (allDead) cleared = true;
    }
}

void Room::draw(sf::RenderWindow& window)
{
    for (auto& e : enemies)
        e.draw(window);
}

void Room::drawDoors(sf::RenderWindow& window)
{
    // door positions: up=top center, right=right center, down=bottom center, left=left center
    float doorW = 60.f, doorH = 16.f;
    sf::Color closedCol(80, 40, 40);
    sf::Color openCol(60, 180, 60);

    struct DoorPos { float x, y, w, h; };
    DoorPos positions[4] = {
        { 400.f - doorW / 2.f, 0.f,           doorW, doorH },  // up
        { 800.f - doorH,       300.f - doorW / 2.f, doorH, doorW },  // right
        { 400.f - doorW / 2.f, 600.f - doorH, doorW, doorH },  // down
        { 0.f,                 300.f - doorW / 2.f, doorH, doorW },  // left
    };

    for (int i = 0; i < 4; i++)
    {
        if (!doors[i]) continue;

        sf::RectangleShape door(sf::Vector2f(positions[i].w, positions[i].h));
        door.setPosition(positions[i].x, positions[i].y);
        door.setFillColor(cleared ? openCol : closedCol);
        door.setOutlineColor(sf::Color(200, 200, 200));
        door.setOutlineThickness(1.f);
        window.draw(door);
    }
}

bool Room::isCleared() const { return cleared; }
bool Room::isBossRoom() const { return boss; }
bool Room::isVisited() const { return visited; }
void Room::markVisited() { visited = true; }

std::vector<Enemy>& Room::getEnemies() { return enemies; }

void Room::setDoor(int direction, bool exists) { doors[direction] = exists; }
bool Room::hasDoor(int direction) const { return doors[direction]; }
bool Room::doorOpen(int direction) const { return doors[direction] && cleared; }
