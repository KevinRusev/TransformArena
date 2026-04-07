#include "Player.h"
#include <cmath>

Player::Player(float startX, float startY)
    : position(startX, startY)
    , velocity(0.f, 0.f)
    , speed(250.f)
    , size(18.f)
    , health(100)
    , maxHealth(100)
{
}

void Player::handleInput(float dt)
{
    velocity = sf::Vector2f(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        velocity.y = -1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        velocity.y = 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        velocity.x = -1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        velocity.x = 1.f;

    float len = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    if (len > 0.f)
    {
        velocity.x /= len;
        velocity.y /= len;
    }
}

void Player::update(float dt)
{
    position.x += velocity.x * speed * dt;
    position.y += velocity.y * speed * dt;

    if (position.x < size) position.x = size;
    if (position.x > 800.f - size) position.x = 800.f - size;
    if (position.y < size) position.y = size;
    if (position.y > 600.f - size) position.y = 600.f - size;
}

void Player::draw(sf::RenderWindow& window)
{
    sf::CircleShape shape(size);
    shape.setOrigin(size, size);
    shape.setPosition(position);
    shape.setFillColor(sf::Color(80, 180, 255));
    shape.setOutlineColor(sf::Color(140, 210, 255));
    shape.setOutlineThickness(2.f);
    window.draw(shape);
}

sf::Vector2f Player::getPosition() const { return position; }
float Player::getRadius() const { return size; }
int Player::getHealth() const { return health; }
int Player::getMaxHealth() const { return maxHealth; }
bool Player::isAlive() const { return health > 0; }

void Player::takeDamage(int amount)
{
    health -= amount;
    if (health < 0) health = 0;
}

void Player::reset(float x, float y)
{
    position = sf::Vector2f(x, y);
    health = maxHealth;
}
