#include "Game.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <algorithm>

Game::Game(sf::RenderWindow& win)
    : window(win)
    , player(400.f, 300.f)
    , state(GameState::Title)
    , score(0)
    , scoreMultiplier(1)
    , multiplierTimer(0.f)
    , wave(0)
    , enemiesPerWave(3)
    , waveTimer(0.f)
    , waveDelay(2.5f)
    , damageCooldown(0.f)
    , shakeIntensity(0.f)
    , shakeTimer(0.f)
    , shakeOffset(0.f, 0.f)
    , waveAnnounceTimer(0.f)
    , waveAnnounceNum(0)
    , fontLoaded(false)
{
    std::srand((unsigned int)std::time(nullptr));

    if (font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
        fontLoaded = true;
    else if (font.loadFromFile("assets/font.ttf"))
        fontLoaded = true;
}

void Game::handleEvent(const sf::Event& event)
{
    if (event.type != sf::Event::KeyPressed)
        return;

    switch (state)
    {
    case GameState::Title:
        if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space)
        {
            state = GameState::Playing;
            restart();
        }
        break;

    case GameState::Playing:
        if (event.key.code == sf::Keyboard::Num1)
            player.transform(Form::Circle);
        else if (event.key.code == sf::Keyboard::Num2)
            player.transform(Form::Triangle);
        else if (event.key.code == sf::Keyboard::Num3)
            player.transform(Form::Square);
        else if (event.key.code == sf::Keyboard::Space)
            player.useAbility();

        // spawn transform particles
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
    }
}

void Game::update(float dt)
{
    // screen shake decay
    if (shakeTimer > 0.f)
    {
        shakeTimer -= dt;
        float t = shakeTimer / 0.3f;
        shakeOffset.x = ((float)(std::rand() % 100) / 100.f - 0.5f) * shakeIntensity * t * 2.f;
        shakeOffset.y = ((float)(std::rand() % 100) / 100.f - 0.5f) * shakeIntensity * t * 2.f;
    }
    else
    {
        shakeOffset = sf::Vector2f(0.f, 0.f);
    }

    if (state != GameState::Playing)
        return;

    player.handleInput(dt);
    player.update(dt);

    // triangle shooting: spawn projectile when player requests
    if (player.wantsToShoot())
    {
        sf::Vector2f pos = player.getPosition();
        sf::Vector2f dir = player.getFacing();
        float bulletSpeed = 500.f;
        sf::Vector2f vel(dir.x * bulletSpeed, dir.y * bulletSpeed);
        projectiles.emplace_back(pos, vel, 4.f, 20.f, true, sf::Color(255, 200, 60));
    }

    // wave spawning
    if (enemies.empty() && waveAnnounceTimer <= 0.f)
    {
        waveTimer -= dt;
        if (waveTimer <= 0.f)
        {
            spawnWave();
            waveTimer = waveDelay;
        }
    }

    if (waveAnnounceTimer > 0.f)
        waveAnnounceTimer -= dt;

    // update enemies (they may spawn projectiles)
    for (auto& enemy : enemies)
        enemy.update(dt, player.getPosition(), projectiles);

    // update projectiles
    for (auto& proj : projectiles)
        proj.update(dt);

    // update particles
    for (auto& p : particles)
        p.update(dt);

    // update pickups
    for (auto& hp : pickups)
        hp.update(dt);

    checkCollisions();

    // remove dead enemies, spawn death particles, maybe drop health
    for (auto& enemy : enemies)
    {
        if (!enemy.isAlive())
        {
            spawnDeathParticles(enemy.getPosition(), enemy.getType());

            int points = 10;
            if (isEffectiveForm(player.getForm(), enemy.getType()))
            {
                points *= 2;
                scoreMultiplier = std::min(scoreMultiplier + 1, 8);
                multiplierTimer = 3.f;
            }
            score += points * scoreMultiplier;

            // 25% chance to drop health
            if (std::rand() % 100 < 25)
                pickups.emplace_back(enemy.getPosition(), 15);
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

    // multiplier decay
    if (multiplierTimer > 0.f)
    {
        multiplierTimer -= dt;
        if (multiplierTimer <= 0.f)
            scoreMultiplier = 1;
    }

    if (damageCooldown > 0.f)
        damageCooldown -= dt;

    if (!player.isAlive())
    {
        state = GameState::GameOver;
        addScreenShake(12.f, 0.4f);
        spawnParticles(player.getPosition(), sf::Color::White, 30, 250.f, 6.f);
    }
}

void Game::draw()
{
    window.clear(sf::Color(18, 18, 28));

    // apply screen shake via view offset
    sf::View view = window.getDefaultView();
    view.move(shakeOffset);
    window.setView(view);

    drawBackground();

    for (auto& hp : pickups)
        hp.draw(window);

    for (auto& p : particles)
        p.draw(window);

    for (auto& enemy : enemies)
        enemy.draw(window);

    for (auto& proj : projectiles)
        proj.draw(window);

    player.draw(window);

    // reset view for HUD (HUD shouldn't shake)
    window.setView(window.getDefaultView());

    drawHUD();

    if (waveAnnounceTimer > 0.f)
        drawWaveAnnounce();

    if (state == GameState::Title)
        drawTitle();
    else if (state == GameState::GameOver)
        drawGameOver();
}

void Game::spawnWave()
{
    wave++;
    waveAnnounceTimer = 1.5f;
    waveAnnounceNum = wave;

    int count = enemiesPerWave + (wave - 1) * 2;

    for (int i = 0; i < count; i++)
    {
        sf::Vector2f pos = randomEdgePos();
        EnemyType type;
        int roll = std::rand() % 100;

        if (wave <= 2)
        {
            type = EnemyType::Chaser;
        }
        else if (wave <= 4)
        {
            type = (roll < 60) ? EnemyType::Chaser : EnemyType::Shooter;
        }
        else if (wave <= 6)
        {
            if (roll < 40) type = EnemyType::Chaser;
            else if (roll < 70) type = EnemyType::Shooter;
            else type = EnemyType::Brute;
        }
        else
        {
            if (roll < 35) type = EnemyType::Chaser;
            else if (roll < 60) type = EnemyType::Shooter;
            else type = EnemyType::Brute;
        }

        enemies.emplace_back(pos.x, pos.y, type);
    }
}

void Game::checkCollisions()
{
    sf::Vector2f pp = player.getPosition();
    float pr = player.getRadius();

    // player dash damages enemies it passes through
    if (player.isDashing())
    {
        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            if (dist(pp, enemy.getPosition()) < pr + enemy.getRadius() + 10.f)
            {
                enemy.takeDamage(player.getDashDamage());
                spawnParticles(enemy.getPosition(), sf::Color(80, 180, 255), 6, 120.f, 3.f);
            }
        }
    }

    // ground pound damages nearby enemies
    if (player.isGroundPounding())
    {
        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            if (dist(pp, enemy.getPosition()) < player.getGroundPoundRadius() + enemy.getRadius())
            {
                enemy.takeDamage(player.getGroundPoundDamage());
                addScreenShake(6.f, 0.2f);
            }
        }
    }

    // player projectiles hit enemies
    for (auto& proj : projectiles)
    {
        if (!proj.fromPlayer || !proj.isAlive()) continue;

        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            if (dist(proj.position, enemy.getPosition()) < proj.radius + enemy.getRadius())
            {
                enemy.takeDamage(proj.damage);
                proj.lifetime = 0.f;
                spawnParticles(proj.position, sf::Color(255, 200, 60), 4, 80.f, 2.f);
                break;
            }
        }
    }

    // enemy projectiles hit player
    for (auto& proj : projectiles)
    {
        if (proj.fromPlayer || !proj.isAlive()) continue;

        if (dist(proj.position, pp) < proj.radius + pr)
        {
            player.takeDamage((int)proj.damage);
            proj.lifetime = 0.f;
            addScreenShake(4.f, 0.15f);
            spawnParticles(proj.position, sf::Color(255, 80, 80), 5, 100.f, 3.f);
        }
    }

    // enemy contact damage
    for (auto& enemy : enemies)
    {
        if (!enemy.isAlive()) continue;
        if (dist(pp, enemy.getPosition()) < pr + enemy.getRadius() && damageCooldown <= 0.f)
        {
            player.takeDamage(enemy.getContactDamage());
            damageCooldown = 0.4f;
            addScreenShake(3.f, 0.1f);
        }
    }

    // health pickup collection
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

void Game::drawBackground()
{
    // subtle dot grid
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

    // arena border
    sf::RectangleShape border(sf::Vector2f(790.f, 590.f));
    border.setPosition(5.f, 5.f);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(50, 50, 70));
    border.setOutlineThickness(2.f);
    window.draw(border);
}

void Game::drawHUD()
{
    if (!fontLoaded) return;

    // health bar
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

    // score
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

    // wave
    sf::Text waveTxt;
    waveTxt.setFont(font);
    waveTxt.setCharacterSize(16);
    waveTxt.setFillColor(sf::Color(180, 180, 200));
    waveTxt.setString("Wave " + std::to_string(wave));
    waveTxt.setPosition(15.f, 58.f);
    window.draw(waveTxt);

    // form indicator with descriptions
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
        abilityDesc = "Space: Shoot";
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

    // controls hint
    sf::Text hint;
    hint.setFont(font);
    hint.setCharacterSize(12);
    hint.setFillColor(sf::Color(80, 80, 100));
    hint.setString("WASD: Move  |  1/2/3: Transform  |  Space: Ability  |  ESC: Quit");
    hint.setPosition(160.f, 580.f);
    window.draw(hint);
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
    title.setPosition(400.f - b.width / 2.f, 160.f);
    window.draw(title);

    sf::Text sub;
    sub.setFont(font);
    sub.setCharacterSize(16);
    sub.setFillColor(sf::Color(180, 180, 200));
    sub.setString("Shift between forms to survive the arena");
    b = sub.getLocalBounds();
    sub.setPosition(400.f - b.width / 2.f, 230.f);
    window.draw(sub);

    // form descriptions
    float yStart = 290.f;
    const char* descs[] = {
        "[1] CIRCLE  -  Fast. Dash through enemies.",
        "[2] TRIANGLE  -  Ranged. Shoot projectiles.",
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

    sf::Text start;
    start.setFont(font);
    start.setCharacterSize(20);
    // pulsing alpha
    float pulse = (std::sin((float)std::clock() / 300.f) + 1.f) / 2.f;
    start.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)(120 + 135 * pulse)));
    start.setString("Press ENTER or SPACE to start");
    b = start.getLocalBounds();
    start.setPosition(400.f - b.width / 2.f, 420.f);
    window.draw(start);
}

void Game::drawGameOver()
{
    sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    if (!fontLoaded) return;

    sf::Text goText;
    goText.setFont(font);
    goText.setCharacterSize(48);
    goText.setFillColor(sf::Color(220, 40, 40));
    goText.setString("GAME OVER");
    sf::FloatRect b = goText.getLocalBounds();
    goText.setPosition(400.f - b.width / 2.f, 200.f);
    window.draw(goText);

    sf::Text scoreTxt;
    scoreTxt.setFont(font);
    scoreTxt.setCharacterSize(24);
    scoreTxt.setFillColor(sf::Color::White);
    scoreTxt.setString("Score: " + std::to_string(score) + "   Wave: " + std::to_string(wave));
    b = scoreTxt.getLocalBounds();
    scoreTxt.setPosition(400.f - b.width / 2.f, 275.f);
    window.draw(scoreTxt);

    sf::Text restart;
    restart.setFont(font);
    restart.setCharacterSize(18);
    restart.setFillColor(sf::Color(180, 180, 180));
    restart.setString("Press R to restart");
    b = restart.getLocalBounds();
    restart.setPosition(400.f - b.width / 2.f, 330.f);
    window.draw(restart);
}

void Game::drawWaveAnnounce()
{
    if (!fontLoaded) return;

    float alpha = std::min(1.f, waveAnnounceTimer / 0.5f);
    sf::Text txt;
    txt.setFont(font);
    txt.setCharacterSize(36);
    txt.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)(255 * alpha)));
    txt.setString("WAVE " + std::to_string(waveAnnounceNum));
    sf::FloatRect b = txt.getLocalBounds();
    txt.setPosition(400.f - b.width / 2.f, 260.f);
    window.draw(txt);
}

void Game::restart()
{
    player.reset(400.f, 300.f);
    enemies.clear();
    projectiles.clear();
    particles.clear();
    pickups.clear();
    score = 0;
    scoreMultiplier = 1;
    multiplierTimer = 0.f;
    wave = 0;
    enemiesPerWave = 3;
    waveTimer = 1.5f;
    damageCooldown = 0.f;
    shakeIntensity = 0.f;
    shakeTimer = 0.f;
    shakeOffset = sf::Vector2f(0.f, 0.f);
    waveAnnounceTimer = 0.f;
}

bool Game::isEffectiveForm(Form form, EnemyType enemy)
{
    // circle dash vs chasers, triangle shoot vs shooters, square pound vs brutes
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

sf::Vector2f Game::randomEdgePos()
{
    int side = std::rand() % 4;
    float x, y;
    switch (side)
    {
    case 0: x = (float)(std::rand() % 800); y = -30.f; break;
    case 1: x = (float)(std::rand() % 800); y = 630.f; break;
    case 2: x = -30.f; y = (float)(std::rand() % 600); break;
    default: x = 830.f; y = (float)(std::rand() % 600); break;
    }
    return sf::Vector2f(x, y);
}
