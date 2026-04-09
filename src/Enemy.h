#pragma once
#include <SFML/Graphics.hpp>
#include "Projectile.h"
#include <vector>

enum class EnemyType { Chaser, Brute, Shooter, Dasher, Shielder };

class Enemy
{
public:
    Enemy(float x, float y, EnemyType type, bool isBoss = false, int bossType = 0);

    void update(float dt, sf::Vector2f playerPos, std::vector<Projectile>& projectiles);
    void draw(sf::RenderWindow& window);

    sf::Vector2f getPosition() const;
    float getRadius() const;
    float getHealth() const;
    float getMaxHealth() const;
    bool isAlive() const;
    bool isBoss() const;
    int getBossType() const;
    int getContactDamage() const;
    EnemyType getType() const;

    void takeDamage(float amount);
    void pushAway(sf::Vector2f from, float force);
    bool canBeDashHit() const;
    void markDashHit();

    bool isShieldUp() const;
    void breakShield(float duration);
    bool isStunned() const;
    void stun(float duration);
    void setPosition(float x, float y);

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f knockbackVel;
    EnemyType type;

    float speed;
    float size;
    float health;
    float maxHealth;
    int contactDamage;

    float hitFlash;
    float dashHitCooldown;
    float shootTimer;
    float shootInterval;
    float preferredRange;
    bool bossFlag;
    int bossTypeId;

    // boss-specific state
    bool shieldUp;
    float shieldBreakTimer;
    float shieldAngle;
    bool stunned;
    float stunnedTimer;
    float teleportTimer;
    float bossPhaseTimer;
    int bossPhase;

    // dasher enemy state
    bool dashCharging;
    float dashChargeTimer;
    sf::Vector2f dashDir;
    float dashSpeed;
};
