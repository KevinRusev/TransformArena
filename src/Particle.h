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

struct HealthPickup
{
    sf::Vector2f position;
    float lifetime;
    int healAmount;

    HealthPickup(sf::Vector2f pos, int amount)
        : position(pos), lifetime(8.f), healAmount(amount) {}

    void update(float dt) { lifetime -= dt; }
    bool isAlive() const { return lifetime > 0.f; }

    void draw(sf::RenderWindow& window) const
    {
        float pulse = 1.f + 0.15f * std::sin(lifetime * 5.f);
        float r = 8.f * pulse;

        sf::CircleShape glow(r * 2.f);
        glow.setOrigin(r * 2.f, r * 2.f);
        glow.setPosition(position);
        glow.setFillColor(sf::Color(50, 255, 50, 25));
        window.draw(glow);

        // green cross shape
        sf::RectangleShape h(sf::Vector2f(r * 1.6f, r * 0.5f));
        h.setOrigin(r * 0.8f, r * 0.25f);
        h.setPosition(position);
        h.setFillColor(sf::Color(80, 255, 80));
        window.draw(h);

        sf::RectangleShape v(sf::Vector2f(r * 0.5f, r * 1.6f));
        v.setOrigin(r * 0.25f, r * 0.8f);
        v.setPosition(position);
        v.setFillColor(sf::Color(80, 255, 80));
        window.draw(v);
    }
};
