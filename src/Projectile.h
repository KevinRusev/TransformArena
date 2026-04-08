#pragma once
#include <SFML/Graphics.hpp>

struct Projectile
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    float radius;
    float damage;
    float lifetime;
    bool fromPlayer;
    sf::Color color;

    Projectile(sf::Vector2f pos, sf::Vector2f vel, float r, float dmg, bool player, sf::Color col)
        : position(pos), velocity(vel), radius(r), damage(dmg), lifetime(3.f), fromPlayer(player), color(col) {}

    void update(float dt)
    {
        position += velocity * dt;
        lifetime -= dt;
    }

    bool isAlive() const
    {
        return lifetime > 0.f
            && position.x > -50.f && position.x < 850.f
            && position.y > -50.f && position.y < 650.f;
    }

    void draw(sf::RenderWindow& window) const
    {
        sf::CircleShape shape(radius);
        shape.setOrigin(radius, radius);
        shape.setPosition(position);
        shape.setFillColor(color);

        // glow ring around projectile
        sf::CircleShape glow(radius * 2.f);
        glow.setOrigin(radius * 2.f, radius * 2.f);
        glow.setPosition(position);
        glow.setFillColor(sf::Color(color.r, color.g, color.b, 30));
        window.draw(glow);
        window.draw(shape);
    }
};
