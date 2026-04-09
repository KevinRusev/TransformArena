#include "Player.h"
#include <cmath>

Player::Player(float startX, float startY)
    : position(startX, startY)
    , velocity(0.f, 0.f)
    , facing(0.f, -1.f)
    , currentForm(Form::Circle)
    , dashing(false)
    , dashTimer(0.f)
    , shootRequest(false)
    , shootTimer(0.f)
    , groundPounding(false)
    , poundDamageReady(false)
    , poundTimer(0.f)
    , cooldownTimer(0.f)
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
        dashSpeed = 700.f;
        dashDamage = 25.f;
        dashDuration = 0.18f;
        abilityCooldown = 0.4f;
        break;

    case Form::Triangle:
        speed = 220.f;
        size = 20.f;
        maxHealth = 90;
        shootCooldown = 0.25f;
        abilityCooldown = 0.25f;
        break;

    case Form::Square:
        speed = 140.f;
        size = 24.f;
        maxHealth = 160;
        poundRadius = 100.f;
        poundDamage = 35.f;
        poundDuration = 0.35f;
        abilityCooldown = 1.0f;
        break;
    }
}

void Player::handleInput(float dt)
{
    if (dashing)
        return;

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
    if (dashing)
    {
        position.x += facing.x * dashSpeed * dt;
        position.y += facing.y * dashSpeed * dt;
        dashTimer -= dt;
        if (dashTimer <= 0.f)
            dashing = false;
    }
    else if (groundPounding)
    {
        poundTimer -= dt;
        if (poundTimer <= 0.f)
            groundPounding = false;
    }
    else
    {
        position.x += velocity.x * speed * dt;
        position.y += velocity.y * speed * dt;
    }

    float padding = size + 5.f;
    if (position.x < padding) position.x = padding;
    if (position.x > 800.f - padding) position.x = 800.f - padding;
    if (position.y < padding) position.y = padding;
    if (position.y > 600.f - padding) position.y = 600.f - padding;

    if (cooldownTimer > 0.f)
        cooldownTimer -= dt;
    if (shootTimer > 0.f)
        shootTimer -= dt;
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

    // invincibility blink
    if (isInvincible())
    {
        int blink = (int)(invincibleTimer * 20.f) % 2;
        if (blink == 0)
            bodyColor.a = 120;
    }

    // ground pound shockwave ring
    if (groundPounding)
    {
        float progress = 1.f - (poundTimer / poundDuration);
        float ringSize = poundRadius * progress;
        sf::CircleShape ring(ringSize);
        ring.setOrigin(ringSize, ringSize);
        ring.setPosition(position);
        sf::Uint8 alpha = (sf::Uint8)(180 * (1.f - progress));
        ring.setFillColor(sf::Color(80, 210, 80, (sf::Uint8)(alpha / 3)));
        ring.setOutlineColor(sf::Color(150, 255, 150, alpha));
        ring.setOutlineThickness(3.f);
        window.draw(ring);
    }

    // dash trail effect
    if (dashing)
    {
        for (int i = 1; i <= 4; i++)
        {
            float offset = (float)i * 12.f;
            sf::CircleShape trail(size * (1.f - i * 0.15f));
            trail.setOrigin(trail.getRadius(), trail.getRadius());
            trail.setPosition(position.x - facing.x * offset, position.y - facing.y * offset);
            sf::Uint8 a = (sf::Uint8)(100 - i * 20);
            trail.setFillColor(sf::Color(80, 180, 255, a));
            window.draw(trail);
        }
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

        // small direction indicator dot
        sf::CircleShape dot(3.f);
        dot.setOrigin(3.f, 3.f);
        dot.setPosition(position.x + facing.x * size * 0.6f, position.y + facing.y * size * 0.6f);
        dot.setFillColor(sf::Color::White);
        window.draw(dot);
        break;
    }
    case Form::Triangle:
    {
        // rotate triangle to face the movement direction
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
        float poundScale = 1.f;
        if (groundPounding)
        {
            float progress = 1.f - (poundTimer / poundDuration);
            poundScale = 1.f + 0.3f * std::sin(progress * 3.14159f);
        }
        float s = size * poundScale;
        sf::RectangleShape shape(sf::Vector2f(s * 2.f, s * 2.f));
        shape.setOrigin(s, s);
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
    if (newForm == currentForm || dashing || groundPounding)
        return;

    int oldMax = maxHealth;
    currentForm = newForm;
    applyFormStats();

    health = (int)((float)health / oldMax * maxHealth);
    if (health < 1) health = 1;

    transformFlash = 0.25f;
    invincibleTimer = 0.15f;
    cooldownTimer = 0.f;
}

void Player::useAbility()
{
    if (cooldownTimer > 0.f)
        return;

    switch (currentForm)
    {
    case Form::Circle:
        if (!dashing)
        {
            dashing = true;
            dashTimer = dashDuration;
            invincibleTimer = dashDuration;
            cooldownTimer = abilityCooldown;
        }
        break;

    case Form::Triangle:
        if (shootTimer <= 0.f)
        {
            shootRequest = true;
            shootTimer = shootCooldown;
            cooldownTimer = abilityCooldown;
        }
        break;

    case Form::Square:
        if (!groundPounding)
        {
            groundPounding = true;
            poundDamageReady = true;
            poundTimer = poundDuration;
            cooldownTimer = abilityCooldown;
        }
        break;
    }
}

bool Player::wantsToShoot()
{
    if (shootRequest)
    {
        shootRequest = false;
        return true;
    }
    return false;
}

void Player::aimAt(sf::Vector2f target)
{
    sf::Vector2f dir = target - position;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 1.f)
    {
        facing.x = dir.x / len;
        facing.y = dir.y / len;
    }
}

sf::Vector2f Player::getPosition() const { return position; }
sf::Vector2f Player::getFacing() const { return facing; }
float Player::getRadius() const { return size; }
Form Player::getForm() const { return currentForm; }
int Player::getHealth() const { return health; }
int Player::getMaxHealth() const { return maxHealth; }
bool Player::isAlive() const { return health > 0; }
bool Player::isDashing() const { return dashing; }
bool Player::isInvincible() const { return invincibleTimer > 0.f; }
bool Player::isGroundPounding() const { return groundPounding; }
bool Player::consumePoundDamage()
{
    if (poundDamageReady)
    {
        poundDamageReady = false;
        return true;
    }
    return false;
}
float Player::getGroundPoundRadius() const { return poundRadius; }
float Player::getGroundPoundDamage() const { return poundDamage; }
float Player::getDashDamage() const { return dashDamage; }
bool Player::justTransformed() const { return transformFlash > 0.2f; }

float Player::getCooldownPercent() const
{
    if (abilityCooldown <= 0.f) return 1.f;
    float pct = 1.f - (cooldownTimer / abilityCooldown);
    if (pct < 0.f) pct = 0.f;
    if (pct > 1.f) pct = 1.f;
    return pct;
}

void Player::takeDamage(int amount)
{
    if (isInvincible())
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
    dashing = false;
    dashTimer = 0.f;
    shootRequest = false;
    shootTimer = 0.f;
    groundPounding = false;
    poundDamageReady = false;
    poundTimer = 0.f;
    cooldownTimer = 0.f;
    transformFlash = 0.f;
    invincibleTimer = 0.f;
}
