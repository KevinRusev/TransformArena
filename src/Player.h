#pragma once
#include <SFML/Graphics.hpp>

enum class Form { Circle, Triangle, Square };

class Player
{
public:
    Player(float startX, float startY);

    void handleInput(float dt);
    void update(float dt);
    void draw(sf::RenderWindow& window);

    void transform(Form newForm);
    void useAbility();

    void aimAt(sf::Vector2f target);

    sf::Vector2f getPosition() const;
    sf::Vector2f getFacing() const;
    float getRadius() const;
    Form getForm() const;
    int getHealth() const;
    int getMaxHealth() const;
    bool isAlive() const;
    bool isDashing() const;
    bool isInvincible() const;
    bool isGroundPounding() const;
    bool consumePoundDamage();
    float getGroundPoundRadius() const;
    float getGroundPoundDamage() const;
    float getDashDamage() const;
    bool wantsToShoot();
    float getCooldownPercent() const;

    void takeDamage(int amount);
    void heal(int amount);
    void reset(float x, float y);
    void setPosition(float x, float y);
    bool justTransformed() const;

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f facing;
    Form currentForm;

    float speed;
    float size;
    int health;
    int maxHealth;

    // circle dash
    bool dashing;
    float dashTimer;
    float dashSpeed;
    float dashDamage;
    float dashDuration;

    // triangle shoot
    bool shootRequest;
    float shootCooldown;
    float shootTimer;

    // square ground pound
    bool groundPounding;
    bool poundDamageReady;
    float poundTimer;
    float poundDuration;
    float poundRadius;
    float poundDamage;

    float abilityCooldown;
    float cooldownTimer;

    float transformFlash;
    float invincibleTimer;

    void applyFormStats();
};
