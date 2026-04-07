#pragma once
#include <SFML/Graphics.hpp>

class Player
{
public:
    Player(float startX, float startY);

    void handleInput(float dt);
    void update(float dt);
    void draw(sf::RenderWindow& window);

    sf::Vector2f getPosition() const;
    float getRadius() const;
    int getHealth() const;
    int getMaxHealth() const;
    bool isAlive() const;

    void takeDamage(int amount);
    void reset(float x, float y);

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    float speed;
    float size;
    int health;
    int maxHealth;
};
