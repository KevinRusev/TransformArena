#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"

class Game
{
public:
    Game(sf::RenderWindow& window);

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void draw();

private:
    sf::RenderWindow& window;
    Player player;
    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;

    int score;
    int wave;
    float waveTimer;
    float damageCooldown;

    sf::Font font;
    bool fontLoaded;

    void spawnWave();
    void checkCollisions();
    void drawHUD();
    void restart();

    float dist(sf::Vector2f a, sf::Vector2f b);
    sf::Vector2f randomEdgePos();
};
