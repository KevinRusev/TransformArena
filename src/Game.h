#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Particle.h"

enum class GameState { Title, Playing, GameOver };

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
    std::vector<Particle> particles;
    std::vector<HealthPickup> pickups;

    GameState state;
    int score;
    int scoreMultiplier;
    float multiplierTimer;
    int wave;
    int enemiesPerWave;
    float waveTimer;
    float waveDelay;
    float damageCooldown;

    // screen shake
    float shakeIntensity;
    float shakeTimer;
    sf::Vector2f shakeOffset;

    // wave announcement
    float waveAnnounceTimer;
    int waveAnnounceNum;

    sf::Font font;
    bool fontLoaded;

    void spawnWave();
    void checkCollisions();
    void spawnParticles(sf::Vector2f pos, sf::Color color, int count, float speed, float size);
    void spawnDeathParticles(sf::Vector2f pos, EnemyType type);
    void addScreenShake(float intensity, float duration);
    void drawBackground();
    void drawHUD();
    void drawTitle();
    void drawGameOver();
    void drawWaveAnnounce();
    void restart();

    float dist(sf::Vector2f a, sf::Vector2f b);
    sf::Vector2f randomEdgePos();

    // form-enemy effectiveness for score bonus
    bool isEffectiveForm(Form form, EnemyType enemy);
};
