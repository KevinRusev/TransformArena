#include "Game.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <algorithm>

Game::Game(sf::RenderWindow& win)
    : window(win)
    , player(400.f, 300.f)
    , score(0)
    , wave(0)
    , waveTimer(1.5f)
    , damageCooldown(0.f)
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

    if (!player.isAlive())
    {
        if (event.key.code == sf::Keyboard::R)
            restart();
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
}

void Game::update(float dt)
{
    if (!player.isAlive())
        return;

    player.handleInput(dt);
    player.update(dt);

    if (player.wantsToShoot())
    {
        sf::Vector2f pos = player.getPosition();
        sf::Vector2f dir = player.getFacing();
        float bulletSpeed = 500.f;
        sf::Vector2f vel(dir.x * bulletSpeed, dir.y * bulletSpeed);
        projectiles.emplace_back(pos, vel, 4.f, 20.f, true, sf::Color(255, 200, 60));
    }

    if (enemies.empty())
    {
        waveTimer -= dt;
        if (waveTimer <= 0.f)
        {
            spawnWave();
            waveTimer = 2.5f;
        }
    }

    for (auto& enemy : enemies)
        enemy.update(dt, player.getPosition());

    for (auto& proj : projectiles)
        proj.update(dt);

    checkCollisions();

    for (auto& enemy : enemies)
    {
        if (!enemy.isAlive())
            score += 10;
    }

    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& e) { return !e.isAlive(); }),
        enemies.end());

    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& p) { return !p.isAlive(); }),
        projectiles.end());

    if (damageCooldown > 0.f)
        damageCooldown -= dt;
}

void Game::draw()
{
    window.clear(sf::Color(18, 18, 28));

    for (auto& enemy : enemies)
        enemy.draw(window);

    for (auto& proj : projectiles)
        proj.draw(window);

    player.draw(window);

    drawHUD();
}

void Game::spawnWave()
{
    wave++;
    int count = 3 + (wave - 1) * 2;

    for (int i = 0; i < count; i++)
    {
        sf::Vector2f pos = randomEdgePos();
        enemies.emplace_back(pos.x, pos.y);
    }
}

void Game::checkCollisions()
{
    sf::Vector2f pp = player.getPosition();
    float pr = player.getRadius();

    // dash damage
    if (player.isDashing())
    {
        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            if (dist(pp, enemy.getPosition()) < pr + enemy.getRadius() + 10.f)
                enemy.takeDamage(player.getDashDamage());
        }
    }

    // ground pound damage
    if (player.isGroundPounding())
    {
        for (auto& enemy : enemies)
        {
            if (!enemy.isAlive()) continue;
            if (dist(pp, enemy.getPosition()) < player.getGroundPoundRadius() + enemy.getRadius())
                enemy.takeDamage(player.getGroundPoundDamage());
        }
    }

    // projectile vs enemy
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
                break;
            }
        }
    }

    // enemy contact
    for (auto& enemy : enemies)
    {
        if (!enemy.isAlive()) continue;
        if (dist(pp, enemy.getPosition()) < pr + enemy.getRadius() && damageCooldown <= 0.f)
        {
            player.takeDamage(enemy.getContactDamage());
            damageCooldown = 0.4f;
        }
    }
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
    scoreTxt.setString("Score: " + std::to_string(score));
    scoreTxt.setPosition(15.f, 36.f);
    window.draw(scoreTxt);

    sf::Text waveTxt;
    waveTxt.setFont(font);
    waveTxt.setCharacterSize(16);
    waveTxt.setFillColor(sf::Color(180, 180, 200));
    waveTxt.setString("Wave " + std::to_string(wave));
    waveTxt.setPosition(15.f, 58.f);
    window.draw(waveTxt);

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

    if (!player.isAlive())
    {
        sf::Text goText;
        goText.setFont(font);
        goText.setCharacterSize(48);
        goText.setFillColor(sf::Color(220, 40, 40));
        goText.setString("GAME OVER");
        sf::FloatRect b = goText.getLocalBounds();
        goText.setPosition(400.f - b.width / 2.f, 250.f);
        window.draw(goText);

        sf::Text restartTxt;
        restartTxt.setFont(font);
        restartTxt.setCharacterSize(18);
        restartTxt.setFillColor(sf::Color(180, 180, 180));
        restartTxt.setString("Press R to restart");
        b = restartTxt.getLocalBounds();
        restartTxt.setPosition(400.f - b.width / 2.f, 310.f);
        window.draw(restartTxt);
    }

    sf::Text hint;
    hint.setFont(font);
    hint.setCharacterSize(12);
    hint.setFillColor(sf::Color(80, 80, 100));
    hint.setString("WASD: Move  |  1/2/3: Transform  |  Space: Ability  |  ESC: Quit");
    hint.setPosition(160.f, 580.f);
    window.draw(hint);
}

void Game::restart()
{
    player.reset(400.f, 300.f);
    enemies.clear();
    projectiles.clear();
    score = 0;
    wave = 0;
    waveTimer = 1.5f;
    damageCooldown = 0.f;
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
