#include "Enemy.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(float x, float y, EnemyType type, bool isBoss, int bossType)
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
    , bossTypeId(bossType)
    , shieldUp(false)
    , shieldBreakTimer(0.f)
    , shieldAngle(0.f)
    , stunned(false)
    , stunnedTimer(0.f)
    , teleportTimer(3.f)
    , bossPhaseTimer(0.f)
    , bossPhase(0)
    , dashCharging(false)
    , dashChargeTimer(0.f)
    , dashDir(0.f, 0.f)
    , dashSpeed(0.f)
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
    case EnemyType::Dasher:
        speed = 90.f;
        size = 13.f;
        health = 35.f;
        contactDamage = 15;
        dashSpeed = 500.f;
        break;
    case EnemyType::Shielder:
        speed = 45.f;
        size = 18.f;
        health = 80.f;
        contactDamage = 12;
        shieldUp = true;
        break;
    }
    maxHealth = health;

    if (bossFlag)
    {
        size *= 2.f;
        contactDamage *= 2;
        shootInterval = 2.f;

        switch (bossTypeId)
        {
        case 1: // Guardian
            health = 500.f;
            speed = 70.f;
            shieldUp = true;
            break;
        case 2: // Phantom
            health = 600.f;
            speed = 220.f;
            teleportTimer = 1.2f;
            shootInterval = 0.6f;
            break;
        case 3: // Hive
            health = 600.f;
            speed = 40.f;
            shootInterval = 1.5f;
            break;
        default:
            health *= 5.f;
            speed *= 1.2f;
            break;
        }
        maxHealth = health;
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

    if (stunned)
    {
        stunnedTimer -= dt;
        if (stunnedTimer <= 0.f) stunned = false;
        currentSpeed = speed * 0.15f;
    }

    if (shieldBreakTimer > 0.f)
    {
        shieldBreakTimer -= dt;
        if (shieldBreakTimer <= 0.f && bossFlag && bossTypeId == 1)
            shieldUp = true;
    }

    if (bossFlag)
        shieldAngle += dt * 120.f;

    // --- BOSS BEHAVIOR ---
    if (bossFlag && bossTypeId == 1) // Guardian
    {
        if (dist < 200.f)
            currentSpeed = speed * 2.2f;

        float wobble = std::sin(bossPhaseTimer * 2.f) * 0.3f;
        position.x += (dir.x + wobble * dir.y) * currentSpeed * dt;
        position.y += (dir.y - wobble * dir.x) * currentSpeed * dt;

        bossPhaseTimer += dt;
        shootTimer -= dt;
        if (shootTimer <= 0.f && !stunned)
        {
            shootTimer = shootInterval;
            float bulletSpeed = 170.f;
            float baseAngle = std::atan2(dir.y, dir.x);

            if (health / maxHealth < 0.5f)
            {
                // rage: wider spread + ring
                for (int i = 0; i < 8; i++)
                {
                    float a = baseAngle - 0.8f + 1.6f * ((float)i / 7.f);
                    sf::Vector2f bv(std::cos(a) * bulletSpeed, std::sin(a) * bulletSpeed);
                    projectiles.emplace_back(position, bv, 5.f, 15.f, false, sf::Color(180, 80, 255));
                }
            }
            else
            {
                for (int i = 0; i < 5; i++)
                {
                    float a = baseAngle - 0.4f + 0.8f * ((float)i / 4.f);
                    sf::Vector2f bv(std::cos(a) * bulletSpeed, std::sin(a) * bulletSpeed);
                    projectiles.emplace_back(position, bv, 5.f, 12.f, false, sf::Color(160, 60, 220));
                }
            }
        }
    }
    else if (bossFlag && bossTypeId == 2) // Phantom
    {
        float hpPct = health / maxHealth;

        if (!stunned)
        {
            teleportTimer -= dt;
            if (teleportTimer <= 0.f)
            {
                teleportTimer = hpPct < 0.4f ? 0.7f + (float)(std::rand() % 50) / 100.f
                                             : 1.0f + (float)(std::rand() % 60) / 100.f;
                // teleport to a spot away from the player
                for (int tries = 0; tries < 10; tries++)
                {
                    float nx = 80.f + (float)(std::rand() % 640);
                    float ny = 80.f + (float)(std::rand() % 440);
                    float ddx = nx - playerPos.x, ddy = ny - playerPos.y;
                    float dd = std::sqrt(ddx * ddx + ddy * ddy);
                    if (dd > 200.f || tries == 9) { position.x = nx; position.y = ny; break; }
                }
            }

            // actively flee from player when close, strafe when far
            if (dist < 180.f)
            {
                position.x -= dir.x * currentSpeed * 1.5f * dt;
                position.y -= dir.y * currentSpeed * 1.5f * dt;
            }
            else
            {
                float zigzag = std::sin(bossPhaseTimer * 6.f);
                position.x += (dir.y * zigzag) * currentSpeed * dt;
                position.y += (-dir.x * zigzag) * currentSpeed * dt;
            }
        }
        else
        {
            position.x += dir.x * currentSpeed * 0.1f * dt;
            position.y += dir.y * currentSpeed * 0.1f * dt;
        }

        bossPhaseTimer += dt;
        shootTimer -= dt;
        if (shootTimer <= 0.f)
        {
            shootTimer = stunned ? shootInterval * 2.f : shootInterval;
            float bulletSpeed = 280.f;

            // alternate between aimed burst and spread
            bossPhase = (bossPhase + 1) % 3;

            if (bossPhase == 0)
            {
                // 5-shot aimed fan
                for (int i = 0; i < 5; i++)
                {
                    float spread = ((float)i - 2.f) * 0.18f;
                    float a = std::atan2(dir.y, dir.x) + spread;
                    sf::Vector2f bv(std::cos(a) * bulletSpeed, std::sin(a) * bulletSpeed);
                    projectiles.emplace_back(position, bv, 4.f, 12.f, false, sf::Color(80, 255, 180));
                }
            }
            else if (bossPhase == 1)
            {
                // cross pattern (4 directions + diagonals)
                for (int i = 0; i < 8; i++)
                {
                    float a = (float)i / 8.f * 6.2832f + bossPhaseTimer * 0.5f;
                    sf::Vector2f bv(std::cos(a) * bulletSpeed * 0.9f, std::sin(a) * bulletSpeed * 0.9f);
                    projectiles.emplace_back(position, bv, 4.f, 10.f, false, sf::Color(60, 220, 160));
                }
            }
            else
            {
                // predict player position and fire ahead
                float a = std::atan2(dir.y, dir.x);
                for (int i = -1; i <= 1; i++)
                {
                    float predict = a + i * 0.3f;
                    sf::Vector2f bv(std::cos(predict) * bulletSpeed * 1.1f, std::sin(predict) * bulletSpeed * 1.1f);
                    projectiles.emplace_back(position, bv, 5.f, 14.f, false, sf::Color(100, 255, 200));
                }
            }

            if (hpPct < 0.5f)
            {
                // bonus ring every shot when below half
                int ringCount = hpPct < 0.25f ? 14 : 10;
                for (int i = 0; i < ringCount; i++)
                {
                    float a = (float)i / ringCount * 6.2832f + bossPhaseTimer;
                    sf::Vector2f bv(std::cos(a) * 180.f, std::sin(a) * 180.f);
                    projectiles.emplace_back(position, bv, 4.f, 10.f, false, sf::Color(60, 220, 160));
                }
            }
        }
    }
    else if (bossFlag && bossTypeId == 3) // Hive
    {
        position.x += dir.x * currentSpeed * dt;
        position.y += dir.y * currentSpeed * dt;

        bossPhaseTimer += dt;
        shootTimer -= dt;

        float hpPct = health / maxHealth;
        float actualInterval = hpPct < 0.5f ? shootInterval * 0.6f : shootInterval;

        if (shootTimer <= 0.f)
        {
            shootTimer = actualInterval;
            float bulletSpeed = 150.f;

            // ring of bullets
            int count = hpPct < 0.5f ? 16 : 10;
            float offset = bossPhaseTimer * 0.5f;
            for (int i = 0; i < count; i++)
            {
                float a = offset + (float)i / count * 6.2832f;
                sf::Vector2f bv(std::cos(a) * bulletSpeed, std::sin(a) * bulletSpeed);
                projectiles.emplace_back(position, bv, 5.f, 12.f, false, sf::Color(255, 160, 40));
            }
        }
    }
    else if (bossFlag) // generic boss fallback
    {
        if (dist < 200.f) currentSpeed = speed * 2.5f;
        position.x += dir.x * currentSpeed * dt;
        position.y += dir.y * currentSpeed * dt;

        shootTimer -= dt;
        if (shootTimer <= 0.f)
        {
            shootTimer = shootInterval;
            float baseAngle = std::atan2(dir.y, dir.x);
            for (int i = 0; i < 5; i++)
            {
                float a = baseAngle - 0.4f + 0.8f * ((float)i / 4.f);
                sf::Vector2f bv(std::cos(a) * 180.f, std::sin(a) * 180.f);
                projectiles.emplace_back(position, bv, 5.f, 12.f, false, sf::Color(255, 80, 80));
            }
        }
    }
    // --- REGULAR ENEMY BEHAVIOR ---
    else
    {
        switch (type)
        {
        case EnemyType::Chaser:
            if (dist < 120.f) currentSpeed = speed * 1.3f;
            position.x += dir.x * currentSpeed * dt;
            position.y += dir.y * currentSpeed * dt;
            break;

        case EnemyType::Brute:
        {
            if (dist < 150.f) currentSpeed = speed * 2.f;
            float wobble = std::sin(position.x * 0.03f + position.y * 0.03f) * 0.2f;
            position.x += (dir.x + wobble * dir.y) * currentSpeed * dt;
            position.y += (dir.y - wobble * dir.x) * currentSpeed * dt;
            break;
        }

        case EnemyType::Shooter:
        {
            if (dist < preferredRange - 40.f)
            {
                position.x -= dir.x * currentSpeed * dt;
                position.y -= dir.y * currentSpeed * dt;
            }
            else if (dist > preferredRange + 40.f)
            {
                position.x += dir.x * currentSpeed * dt;
                position.y += dir.y * currentSpeed * dt;
            }
            else
            {
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

        case EnemyType::Dasher:
        {
            if (dashCharging)
            {
                dashChargeTimer -= dt;
                if (dashChargeTimer <= 0.f)
                {
                    dashCharging = false;
                    dashChargeTimer = 2.f + (float)(std::rand() % 200) / 100.f;
                }
                else
                {
                    position.x += dashDir.x * dashSpeed * dt;
                    position.y += dashDir.y * dashSpeed * dt;
                }
            }
            else
            {
                dashDir = dir;
                position.x += dir.y * currentSpeed * 0.6f * dt;
                position.y -= dir.x * currentSpeed * 0.6f * dt;
                if (dist > 300.f)
                {
                    position.x += dir.x * currentSpeed * dt;
                    position.y += dir.y * currentSpeed * dt;
                }

                dashChargeTimer -= dt;
                if (dashChargeTimer <= 0.f && dist < 350.f)
                {
                    dashCharging = true;
                    dashChargeTimer = 0.3f;
                    dashDir = dir;
                }
            }
            break;
        }

        case EnemyType::Shielder:
        {
            position.x += dir.x * currentSpeed * dt;
            position.y += dir.y * currentSpeed * dt;
            break;
        }
        }
    }

    // clamp all enemies inside room walls
    float wall = 20.f;
    if (position.x - size < wall) position.x = wall + size;
    if (position.x + size > 800.f - wall) position.x = 800.f - wall - size;
    if (position.y - size < wall) position.y = wall + size;
    if (position.y + size > 600.f - wall) position.y = 600.f - wall - size;

    // apply and decay knockback
    position.x += knockbackVel.x * dt;
    position.y += knockbackVel.y * dt;
    knockbackVel.x *= 0.9f;
    knockbackVel.y *= 0.9f;
    if (std::abs(knockbackVel.x) < 1.f) knockbackVel.x = 0.f;
    if (std::abs(knockbackVel.y) < 1.f) knockbackVel.y = 0.f;

    if (hitFlash > 0.f) hitFlash -= dt;
    if (dashHitCooldown > 0.f) dashHitCooldown -= dt;
}

void Enemy::draw(sf::RenderWindow& window)
{
    sf::Color color;
    switch (type)
    {
    case EnemyType::Chaser:   color = sf::Color(220, 60, 60);   break;
    case EnemyType::Brute:    color = sf::Color(160, 50, 180);  break;
    case EnemyType::Shooter:  color = sf::Color(220, 180, 40);  break;
    case EnemyType::Dasher:   color = sf::Color(255, 100, 60);  break;
    case EnemyType::Shielder: color = sf::Color(60, 160, 200);  break;
    }

    if (hitFlash > 0.f)
        color = sf::Color(255, 255, 255);

    // boss visuals
    if (bossFlag)
    {
        // glow
        float glowSize = size * 1.6f;
        sf::CircleShape glow(glowSize, 32);
        glow.setOrigin(glowSize, glowSize);
        glow.setPosition(position);

        if (bossTypeId == 1) // Guardian - purple glow
            glow.setFillColor(sf::Color(140, 40, 200, 35));
        else if (bossTypeId == 2) // Phantom - green glow
            glow.setFillColor(sf::Color(40, 200, 120, 35));
        else if (bossTypeId == 3) // Hive - orange glow
            glow.setFillColor(sf::Color(200, 120, 30, 35));
        else
            glow.setFillColor(sf::Color(255, 40, 40, 30));
        window.draw(glow);

        // Guardian shield rings
        if (bossTypeId == 1 && shieldUp)
        {
            for (int i = 0; i < 2; i++)
            {
                float r = size + 10.f + i * 8.f;
                float angle = shieldAngle + i * 45.f;
                sf::CircleShape shield(r, 6);
                shield.setOrigin(r, r);
                shield.setPosition(position);
                shield.setRotation(angle);
                shield.setFillColor(sf::Color::Transparent);
                shield.setOutlineColor(sf::Color(180, 100, 255, 160 - i * 40));
                shield.setOutlineThickness(2.5f);
                window.draw(shield);
            }
        }

        // Phantom afterimage when not stunned
        if (bossTypeId == 2 && !stunned)
        {
            for (int i = 1; i <= 3; i++)
            {
                sf::CircleShape ghost(size * (1.f - i * 0.1f), 3);
                ghost.setOrigin(ghost.getRadius(), ghost.getRadius());
                float ox = std::sin(bossPhaseTimer * 3.f + i) * 10.f;
                float oy = std::cos(bossPhaseTimer * 3.f + i) * 10.f;
                ghost.setPosition(position.x + ox, position.y + oy);
                ghost.setFillColor(sf::Color(60, 200, 140, 60 - i * 15));
                window.draw(ghost);
            }
        }

        // Phantom stun indicator
        if (bossTypeId == 2 && stunned)
        {
            for (int i = 0; i < 3; i++)
            {
                float a = bossPhaseTimer * 4.f + i * 2.094f;
                float ox = std::cos(a) * (size + 8.f);
                float oy = std::sin(a) * (size + 8.f);
                sf::CircleShape star(4.f, 4);
                star.setOrigin(4.f, 4.f);
                star.setPosition(position.x + ox, position.y - size - 5.f + oy * 0.3f);
                star.setFillColor(sf::Color(255, 255, 100));
                window.draw(star);
            }
        }

        // Hive orbiting particles
        if (bossTypeId == 3)
        {
            for (int i = 0; i < 6; i++)
            {
                float a = bossPhaseTimer * 1.5f + i * 1.047f;
                float r = size + 15.f;
                float ox = std::cos(a) * r;
                float oy = std::sin(a) * r;
                sf::CircleShape orb(4.f);
                orb.setOrigin(4.f, 4.f);
                orb.setPosition(position.x + ox, position.y + oy);
                orb.setFillColor(sf::Color(255, 180, 60, 180));
                window.draw(orb);
            }
        }
    }

    // health bar (non-boss only)
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

    // draw enemy body shape
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
    case EnemyType::Dasher:
    {
        sf::Vector2f fDir = dashDir;
        if (std::abs(fDir.x) < 0.01f && std::abs(fDir.y) < 0.01f)
            fDir = sf::Vector2f(0.f, 1.f);
        float angle = std::atan2(fDir.y, fDir.x) * 57.3f + 90.f;
        sf::ConvexShape shape(3);
        shape.setPoint(0, sf::Vector2f(0.f, -size * 1.3f));
        shape.setPoint(1, sf::Vector2f(-size, size * 0.8f));
        shape.setPoint(2, sf::Vector2f(size, size * 0.8f));
        shape.setPosition(position);
        shape.setRotation(angle);
        shape.setFillColor(color);
        shape.setOutlineColor(sf::Color(200, 60, 30));
        shape.setOutlineThickness(1.5f);
        window.draw(shape);

        // charge warning flash
        if (dashCharging)
        {
            sf::CircleShape flash(size * 1.3f);
            flash.setOrigin(size * 1.3f, size * 1.3f);
            flash.setPosition(position);
            flash.setFillColor(sf::Color(255, 100, 40, 60));
            window.draw(flash);
        }
        break;
    }
    case EnemyType::Shielder:
    {
        sf::CircleShape shape(size, 5);
        shape.setOrigin(size, size);
        shape.setPosition(position);
        shape.setFillColor(color);
        shape.setOutlineColor(sf::Color(40, 120, 180));
        shape.setOutlineThickness(2.f);
        window.draw(shape);

        if (shieldUp)
        {
            sf::CircleShape shieldVis(size + 6.f, 5);
            shieldVis.setOrigin(size + 6.f, size + 6.f);
            shieldVis.setPosition(position);
            shieldVis.setFillColor(sf::Color::Transparent);
            shieldVis.setOutlineColor(sf::Color(100, 200, 255, 140));
            shieldVis.setOutlineThickness(2.f);
            window.draw(shieldVis);
        }
        break;
    }
    }
}

void Enemy::takeDamage(float amount)
{
    // Shielder enemies: shield absorbs 60% damage
    if (type == EnemyType::Shielder && shieldUp && !bossFlag)
        amount *= 0.4f;

    // Guardian boss: shield absorbs 80% damage
    if (bossFlag && bossTypeId == 1 && shieldUp)
        amount *= 0.2f;

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
            position.x += dir.x * force;
            position.y += dir.y * force;
        }
        else
        {
            knockbackVel.x += dir.x * force * 8.f;
            knockbackVel.y += dir.y * force * 8.f;
        }
    }
}

void Enemy::breakShield(float duration)
{
    shieldUp = false;
    shieldBreakTimer = duration;
}

void Enemy::stun(float duration)
{
    stunned = true;
    stunnedTimer = duration;
}

void Enemy::setPosition(float x, float y) { position = sf::Vector2f(x, y); }
bool Enemy::isShieldUp() const { return shieldUp; }
bool Enemy::isStunned() const { return stunned; }

bool Enemy::canBeDashHit() const { return dashHitCooldown <= 0.f; }
void Enemy::markDashHit() { dashHitCooldown = 0.3f; }

sf::Vector2f Enemy::getPosition() const { return position; }
float Enemy::getRadius() const { return size; }
float Enemy::getHealth() const { return health; }
float Enemy::getMaxHealth() const { return maxHealth; }
bool Enemy::isAlive() const { return health > 0.f; }
bool Enemy::isBoss() const { return bossFlag; }
int Enemy::getBossType() const { return bossTypeId; }
int Enemy::getContactDamage() const { return contactDamage; }
EnemyType Enemy::getType() const { return type; }
