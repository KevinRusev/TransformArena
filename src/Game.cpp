#include "Game.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

Game::Game(sf::RenderWindow& win)
    : window(win)
    , player(400.f, 300.f)
    , currentRoomX(2), currentRoomY(2)
    , currentFloor(1), totalFloors(3)
    , bossRoomX(0), bossRoomY(0)
    , state(GameState::Title)
    , score(0), scoreMultiplier(1), multiplierTimer(0.f)
    , damageCooldown(0.f), totalKills(0), playTime(0.f)
    , shakeIntensity(0.f), shakeTimer(0.f), shakeOffset(0.f, 0.f)
    , transitionTimer(0.f), transitionDir(-1)
    , bossAlive(false), bossIntroTimer(0.f)
    , choosingBuff(false)
    , hasContinue(false)
    , titleSelection(0)
    , fontLoaded(false)
{
    std::srand((unsigned int)std::time(nullptr));

    if (font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
        fontLoaded = true;
    else if (font.loadFromFile("assets/font.ttf"))
        fontLoaded = true;

    saveData.load();
    hasContinue = saveData.hasRun;
}

void Game::generateFloor()
{
    for (int x = 0; x < MAP_SIZE; x++)
        for (int y = 0; y < MAP_SIZE; y++)
            rooms[x][y] = Room();

    int startX = 2, startY = 2;
    currentRoomX = startX;
    currentRoomY = startY;

    std::vector<std::pair<int,int>> path;
    path.push_back({startX, startY});
    rooms[startX][startY].gridX = startX;
    rooms[startX][startY].gridY = startY;
    rooms[startX][startY].generate(currentFloor, false);

    int roomCount = 4 + currentFloor;
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};

    int cx = startX, cy = startY;
    for (int i = 0; i < roomCount; i++)
    {
        bool placed = false;
        for (int attempts = 0; attempts < 20; attempts++)
        {
            int dir = std::rand() % 4;
            int nx = cx + dx[dir];
            int ny = cy + dy[dir];
            if (nx < 0 || nx >= MAP_SIZE || ny < 0 || ny >= MAP_SIZE) continue;

            bool inPath = false;
            for (auto& p : path)
                if (p.first == nx && p.second == ny) { inPath = true; break; }
            if (inPath) continue;

            rooms[cx][cy].setDoor(dir, true);
            int oppDir = (dir + 2) % 4;
            rooms[nx][ny].setDoor(oppDir, true);

            bool isBoss = (i == roomCount - 1);
            rooms[nx][ny].gridX = nx;
            rooms[nx][ny].gridY = ny;
            rooms[nx][ny].generate(currentFloor, isBoss);

            if (isBoss)
            {
                bossRoomX = nx;
                bossRoomY = ny;
            }

            path.push_back({nx, ny});
            cx = nx;
            cy = ny;
            placed = true;
            break;
        }

        if (!placed)
        {
            if (path.size() > 1)
            {
                cx = path[path.size() - 2].first;
                cy = path[path.size() - 2].second;
            }
            i--;
        }
    }

    // branch rooms
    for (int b = 0; b < 2; b++)
    {
        int idx = std::rand() % (int)path.size();
        int bx = path[idx].first, by = path[idx].second;
        int dir = std::rand() % 4;
        int nx = bx + dx[dir], ny = by + dy[dir];
        if (nx >= 0 && nx < MAP_SIZE && ny >= 0 && ny < MAP_SIZE)
        {
            bool exists = false;
            for (auto& p : path)
                if (p.first == nx && p.second == ny) { exists = true; break; }
            if (!exists)
            {
                rooms[bx][by].setDoor(dir, true);
                rooms[nx][ny].setDoor((dir + 2) % 4, true);
                rooms[nx][ny].gridX = nx;
                rooms[nx][ny].gridY = ny;
                rooms[nx][ny].generate(currentFloor, false);
                path.push_back({nx, ny});
            }
        }
    }

    rooms[startX][startY].markVisited();
    player.reset(400.f, 300.f);
}

Room& Game::currentRoom()
{
    return rooms[currentRoomX][currentRoomY];
}

void Game::handleEvent(const sf::Event& event)
{
    if (event.type != sf::Event::KeyPressed)
        return;

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
            { applyBuff(buffChoices[0]); choosingBuff = false; nextFloor(); }
            else if (event.key.code == sf::Keyboard::Num2 && buffChoices.size() > 1)
            { applyBuff(buffChoices[1]); choosingBuff = false; nextFloor(); }
            else if (event.key.code == sf::Keyboard::Num3 && buffChoices.size() > 2)
            { applyBuff(buffChoices[2]); choosingBuff = false; nextFloor(); }
            return;
        }

        if (event.key.code == sf::Keyboard::Num1)
            player.transform(Form::Circle);
        else if (event.key.code == sf::Keyboard::Num2)
            player.transform(Form::Triangle);
        else if (event.key.code == sf::Keyboard::Num3)
            player.transform(Form::Square);
        else if (event.key.code == sf::Keyboard::Space)
            player.useAbility();

        if (player.justTransformed())
        {
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

void Game::update(float dt)
{
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

    if (state != GameState::Playing || choosingBuff)
        return;

    if (transitionTimer > 0.f)
    {
        transitionTimer -= dt;
        return;
    }

    playTime += dt;
    player.handleInput(dt);

    if (player.getForm() == Form::Triangle)
    {
        sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);
        player.aimAt(mouseWorld);
    }

    player.update(dt);

    if (player.wantsToShoot())
    {
        sf::Vector2f pos = player.getPosition();
        sf::Vector2f dir = player.getFacing();
        float bulletSpeed = 500.f;
        sf::Vector2f vel(dir.x * bulletSpeed, dir.y * bulletSpeed);
        float bulletDmg = 20.f * player.getDamageMultiplier();
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

    auto& enemies = currentRoom().getEnemies();
    for (auto& enemy : enemies)
    {
        if (!enemy.isAlive())
        {
            spawnDeathParticles(enemy.getPosition(), enemy.getType());
            totalKills++;

            int points = enemy.isBoss() ? 100 : 10;
            if (isEffectiveForm(player.getForm(), enemy.getType()))
            {
                points *= 2;
                scoreMultiplier = std::min(scoreMultiplier + 1, 8);
                multiplierTimer = 3.f;
            }
            score += points * scoreMultiplier;

            if (!enemy.isBoss() && std::rand() % 100 < 25)
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

    // boss room cleared -> buff choice or win
    if (currentRoom().isBossRoom() && currentRoom().isCleared() && !choosingBuff && !bossAlive)
    {
        if (currentFloor >= totalFloors)
        {
            state = GameState::GameOver;
            score += 1000 * currentFloor;
            saveData.score = score;
            saveData.totalKills = totalKills;
            saveData.floor = currentFloor;
            updateHighScores();
        }
        else
        {
            generateBuffChoices();
            choosingBuff = true;
        }
    }

    checkDoorTransition();

    if (!player.isAlive())
    {
        state = GameState::GameOver;
        addScreenShake(12.f, 0.4f);
        spawnParticles(player.getPosition(), sf::Color::White, 30, 250.f, 6.f);
        saveData.score = score;
        saveData.totalKills = totalKills;
        saveData.floor = currentFloor;
        updateHighScores();
    }
}

void Game::checkDoorTransition()
{
    if (!currentRoom().isCleared()) return;

    sf::Vector2f pp = player.getPosition();
    float pr = player.getRadius();
    int dir = -1;

    if (pp.y - pr <= 2.f && currentRoom().doorOpen(0) && std::abs(pp.x - 400.f) < 50.f)
        dir = 0;
    else if (pp.x + pr >= 798.f && currentRoom().doorOpen(1) && std::abs(pp.y - 300.f) < 50.f)
        dir = 1;
    else if (pp.y + pr >= 598.f && currentRoom().doorOpen(2) && std::abs(pp.x - 400.f) < 50.f)
        dir = 2;
    else if (pp.x - pr <= 2.f && currentRoom().doorOpen(3) && std::abs(pp.y - 300.f) < 50.f)
        dir = 3;

    if (dir >= 0)
        transitionToRoom(dir);
}

void Game::transitionToRoom(int dir)
{
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};

    int nx = currentRoomX + dx[dir];
    int ny = currentRoomY + dy[dir];
    if (nx < 0 || nx >= MAP_SIZE || ny < 0 || ny >= MAP_SIZE) return;

    currentRoomX = nx;
    currentRoomY = ny;

    projectiles.clear();
    particles.clear();
    pickups.clear();
    dmgNumbers.clear();

    float px = 400.f, py = 300.f;
    switch (dir)
    {
    case 0: py = 560.f; break;
    case 1: px = 40.f;  break;
    case 2: py = 40.f;  break;
    case 3: px = 760.f; break;
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
                enemy.takeDamage(player.getDashDamage());
                enemy.markDashHit();
                dmgNumbers.emplace_back(enemy.getPosition(), (int)player.getDashDamage(), sf::Color(80, 180, 255));
                spawnParticles(enemy.getPosition(), sf::Color(80, 180, 255), 6, 120.f, 3.f);
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
                enemy.takeDamage(player.getGroundPoundDamage());
                dmgNumbers.emplace_back(enemy.getPosition(), (int)player.getGroundPoundDamage(), sf::Color(80, 210, 80));
                float knockback = 80.f + (player.getGroundPoundRadius() - d) * 0.5f;
                enemy.pushAway(pp, knockback);
            }
        }
        addScreenShake(6.f, 0.2f);
    }

    for (auto& proj : projectiles)
    {
        if (!proj.fromPlayer || !proj.isAlive()) continue;
        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            if (dist(proj.position, enemy.getPosition()) < proj.radius + enemy.getRadius())
            {
                enemy.takeDamage(proj.damage);
                dmgNumbers.emplace_back(enemy.getPosition(), (int)proj.damage, sf::Color(255, 200, 60));
                proj.lifetime = 0.f;
                spawnParticles(proj.position, sf::Color(255, 200, 60), 4, 80.f, 2.f);
                break;
            }
        }
    }

    for (auto& proj : projectiles)
    {
        if (proj.fromPlayer || !proj.isAlive()) continue;
        if (dist(proj.position, pp) < proj.radius + pr)
        {
            if (!player.isInvincible())
            {
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
    window.clear(sf::Color(18, 18, 28));

    sf::View view = window.getDefaultView();
    view.move(shakeOffset);
    window.setView(view);

    drawBackground();
    currentRoom().drawDoors(window);

    for (auto& hp : pickups)
        hp.draw(window);

    for (auto& p : particles)
        p.draw(window);

    currentRoom().draw(window);

    for (auto& proj : projectiles)
        proj.draw(window);

    player.draw(window);

    if (fontLoaded)
    {
        for (auto& dn : dmgNumbers)
            dn.draw(window, font);
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
        sf::Text txt;
        txt.setFont(font);
        txt.setCharacterSize(40);
        txt.setFillColor(sf::Color(220, 40, 40));
        txt.setString("BOSS ROOM");
        sf::FloatRect b = txt.getLocalBounds();
        txt.setPosition(400.f - b.width / 2.f, 250.f);
        window.draw(txt);
    }

    if (choosingBuff)
        drawBuffChoice();

    if (transitionTimer > 0.f)
        drawRoomTransition();
}

void Game::drawBackground()
{
    float spacing = 40.f;
    for (float x = spacing; x < 800.f; x += spacing)
    {
        for (float y = spacing; y < 600.f; y += spacing)
        {
            sf::CircleShape dot(1.2f);
            dot.setOrigin(1.2f, 1.2f);
            dot.setPosition(x, y);
            dot.setFillColor(sf::Color(40, 40, 55));
            window.draw(dot);
        }
    }

    sf::RectangleShape border(sf::Vector2f(790.f, 590.f));
    border.setPosition(5.f, 5.f);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(50, 50, 70));
    border.setOutlineThickness(2.f);
    window.draw(border);
}

void Game::drawMinimap()
{
    if (!fontLoaded) return;

    float cellSize = 12.f;
    float padding = 2.f;
    float startX = 800.f - (MAP_SIZE * (cellSize + padding)) - 15.f;
    float startY = 80.f;

    sf::Text label;
    label.setFont(font);
    label.setCharacterSize(10);
    label.setFillColor(sf::Color(120, 120, 140));
    label.setString("MAP");
    label.setPosition(startX, startY - 14.f);
    window.draw(label);

    for (int x = 0; x < MAP_SIZE; x++)
    {
        for (int y = 0; y < MAP_SIZE; y++)
        {
            Room& r = rooms[x][y];
            bool hasRoom = false;
            for (int d = 0; d < 4; d++)
                if (r.hasDoor(d)) { hasRoom = true; break; }
            if (x == 2 && y == 2) hasRoom = true;
            if (!hasRoom && !r.isVisited()) continue;

            sf::RectangleShape cell(sf::Vector2f(cellSize, cellSize));
            cell.setPosition(startX + x * (cellSize + padding), startY + y * (cellSize + padding));

            if (x == currentRoomX && y == currentRoomY)
                cell.setFillColor(sf::Color(255, 255, 255));
            else if (r.isBossRoom() && r.isVisited())
                cell.setFillColor(sf::Color(220, 40, 40));
            else if (r.isCleared())
                cell.setFillColor(sf::Color(60, 120, 60));
            else if (r.isVisited())
                cell.setFillColor(sf::Color(100, 100, 140));
            else if (hasRoom)
                cell.setFillColor(sf::Color(40, 40, 55));
            else
                continue;

            cell.setOutlineColor(sf::Color(80, 80, 100));
            cell.setOutlineThickness(1.f);
            window.draw(cell);
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

    sf::Text label;
    label.setFont(font);
    label.setCharacterSize(12);
    label.setFillColor(sf::Color(220, 40, 40));
    label.setString("FLOOR " + std::to_string(currentFloor) + " BOSS");
    sf::FloatRect lb = label.getLocalBounds();
    label.setPosition(400.f - lb.width / 2.f, barY - 16.f);
    window.draw(label);
}

void Game::drawHUD()
{
    if (!fontLoaded) return;

    float barW = 200.f, barH = 14.f;
    float hp = (float)player.getHealth() / player.getMaxHealth();

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

    sf::Text scoreTxt;
    scoreTxt.setFont(font);
    scoreTxt.setCharacterSize(18);
    scoreTxt.setFillColor(sf::Color::White);
    std::string scoreStr = "Score: " + std::to_string(score);
    if (scoreMultiplier > 1)
        scoreStr += "  x" + std::to_string(scoreMultiplier);
    scoreTxt.setString(scoreStr);
    scoreTxt.setPosition(15.f, 36.f);
    window.draw(scoreTxt);

    sf::Text floorTxt;
    floorTxt.setFont(font);
    floorTxt.setCharacterSize(16);
    floorTxt.setFillColor(sf::Color(180, 180, 200));
    floorTxt.setString("Floor " + std::to_string(currentFloor) + "/" + std::to_string(totalFloors));
    floorTxt.setPosition(15.f, 58.f);
    window.draw(floorTxt);

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
    formTxt.setPosition(660.f, 15.f);
    window.draw(formTxt);

    sf::Text abilTxt;
    abilTxt.setFont(font);
    abilTxt.setCharacterSize(13);
    abilTxt.setFillColor(sf::Color(180, 180, 180));
    abilTxt.setString(abilityDesc);
    abilTxt.setPosition(660.f, 36.f);
    window.draw(abilTxt);

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

    // buff icons
    float buffStartY = 580.f;
    for (size_t i = 0; i < ownedBuffs.size() && i < 3; i++)
    {
        sf::RectangleShape bg(sf::Vector2f(18.f, 18.f));
        bg.setPosition(15.f + i * 22.f, buffStartY);
        bg.setFillColor(sf::Color(60, 60, 100));
        bg.setOutlineColor(sf::Color(120, 120, 180));
        bg.setOutlineThickness(1.f);
        window.draw(bg);

        sf::Text t;
        t.setFont(font);
        t.setCharacterSize(9);
        t.setFillColor(sf::Color(200, 200, 255));
        t.setString(ownedBuffs[i].name.substr(0, 3));
        t.setPosition(16.f + i * 22.f, buffStartY + 3.f);
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
    title.setPosition(400.f - b.width / 2.f, 100.f);
    window.draw(title);

    sf::Text sub;
    sub.setFont(font);
    sub.setCharacterSize(16);
    sub.setFillColor(sf::Color(180, 180, 200));
    sub.setString("Clear rooms, defeat bosses, collect buffs across 3 floors");
    b = sub.getLocalBounds();
    sub.setPosition(400.f - b.width / 2.f, 170.f);
    window.draw(sub);

    float yStart = 220.f;
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
        desc.setCharacterSize(15);
        desc.setFillColor(colors[i]);
        desc.setString(descs[i]);
        b = desc.getLocalBounds();
        desc.setPosition(400.f - b.width / 2.f, yStart + i * 28.f);
        window.draw(desc);
    }

    sf::Text controls;
    controls.setFont(font);
    controls.setCharacterSize(13);
    controls.setFillColor(sf::Color(120, 120, 140));
    controls.setString("WASD: Move  |  1/2/3: Transform  |  Space: Ability  |  Mouse: Aim (Triangle)");
    b = controls.getLocalBounds();
    controls.setPosition(400.f - b.width / 2.f, 340.f);
    window.draw(controls);

    // menu options
    float menuY = 390.f;
    sf::Text newGame;
    newGame.setFont(font);
    newGame.setCharacterSize(20);
    newGame.setFillColor(titleSelection == 0 ? sf::Color::White : sf::Color(100, 100, 120));
    newGame.setString(titleSelection == 0 ? "> New Game" : "  New Game");
    b = newGame.getLocalBounds();
    newGame.setPosition(400.f - b.width / 2.f, menuY);
    window.draw(newGame);

    if (hasContinue)
    {
        sf::Text cont;
        cont.setFont(font);
        cont.setCharacterSize(20);
        cont.setFillColor(titleSelection == 1 ? sf::Color::White : sf::Color(100, 100, 120));
        cont.setString(titleSelection == 1 ? "> Continue (Floor " + std::to_string(saveData.floor) + ")" : "  Continue (Floor " + std::to_string(saveData.floor) + ")");
        b = cont.getLocalBounds();
        cont.setPosition(400.f - b.width / 2.f, menuY + 28.f);
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
        hi.setPosition(400.f - b.width / 2.f, 470.f);
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
    goText.setPosition(400.f - b.width / 2.f, 120.f);
    window.draw(goText);

    sf::Text scoreTxt;
    scoreTxt.setFont(font);
    scoreTxt.setCharacterSize(22);
    scoreTxt.setFillColor(sf::Color::White);
    scoreTxt.setString("Score: " + std::to_string(score));
    b = scoreTxt.getLocalBounds();
    scoreTxt.setPosition(400.f - b.width / 2.f, 195.f);
    window.draw(scoreTxt);

    int minutes = (int)playTime / 60;
    int seconds = (int)playTime % 60;

    std::string lines[] = {
        "Floor reached: " + std::to_string(currentFloor) + "/" + std::to_string(totalFloors),
        "Enemies killed: " + std::to_string(totalKills),
        "Time played: " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s",
        "Buffs collected: " + std::to_string((int)ownedBuffs.size())
    };

    float statY = 235.f;
    for (int i = 0; i < 4; i++)
    {
        sf::Text t;
        t.setFont(font);
        t.setCharacterSize(16);
        t.setFillColor(sf::Color(180, 180, 200));
        t.setString(lines[i]);
        b = t.getLocalBounds();
        t.setPosition(400.f - b.width / 2.f, statY);
        window.draw(t);
        statY += 24.f;
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

    sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    sf::Text title;
    title.setFont(font);
    title.setCharacterSize(32);
    title.setFillColor(sf::Color(255, 220, 80));
    title.setString("CHOOSE A BUFF");
    sf::FloatRect b = title.getLocalBounds();
    title.setPosition(400.f - b.width / 2.f, 80.f);
    window.draw(title);

    if (ownedBuffs.size() >= 3)
    {
        sf::Text full;
        full.setFont(font);
        full.setCharacterSize(14);
        full.setFillColor(sf::Color(255, 100, 100));
        full.setString("(Max 3 buffs - new buff replaces oldest)");
        b = full.getLocalBounds();
        full.setPosition(400.f - b.width / 2.f, 120.f);
        window.draw(full);
    }

    for (size_t i = 0; i < buffChoices.size(); i++)
    {
        float cardX = 120.f + i * 220.f;
        float cardY = 170.f;
        float cardW = 180.f, cardH = 260.f;

        sf::RectangleShape card(sf::Vector2f(cardW, cardH));
        card.setPosition(cardX, cardY);
        card.setFillColor(sf::Color(30, 30, 50));
        card.setOutlineColor(sf::Color(120, 120, 180));
        card.setOutlineThickness(2.f);
        window.draw(card);

        sf::Text numTxt;
        numTxt.setFont(font);
        numTxt.setCharacterSize(28);
        numTxt.setFillColor(sf::Color(255, 220, 80));
        numTxt.setString("[" + std::to_string(i + 1) + "]");
        b = numTxt.getLocalBounds();
        numTxt.setPosition(cardX + cardW / 2.f - b.width / 2.f, cardY + 15.f);
        window.draw(numTxt);

        sf::Text nameTxt;
        nameTxt.setFont(font);
        nameTxt.setCharacterSize(16);
        nameTxt.setFillColor(sf::Color::White);
        nameTxt.setString(buffChoices[i].name);
        b = nameTxt.getLocalBounds();
        nameTxt.setPosition(cardX + cardW / 2.f - b.width / 2.f, cardY + 70.f);
        window.draw(nameTxt);

        sf::Text descTxt;
        descTxt.setFont(font);
        descTxt.setCharacterSize(12);
        descTxt.setFillColor(sf::Color(160, 160, 180));
        descTxt.setString(buffChoices[i].desc);
        b = descTxt.getLocalBounds();
        descTxt.setPosition(cardX + cardW / 2.f - b.width / 2.f, cardY + 110.f);
        window.draw(descTxt);
    }
}

void Game::drawRoomTransition()
{
    float t = transitionTimer / 0.3f;
    sf::RectangleShape fade(sf::Vector2f(800.f, 600.f));
    fade.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(180 * t)));
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
    score = 0;
    scoreMultiplier = 1;
    multiplierTimer = 0.f;
    damageCooldown = 0.f;
    totalKills = 0;
    playTime = 0.f;
    currentFloor = 1;
    shakeIntensity = 0.f;
    shakeTimer = 0.f;
    shakeOffset = sf::Vector2f(0.f, 0.f);
    transitionTimer = 0.f;
    bossAlive = false;

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
    scoreMultiplier = 1;
    multiplierTimer = 0.f;
    damageCooldown = 0.f;
    bossAlive = false;
    choosingBuff = false;
    ownedBuffs.clear();
    buffChoices.clear();

    generateFloor();

    // restore player state
    player.heal(saveData.health - player.getHealth());
    if (saveData.form == 1) player.transform(Form::Triangle);
    else if (saveData.form == 2) player.transform(Form::Square);

    // rebuild buffs
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

    // apply buff multipliers
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
    case 0:
        b.name = "Swift Feet";
        b.desc = "+15% move speed";
        b.id = 0;
        b.speedBonus = 0.15f;
        break;
    case 1:
        b.name = "Sharp Edge";
        b.desc = "+20% damage";
        b.id = 1;
        b.damageBonus = 0.2f;
        break;
    case 2:
        b.name = "Iron Skin";
        b.desc = "+30 max health";
        b.id = 2;
        b.healthBonus = 30;
        break;
    case 3:
        b.name = "Quick Hands";
        b.desc = "-25% cooldowns";
        b.id = 3;
        b.cooldownReduction = 0.25f;
        break;
    case 4:
        b.name = "Glass Cannon";
        b.desc = "+40% dmg, -20 hp";
        b.id = 4;
        b.damageBonus = 0.4f;
        b.healthBonus = -20;
        break;
    case 5:
        b.name = "Vitality";
        b.desc = "+50 max health";
        b.id = 5;
        b.healthBonus = 50;
        break;
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

    // recalculate all multipliers from owned buffs
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
    case EnemyType::Chaser:  c = sf::Color(220, 60, 60);  count = 10; break;
    case EnemyType::Brute:   c = sf::Color(160, 50, 180); count = 18; break;
    case EnemyType::Shooter: c = sf::Color(220, 180, 40); count = 12; break;
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
