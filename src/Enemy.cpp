#include "Enemy.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(float x, float y, EnemyType type, bool isBoss)
    : position(x, y)
    , velocity(0.f, 0.f)
    , knockbackVel(0.f, 0.f)
    , type(type)
    , hitFlash(0.f)
    , dashHitCooldown(0.f)
    , shootTimer(0.f)
    , shootInterval(1.5f)
    , preferredRange(250.f)
    , bossFlag(isBoss)
{
    switch (type)
    {
    case EnemyType::Chaser:
        speed = 130.f;
        size = 11.f;
        health = 25.f;
        contactDamage = 8;
        break;
    case EnemyType::Brute:
        speed = 55.f;
        size = 26.f;
        health = 120.f;
        contactDamage = 25;
        break;
    case EnemyType::Shooter:
        speed = 80.f;
        size = 14.f;
        health = 40.f;
        contactDamage = 10;
        shootInterval = 1.2f;
        preferredRange = 250.f;
        break;
    }
    maxHealth = health;

    if (bossFlag)
    {
        health *= 5.f;
        maxHealth = health;
        size *= 2.f;
        speed *= 1.2f;
        contactDamage *= 2;
        shootInterval = 1.8f;
    }

    shootTimer = (float)(std::rand() % 100) / 100.f * shootInterval;
}

void Enemy::update(float dt, sf::Vector2f playerPos, std::vector<Projectile>& projectiles)
{
    sf::Vector2f dir = playerPos - position;
    float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (dist > 1.f)
    {
        dir.x /= dist;
        dir.y /= dist;
    }

    float currentSpeed = speed;

    switch (type)
    {
    case EnemyType::Chaser:
        // straight chase, speed up slightly when close
        if (dist < 120.f)
            currentSpeed = speed * 1.3f;
        position.x += dir.x * currentSpeed * dt;
        position.y += dir.y * currentSpeed * dt;
        break;

    case EnemyType::Brute:
    {
        if (bossFlag)
        {
            // boss brute: aggressive chase, charge when close
            if (dist < 200.f)
                currentSpeed = speed * 2.5f;
            else
                currentSpeed = speed * 1.2f;

            // strafe slightly to be less predictable
            float wobble = std::sin(position.x * 0.02f + position.y * 0.02f) * 0.3f;
            position.x += (dir.x + wobble * dir.y) * currentSpeed * dt;
            position.y += (dir.y - wobble * dir.x) * currentSpeed * dt;
        }
        else
        {
            if (dist < 150.f)
                currentSpeed = speed * 2.f;
            float wobble = std::sin(position.x * 0.03f + position.y * 0.03f) * 0.2f;
            position.x += (dir.x + wobble * dir.y) * currentSpeed * dt;
            position.y += (dir.y - wobble * dir.x) * currentSpeed * dt;
        }
        break;
    }

    case EnemyType::Shooter:
    {
        // try to maintain distance from player
        if (dist < preferredRange - 40.f)
        {
            // too close, back away
            position.x -= dir.x * currentSpeed * dt;
            position.y -= dir.y * currentSpeed * dt;
        }
        else if (dist > preferredRange + 40.f)
        {
            // too far, approach
            position.x += dir.x * currentSpeed * dt;
            position.y += dir.y * currentSpeed * dt;
        }
        else
        {
            // circle strafe at preferred range
            position.x += dir.y * currentSpeed * 0.5f * dt;
            position.y -= dir.x * currentSpeed * 0.5f * dt;
        }

        shootTimer -= dt;
        if (shootTimer <= 0.f && dist < 400.f)
        {
            shootTimer = shootInterval;
            float bulletSpeed = 200.f;
            sf::Vector2f bulletVel(dir.x * bulletSpeed, dir.y * bulletSpeed);
            projectiles.emplace_back(position, bulletVel, 5.f, 12.f, false, sf::Color(255, 100, 100));
        }
        break;
    }
    }

    // clamp all enemies inside room walls
    float wall = 20.f;
    if (position.x - size < wall) position.x = wall + size;
    if (position.x + size > 800.f - wall) position.x = 800.f - wall - size;
    if (position.y - size < wall) position.y = wall + size;
    if (position.y + size > 600.f - wall) position.y = 600.f - wall - size;

    // boss shooting: spread of projectiles on a timer
    if (bossFlag)
    {
        shootTimer -= dt;
        if (shootTimer <= 0.f && dist < 500.f)
        {
            shootTimer = shootInterval;
            float bulletSpeed = 180.f;
            int numBullets = 5 + (int)(maxHealth / 200.f);
            float hpPct = health / maxHealth;

            if (hpPct < 0.4f)
            {
                // rage mode: ring of bullets
                for (int i = 0; i < 12; i++)
                {
                    float angle = (float)i / 12.f * 6.2832f;
                    sf::Vector2f bv(std::cos(angle) * bulletSpeed, std::sin(angle) * bulletSpeed);
                    projectiles.emplace_back(position, bv, 5.f, 15.f, false, sf::Color(255, 60, 60));
                }
            }
            else
            {
                // spread shot toward player
                float baseAngle = std::atan2(dir.y, dir.x);
                float spread = 0.6f;
                for (int i = 0; i < numBullets; i++)
                {
                    float a = baseAngle - spread / 2.f + spread * ((float)i / (numBullets - 1));
                    sf::Vector2f bv(std::cos(a) * bulletSpeed, std::sin(a) * bulletSpeed);
                    projectiles.emplace_back(position, bv, 5.f, 12.f, false, sf::Color(255, 80, 80));
                }
            }
        }
    }

    // apply and decay knockback
    position.x += knockbackVel.x * dt;
    position.y += knockbackVel.y * dt;
    knockbackVel.x *= 0.9f;
    knockbackVel.y *= 0.9f;
    if (std::abs(knockbackVel.x) < 1.f) knockbackVel.x = 0.f;
    if (std::abs(knockbackVel.y) < 1.f) knockbackVel.y = 0.f;

    if (hitFlash > 0.f)
        hitFlash -= dt;
    if (dashHitCooldown > 0.f)
        dashHitCooldown -= dt;
}

void Enemy::draw(sf::RenderWindow& window)
{
    sf::Color color;
    switch (type)
    {
    case EnemyType::Chaser:  color = sf::Color(220, 60, 60);   break;
    case EnemyType::Brute:   color = sf::Color(160, 50, 180);  break;
    case EnemyType::Shooter: color = sf::Color(220, 180, 40);  break;
    }

    if (hitFlash > 0.f)
        color = sf::Color(255, 255, 255);

    if (bossFlag)
    {
        float glowSize = size * 1.5f;
        sf::CircleShape glow(glowSize);
        glow.setOrigin(glowSize, glowSize);
        glow.setPosition(position);
        glow.setFillColor(sf::Color(255, 40, 40, 30));
        window.draw(glow);
    }

    if (health < maxHealth && !bossFlag)
    {
        float barWidth = size * 2.2f;
        float barHeight = 3.f;
        float healthPct = health / maxHealth;

        sf::RectangleShape bgBar(sf::Vector2f(barWidth, barHeight));
        bgBar.setOrigin(barWidth / 2.f, 0.f);
        bgBar.setPosition(position.x, position.y - size - 10.f);
        bgBar.setFillColor(sf::Color(40, 40, 40));
        window.draw(bgBar);

        sf::RectangleShape hpBar(sf::Vector2f(barWidth * healthPct, barHeight));
        hpBar.setOrigin(barWidth / 2.f, 0.f);
        hpBar.setPosition(position.x, position.y - size - 10.f);
        hpBar.setFillColor(sf::Color(220, 40, 40));
        window.draw(hpBar);
    }

    switch (type)
    {
    case EnemyType::Chaser:
    {
        sf::CircleShape shape(size);
        shape.setOrigin(size, size);
        shape.setPosition(position);
        shape.setFillColor(color);
        shape.setOutlineColor(sf::Color(150, 30, 30));
        shape.setOutlineThickness(1.5f);
        window.draw(shape);
        break;
    }
    case EnemyType::Brute:
    {
        sf::RectangleShape shape(sf::Vector2f(size * 2.f, size * 2.f));
        shape.setOrigin(size, size);
        shape.setPosition(position);
        shape.setFillColor(color);
        shape.setOutlineColor(sf::Color(100, 20, 120));
        shape.setOutlineThickness(2.5f);
        window.draw(shape);

        // inner cross marking
        sf::RectangleShape cross1(sf::Vector2f(size * 1.2f, 3.f));
        cross1.setOrigin(size * 0.6f, 1.5f);
        cross1.setPosition(position);
        cross1.setFillColor(sf::Color(200, 80, 220));
        window.draw(cross1);
        sf::RectangleShape cross2(sf::Vector2f(3.f, size * 1.2f));
        cross2.setOrigin(1.5f, size * 0.6f);
        cross2.setPosition(position);
        cross2.setFillColor(sf::Color(200, 80, 220));
        window.draw(cross2);
        break;
    }
    case EnemyType::Shooter:
    {
        // diamond with inner dot (looks like an eye)
        sf::ConvexShape shape(4);
        shape.setPoint(0, sf::Vector2f(0.f, -size * 1.2f));
        shape.setPoint(1, sf::Vector2f(size, 0.f));
        shape.setPoint(2, sf::Vector2f(0.f, size * 1.2f));
        shape.setPoint(3, sf::Vector2f(-size, 0.f));
        shape.setPosition(position);
        shape.setFillColor(color);
        shape.setOutlineColor(sf::Color(180, 140, 20));
        shape.setOutlineThickness(1.5f);
        window.draw(shape);

        sf::CircleShape eye(3.f);
        eye.setOrigin(3.f, 3.f);
        eye.setPosition(position);
        eye.setFillColor(sf::Color(60, 20, 0));
        window.draw(eye);
        break;
    }
    }
}

void Enemy::takeDamage(float amount)
{
    health -= amount;
    hitFlash = 0.1f;
}

void Enemy::pushAway(sf::Vector2f from, float force)
{
    sf::Vector2f dir = position - from;
    float d = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (d > 0.1f)
    {
        dir.x /= d;
        dir.y /= d;
        if (force < 20.f)
        {
            // small nudge for separation - apply directly
            position.x += dir.x * force;
            position.y += dir.y * force;
        }
        else
        {
            // big hit like ground pound - smooth slide via velocity
            knockbackVel.x += dir.x * force * 8.f;
            knockbackVel.y += dir.y * force * 8.f;
        }
    }
}

bool Enemy::canBeDashHit() const { return dashHitCooldown <= 0.f; }
void Enemy::markDashHit() { dashHitCooldown = 0.3f; }

sf::Vector2f Enemy::getPosition() const { return position; }
float Enemy::getRadius() const { return size; }
float Enemy::getHealth() const { return health; }
float Enemy::getMaxHealth() const { return maxHealth; }
bool Enemy::isAlive() const { return health > 0.f; }
bool Enemy::isBoss() const { return bossFlag; }
int Enemy::getContactDamage() const { return contactDamage; }
EnemyType Enemy::getType() const { return type; }
