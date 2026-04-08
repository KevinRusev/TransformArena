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

    sf::Vector2f getPosition() const;
    float getRadius() const;
    Form getForm() const;
    int getHealth() const;
    int getMaxHealth() const;
    bool isAlive() const;
    bool justTransformed() const;

    void takeDamage(int amount);
    void heal(int amount);
    void reset(float x, float y);

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f facing;
    Form currentForm;

    float speed;
    float size;
    int health;
    int maxHealth;

    float transformFlash;
    float invincibleTimer;

    void applyFormStats();
};
