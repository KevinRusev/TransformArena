#include "Enemy.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(float x, float y)
    : position(x, y)
    , speed(120.f)
    , size(12.f)
    , health(30.f)
    , contactDamage(10)
    , hitFlash(0.f)
{
}

void Enemy::update(float dt, sf::Vector2f playerPos)
{
    sf::Vector2f dir = playerPos - position;
    float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (dist > 1.f)
    {
        dir.x /= dist;
        dir.y /= dist;
    }

    position.x += dir.x * speed * dt;
    position.y += dir.y * speed * dt;

    if (hitFlash > 0.f)
        hitFlash -= dt;
}

void Enemy::draw(sf::RenderWindow& window)
{
    sf::Color color(220, 60, 60);
    if (hitFlash > 0.f)
        color = sf::Color(255, 255, 255);

    sf::CircleShape shape(size);
    shape.setOrigin(size, size);
    shape.setPosition(position);
    shape.setFillColor(color);
    shape.setOutlineColor(sf::Color(150, 30, 30));
    shape.setOutlineThickness(1.5f);
    window.draw(shape);
}

void Enemy::takeDamage(float amount)
{
    health -= amount;
    hitFlash = 0.1f;
}

sf::Vector2f Enemy::getPosition() const { return position; }
float Enemy::getRadius() const { return size; }
bool Enemy::isAlive() const { return health > 0.f; }
int Enemy::getContactDamage() const { return contactDamage; }
