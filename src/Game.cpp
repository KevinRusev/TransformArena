#include "Game.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

Game::Game(sf::RenderWindow& win)
    : window(win)
    , player(400.f, 300.f)
    , mapSize(5)
    , currentRoomX(2), currentRoomY(2)
    , currentFloor(1), totalFloors(3)
    , state(GameState::Title)
    , score(0), coins(0), scoreMultiplier(1), multiplierTimer(0.f)
    , damageCooldown(0.f), totalKills(0), playTime(0.f)
    , armor(3), maxArmor(3), armorRegenTimer(0.f)
    , shakeIntensity(0.f), shakeTimer(0.f), shakeOffset(0.f, 0.f)
    , transitionTimer(0.f), transitionDir(-1)
    , bossAlive(false), bossIntroTimer(0.f)
    , portalActive(false), portalPulse(0.f), portalPos(400.f, 300.f)
    , floorFadeTimer(0.f), floorFadeDir(0.f)
    , choosingBuff(false), buffChoiceTimer(0.f)
    , barrierTimer(0.f)
    , paused(false)
    , deathSlowTimer(0.f)
    , hasContinue(false), titleSelection(0)
    , fontLoaded(false)
{
    std::srand((unsigned int)std::time(nullptr));

    if (font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
        fontLoaded = true;
    else if (font.loadFromFile("assets/font.ttf"))
        fontLoaded = true;

    sfx.setVolume(SoundSystem::HIT, 40.f);
    sfx.setVolume(SoundSystem::DASH, 50.f);
    sfx.setVolume(SoundSystem::SHOOT, 35.f);
    sfx.setVolume(SoundSystem::SLAM, 55.f);
    sfx.setVolume(SoundSystem::COIN, 30.f);
    sfx.setVolume(SoundSystem::BOSS_HIT, 65.f);
    sfx.setVolume(SoundSystem::PORTAL, 50.f);
    sfx.setVolume(SoundSystem::DEATH, 70.f);
    sfx.setVolume(SoundSystem::TRANSFORM, 40.f);
    sfx.setVolume(SoundSystem::BUY, 45.f);

    saveData.load();
    hasContinue = saveData.hasRun;
}

void Game::generateFloor()
{
    mapSize = 5 + (currentFloor - 1);
    if (mapSize > MAX_MAP) mapSize = MAX_MAP;

    for (int x = 0; x < MAX_MAP; x++)
        for (int y = 0; y < MAX_MAP; y++)
            rooms[x][y] = Room();

    int startX = mapSize / 2, startY = mapSize / 2;
    currentRoomX = startX;
    currentRoomY = startY;

    // track which grid cells are occupied
    bool used[MAX_MAP][MAX_MAP] = {};

    std::vector<std::pair<int,int>> path;
    path.push_back({startX, startY});
    used[startX][startY] = true;
    rooms[startX][startY].gridX = startX;
    rooms[startX][startY].gridY = startY;
    rooms[startX][startY].generate(currentFloor, RoomType::Start);

    int roomCount = 4 + currentFloor;
    int dxDir[] = {0, 1, 0, -1};
    int dyDir[] = {-1, 0, 1, 0};

    int cx = startX, cy = startY;
    int shopPlaced = -1;
    int shopTarget = roomCount / 2;
    int failCount = 0;

    for (int i = 0; i < roomCount; i++)
    {
        // check if current position has any free neighbor
        bool hasNeighbor = false;
        for (int d = 0; d < 4; d++)
        {
            int nx = cx + dxDir[d], ny = cy + dyDir[d];
            if (nx >= 0 && nx < mapSize && ny >= 0 && ny < mapSize && !used[nx][ny])
            { hasNeighbor = true; break; }
        }

        if (!hasNeighbor)
        {
            // backtrack along path until we find a room with free neighbors
            bool found = false;
            for (int back = (int)path.size() - 2; back >= 0; back--)
            {
                int bx = path[back].first, by = path[back].second;
                for (int d = 0; d < 4; d++)
                {
                    int nx = bx + dxDir[d], ny = by + dyDir[d];
                    if (nx >= 0 && nx < mapSize && ny >= 0 && ny < mapSize && !used[nx][ny])
                    { found = true; break; }
                }
                if (found) { cx = bx; cy = by; break; }
            }
            if (!found) break;
            i--;
            failCount++;
            if (failCount > 50) break;
            continue;
        }

        // pick random free neighbor
        bool placed = false;
        bool isBossRoom = (i == roomCount - 1);
        int minBossDist = 3;

        for (int attempts = 0; attempts < 40; attempts++)
        {
            // bias direction away from start for straighter paths
            int dir;
            if (attempts < 10 && i > 0)
            {
                int bestDir = -1;
                int bestDist = -1;
                for (int d = 0; d < 4; d++)
                {
                    int tx = cx + dxDir[d], ty = cy + dyDir[d];
                    if (tx < 0 || tx >= mapSize || ty < 0 || ty >= mapSize || used[tx][ty]) continue;
                    int md = std::abs(tx - startX) + std::abs(ty - startY);
                    if (md > bestDist) { bestDist = md; bestDir = d; }
                }
                dir = (bestDir >= 0 && std::rand() % 3 != 0) ? bestDir : std::rand() % 4;
            }
            else
                dir = std::rand() % 4;

            int nx = cx + dxDir[dir], ny = cy + dyDir[dir];
            if (nx < 0 || nx >= mapSize || ny < 0 || ny >= mapSize) continue;
            if (used[nx][ny]) continue;

            // boss must be far from start (relax after 20 tries)
            if (isBossRoom && attempts < 20)
            {
                int md = std::abs(nx - startX) + std::abs(ny - startY);
                if (md < minBossDist) continue;
            }

            rooms[cx][cy].setDoor(dir, true);
            rooms[nx][ny].setDoor((dir + 2) % 4, true);

            RoomType type = RoomType::Normal;
            if (isBossRoom)
                type = RoomType::Boss;
            else if (i == shopTarget && shopPlaced < 0)
            {
                type = RoomType::Shop;
                shopPlaced = (int)path.size();
            }

            rooms[nx][ny].gridX = nx;
            rooms[nx][ny].gridY = ny;
            rooms[nx][ny].generate(currentFloor, type);
            used[nx][ny] = true;

            path.push_back({nx, ny});
            cx = nx;
            cy = ny;
            placed = true;
            break;
        }

        if (!placed)
        {
            i--;
            failCount++;
            if (failCount > 50) break;
        }
    }

    // branch rooms off main path (not off boss room)
    for (int b = 0; b < 1 + currentFloor; b++)
    {
        bool branchPlaced = false;
        for (int tries = 0; tries < 20 && !branchPlaced; tries++)
        {
            int idx = std::rand() % std::max(1, (int)path.size() - 1);
            int bx = path[idx].first, by = path[idx].second;
            int dir = std::rand() % 4;
            int nx = bx + dxDir[dir], ny = by + dyDir[dir];
            if (nx >= 0 && nx < mapSize && ny >= 0 && ny < mapSize && !used[nx][ny])
            {
                rooms[bx][by].setDoor(dir, true);
                rooms[nx][ny].setDoor((dir + 2) % 4, true);
                rooms[nx][ny].gridX = nx;
                rooms[nx][ny].gridY = ny;
                rooms[nx][ny].generate(currentFloor, RoomType::Normal);
                used[nx][ny] = true;
                path.push_back({nx, ny});
                branchPlaced = true;
            }
        }
    }

    // connect all adjacent rooms with doors so the map feels connected
    for (int x = 0; x < mapSize; x++)
    {
        for (int y = 0; y < mapSize; y++)
        {
            if (!used[x][y]) continue;
            // check right neighbor
            if (x + 1 < mapSize && used[x + 1][y])
            {
                rooms[x][y].setDoor(1, true);      // right
                rooms[x + 1][y].setDoor(3, true);  // left
            }
            // check down neighbor
            if (y + 1 < mapSize && used[x][y + 1])
            {
                rooms[x][y].setDoor(2, true);      // down
                rooms[x][y + 1].setDoor(0, true);  // up
            }
        }
    }

    // ensure shop exists
    if (shopPlaced < 0)
    {
        for (size_t i = 1; i < path.size() - 1; i++)
        {
            auto& p = path[i];
            Room& r = rooms[p.first][p.second];
            if (r.getType() == RoomType::Normal)
            {
                r.generate(currentFloor, RoomType::Shop);
                shopPlaced = (int)i;
                break;
            }
        }
    }

    rooms[startX][startY].markVisited();
    player.reset(400.f, 300.f);

    float spd = 1.f, dmg = 1.f, cd = 1.f;
    for (auto& buff : ownedBuffs)
    {
        spd += buff.speedBonus;
        dmg += buff.damageBonus;
        cd -= buff.cooldownReduction;
    }
    if (cd < 0.3f) cd = 0.3f;
    player.setSpeedMultiplier(spd);
    player.setDamageMultiplier(dmg);
    player.setCooldownMultiplier(cd);
}

Room& Game::currentRoom()
{
    return rooms[currentRoomX][currentRoomY];
}

void Game::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::MouseWheelScrolled && state == GameState::Playing && !choosingBuff && !paused)
    {
        int dir = (event.mouseWheelScroll.delta > 0) ? 1 : -1;
        int f = (int)player.getForm();
        f = (f + dir + 3) % 3;
        player.transform((Form)f);
        if (player.justTransformed())
        {
            sfx.play(SoundSystem::TRANSFORM);
            sf::Color c;
            switch (player.getForm())
            {
            case Form::Circle:   c = sf::Color(80, 180, 255); break;
            case Form::Triangle: c = sf::Color(255, 160, 40);  break;
            case Form::Square:   c = sf::Color(80, 210, 80);   break;
            }
            spawnParticles(player.getPosition(), c, 20, 200.f, 5.f);
        }
        return;
    }

    if (event.type != sf::Event::KeyPressed)
        return;

    if (event.key.code == sf::Keyboard::Escape && state == GameState::Playing && !choosingBuff)
    {
        paused = !paused;
        return;
    }

    if (paused) return;

    switch (state)
    {
    case GameState::Title:
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W)
        {
            if (hasContinue) titleSelection = (titleSelection == 0) ? 1 : 0;
        }
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
        {
            if (hasContinue) titleSelection = (titleSelection == 0) ? 1 : 0;
        }
        else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space)
        {
            if (titleSelection == 1 && hasContinue)
            {
                loadGame();
                state = GameState::Playing;
            }
            else
            {
                state = GameState::Playing;
                restart();
            }
        }
        break;

    case GameState::Playing:
        if (choosingBuff)
        {
            if (event.key.code == sf::Keyboard::Num1 && buffChoices.size() > 0)
            { applyBuff(buffChoices[0]); choosingBuff = false; floorFadeTimer = 1.5f; floorFadeDir = 1.f; }
            else if (event.key.code == sf::Keyboard::Num2 && buffChoices.size() > 1)
            { applyBuff(buffChoices[1]); choosingBuff = false; floorFadeTimer = 1.5f; floorFadeDir = 1.f; }
            else if (event.key.code == sf::Keyboard::Num3 && buffChoices.size() > 2)
            { applyBuff(buffChoices[2]); choosingBuff = false; floorFadeTimer = 1.5f; floorFadeDir = 1.f; }
            return;
        }

        // shop purchases
        if (currentRoom().isShopRoom())
        {
            if (event.key.code == sf::Keyboard::Num1)
                tryBuyShopItem(0);
            else if (event.key.code == sf::Keyboard::Num2)
                tryBuyShopItem(1);
            else if (event.key.code == sf::Keyboard::Num3)
                tryBuyShopItem(2);
        }

        if (event.key.code == sf::Keyboard::E)
            activateItem();
        else if (event.key.code == sf::Keyboard::Num1 && !currentRoom().isShopRoom())
            player.transform(Form::Circle);
        else if (event.key.code == sf::Keyboard::Num2 && !currentRoom().isShopRoom())
            player.transform(Form::Triangle);
        else if (event.key.code == sf::Keyboard::Num3 && !currentRoom().isShopRoom())
            player.transform(Form::Square);
        else if (event.key.code == sf::Keyboard::Space)
        {
            float prevCd = player.getCooldownPercent();
            player.useAbility();
            if (player.getCooldownPercent() < prevCd)
            {
                if (player.getForm() == Form::Circle)
                    sfx.play(SoundSystem::DASH);
                else if (player.getForm() == Form::Triangle)
                    sfx.play(SoundSystem::SHOOT);
                else if (player.getForm() == Form::Square)
                    sfx.play(SoundSystem::SLAM);
            }
        }

        if (player.justTransformed())
        {
            sfx.play(SoundSystem::TRANSFORM);
            sf::Color c;
            switch (player.getForm())
            {
            case Form::Circle:   c = sf::Color(80, 180, 255); break;
            case Form::Triangle: c = sf::Color(255, 160, 40);  break;
            case Form::Square:   c = sf::Color(80, 210, 80);   break;
            }
            spawnParticles(player.getPosition(), c, 20, 200.f, 5.f);
        }
        break;

    case GameState::GameOver:
        if (event.key.code == sf::Keyboard::R)
        {
            state = GameState::Playing;
            restart();
        }
        break;

    case GameState::BossIntro:
        break;
    }
}

void Game::tryBuyShopItem(int index)
{
    auto& items = currentRoom().getShopItems();
    if (index < 0 || index >= (int)items.size()) return;
    auto& item = items[index];
    if (item.sold || coins < item.cost) return;

    coins -= item.cost;
    item.sold = true;

    equippedItem = item;
    equippedItem.cooldownTimer = 0.f;

    sfx.play(SoundSystem::BUY);
    spawnParticles(player.getPosition(), sf::Color(255, 220, 80), 20, 180.f, 5.f);
}

void Game::update(float dt)
{
    if (paused) return;

    float realDt = dt;
    if (deathSlowTimer > 0.f)
        dt *= 0.2f;

    if (shakeTimer > 0.f)
    {
        shakeTimer -= dt;
        float t = shakeTimer / 0.3f;
        shakeOffset.x = ((float)(std::rand() % 100) / 100.f - 0.5f) * shakeIntensity * t * 2.f;
        shakeOffset.y = ((float)(std::rand() % 100) / 100.f - 0.5f) * shakeIntensity * t * 2.f;
    }
    else
        shakeOffset = sf::Vector2f(0.f, 0.f);

    if (state == GameState::BossIntro)
    {
        bossIntroTimer -= dt;
        if (bossIntroTimer <= 0.f)
            state = GameState::Playing;
        return;
    }

    if (choosingBuff)
    {
        buffChoiceTimer += dt;
        return;
    }

    if (state != GameState::Playing)
        return;

    if (transitionTimer > 0.f)
    {
        transitionTimer -= dt;
        return;
    }

    if (floorFadeTimer > 0.f)
    {
        floorFadeTimer -= dt;
        if (floorFadeDir > 0.f && floorFadeTimer <= 0.75f)
        {
            floorFadeDir = -1.f;
            portalActive = false;
            nextFloor();
        }
        if (floorFadeTimer <= 0.f)
        {
            floorFadeTimer = 0.f;
            floorFadeDir = 0.f;
        }
        return;
    }

    playTime += dt;
    player.handleInput(dt);

    // clamp player inside walls (16px thick)
    {
        sf::Vector2f pp = player.getPosition();
        float pr = player.getRadius();
        float wall = 18.f;
        if (pp.x - pr < wall) player.setPosition(wall + pr, pp.y);
        if (pp.x + pr > 800.f - wall) player.setPosition(800.f - wall - pr, pp.y);
        pp = player.getPosition();
        if (pp.y - pr < wall) player.setPosition(pp.x, wall + pr);
        if (pp.y + pr > 600.f - wall) player.setPosition(pp.x, 600.f - wall - pr);
    }

    // but allow through doors
    {
        sf::Vector2f pp = player.getPosition();
        float pr = player.getRadius();
        for (int d = 0; d < 4; d++)
        {
            if (!currentRoom().doorOpen(d)) continue;
            // if near door zone, don't clamp
        }
    }

    if (player.getForm() == Form::Triangle)
    {
        sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);
        player.aimAt(mouseWorld);
    }

    player.update(dt);

    // armor regen
    if (armor < maxArmor)
    {
        armorRegenTimer += dt;
        if (armorRegenTimer > 3.f)
        {
            armor++;
            armorRegenTimer = 0.f;
        }
    }

    if (equippedItem.cooldownTimer > 0.f)
        equippedItem.cooldownTimer -= dt;

    if (barrierTimer > 0.f)
        barrierTimer -= dt;

    if (player.wantsToShoot())
    {
        sf::Vector2f pos = player.getPosition();
        sf::Vector2f dir = player.getFacing();
        float bulletSpeed = 500.f;
        float bulletDmg = 20.f * player.getDamageMultiplier();
        sf::Vector2f vel(dir.x * bulletSpeed, dir.y * bulletSpeed);
        projectiles.emplace_back(pos, vel, 4.f, bulletDmg, true, sf::Color(255, 200, 60));
    }

    currentRoom().update(dt, player.getPosition(), projectiles);

    for (auto& proj : projectiles)
        proj.update(dt);

    for (auto& p : particles)
        p.update(dt);

    for (auto& hp : pickups)
        hp.update(dt);

    for (auto& dn : dmgNumbers)
        dn.update(dt);

    checkCollisions();

    // Hive boss minion spawning
    {
        auto& en = currentRoom().getEnemies();
        for (auto& enemy : en)
        {
            if (!enemy.isAlive() || !enemy.isBoss() || enemy.getBossType() != 3) continue;

            // count alive non-boss enemies
            int minionCount = 0;
            for (auto& e2 : en)
                if (e2.isAlive() && !e2.isBoss()) minionCount++;

            int maxMinions = enemy.getHealth() / enemy.getMaxHealth() < 0.5f ? 6 : 4;
            if (minionCount < maxMinions)
            {
                float angle = (float)(std::rand() % 628) / 100.f;
                float r = enemy.getRadius() + 40.f;
                float sx = enemy.getPosition().x + std::cos(angle) * r;
                float sy = enemy.getPosition().y + std::sin(angle) * r;
                if (sx < 40.f) sx = 40.f; if (sx > 760.f) sx = 760.f;
                if (sy < 40.f) sy = 40.f; if (sy > 560.f) sy = 560.f;

                EnemyType mt = (std::rand() % 2 == 0) ? EnemyType::Chaser : EnemyType::Dasher;
                en.emplace_back(sx, sy, mt);
                spawnParticles(sf::Vector2f(sx, sy), sf::Color(255, 180, 60), 6, 80.f, 2.f);
            }
        }
    }

    auto& enemies = currentRoom().getEnemies();
    for (auto& enemy : enemies)
    {
        if (!enemy.isAlive())
        {
            spawnDeathParticles(enemy.getPosition(), enemy.getType());
            totalKills++;

            int coinDrop = 1 + std::rand() % 2;
            if (enemy.isBoss()) coinDrop = 10 + currentFloor * 3;
            coins += coinDrop;
            sfx.play(SoundSystem::COIN);

            int points = enemy.isBoss() ? 100 : 10;
            if (isEffectiveForm(player.getForm(), enemy.getType()))
            {
                points *= 2;
                scoreMultiplier = std::min(scoreMultiplier + 1, 8);
                multiplierTimer = 3.f;
            }
            score += points * scoreMultiplier;

            if (!enemy.isBoss() && std::rand() % 100 < 20)
                pickups.emplace_back(enemy.getPosition(), 15);

            if (enemy.isBoss())
            {
                pickups.emplace_back(enemy.getPosition(), 50);
                bossAlive = false;
            }
        }
    }

    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& e) { return !e.isAlive(); }),
        enemies.end());

    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& p) { return !p.isAlive(); }),
        projectiles.end());

    particles.erase(
        std::remove_if(particles.begin(), particles.end(), [](const Particle& p) { return !p.isAlive(); }),
        particles.end());

    pickups.erase(
        std::remove_if(pickups.begin(), pickups.end(), [](const HealthPickup& h) { return !h.isAlive(); }),
        pickups.end());

    dmgNumbers.erase(
        std::remove_if(dmgNumbers.begin(), dmgNumbers.end(), [](const DamageNumber& d) { return !d.isAlive(); }),
        dmgNumbers.end());

    if (multiplierTimer > 0.f)
    {
        multiplierTimer -= dt;
        if (multiplierTimer <= 0.f)
            scoreMultiplier = 1;
    }

    if (damageCooldown > 0.f)
        damageCooldown -= dt;

    if (currentRoom().isBossRoom() && currentRoom().isCleared() && !portalActive && !choosingBuff && !bossAlive)
    {
        portalActive = true;
        portalPulse = 0.f;
        portalPos = sf::Vector2f(400.f, 300.f);
    }

    if (portalActive && currentRoom().isBossRoom())
        portalPulse += dt;

    if (portalActive && currentRoom().isBossRoom() && !choosingBuff && dist(player.getPosition(), portalPos) < 35.f)
    {
        sfx.play(SoundSystem::PORTAL);
        if (currentFloor >= totalFloors)
        {
            state = GameState::GameOver;
            score += 1000 * currentFloor;
            saveData.score = score;
            saveData.totalKills = totalKills;
            saveData.floor = currentFloor;
            updateHighScores();
            portalActive = false;
        }
        else
        {
            generateBuffChoices();
            choosingBuff = true;
            buffChoiceTimer = 0.f;
        }
    }

    checkDoorTransition();

    if (!player.isAlive() && deathSlowTimer <= 0.f && state != GameState::GameOver)
    {
        sfx.play(SoundSystem::DEATH);
        addScreenShake(12.f, 0.4f);
        spawnParticles(player.getPosition(), sf::Color::White, 30, 250.f, 6.f);
        deathSlowTimer = 1.2f;
    }

    if (deathSlowTimer > 0.f)
    {
        deathSlowTimer -= realDt;
        if (deathSlowTimer <= 0.f)
        {
            state = GameState::GameOver;
            saveData.score = score;
            saveData.totalKills = totalKills;
            saveData.floor = currentFloor;
            updateHighScores();
        }
    }
}

void Game::checkDoorTransition()
{
    if (!currentRoom().isCleared()) return;

    sf::Vector2f pp = player.getPosition();
    float pr = player.getRadius();
    int dir = -1;
    float doorZone = 80.f; // how close to center of edge the player needs to be

    if (pp.y - pr <= 20.f && currentRoom().doorOpen(0) && std::abs(pp.x - 400.f) < doorZone)
        dir = 0;
    else if (pp.x + pr >= 780.f && currentRoom().doorOpen(1) && std::abs(pp.y - 300.f) < doorZone)
        dir = 1;
    else if (pp.y + pr >= 580.f && currentRoom().doorOpen(2) && std::abs(pp.x - 400.f) < doorZone)
        dir = 2;
    else if (pp.x - pr <= 20.f && currentRoom().doorOpen(3) && std::abs(pp.y - 300.f) < doorZone)
        dir = 3;

    if (dir >= 0)
        transitionToRoom(dir);
}

void Game::transitionToRoom(int dir)
{
    int dxArr[] = {0, 1, 0, -1};
    int dyArr[] = {-1, 0, 1, 0};

    int nx = currentRoomX + dxArr[dir];
    int ny = currentRoomY + dyArr[dir];
    if (nx < 0 || nx >= mapSize || ny < 0 || ny >= mapSize) return;

    currentRoomX = nx;
    currentRoomY = ny;

    projectiles.clear();
    particles.clear();
    pickups.clear();
    dmgNumbers.clear();

    float px = 400.f, py = 300.f;
    switch (dir)
    {
    case 0: py = 550.f; break;
    case 1: px = 50.f;  break;
    case 2: py = 50.f;  break;
    case 3: px = 750.f; break;
    }
    player.setPosition(px, py);

    bool wasVisited = currentRoom().isVisited();
    currentRoom().markVisited();

    if (currentRoom().isBossRoom() && !wasVisited)
    {
        bossAlive = true;
        state = GameState::BossIntro;
        bossIntroTimer = 1.5f;
    }

    transitionTimer = 0.3f;
    transitionDir = dir;
}

void Game::checkCollisions()
{
    sf::Vector2f pp = player.getPosition();
    float pr = player.getRadius();
    auto& enemies = currentRoom().getEnemies();

    if (player.isDashing())
    {
        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive() || !enemy.canBeDashHit()) continue;
            if (dist(pp, enemy.getPosition()) < pr + enemy.getRadius() + 10.f)
            {
                float dmg = player.getDashDamage();
                bool effective = isEffectiveForm(player.getForm(), enemy.getType());
                if (effective) dmg *= 1.5f;

                // Phantom boss: dash deals bonus damage and briefly slows
                if (enemy.isBoss() && enemy.getBossType() == 2)
                {
                    dmg *= 2.5f;
                    enemy.stun(0.8f);
                    sfx.play(SoundSystem::BOSS_HIT);
                    spawnParticles(enemy.getPosition(), sf::Color(255, 255, 100), 20, 200.f, 4.f);
                    addScreenShake(6.f, 0.2f);
                }

                if (enemy.isBoss() && enemy.getBossType() == 1 && enemy.isShieldUp())
                {
                    dmgNumbers.emplace_back(sf::Vector2f(enemy.getPosition().x, enemy.getPosition().y - 30.f), "SHIELDED! Use SLAM [3]", sf::Color(255, 150, 255));
                    spawnParticles(enemy.getPosition(), sf::Color(180, 100, 255), 8, 100.f, 3.f);
                    enemy.markDashHit();
                }
                else
                {
                    enemy.takeDamage(dmg);
                    enemy.markDashHit();
                    sfx.play(SoundSystem::HIT);
                    dmgNumbers.emplace_back(enemy.getPosition(), (int)dmg, sf::Color(80, 180, 255));
                    if (effective)
                        dmgNumbers.emplace_back(sf::Vector2f(enemy.getPosition().x, enemy.getPosition().y - 20.f), "EFFECTIVE!", sf::Color(80, 255, 200));
                }
                spawnParticles(enemy.getPosition(), sf::Color(80, 180, 255), 12, 160.f, 4.f);
                spawnParticles(enemy.getPosition(), sf::Color(180, 230, 255), 6, 80.f, 2.f);
            }
        }
    }

    if (player.isGroundPounding() && player.consumePoundDamage())
    {
        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            float d = dist(pp, enemy.getPosition());
            if (d < player.getGroundPoundRadius() + enemy.getRadius())
            {
                // Guardian boss: slam breaks shield
                if (enemy.isBoss() && enemy.getBossType() == 1 && enemy.isShieldUp())
                {
                    enemy.breakShield(4.f);
                    sfx.play(SoundSystem::BOSS_HIT);
                    spawnParticles(enemy.getPosition(), sf::Color(180, 100, 255), 25, 250.f, 5.f);
                    addScreenShake(10.f, 0.3f);
                }

                // Shielder enemies: slam breaks their shield
                if (enemy.getType() == EnemyType::Shielder && enemy.isShieldUp())
                    enemy.breakShield(3.f);

                float slamDmg = player.getGroundPoundDamage();
                bool effective = isEffectiveForm(player.getForm(), enemy.getType());
                if (effective) slamDmg *= 1.5f;
                enemy.takeDamage(slamDmg);
                dmgNumbers.emplace_back(enemy.getPosition(), (int)slamDmg, sf::Color(80, 210, 80));
                if (effective)
                    dmgNumbers.emplace_back(sf::Vector2f(enemy.getPosition().x, enemy.getPosition().y - 20.f), "EFFECTIVE!", sf::Color(80, 255, 200));
                float knockback = 80.f + (player.getGroundPoundRadius() - d) * 0.5f;
                enemy.pushAway(pp, knockback);
                spawnParticles(enemy.getPosition(), sf::Color(80, 210, 80), 8, 140.f, 3.f);
            }
        }
        spawnParticles(pp, sf::Color(100, 255, 100), 20, 200.f, 4.f);
        spawnParticles(pp, sf::Color(200, 255, 200), 10, 100.f, 2.f);
        addScreenShake(8.f, 0.25f);
    }

    for (auto& proj : projectiles)
    {
        if (!proj.fromPlayer || !proj.isAlive()) continue;
        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            if (dist(proj.position, enemy.getPosition()) < proj.radius + enemy.getRadius())
            {
                float projDmg = proj.damage;
                bool effective = isEffectiveForm(player.getForm(), enemy.getType());
                if (effective) projDmg *= 1.5f;

                if (enemy.isBoss() && enemy.getBossType() == 1 && enemy.isShieldUp())
                {
                    dmgNumbers.emplace_back(sf::Vector2f(enemy.getPosition().x, enemy.getPosition().y - 30.f), "SHIELDED! Use SLAM [3]", sf::Color(255, 150, 255));
                    proj.lifetime = 0.f;
                    spawnParticles(proj.position, sf::Color(180, 100, 255), 6, 80.f, 2.f);
                }
                else
                {
                    enemy.takeDamage(projDmg);
                    sfx.play(SoundSystem::HIT);
                    dmgNumbers.emplace_back(enemy.getPosition(), (int)projDmg, sf::Color(255, 200, 60));
                    if (effective)
                        dmgNumbers.emplace_back(sf::Vector2f(enemy.getPosition().x, enemy.getPosition().y - 20.f), "EFFECTIVE!", sf::Color(80, 255, 200));
                    proj.lifetime = 0.f;
                    spawnParticles(proj.position, sf::Color(255, 200, 60), 8, 120.f, 3.f);
                }
                break;
            }
        }
    }

    for (auto& proj : projectiles)
    {
        if (proj.fromPlayer || !proj.isAlive()) continue;

        // barrier shield blocks enemy projectiles near the player
        if (barrierTimer > 0.f && dist(proj.position, pp) < pr + 50.f)
        {
            proj.lifetime = 0.f;
            spawnParticles(proj.position, sf::Color(100, 160, 255), 4, 80.f, 2.f);
            continue;
        }

        if (dist(proj.position, pp) < proj.radius + pr)
        {
            if (!player.isInvincible())
            {
                if (armor > 0)
                {
                    armor--;
                    armorRegenTimer = 0.f;
                }
                else
                    player.takeDamage((int)proj.damage);
                addScreenShake(4.f, 0.15f);
            }
            proj.lifetime = 0.f;
            spawnParticles(proj.position, sf::Color(255, 80, 80), 5, 100.f, 3.f);
        }
    }

    if (!player.isInvincible() && damageCooldown <= 0.f)
    {
        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            if (dist(pp, enemy.getPosition()) < pr + enemy.getRadius())
            {
                if (armor > 0)
                {
                    armor--;
                    armorRegenTimer = 0.f;
                }
                else
                    player.takeDamage(enemy.getContactDamage());
                damageCooldown = 0.4f;
                addScreenShake(3.f, 0.1f);
                break;
            }
        }
    }

    for (auto& hp : pickups)
    {
        if (!hp.isAlive()) continue;
        if (dist(pp, hp.position) < pr + 12.f)
        {
            player.heal(hp.healAmount);
            hp.lifetime = 0.f;
            spawnParticles(hp.position, sf::Color(80, 255, 80), 8, 100.f, 3.f);
        }
    }
}

void Game::draw()
{
    window.clear(sf::Color(10, 10, 16));

    sf::View view = window.getDefaultView();
    view.move(shakeOffset);

    if (deathSlowTimer > 0.f)
    {
        float progress = 1.f - (deathSlowTimer / 1.2f);
        float zoom = 1.f - progress * 0.3f;
        view.setSize(800.f * zoom, 600.f * zoom);
        sf::Vector2f center = view.getCenter();
        sf::Vector2f target = player.getPosition();
        view.setCenter(center + (target - center) * progress * 0.5f);
    }

    window.setView(view);

    currentRoom().drawFloor(window);
    currentRoom().drawWalls(window);
    currentRoom().drawDoors(window);

    for (auto& hp : pickups)
        hp.draw(window);

    for (auto& p : particles)
        p.draw(window);

    currentRoom().draw(window);

    if (portalActive && currentRoom().isBossRoom())
        drawPortal();

    for (auto& proj : projectiles)
        proj.draw(window);

    player.draw(window);

    if (barrierTimer > 0.f)
    {
        float alpha = std::min(1.f, barrierTimer) * 0.4f;
        float pr = player.getRadius();
        sf::CircleShape shield(pr + 30.f);
        shield.setOrigin(pr + 30.f, pr + 30.f);
        shield.setPosition(player.getPosition());
        shield.setFillColor(sf::Color(60, 120, 255, (sf::Uint8)(80 * alpha)));
        shield.setOutlineColor(sf::Color(100, 160, 255, (sf::Uint8)(200 * alpha)));
        shield.setOutlineThickness(2.f);
        window.draw(shield);
    }

    if (fontLoaded)
    {
        for (auto& dn : dmgNumbers)
            dn.draw(window, font);
    }

    if (fontLoaded && currentRoom().isShopRoom())
        currentRoom().drawShop(window, font);

    if (deathSlowTimer > 0.f)
    {
        float progress = 1.f - (deathSlowTimer / 1.2f);
        sf::RectangleShape fade(sf::Vector2f(800.f, 600.f));
        fade.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(progress * 120)));
        window.draw(fade);
    }

    window.setView(window.getDefaultView());

    drawHUD();
    drawMinimap();

    if (bossAlive)
        drawBossBar();

    if (state == GameState::Title)
        drawTitle();
    else if (state == GameState::GameOver)
        drawGameOver();
    else if (state == GameState::BossIntro && fontLoaded)
    {
        sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
        overlay.setFillColor(sf::Color(0, 0, 0, 120));
        window.draw(overlay);

        sf::Text txt;
        txt.setFont(font);
        txt.setCharacterSize(44);
        txt.setFillColor(sf::Color(220, 40, 40));
        std::string introName;
        std::string introHint;
        sf::Color introColor(220, 40, 40);
        switch (currentFloor)
        {
        case 1: introName = "THE GUARDIAN"; introHint = "Break its shield with SLAM [3]"; introColor = sf::Color(180, 80, 255); break;
        case 2: introName = "THE PHANTOM"; introHint = "DASH [1] deals bonus damage"; introColor = sf::Color(60, 220, 140); break;
        case 3: introName = "THE HIVE"; introHint = "Clear minions with SLAM, snipe the core"; introColor = sf::Color(255, 160, 40); break;
        default: introName = "BOSS"; introHint = ""; break;
        }
        txt.setString(introName);
        txt.setFillColor(introColor);
        sf::FloatRect b = txt.getLocalBounds();
        txt.setPosition(400.f - b.width / 2.f, 220.f);
        window.draw(txt);

        sf::Text hint;
        hint.setFont(font);
        hint.setCharacterSize(14);
        hint.setFillColor(sf::Color(200, 200, 220));
        hint.setString(introHint);
        b = hint.getLocalBounds();
        hint.setPosition(400.f - b.width / 2.f, 275.f);
        window.draw(hint);

        sf::Text sub;
        sub.setFont(font);
        sub.setCharacterSize(18);
        sub.setFillColor(sf::Color(180, 80, 80));
        sub.setString("Floor " + std::to_string(currentFloor));
        b = sub.getLocalBounds();
        sub.setPosition(400.f - b.width / 2.f, 295.f);
        window.draw(sub);
    }

    if (choosingBuff)
        drawBuffChoice();

    if (transitionTimer > 0.f)
        drawRoomTransition();

    if (floorFadeTimer > 0.f)
        drawFloorFade();

    if (paused && fontLoaded)
    {
        sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
        overlay.setFillColor(sf::Color(0, 0, 0, 160));
        window.draw(overlay);

        sf::Text pauseText;
        pauseText.setFont(font);
        pauseText.setCharacterSize(52);
        pauseText.setFillColor(sf::Color(240, 240, 255));
        pauseText.setString("PAUSED");
        sf::FloatRect pb = pauseText.getLocalBounds();
        pauseText.setPosition(400.f - pb.width / 2.f, 220.f);
        window.draw(pauseText);

        sf::Text hint;
        hint.setFont(font);
        hint.setCharacterSize(16);
        hint.setFillColor(sf::Color(160, 160, 180));
        hint.setString("Press ESC to resume");
        pb = hint.getLocalBounds();
        hint.setPosition(400.f - pb.width / 2.f, 290.f);
        window.draw(hint);
    }
}

void Game::drawMinimap()
{
    if (!fontLoaded) return;

    float cellSize = 14.f;
    float padding = 2.f;
    float startX = 800.f - (mapSize * (cellSize + padding)) - 10.f;
    float startY = 80.f;

    for (int x = 0; x < mapSize; x++)
    {
        for (int y = 0; y < mapSize; y++)
        {
            Room& r = rooms[x][y];
            bool hasRoom = false;
            for (int d = 0; d < 4; d++)
                if (r.hasDoor(d)) { hasRoom = true; break; }
            if (r.getType() == RoomType::Start) hasRoom = true;
            if (!hasRoom && !r.isVisited()) continue;

            float cx = startX + x * (cellSize + padding);
            float cy = startY + y * (cellSize + padding);

            sf::RectangleShape cell(sf::Vector2f(cellSize, cellSize));
            cell.setPosition(cx, cy);

            if (x == currentRoomX && y == currentRoomY)
                cell.setFillColor(sf::Color(255, 255, 255));
            else if (r.isBossRoom() && r.isVisited())
                cell.setFillColor(sf::Color(180, 30, 30));
            else if (r.isBossRoom())
                cell.setFillColor(sf::Color(80, 20, 20));
            else if (r.isShopRoom() && r.isVisited())
                cell.setFillColor(sf::Color(180, 160, 40));
            else if (r.isShopRoom())
                cell.setFillColor(sf::Color(60, 55, 20));
            else if (r.isCleared())
                cell.setFillColor(sf::Color(60, 120, 60));
            else if (r.isVisited())
                cell.setFillColor(sf::Color(100, 100, 140));
            else if (hasRoom)
                cell.setFillColor(sf::Color(35, 35, 48));
            else
                continue;

            cell.setOutlineColor(sf::Color(60, 60, 80));
            cell.setOutlineThickness(1.f);
            window.draw(cell);

            // boss skull icon
            if (r.isBossRoom())
            {
                sf::Text icon;
                icon.setFont(font);
                icon.setCharacterSize(9);
                icon.setFillColor(sf::Color(220, 40, 40));
                icon.setString("B");
                icon.setPosition(cx + 3.f, cy + 1.f);
                window.draw(icon);
            }

            // shop coin icon
            if (r.isShopRoom())
            {
                sf::Text icon;
                icon.setFont(font);
                icon.setCharacterSize(9);
                icon.setFillColor(sf::Color(255, 220, 80));
                icon.setString("$");
                icon.setPosition(cx + 3.f, cy + 1.f);
                window.draw(icon);
            }
        }
    }
}

void Game::drawBossBar()
{
    if (!fontLoaded) return;

    auto& enemies = currentRoom().getEnemies();
    float totalHp = 0.f, totalMaxHp = 0.f;
    for (auto& e : enemies)
    {
        if (e.isBoss() && e.isAlive())
        {
            totalHp += e.getHealth();
            totalMaxHp += e.getMaxHealth();
        }
    }
    if (totalMaxHp <= 0.f) return;

    float barW = 400.f, barH = 12.f;
    float barX = 200.f, barY = 580.f;
    float pct = totalHp / totalMaxHp;

    sf::RectangleShape bg(sf::Vector2f(barW, barH));
    bg.setPosition(barX, barY);
    bg.setFillColor(sf::Color(40, 20, 20));
    window.draw(bg);

    sf::RectangleShape bar(sf::Vector2f(barW * pct, barH));
    bar.setPosition(barX, barY);
    bar.setFillColor(sf::Color(200, 30, 30));
    window.draw(bar);

    sf::RectangleShape border(sf::Vector2f(barW, barH));
    border.setPosition(barX, barY);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(200, 200, 200));
    border.setOutlineThickness(1.f);
    window.draw(border);

    sf::Text labelTxt;
    labelTxt.setFont(font);
    labelTxt.setCharacterSize(12);
    labelTxt.setFillColor(sf::Color(220, 40, 40));
    std::string bossName;
    switch (currentFloor)
    {
    case 1: bossName = "THE GUARDIAN"; break;
    case 2: bossName = "THE PHANTOM"; break;
    case 3: bossName = "THE HIVE"; break;
    default: bossName = "BOSS"; break;
    }
    labelTxt.setString(bossName);
    sf::FloatRect lb = labelTxt.getLocalBounds();
    labelTxt.setPosition(400.f - lb.width / 2.f, barY - 16.f);
    window.draw(labelTxt);
}

void Game::drawHUD()
{
    if (!fontLoaded) return;

    // health bar
    float barW = 200.f, barH = 14.f;
    float hp = (float)player.getHealth() / player.getMaxHealth();
    if (hp < 0.f) hp = 0.f;

    sf::RectangleShape hpBg(sf::Vector2f(barW, barH));
    hpBg.setPosition(15.f, 15.f);
    hpBg.setFillColor(sf::Color(40, 40, 40));
    window.draw(hpBg);

    sf::RectangleShape hpBar(sf::Vector2f(barW * hp, barH));
    hpBar.setPosition(15.f, 15.f);
    sf::Color hpCol((sf::Uint8)(255 * (1.f - hp)), (sf::Uint8)(200 * hp), 50);
    hpBar.setFillColor(hpCol);
    window.draw(hpBar);

    sf::RectangleShape hpBorder(sf::Vector2f(barW, barH));
    hpBorder.setPosition(15.f, 15.f);
    hpBorder.setFillColor(sf::Color::Transparent);
    hpBorder.setOutlineColor(sf::Color(200, 200, 200));
    hpBorder.setOutlineThickness(1.f);
    window.draw(hpBorder);

    // armor pips
    for (int i = 0; i < maxArmor; i++)
    {
        sf::RectangleShape pip(sf::Vector2f(12.f, 6.f));
        pip.setPosition(15.f + i * 16.f, 32.f);
        pip.setFillColor(i < armor ? sf::Color(100, 150, 255) : sf::Color(30, 30, 40));
        pip.setOutlineColor(sf::Color(80, 80, 120));
        pip.setOutlineThickness(1.f);
        window.draw(pip);
    }

    // score and coins
    sf::Text scoreTxt;
    scoreTxt.setFont(font);
    scoreTxt.setCharacterSize(16);
    scoreTxt.setFillColor(sf::Color::White);
    std::string scoreStr = "Score: " + std::to_string(score);
    if (scoreMultiplier > 1)
        scoreStr += "  x" + std::to_string(scoreMultiplier);
    scoreTxt.setString(scoreStr);
    scoreTxt.setPosition(15.f, 42.f);
    window.draw(scoreTxt);

    sf::Text coinTxt;
    coinTxt.setFont(font);
    coinTxt.setCharacterSize(16);
    coinTxt.setFillColor(sf::Color(255, 220, 80));
    coinTxt.setString("Coins: " + std::to_string(coins));
    coinTxt.setPosition(15.f, 60.f);
    window.draw(coinTxt);

    // floor indicator
    sf::Text floorTxt;
    floorTxt.setFont(font);
    floorTxt.setCharacterSize(14);
    floorTxt.setFillColor(sf::Color(140, 140, 160));
    floorTxt.setString("Floor " + std::to_string(currentFloor) + "/" + std::to_string(totalFloors));
    floorTxt.setPosition(220.f, 15.f);
    window.draw(floorTxt);

    // form indicator
    std::string formName;
    std::string abilityDesc;
    sf::Color formColor;
    switch (player.getForm())
    {
    case Form::Circle:
        formName = "[1] CIRCLE";
        abilityDesc = "Space: Dash";
        formColor = sf::Color(80, 180, 255);
        break;
    case Form::Triangle:
        formName = "[2] TRIANGLE";
        abilityDesc = "Space: Shoot (mouse aim)";
        formColor = sf::Color(255, 160, 40);
        break;
    case Form::Square:
        formName = "[3] SQUARE";
        abilityDesc = "Space: Slam";
        formColor = sf::Color(80, 210, 80);
        break;
    }

    sf::Text formTxt;
    formTxt.setFont(font);
    formTxt.setCharacterSize(17);
    formTxt.setFillColor(formColor);
    formTxt.setString(formName);
    sf::FloatRect formBounds = formTxt.getLocalBounds();
    formTxt.setPosition(785.f - formBounds.width, 15.f);
    window.draw(formTxt);

    sf::Text abilTxt;
    abilTxt.setFont(font);
    abilTxt.setCharacterSize(13);
    abilTxt.setFillColor(sf::Color(180, 180, 180));
    abilTxt.setString(abilityDesc);
    sf::FloatRect abilBounds = abilTxt.getLocalBounds();
    abilTxt.setPosition(785.f - abilBounds.width, 36.f);
    window.draw(abilTxt);

    // cooldown bar
    float cdBarW = 80.f, cdBarH = 4.f;
    float cdPct = player.getCooldownPercent();
    sf::RectangleShape cdBg(sf::Vector2f(cdBarW, cdBarH));
    cdBg.setPosition(660.f, 55.f);
    cdBg.setFillColor(sf::Color(40, 40, 40));
    window.draw(cdBg);
    sf::RectangleShape cdBar(sf::Vector2f(cdBarW * cdPct, cdBarH));
    cdBar.setPosition(660.f, 55.f);
    cdBar.setFillColor(cdPct >= 1.f ? sf::Color(255, 255, 255) : formColor);
    window.draw(cdBar);

    // equipped item display
    if (equippedItem.type != ItemType::None)
    {
        sf::RectangleShape itemBg(sf::Vector2f(36.f, 36.f));
        itemBg.setPosition(660.f, 64.f);
        itemBg.setFillColor(sf::Color(30, 30, 50));
        itemBg.setOutlineColor(sf::Color(120, 120, 180));
        itemBg.setOutlineThickness(1.5f);
        window.draw(itemBg);

        Item::drawIcon(window, equippedItem.type, 678.f, 82.f, 1.f);

        // item cooldown overlay
        float itemCdPct = equippedItem.getCooldownPercent();
        if (itemCdPct < 1.f)
        {
            float overlayH = 36.f * (1.f - itemCdPct);
            sf::RectangleShape cdOverlay(sf::Vector2f(36.f, overlayH));
            cdOverlay.setPosition(660.f, 64.f);
            cdOverlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(cdOverlay);
        }

        sf::Text itemKey;
        itemKey.setFont(font);
        itemKey.setCharacterSize(10);
        itemKey.setFillColor(equippedItem.isReady() ? sf::Color(120, 255, 120) : sf::Color(120, 120, 140));
        itemKey.setString("[E]");
        itemKey.setPosition(668.f, 101.f);
        window.draw(itemKey);

        sf::Text itemName;
        itemName.setFont(font);
        itemName.setCharacterSize(10);
        itemName.setFillColor(sf::Color(180, 180, 200));
        itemName.setString(equippedItem.name);
        itemName.setPosition(700.f, 68.f);
        window.draw(itemName);
    }

    // buff icons below item
    float buffY = equippedItem.type != ItemType::None ? 118.f : 64.f;
    for (size_t i = 0; i < ownedBuffs.size() && i < 3; i++)
    {
        sf::RectangleShape bg(sf::Vector2f(18.f, 18.f));
        bg.setPosition(660.f + i * 22.f, buffY);
        bg.setFillColor(sf::Color(50, 50, 80));
        bg.setOutlineColor(sf::Color(100, 100, 160));
        bg.setOutlineThickness(1.f);
        window.draw(bg);

        sf::Text t;
        t.setFont(font);
        t.setCharacterSize(9);
        t.setFillColor(sf::Color(200, 200, 255));
        t.setString(ownedBuffs[i].name.substr(0, 3));
        t.setPosition(661.f + i * 22.f, buffY + 3.f);
        window.draw(t);
    }
}

void Game::drawTitle()
{
    sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    if (!fontLoaded) return;

    sf::Text title;
    title.setFont(font);
    title.setCharacterSize(52);
    title.setFillColor(sf::Color::White);
    title.setString("TRANSFORM ARENA");
    sf::FloatRect b = title.getLocalBounds();
    title.setPosition(400.f - b.width / 2.f, 80.f);
    window.draw(title);

    sf::Text sub;
    sub.setFont(font);
    sub.setCharacterSize(15);
    sub.setFillColor(sf::Color(180, 180, 200));
    sub.setString("Clear rooms, defeat bosses, buy weapons across 3 floors");
    b = sub.getLocalBounds();
    sub.setPosition(400.f - b.width / 2.f, 145.f);
    window.draw(sub);

    float yStart = 190.f;
    const char* descs[] = {
        "[1] CIRCLE  -  Fast. Dash through enemies.",
        "[2] TRIANGLE  -  Ranged. Aim with mouse.",
        "[3] SQUARE  -  Tank. Ground pound slam."
    };
    sf::Color colors[] = {
        sf::Color(80, 180, 255),
        sf::Color(255, 160, 40),
        sf::Color(80, 210, 80)
    };

    for (int i = 0; i < 3; i++)
    {
        sf::Text desc;
        desc.setFont(font);
        desc.setCharacterSize(14);
        desc.setFillColor(colors[i]);
        desc.setString(descs[i]);
        b = desc.getLocalBounds();
        desc.setPosition(400.f - b.width / 2.f, yStart + i * 24.f);
        window.draw(desc);
    }

    sf::Text controls;
    controls.setFont(font);
    controls.setCharacterSize(12);
    controls.setFillColor(sf::Color(100, 100, 120));
    controls.setString("WASD: Move | 1/2/3 or Scroll: Transform | Space: Ability | E: Item | ESC: Pause");
    b = controls.getLocalBounds();
    controls.setPosition(400.f - b.width / 2.f, 290.f);
    window.draw(controls);

    // menu
    float menuY = 340.f;
    sf::Text newGame;
    newGame.setFont(font);
    newGame.setCharacterSize(22);
    newGame.setFillColor(titleSelection == 0 ? sf::Color::White : sf::Color(80, 80, 100));
    newGame.setString(titleSelection == 0 ? "> New Game" : "  New Game");
    b = newGame.getLocalBounds();
    newGame.setPosition(400.f - b.width / 2.f, menuY);
    window.draw(newGame);

    if (hasContinue)
    {
        sf::Text cont;
        cont.setFont(font);
        cont.setCharacterSize(22);
        cont.setFillColor(titleSelection == 1 ? sf::Color::White : sf::Color(80, 80, 100));
        std::string contStr = titleSelection == 1 ? "> Continue (Floor " + std::to_string(saveData.floor) + ")" : "  Continue (Floor " + std::to_string(saveData.floor) + ")";
        cont.setString(contStr);
        b = cont.getLocalBounds();
        cont.setPosition(400.f - b.width / 2.f, menuY + 30.f);
        window.draw(cont);
    }

    if (saveData.highScore > 0)
    {
        sf::Text hi;
        hi.setFont(font);
        hi.setCharacterSize(14);
        hi.setFillColor(sf::Color(255, 220, 80));
        hi.setString("High Score: " + std::to_string(saveData.highScore) + "  |  Best Floor: " + std::to_string(saveData.highFloor));
        b = hi.getLocalBounds();
        hi.setPosition(400.f - b.width / 2.f, 440.f);
        window.draw(hi);
    }
}

void Game::drawGameOver()
{
    sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    if (!fontLoaded) return;

    bool won = (currentFloor >= totalFloors && currentRoom().isBossRoom() && currentRoom().isCleared());

    sf::Text goText;
    goText.setFont(font);
    goText.setCharacterSize(48);
    goText.setFillColor(won ? sf::Color(80, 255, 80) : sf::Color(220, 40, 40));
    goText.setString(won ? "YOU WIN!" : "GAME OVER");
    sf::FloatRect b = goText.getLocalBounds();
    goText.setPosition(400.f - b.width / 2.f, 100.f);
    window.draw(goText);

    sf::Text scoreTxt;
    scoreTxt.setFont(font);
    scoreTxt.setCharacterSize(22);
    scoreTxt.setFillColor(sf::Color::White);
    scoreTxt.setString("Score: " + std::to_string(score));
    b = scoreTxt.getLocalBounds();
    scoreTxt.setPosition(400.f - b.width / 2.f, 170.f);
    window.draw(scoreTxt);

    int minutes = (int)playTime / 60;
    int seconds = (int)playTime % 60;

    std::string weaponStr = equippedItem.type != ItemType::None ? equippedItem.name : "None";
    std::string lines[] = {
        "Floor reached: " + std::to_string(currentFloor) + "/" + std::to_string(totalFloors),
        "Enemies killed: " + std::to_string(totalKills),
        "Coins earned: " + std::to_string(coins),
        "Time played: " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s",
        "Weapon: " + weaponStr
    };

    float statY = 210.f;
    for (int i = 0; i < 5; i++)
    {
        sf::Text t;
        t.setFont(font);
        t.setCharacterSize(15);
        t.setFillColor(sf::Color(180, 180, 200));
        t.setString(lines[i]);
        b = t.getLocalBounds();
        t.setPosition(400.f - b.width / 2.f, statY);
        window.draw(t);
        statY += 22.f;
    }

    if (saveData.highScore > 0)
    {
        sf::Text hi;
        hi.setFont(font);
        hi.setCharacterSize(14);
        hi.setFillColor(sf::Color(255, 220, 80));
        hi.setString("High Score: " + std::to_string(saveData.highScore));
        b = hi.getLocalBounds();
        hi.setPosition(400.f - b.width / 2.f, statY + 10.f);
        window.draw(hi);
    }

    sf::Text restartTxt;
    restartTxt.setFont(font);
    restartTxt.setCharacterSize(18);
    restartTxt.setFillColor(sf::Color(180, 180, 180));
    restartTxt.setString("Press R to restart");
    b = restartTxt.getLocalBounds();
    restartTxt.setPosition(400.f - b.width / 2.f, 400.f);
    window.draw(restartTxt);
}

void Game::drawBuffChoice()
{
    if (!fontLoaded) return;

    // black bg fades in over 0.4s
    float bgFade = buffChoiceTimer / 0.4f;
    if (bgFade > 1.f) bgFade = 1.f;

    sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
    overlay.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(220 * bgFade)));
    window.draw(overlay);

    if (bgFade < 0.6f) return;

    float contentAlpha = (bgFade - 0.6f) / 0.4f;
    if (contentAlpha > 1.f) contentAlpha = 1.f;

    sf::Text title;
    title.setFont(font);
    title.setCharacterSize(32);
    title.setFillColor(sf::Color(255, 220, 80, (sf::Uint8)(255 * contentAlpha)));
    title.setString("CHOOSE A BUFF");
    sf::FloatRect b = title.getLocalBounds();
    title.setPosition(400.f - b.width / 2.f, 70.f);
    window.draw(title);

    if (ownedBuffs.size() >= 3)
    {
        sf::Text full;
        full.setFont(font);
        full.setCharacterSize(14);
        full.setFillColor(sf::Color(255, 100, 100, (sf::Uint8)(255 * contentAlpha)));
        full.setString("(Max 3 buffs - new buff replaces oldest)");
        b = full.getLocalBounds();
        full.setPosition(400.f - b.width / 2.f, 110.f);
        window.draw(full);
    }

    for (size_t i = 0; i < buffChoices.size(); i++)
    {
        // each card pops in with delay: card 0 at 0.5s, card 1 at 0.7s, card 2 at 0.9s
        float cardDelay = 0.5f + i * 0.2f;
        float cardT = (buffChoiceTimer - cardDelay) / 0.3f;
        if (cardT < 0.f) continue;
        if (cardT > 1.f) cardT = 1.f;

        // ease out bounce
        float scale;
        if (cardT < 0.6f)
            scale = cardT / 0.6f * 1.1f;
        else
            scale = 1.1f - (cardT - 0.6f) / 0.4f * 0.1f;

        float cardW = 180.f * scale, cardH = 260.f * scale;
        float baseX = 120.f + i * 220.f;
        float baseY = 170.f;
        float cardX = baseX + 90.f - cardW / 2.f;
        float cardY = baseY + 130.f - cardH / 2.f;

        sf::Uint8 cardAlpha = (sf::Uint8)(255 * std::min(1.f, cardT * 2.f));

        sf::RectangleShape card(sf::Vector2f(cardW, cardH));
        card.setPosition(cardX, cardY);
        card.setFillColor(sf::Color(30, 30, 50, cardAlpha));
        card.setOutlineColor(sf::Color(120, 120, 180, cardAlpha));
        card.setOutlineThickness(2.f);
        window.draw(card);

        // glow behind card
        sf::RectangleShape glow(sf::Vector2f(cardW + 8.f, cardH + 8.f));
        glow.setPosition(cardX - 4.f, cardY - 4.f);
        glow.setFillColor(sf::Color(80, 60, 160, (sf::Uint8)(30 * cardT)));
        window.draw(glow);
        window.draw(card);

        float cx = baseX + 90.f;
        float cy = baseY + 130.f;

        sf::Text numTxt;
        numTxt.setFont(font);
        numTxt.setCharacterSize((unsigned int)(28 * scale));
        numTxt.setFillColor(sf::Color(255, 220, 80, cardAlpha));
        numTxt.setString("[" + std::to_string(i + 1) + "]");
        b = numTxt.getLocalBounds();
        numTxt.setPosition(cx - b.width / 2.f, cardY + 15.f * scale);
        window.draw(numTxt);

        sf::Text nameTxt;
        nameTxt.setFont(font);
        nameTxt.setCharacterSize((unsigned int)(16 * scale));
        nameTxt.setFillColor(sf::Color(255, 255, 255, cardAlpha));
        nameTxt.setString(buffChoices[i].name);
        b = nameTxt.getLocalBounds();
        nameTxt.setPosition(cx - b.width / 2.f, cardY + 70.f * scale);
        window.draw(nameTxt);

        sf::Text descTxt;
        descTxt.setFont(font);
        descTxt.setCharacterSize((unsigned int)(12 * scale));
        descTxt.setFillColor(sf::Color(160, 160, 180, cardAlpha));
        descTxt.setString(buffChoices[i].desc);
        b = descTxt.getLocalBounds();
        descTxt.setPosition(cx - b.width / 2.f, cardY + 110.f * scale);
        window.draw(descTxt);
    }
}

void Game::drawPortal()
{
    float pulse = std::sin(portalPulse * 3.f) * 0.15f + 1.f;
    float baseR = 28.f * pulse;

    // outer glow
    sf::CircleShape glow(baseR + 16.f, 32);
    glow.setOrigin(baseR + 16.f, baseR + 16.f);
    glow.setPosition(portalPos);
    glow.setFillColor(sf::Color(80, 40, 200, 40));
    window.draw(glow);

    // spinning ring
    sf::CircleShape ring(baseR, 32);
    ring.setOrigin(baseR, baseR);
    ring.setPosition(portalPos);
    ring.setFillColor(sf::Color(20, 10, 60, 180));
    ring.setOutlineColor(sf::Color(140, 80, 255, (sf::Uint8)(180 + 40 * std::sin(portalPulse * 5.f))));
    ring.setOutlineThickness(3.f);
    window.draw(ring);

    // inner swirl
    sf::CircleShape inner(baseR * 0.6f, 32);
    inner.setOrigin(baseR * 0.6f, baseR * 0.6f);
    inner.setPosition(portalPos);
    inner.setFillColor(sf::Color(100, 50, 220, 120));
    window.draw(inner);

    // core bright dot
    sf::CircleShape core(5.f * pulse, 16);
    core.setOrigin(5.f * pulse, 5.f * pulse);
    core.setPosition(portalPos);
    core.setFillColor(sf::Color(220, 180, 255));
    window.draw(core);

    // orbiting particles
    for (int i = 0; i < 4; i++)
    {
        float angle = portalPulse * 2.5f + i * 1.5708f;
        float ox = std::cos(angle) * baseR * 0.8f;
        float oy = std::sin(angle) * baseR * 0.8f;
        sf::CircleShape orb(3.f);
        orb.setOrigin(3.f, 3.f);
        orb.setPosition(portalPos.x + ox, portalPos.y + oy);
        orb.setFillColor(sf::Color(180, 140, 255, 200));
        window.draw(orb);
    }

    if (fontLoaded)
    {
        sf::Text hint;
        hint.setFont(font);
        hint.setCharacterSize(12);
        hint.setFillColor(sf::Color(180, 140, 255, (sf::Uint8)(150 + 80 * std::sin(portalPulse * 2.f))));
        hint.setString("Enter Portal");
        sf::FloatRect b = hint.getLocalBounds();
        hint.setPosition(portalPos.x - b.width / 2.f, portalPos.y - baseR - 22.f);
        window.draw(hint);
    }
}

void Game::drawFloorFade()
{
    float alpha;
    if (floorFadeDir > 0.f)
    {
        // fading to black (1.5 -> 0.75)
        float t = (floorFadeTimer - 0.75f) / 0.75f;
        if (t < 0.f) t = 0.f;
        alpha = 1.f - t;
    }
    else
    {
        // fading back in (0.75 -> 0)
        float t = floorFadeTimer / 0.75f;
        alpha = t;
    }
    if (alpha < 0.f) alpha = 0.f;
    if (alpha > 1.f) alpha = 1.f;

    sf::RectangleShape fade(sf::Vector2f(800.f, 600.f));
    fade.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(255 * alpha)));
    window.draw(fade);

    if (fontLoaded && alpha > 0.8f)
    {
        sf::Text floorTxt;
        floorTxt.setFont(font);
        floorTxt.setCharacterSize(36);
        floorTxt.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)(255 * std::min(1.f, (alpha - 0.8f) * 5.f))));
        floorTxt.setString("Floor " + std::to_string(currentFloor));
        sf::FloatRect b = floorTxt.getLocalBounds();
        floorTxt.setPosition(400.f - b.width / 2.f, 270.f);
        window.draw(floorTxt);
    }
}

void Game::drawRoomTransition()
{
    float t = transitionTimer / 0.3f;
    sf::RectangleShape fade(sf::Vector2f(800.f, 600.f));
    fade.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(200 * t)));
    window.draw(fade);
}

void Game::restart()
{
    player.reset(400.f, 300.f);
    projectiles.clear();
    particles.clear();
    pickups.clear();
    dmgNumbers.clear();
    ownedBuffs.clear();
    buffChoices.clear();
    choosingBuff = false;
    buffChoiceTimer = 0.f;
    equippedItem = Item();
    barrierTimer = 0.f;
    score = 0;
    coins = 0;
    scoreMultiplier = 1;
    multiplierTimer = 0.f;
    damageCooldown = 0.f;
    totalKills = 0;
    playTime = 0.f;
    armor = 3;
    maxArmor = 3;
    armorRegenTimer = 0.f;
    currentFloor = 1;
    shakeIntensity = 0.f;
    shakeTimer = 0.f;
    shakeOffset = sf::Vector2f(0.f, 0.f);
    transitionTimer = 0.f;
    bossAlive = false;
    deathSlowTimer = 0.f;
    portalActive = false;
    portalPulse = 0.f;
    floorFadeTimer = 0.f;
    floorFadeDir = 0.f;

    generateFloor();
}

void Game::nextFloor()
{
    currentFloor++;
    projectiles.clear();
    particles.clear();
    pickups.clear();
    dmgNumbers.clear();
    bossAlive = false;
    generateFloor();
    saveGame();
}

void Game::saveGame()
{
    saveData.hasRun = true;
    saveData.floor = currentFloor;
    saveData.score = score;
    saveData.totalKills = totalKills;
    saveData.playTime = playTime;
    saveData.health = player.getHealth();
    saveData.maxHealth = player.getMaxHealth();
    saveData.form = (int)player.getForm();
    saveData.equippedItemType = (int)equippedItem.type;

    saveData.buffIds.clear();
    for (auto& b : ownedBuffs)
        saveData.buffIds.push_back(b.id);

    saveData.save();
}

void Game::loadGame()
{
    currentFloor = saveData.floor;
    score = saveData.score;
    totalKills = saveData.totalKills;
    playTime = saveData.playTime;
    coins = 0;
    scoreMultiplier = 1;
    multiplierTimer = 0.f;
    damageCooldown = 0.f;
    armor = 3;
    maxArmor = 3;
    armorRegenTimer = 0.f;
    barrierTimer = 0.f;
    bossAlive = false;
    choosingBuff = false;
    ownedBuffs.clear();
    buffChoices.clear();

    if (saveData.equippedItemType > 0 && saveData.equippedItemType <= 5)
        equippedItem = Item::create((ItemType)saveData.equippedItemType, currentFloor);
    else
        equippedItem = Item();

    generateFloor();

    player.heal(saveData.health - player.getHealth());
    if (saveData.form == 1) player.transform(Form::Triangle);
    else if (saveData.form == 2) player.transform(Form::Square);

    for (int id : saveData.buffIds)
    {
        Buff b;
        b.id = id;
        b.speedBonus = 0.f;
        b.damageBonus = 0.f;
        b.healthBonus = 0;
        b.cooldownReduction = 0.f;
        switch (id)
        {
        case 0: b.name = "Swift Feet"; b.speedBonus = 0.15f; break;
        case 1: b.name = "Sharp Edge"; b.damageBonus = 0.2f; break;
        case 2: b.name = "Iron Skin"; b.healthBonus = 30; break;
        case 3: b.name = "Quick Hands"; b.cooldownReduction = 0.25f; break;
        case 4: b.name = "Glass Cannon"; b.damageBonus = 0.4f; b.healthBonus = -20; break;
        case 5: b.name = "Vitality"; b.healthBonus = 50; break;
        }
        ownedBuffs.push_back(b);
    }

    float spd = 1.f, dmg = 1.f, cd = 1.f;
    for (auto& b : ownedBuffs)
    {
        spd += b.speedBonus;
        dmg += b.damageBonus;
        cd -= b.cooldownReduction;
    }
    if (cd < 0.3f) cd = 0.3f;
    player.setSpeedMultiplier(spd);
    player.setDamageMultiplier(dmg);
    player.setCooldownMultiplier(cd);
}

void Game::updateHighScores()
{
    saveData.updateHighScores();
    saveData.clearRun();
    saveData.save();
    hasContinue = false;
}

Buff Game::randomBuff()
{
    Buff b;
    b.speedBonus = 0.f;
    b.damageBonus = 0.f;
    b.healthBonus = 0;
    b.cooldownReduction = 0.f;

    int type = std::rand() % 6;
    switch (type)
    {
    case 0: b.name = "Swift Feet"; b.desc = "+15% move speed"; b.id = 0; b.speedBonus = 0.15f; break;
    case 1: b.name = "Sharp Edge"; b.desc = "+20% damage"; b.id = 1; b.damageBonus = 0.2f; break;
    case 2: b.name = "Iron Skin"; b.desc = "+30 max health"; b.id = 2; b.healthBonus = 30; break;
    case 3: b.name = "Quick Hands"; b.desc = "-25% cooldowns"; b.id = 3; b.cooldownReduction = 0.25f; break;
    case 4: b.name = "Glass Cannon"; b.desc = "+40% dmg, -20 hp"; b.id = 4; b.damageBonus = 0.4f; b.healthBonus = -20; break;
    case 5: b.name = "Vitality"; b.desc = "+50 max health"; b.id = 5; b.healthBonus = 50; break;
    }
    return b;
}

void Game::generateBuffChoices()
{
    buffChoices.clear();
    for (int i = 0; i < 3; i++)
    {
        Buff b = randomBuff();
        bool dup = false;
        for (auto& existing : buffChoices)
            if (existing.id == b.id) { dup = true; break; }
        if (dup) { i--; continue; }
        buffChoices.push_back(b);
    }
}

void Game::applyBuff(const Buff& buff)
{
    if (ownedBuffs.size() >= 3)
        ownedBuffs.erase(ownedBuffs.begin());

    ownedBuffs.push_back(buff);

    float spd = 1.f, dmg = 1.f, cd = 1.f;
    for (auto& b : ownedBuffs)
    {
        spd += b.speedBonus;
        dmg += b.damageBonus;
        cd -= b.cooldownReduction;
    }
    if (cd < 0.3f) cd = 0.3f;
    player.setSpeedMultiplier(spd);
    player.setDamageMultiplier(dmg);
    player.setCooldownMultiplier(cd);

    if (buff.healthBonus != 0)
        player.addMaxHealth(buff.healthBonus);

    player.heal(30);
}

void Game::activateItem()
{
    if (!equippedItem.isReady()) return;
    equippedItem.cooldownTimer = equippedItem.cooldown;

    sf::Vector2f pp = player.getPosition();
    auto& enemies = currentRoom().getEnemies();

    switch (equippedItem.type)
    {
    case ItemType::FlameRing:
    {
        float radius = 130.f;
        float dmg = 40.f * player.getDamageMultiplier();
        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            if (dist(pp, enemy.getPosition()) < radius + enemy.getRadius())
            {
                enemy.takeDamage(dmg);
                dmgNumbers.emplace_back(enemy.getPosition(), (int)dmg, sf::Color(255, 120, 30));
                enemy.pushAway(pp, 60.f);
            }
        }
        // large fire burst effect
        spawnParticles(pp, sf::Color(255, 120, 30), 40, 300.f, 6.f);
        spawnParticles(pp, sf::Color(255, 60, 20), 25, 200.f, 5.f);
        spawnParticles(pp, sf::Color(255, 200, 60), 15, 150.f, 3.f);
        addScreenShake(7.f, 0.25f);
        break;
    }
    case ItemType::FrostShard:
    {
        sf::Vector2f dir = player.getFacing();
        float bulletSpeed = 600.f;
        float dmg = 35.f * player.getDamageMultiplier();
        float spreadAngle = 0.25f;

        for (int i = -1; i <= 1; i++)
        {
            float angle = std::atan2(dir.y, dir.x) + i * spreadAngle;
            sf::Vector2f vel(std::cos(angle) * bulletSpeed, std::sin(angle) * bulletSpeed);
            Projectile p(pp, vel, 6.f, dmg, true, sf::Color(100, 220, 255));
            p.lifetime = 2.f;
            projectiles.push_back(p);
        }
        spawnParticles(pp, sf::Color(100, 220, 255), 20, 160.f, 4.f);
        spawnParticles(pp, sf::Color(200, 240, 255), 10, 80.f, 2.f);
        addScreenShake(3.f, 0.1f);
        break;
    }
    case ItemType::ThunderStrike:
    {
        float dmg = 55.f * player.getDamageMultiplier();
        int targets = 3;
        int hit = 0;

        // sort enemies by distance, zap closest 3
        for (int t = 0; t < targets; t++)
        {
            float closestDist = 99999.f;
            int closestIdx = -1;
            for (size_t i = 0; i < enemies.size(); i++)
            {
                if (!enemies[i].isAlive()) continue;
                float d = dist(pp, enemies[i].getPosition());
                if (d < closestDist && d < 350.f)
                {
                    bool alreadyHit = false;
                    for (auto& dn : dmgNumbers)
                    {
                        if (dist(dn.position, enemies[i].getPosition()) < 5.f && dn.lifetime > 0.75f)
                        { alreadyHit = true; break; }
                    }
                    if (!alreadyHit)
                    {
                        closestDist = d;
                        closestIdx = (int)i;
                    }
                }
            }
            if (closestIdx >= 0)
            {
                enemies[closestIdx].takeDamage(dmg);
                dmgNumbers.emplace_back(enemies[closestIdx].getPosition(), (int)dmg, sf::Color(255, 240, 60));
                spawnParticles(enemies[closestIdx].getPosition(), sf::Color(255, 240, 60), 12, 200.f, 3.f);
                hit++;
            }
        }
        if (hit > 0) addScreenShake(4.f, 0.15f);
        spawnParticles(pp, sf::Color(255, 240, 60), 8, 100.f, 3.f);
        break;
    }
    case ItemType::ShadowDash:
    {
        sf::Vector2f dir = player.getFacing();
        float teleportDist = 200.f;
        float dmg = 30.f * player.getDamageMultiplier();
        sf::Vector2f target = pp + dir * teleportDist;

        // clamp within walls
        float wall = 30.f;
        if (target.x < wall) target.x = wall;
        if (target.x > 800.f - wall) target.x = 800.f - wall;
        if (target.y < wall) target.y = wall;
        if (target.y > 600.f - wall) target.y = 600.f - wall;

        // damage enemies along the path
        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            sf::Vector2f ep = enemy.getPosition();
            // check if enemy is near the line from pp to target
            sf::Vector2f line = target - pp;
            float len = std::sqrt(line.x * line.x + line.y * line.y);
            if (len < 1.f) continue;
            sf::Vector2f n(-line.y / len, line.x / len);
            float perpDist = std::abs((ep.x - pp.x) * n.x + (ep.y - pp.y) * n.y);
            float projDist = ((ep.x - pp.x) * line.x + (ep.y - pp.y) * line.y) / len;
            if (perpDist < enemy.getRadius() + 25.f && projDist > -20.f && projDist < len + 20.f)
            {
                enemy.takeDamage(dmg);
                dmgNumbers.emplace_back(ep, (int)dmg, sf::Color(160, 60, 220));
            }
        }

        // trail particles
        for (float s = 0.f; s < 8.f; s++)
        {
            float t = s / 8.f;
            sf::Vector2f tp = pp + (target - pp) * t;
            spawnParticles(tp, sf::Color(160, 60, 220, 180), 3, 60.f, 3.f);
        }

        player.setPosition(target.x, target.y);
        addScreenShake(3.f, 0.1f);
        break;
    }
    case ItemType::BarrierShield:
    {
        barrierTimer = 3.f;
        spawnParticles(pp, sf::Color(100, 160, 255), 15, 120.f, 3.f);
        break;
    }
    case ItemType::VortexPull:
    {
        float pullRadius = 300.f;
        float dmg = 15.f * player.getDamageMultiplier();
        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            float d = dist(pp, enemy.getPosition());
            if (d < pullRadius)
            {
                enemy.pushAway(pp, -120.f);
                enemy.takeDamage(dmg);
                dmgNumbers.emplace_back(enemy.getPosition(), (int)dmg, sf::Color(200, 60, 255));
            }
        }
        for (int r = 0; r < 3; r++)
        {
            float radius = pullRadius * (1.f - r * 0.3f);
            spawnParticles(pp, sf::Color(200, 60, 255, (sf::Uint8)(200 - r * 50)), 15, radius * 0.6f, 4.f);
        }
        addScreenShake(5.f, 0.2f);
        break;
    }
    case ItemType::MirrorClone:
    {
        float offsetX = 60.f + (float)(std::rand() % 40);
        float offsetY = 60.f + (float)(std::rand() % 40);
        if (std::rand() % 2) offsetX = -offsetX;
        if (std::rand() % 2) offsetY = -offsetY;
        sf::Vector2f clonePos(pp.x + offsetX, pp.y + offsetY);
        if (clonePos.x < 30.f) clonePos.x = 30.f;
        if (clonePos.x > 770.f) clonePos.x = 770.f;
        if (clonePos.y < 30.f) clonePos.y = 30.f;
        if (clonePos.y > 570.f) clonePos.y = 570.f;

        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            if (dist(enemy.getPosition(), clonePos) < 250.f)
                enemy.setPosition(enemy.getPosition().x + (clonePos.x - enemy.getPosition().x) * 0.3f,
                                  enemy.getPosition().y + (clonePos.y - enemy.getPosition().y) * 0.3f);
        }
        spawnParticles(clonePos, sf::Color(180, 180, 255), 25, 150.f, 5.f);
        spawnParticles(pp, sf::Color(200, 200, 255), 10, 80.f, 3.f);
        break;
    }
    case ItemType::ChainLightning:
    {
        float dmg = 35.f * player.getDamageMultiplier();
        sf::Vector2f last = pp;
        int maxChain = 8;
        int chained = 0;
        std::vector<bool> hit(enemies.size(), false);

        for (int c = 0; c < maxChain; c++)
        {
            float best = 99999.f;
            int bestIdx = -1;
            for (size_t i = 0; i < enemies.size(); i++)
            {
                if (!enemies[i].isAlive() || hit[i]) continue;
                float d = dist(last, enemies[i].getPosition());
                if (d < best && d < 250.f)
                { best = d; bestIdx = (int)i; }
            }
            if (bestIdx < 0) break;
            hit[bestIdx] = true;
            enemies[bestIdx].takeDamage(dmg);
            dmgNumbers.emplace_back(enemies[bestIdx].getPosition(), (int)dmg, sf::Color(100, 200, 255));
            spawnParticles(enemies[bestIdx].getPosition(), sf::Color(100, 200, 255), 8, 120.f, 3.f);
            last = enemies[bestIdx].getPosition();
            chained++;
            dmg *= 0.85f;
        }
        if (chained > 0) addScreenShake(4.f, 0.15f);
        spawnParticles(pp, sf::Color(100, 200, 255), 10, 100.f, 3.f);
        break;
    }
    default: break;
    }
}

void Game::spawnParticles(sf::Vector2f pos, sf::Color color, int count, float speed, float sz)
{
    for (int i = 0; i < count; i++)
    {
        float angle = (float)(std::rand() % 360) * 3.14159f / 180.f;
        float spd = speed * (0.3f + (float)(std::rand() % 70) / 100.f);
        sf::Vector2f vel(std::cos(angle) * spd, std::sin(angle) * spd);
        float life = 0.3f + (float)(std::rand() % 40) / 100.f;
        particles.emplace_back(pos, vel, color, life, sz);
    }
}

void Game::spawnDeathParticles(sf::Vector2f pos, EnemyType type)
{
    sf::Color c;
    int count;
    switch (type)
    {
    case EnemyType::Chaser:   c = sf::Color(220, 60, 60);   count = 10; break;
    case EnemyType::Brute:    c = sf::Color(160, 50, 180);  count = 18; break;
    case EnemyType::Shooter:  c = sf::Color(220, 180, 40);  count = 12; break;
    case EnemyType::Dasher:   c = sf::Color(255, 100, 60);  count = 12; break;
    case EnemyType::Shielder: c = sf::Color(60, 160, 200);  count = 14; break;
    }
    spawnParticles(pos, c, count, 180.f, 4.f);
}

void Game::addScreenShake(float intensity, float duration)
{
    if (intensity > shakeIntensity)
    {
        shakeIntensity = intensity;
        shakeTimer = duration;
    }
}

bool Game::isEffectiveForm(Form form, EnemyType enemy)
{
    if (form == Form::Circle && enemy == EnemyType::Chaser) return true;
    if (form == Form::Triangle && enemy == EnemyType::Shooter) return true;
    if (form == Form::Square && enemy == EnemyType::Brute) return true;
    return false;
}

float Game::dist(sf::Vector2f a, sf::Vector2f b)
{
    float dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}
