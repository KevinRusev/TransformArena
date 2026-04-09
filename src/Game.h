#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Particle.h"
#include "Room.h"

enum class GameState { Title, Playing, GameOver, BossIntro };

struct Buff
{
    std::string name;
    std::string desc;
    int id;
    float speedBonus;
    float damageBonus;
    int healthBonus;
    float cooldownReduction;
};

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
    std::vector<Projectile> projectiles;
    std::vector<Particle> particles;
    std::vector<HealthPickup> pickups;
    std::vector<DamageNumber> dmgNumbers;

    static const int MAP_SIZE = 5;
    Room rooms[MAP_SIZE][MAP_SIZE];
    int currentRoomX, currentRoomY;
    int currentFloor;
    int totalFloors;
    int bossRoomX, bossRoomY;

    GameState state;
    int score;
    int scoreMultiplier;
    float multiplierTimer;
    float damageCooldown;
    int totalKills;
    float playTime;

    float shakeIntensity;
    float shakeTimer;
    sf::Vector2f shakeOffset;

    float transitionTimer;
    int transitionDir;

    bool bossAlive;
    float bossIntroTimer;

    std::vector<Buff> ownedBuffs;
    std::vector<Buff> buffChoices;
    bool choosingBuff;

    sf::Font font;
    bool fontLoaded;

    void generateFloor();
    Room& currentRoom();
    void checkCollisions();
    void checkDoorTransition();
    void transitionToRoom(int dir);
    void spawnParticles(sf::Vector2f pos, sf::Color color, int count, float speed, float size);
    void spawnDeathParticles(sf::Vector2f pos, EnemyType type);
    void addScreenShake(float intensity, float duration);
    void drawBackground();
    void drawMinimap();
    void drawHUD();
    void drawBossBar();
    void drawTitle();
    void drawGameOver();
    void drawBuffChoice();
    void drawRoomTransition();
    void restart();
    void nextFloor();
    void generateBuffChoices();
    void applyBuff(const Buff& buff);
    Buff randomBuff();

    float dist(sf::Vector2f a, sf::Vector2f b);
    bool isEffectiveForm(Form form, EnemyType enemy);
};
