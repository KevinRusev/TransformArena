#pragma once
#include <SFML/Graphics.hpp>

struct Particle
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float lifetime;
    float maxLifetime;
    float size;

    Particle(sf::Vector2f pos, sf::Vector2f vel, sf::Color col, float life, float sz)
        : position(pos), velocity(vel), color(col), lifetime(life), maxLifetime(life), size(sz) {}

    void update(float dt)
    {
        position += velocity * dt;
        velocity *= 0.97f;
        lifetime -= dt;
    }

    bool isAlive() const { return lifetime > 0.f; }

    void draw(sf::RenderWindow& window) const
    {
        float t = lifetime / maxLifetime;
        sf::CircleShape shape(size * t);
        shape.setOrigin(size * t, size * t);
        shape.setPosition(position);
        sf::Color c = color;
        c.a = (sf::Uint8)(255 * t);
        shape.setFillColor(c);
        window.draw(shape);
    }
};
