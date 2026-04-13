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
        float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (speed > 1.f)
        {
            sf::Vector2f dir = velocity / speed;
            float trailLen = std::min(speed * 0.04f, 20.f);
            for (int i = 1; i <= 4; i++)
            {
                float t = (float)i / 4.f;
                float r = radius * (1.f - t * 0.5f);
                sf::CircleShape trail(r);
                trail.setOrigin(r, r);
                trail.setPosition(position - dir * trailLen * t);
                trail.setFillColor(sf::Color(color.r, color.g, color.b, (sf::Uint8)(100 * (1.f - t))));
                window.draw(trail);
            }
        }

        sf::CircleShape glow(radius * 2.f);
        glow.setOrigin(radius * 2.f, radius * 2.f);
        glow.setPosition(position);
        glow.setFillColor(sf::Color(color.r, color.g, color.b, 30));
        window.draw(glow);

        sf::CircleShape shape(radius);
        shape.setOrigin(radius, radius);
        shape.setPosition(position);
        shape.setFillColor(color);
        window.draw(shape);
    }
};
