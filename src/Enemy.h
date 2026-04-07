#pragma once
#include <SFML/Graphics.hpp>

class Enemy
{
public:
    Enemy(float x, float y);

    void update(float dt, sf::Vector2f playerPos);
    void draw(sf::RenderWindow& window);

    sf::Vector2f getPosition() const;
    float getRadius() const;
    bool isAlive() const;
    int getContactDamage() const;

    void takeDamage(float amount);

private:
    sf::Vector2f position;
    float speed;
    float size;
    float health;
    int contactDamage;
    float hitFlash;
};
