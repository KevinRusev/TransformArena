#include "Room.h"
#include <cstdlib>
#include <cmath>

Room::Room()
    : gridX(0), gridY(0)
    , roomType(RoomType::Normal)
    , cleared(false), visited(false), spawned(false)
    , floorNum(1), floorPattern(0)
{
    for (int i = 0; i < 4; i++)
        doors[i] = false;
}

void Room::generate(int floor, RoomType type)
{
    floorNum = floor;
    roomType = type;
    cleared = (type == RoomType::Start || type == RoomType::Shop);
    spawned = false;
    enemies.clear();
    shopItems.clear();
    floorPattern = std::rand() % 4;

    if (type == RoomType::Shop)
        generateShopItems();
}

void Room::generateShopItems()
{
    ItemType allTypes[] = {
        ItemType::FlameRing, ItemType::FrostShard,
        ItemType::ThunderStrike, ItemType::ShadowDash, ItemType::BarrierShield
    };

    // shuffle to pick 3 unique items
    int count = 5;
    for (int i = count - 1; i > 0; i--)
    {
        int j = std::rand() % (i + 1);
        ItemType tmp = allTypes[i];
        allTypes[i] = allTypes[j];
        allTypes[j] = tmp;
    }

    float startX = 200.f;
    for (int i = 0; i < 3; i++)
    {
        Item item = Item::create(allTypes[i], floorNum);
        item.shopX = startX + i * 200.f;
        item.shopY = 230.f;
        shopItems.push_back(item);
    }
}

void Room::spawnEnemies()
{
    if (spawned) return;
    spawned = true;

    if (roomType == RoomType::Boss)
    {
        // different boss types per floor
        EnemyType bossType = EnemyType::Brute;
        if (floorNum == 2) bossType = EnemyType::Shooter;
        if (floorNum >= 3) bossType = EnemyType::Brute;

        enemies.emplace_back(400.f, 200.f, bossType, true);

        int minions = 3 + floorNum * 2;
        for (int i = 0; i < minions; i++)
        {
            float ex = 100.f + (float)(std::rand() % 600);
            float ey = 80.f + (float)(std::rand() % 400);
            // avoid spawning on top of boss
            float ddx = ex - 400.f, ddy = ey - 200.f;
            if (std::sqrt(ddx * ddx + ddy * ddy) < 80.f)
                ex += 150.f;

            EnemyType mt;
            int roll = std::rand() % 3;
            if (roll == 0) mt = EnemyType::Shooter;
            else if (roll == 1) mt = EnemyType::Chaser;
            else mt = EnemyType::Brute;
            enemies.emplace_back(ex, ey, mt);
        }
        return;
    }

    if (roomType == RoomType::Shop || roomType == RoomType::Start)
        return;

    int baseCount = 3 + floorNum * 2;
    int count = baseCount + std::rand() % 3;

    for (int i = 0; i < count; i++)
    {
        float margin = 80.f;
        float x = margin + (float)(std::rand() % (int)(800.f - margin * 2));
        float y = margin + (float)(std::rand() % (int)(600.f - margin * 2));

        float dx = x - 400.f, dy = y - 300.f;
        if (std::sqrt(dx * dx + dy * dy) < 100.f)
        {
            x += 150.f;
            if (x > 720.f) x = 720.f;
        }

        EnemyType type;
        int roll = std::rand() % 100;

        if (floorNum == 1)
            type = (roll < 70) ? EnemyType::Chaser : EnemyType::Shooter;
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

    for (size_t i = 0; i < enemies.size(); i++)
    {
        for (size_t j = i + 1; j < enemies.size(); j++)
        {
            if (!enemies[i].isAlive() || !enemies[j].isAlive()) continue;
            float minDist = enemies[i].getRadius() + enemies[j].getRadius();
            sf::Vector2f ai = enemies[i].getPosition(), aj = enemies[j].getPosition();
            float ddx = ai.x - aj.x, ddy = ai.y - aj.y;
            float d = std::sqrt(ddx * ddx + ddy * ddy);
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

void Room::drawFloor(sf::RenderWindow& window)
{
    float wallThick = 16.f;

    // floor background based on room type
    sf::Color floorCol;
    switch (roomType)
    {
    case RoomType::Boss:  floorCol = sf::Color(28, 16, 16); break;
    case RoomType::Shop:  floorCol = sf::Color(16, 22, 28); break;
    default:              floorCol = sf::Color(18, 18, 28); break;
    }

    sf::RectangleShape floor(sf::Vector2f(800.f - wallThick * 2, 600.f - wallThick * 2));
    floor.setPosition(wallThick, wallThick);
    floor.setFillColor(floorCol);
    window.draw(floor);

    // floor tile pattern
    float tileSize = 40.f;
    for (float x = wallThick; x < 800.f - wallThick; x += tileSize)
    {
        for (float y = wallThick; y < 600.f - wallThick; y += tileSize)
        {
            int tx = (int)(x / tileSize);
            int ty = (int)(y / tileSize);
            bool alt = ((tx + ty + floorPattern) % 2 == 0);

            sf::RectangleShape tile(sf::Vector2f(tileSize - 1.f, tileSize - 1.f));
            tile.setPosition(x, y);

            sf::Color tileCol = floorCol;
            if (alt)
            {
                tileCol.r = (sf::Uint8)std::min(255, tileCol.r + 6);
                tileCol.g = (sf::Uint8)std::min(255, tileCol.g + 6);
                tileCol.b = (sf::Uint8)std::min(255, tileCol.b + 6);
            }
            tile.setFillColor(tileCol);
            window.draw(tile);
        }
    }
}

void Room::drawWalls(sf::RenderWindow& window)
{
    float w = 16.f;
    sf::Color wallCol(50, 50, 70);
    sf::Color wallEdge(70, 70, 95);

    // top wall
    sf::RectangleShape top(sf::Vector2f(800.f, w));
    top.setFillColor(wallCol);
    top.setOutlineColor(wallEdge);
    top.setOutlineThickness(1.f);
    window.draw(top);

    // bottom wall
    sf::RectangleShape bot(sf::Vector2f(800.f, w));
    bot.setPosition(0.f, 600.f - w);
    bot.setFillColor(wallCol);
    bot.setOutlineColor(wallEdge);
    bot.setOutlineThickness(1.f);
    window.draw(bot);

    // left wall
    sf::RectangleShape left(sf::Vector2f(w, 600.f));
    left.setFillColor(wallCol);
    left.setOutlineColor(wallEdge);
    left.setOutlineThickness(1.f);
    window.draw(left);

    // right wall
    sf::RectangleShape right(sf::Vector2f(w, 600.f));
    right.setPosition(800.f - w, 0.f);
    right.setFillColor(wallCol);
    right.setOutlineColor(wallEdge);
    right.setOutlineThickness(1.f);
    window.draw(right);

    // corner accents
    float cornerSize = w + 4.f;
    sf::Color cornerCol(60, 60, 85);
    sf::RectangleShape corners[4];
    for (int i = 0; i < 4; i++)
    {
        corners[i].setSize(sf::Vector2f(cornerSize, cornerSize));
        corners[i].setFillColor(cornerCol);
    }
    corners[0].setPosition(0.f, 0.f);
    corners[1].setPosition(800.f - cornerSize, 0.f);
    corners[2].setPosition(0.f, 600.f - cornerSize);
    corners[3].setPosition(800.f - cornerSize, 600.f - cornerSize);
    for (int i = 0; i < 4; i++)
        window.draw(corners[i]);
}

void Room::drawDoors(sf::RenderWindow& window)
{
    float doorW = 60.f;
    float wallThick = 16.f;

    sf::Color closedCol(80, 40, 40);
    sf::Color openCol(40, 60, 40);
    sf::Color openBright(60, 180, 60);

    struct DoorDef { float x, y, w, h; };
    DoorDef defs[4] = {
        { 400.f - doorW / 2.f, 0.f,              doorW, wallThick },
        { 800.f - wallThick,   300.f - doorW / 2.f, wallThick, doorW },
        { 400.f - doorW / 2.f, 600.f - wallThick, doorW, wallThick },
        { 0.f,                 300.f - doorW / 2.f, wallThick, doorW },
    };

    for (int i = 0; i < 4; i++)
    {
        if (!doors[i]) continue;

        bool isOpen = doorOpen(i);

        // cut hole in wall for door
        sf::RectangleShape hole(sf::Vector2f(defs[i].w + 4.f, defs[i].h + 4.f));
        hole.setPosition(defs[i].x - 2.f, defs[i].y - 2.f);
        hole.setFillColor(isOpen ? sf::Color(18, 18, 28) : sf::Color(28, 18, 18));
        window.draw(hole);

        // door frame
        sf::RectangleShape door(sf::Vector2f(defs[i].w, defs[i].h));
        door.setPosition(defs[i].x, defs[i].y);
        door.setFillColor(isOpen ? openCol : closedCol);
        window.draw(door);

        if (isOpen)
        {
            // arrow indicator pointing outward
            float cx = defs[i].x + defs[i].w / 2.f;
            float cy = defs[i].y + defs[i].h / 2.f;

            sf::CircleShape arrow(5.f, 3);
            arrow.setOrigin(5.f, 5.f);
            arrow.setPosition(cx, cy);
            arrow.setFillColor(openBright);

            float rot = 0.f;
            switch (i) { case 0: rot = 0.f; break; case 1: rot = 90.f; break; case 2: rot = 180.f; break; case 3: rot = 270.f; break; }
            arrow.setRotation(rot);
            window.draw(arrow);
        }
    }
}

void Room::drawShop(sf::RenderWindow& window, const sf::Font& font)
{
    if (roomType != RoomType::Shop) return;

    sf::Text title;
    title.setFont(font);
    title.setCharacterSize(28);
    title.setFillColor(sf::Color(255, 220, 80));
    title.setString("WEAPON SHOP");
    sf::FloatRect b = title.getLocalBounds();
    title.setPosition(400.f - b.width / 2.f, 60.f);
    window.draw(title);

    sf::Text hint;
    hint.setFont(font);
    hint.setCharacterSize(12);
    hint.setFillColor(sf::Color(140, 140, 160));
    hint.setString("Buy a weapon - press E to use its special move");
    b = hint.getLocalBounds();
    hint.setPosition(400.f - b.width / 2.f, 95.f);
    window.draw(hint);

    for (size_t i = 0; i < shopItems.size(); i++)
    {
        auto& item = shopItems[i];

        sf::RectangleShape card(sf::Vector2f(160.f, 220.f));
        card.setPosition(item.shopX - 80.f, item.shopY);
        card.setFillColor(item.sold ? sf::Color(20, 20, 30) : sf::Color(30, 30, 50));
        card.setOutlineColor(item.sold ? sf::Color(40, 40, 50) : sf::Color(255, 220, 80));
        card.setOutlineThickness(item.sold ? 1.f : 2.f);
        window.draw(card);

        if (item.sold)
        {
            sf::Text soldTxt;
            soldTxt.setFont(font);
            soldTxt.setCharacterSize(18);
            soldTxt.setFillColor(sf::Color(80, 80, 80));
            soldTxt.setString("SOLD");
            b = soldTxt.getLocalBounds();
            soldTxt.setPosition(item.shopX - b.width / 2.f, item.shopY + 90.f);
            window.draw(soldTxt);
            continue;
        }

        Item::drawIcon(window, item.type, item.shopX, item.shopY + 50.f, 2.f);

        sf::Text nameTxt;
        nameTxt.setFont(font);
        nameTxt.setCharacterSize(15);
        nameTxt.setFillColor(sf::Color::White);
        nameTxt.setString(item.name);
        b = nameTxt.getLocalBounds();
        nameTxt.setPosition(item.shopX - b.width / 2.f, item.shopY + 100.f);
        window.draw(nameTxt);

        sf::Text descTxt;
        descTxt.setFont(font);
        descTxt.setCharacterSize(11);
        descTxt.setFillColor(sf::Color(160, 160, 180));
        descTxt.setString(item.desc);
        b = descTxt.getLocalBounds();
        descTxt.setPosition(item.shopX - b.width / 2.f, item.shopY + 125.f);
        window.draw(descTxt);

        sf::Text costTxt;
        costTxt.setFont(font);
        costTxt.setCharacterSize(16);
        costTxt.setFillColor(sf::Color(255, 220, 80));
        costTxt.setString(std::to_string(item.cost) + " coins");
        b = costTxt.getLocalBounds();
        costTxt.setPosition(item.shopX - b.width / 2.f, item.shopY + 150.f);
        window.draw(costTxt);

        sf::Text keyTxt;
        keyTxt.setFont(font);
        keyTxt.setCharacterSize(16);
        keyTxt.setFillColor(sf::Color(120, 200, 120));
        keyTxt.setString("[" + std::to_string(i + 1) + "]");
        b = keyTxt.getLocalBounds();
        keyTxt.setPosition(item.shopX - b.width / 2.f, item.shopY + 180.f);
        window.draw(keyTxt);
    }
}

bool Room::isCleared() const { return cleared; }
bool Room::isBossRoom() const { return roomType == RoomType::Boss; }
bool Room::isShopRoom() const { return roomType == RoomType::Shop; }
RoomType Room::getType() const { return roomType; }
bool Room::isVisited() const { return visited; }
void Room::markVisited() { visited = true; }

std::vector<Enemy>& Room::getEnemies() { return enemies; }
std::vector<Item>& Room::getShopItems() { return shopItems; }

void Room::setDoor(int direction, bool exists) { doors[direction] = exists; }
bool Room::hasDoor(int direction) const { return doors[direction]; }
bool Room::doorOpen(int direction) const { return doors[direction] && cleared; }
