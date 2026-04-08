#include "Player.h"
#include <cmath>

Player::Player(float startX, float startY)
    : position(startX, startY)
    , velocity(0.f, 0.f)
    , facing(0.f, -1.f)
    , currentForm(Form::Circle)
    , transformFlash(0.f)
    , invincibleTimer(0.f)
{
    applyFormStats();
    health = maxHealth;
}

void Player::applyFormStats()
{
    switch (currentForm)
    {
    case Form::Circle:
        speed = 300.f;
        size = 16.f;
        maxHealth = 80;
        break;
    case Form::Triangle:
        speed = 220.f;
        size = 20.f;
        maxHealth = 90;
        break;
    case Form::Square:
        speed = 140.f;
        size = 24.f;
        maxHealth = 160;
        break;
    }
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
        facing = velocity;
    }
}

void Player::update(float dt)
{
    position.x += velocity.x * speed * dt;
    position.y += velocity.y * speed * dt;

    float padding = size + 5.f;
    if (position.x < padding) position.x = padding;
    if (position.x > 800.f - padding) position.x = 800.f - padding;
    if (position.y < padding) position.y = padding;
    if (position.y > 600.f - padding) position.y = 600.f - padding;

    if (invincibleTimer > 0.f)
        invincibleTimer -= dt;
    if (transformFlash > 0.f)
        transformFlash -= dt;
}

void Player::draw(sf::RenderWindow& window)
{
    sf::Color bodyColor;
    switch (currentForm)
    {
    case Form::Circle:   bodyColor = sf::Color(80, 180, 255);  break;
    case Form::Triangle: bodyColor = sf::Color(255, 160, 40);  break;
    case Form::Square:   bodyColor = sf::Color(80, 210, 80);   break;
    }

    if (transformFlash > 0.f)
    {
        float t = transformFlash / 0.25f;
        bodyColor.r = (sf::Uint8)(bodyColor.r + (255 - bodyColor.r) * t);
        bodyColor.g = (sf::Uint8)(bodyColor.g + (255 - bodyColor.g) * t);
        bodyColor.b = (sf::Uint8)(bodyColor.b + (255 - bodyColor.b) * t);
    }

    if (invincibleTimer > 0.f)
    {
        int blink = (int)(invincibleTimer * 20.f) % 2;
        if (blink == 0)
            bodyColor.a = 120;
    }

    switch (currentForm)
    {
    case Form::Circle:
    {
        sf::CircleShape shape(size);
        shape.setOrigin(size, size);
        shape.setPosition(position);
        shape.setFillColor(bodyColor);
        shape.setOutlineColor(sf::Color(140, 210, 255));
        shape.setOutlineThickness(2.f);
        window.draw(shape);
        break;
    }
    case Form::Triangle:
    {
        float angle = std::atan2(facing.y, facing.x) * 180.f / 3.14159f + 90.f;
        sf::ConvexShape shape(3);
        shape.setPoint(0, sf::Vector2f(0.f, -size * 1.2f));
        shape.setPoint(1, sf::Vector2f(-size, size * 0.8f));
        shape.setPoint(2, sf::Vector2f(size, size * 0.8f));
        shape.setOrigin(0.f, 0.f);
        shape.setPosition(position);
        shape.setRotation(angle);
        shape.setFillColor(bodyColor);
        shape.setOutlineColor(sf::Color(255, 210, 100));
        shape.setOutlineThickness(2.f);
        window.draw(shape);
        break;
    }
    case Form::Square:
    {
        sf::RectangleShape shape(sf::Vector2f(size * 2.f, size * 2.f));
        shape.setOrigin(size, size);
        shape.setPosition(position);
        shape.setFillColor(bodyColor);
        shape.setOutlineColor(sf::Color(140, 255, 140));
        shape.setOutlineThickness(2.5f);
        window.draw(shape);
        break;
    }
    }
}

void Player::transform(Form newForm)
{
    if (newForm == currentForm)
        return;

    int oldMax = maxHealth;
    currentForm = newForm;
    applyFormStats();

    health = (int)((float)health / oldMax * maxHealth);
    if (health < 1) health = 1;

    transformFlash = 0.25f;
    invincibleTimer = 0.15f;
}

sf::Vector2f Player::getPosition() const { return position; }
float Player::getRadius() const { return size; }
Form Player::getForm() const { return currentForm; }
int Player::getHealth() const { return health; }
int Player::getMaxHealth() const { return maxHealth; }
bool Player::isAlive() const { return health > 0; }
bool Player::justTransformed() const { return transformFlash > 0.2f; }

void Player::takeDamage(int amount)
{
    if (invincibleTimer > 0.f)
        return;

    if (currentForm == Form::Square)
        amount = (int)(amount * 0.5f);

    health -= amount;
    if (health < 0) health = 0;
    invincibleTimer = 0.3f;
}

void Player::heal(int amount)
{
    health += amount;
    if (health > maxHealth) health = maxHealth;
}

void Player::reset(float x, float y)
{
    position = sf::Vector2f(x, y);
    facing = sf::Vector2f(0.f, -1.f);
    currentForm = Form::Circle;
    applyFormStats();
    health = maxHealth;
    transformFlash = 0.f;
    invincibleTimer = 0.f;
}
