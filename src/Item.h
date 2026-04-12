#pragma once
#include <SFML/Graphics.hpp>
#include <string>

enum class ItemType { None, FlameRing, FrostShard, ThunderStrike, ShadowDash, BarrierShield, VortexPull, MirrorClone, ChainLightning };

struct Item
{
    ItemType type;
    std::string name;
    std::string desc;
    int cost;
    float cooldown;
    float cooldownTimer;
    bool sold;
    float shopX, shopY;

    Item()
        : type(ItemType::None), cost(0), cooldown(0.f), cooldownTimer(0.f)
        , sold(false), shopX(0.f), shopY(0.f) {}

    static Item create(ItemType t, int floor)
    {
        Item item;
        item.type = t;
        item.sold = false;
        item.cooldownTimer = 0.f;

        switch (t)
        {
        case ItemType::FlameRing:
            item.name = "Flame Ring";
            item.desc = "Fire burst around you";
            item.cost = 30 + floor * 8;
            item.cooldown = 4.f;
            break;
        case ItemType::FrostShard:
            item.name = "Frost Shard";
            item.desc = "3 piercing ice shots";
            item.cost = 35 + floor * 8;
            item.cooldown = 3.f;
            break;
        case ItemType::ThunderStrike:
            item.name = "Thunder Strike";
            item.desc = "Zap 3 nearest enemies";
            item.cost = 45 + floor * 10;
            item.cooldown = 5.f;
            break;
        case ItemType::ShadowDash:
            item.name = "Shadow Dash";
            item.desc = "Teleport + damage trail";
            item.cost = 40 + floor * 8;
            item.cooldown = 4.f;
            break;
        case ItemType::BarrierShield:
            item.name = "Barrier Shield";
            item.desc = "Block bullets for 3s";
            item.cost = 35 + floor * 8;
            item.cooldown = 6.f;
            break;
        case ItemType::VortexPull:
            item.name = "Vortex Pull";
            item.desc = "Pull all enemies to you";
            item.cost = 55 + floor * 10;
            item.cooldown = 7.f;
            break;
        case ItemType::MirrorClone:
            item.name = "Mirror Clone";
            item.desc = "Spawn a decoy that draws fire";
            item.cost = 50 + floor * 10;
            item.cooldown = 8.f;
            break;
        case ItemType::ChainLightning:
            item.name = "Chain Lightning";
            item.desc = "Bolt chains between all enemies";
            item.cost = 60 + floor * 12;
            item.cooldown = 6.f;
            break;
        default:
            item.name = "None";
            item.desc = "";
            item.cost = 0;
            item.cooldown = 0.f;
            break;
        }
        return item;
    }

    bool isReady() const { return cooldownTimer <= 0.f && type != ItemType::None; }
    float getCooldownPercent() const
    {
        if (cooldown <= 0.f) return 1.f;
        float pct = 1.f - (cooldownTimer / cooldown);
        return pct < 0.f ? 0.f : (pct > 1.f ? 1.f : pct);
    }

    static void drawIcon(sf::RenderWindow& window, ItemType t, float cx, float cy, float scale)
    {
        switch (t)
        {
        case ItemType::FlameRing:
        {
            // orange/red ring with flame spikes
            sf::CircleShape ring(14.f * scale);
            ring.setOrigin(14.f * scale, 14.f * scale);
            ring.setPosition(cx, cy);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineColor(sf::Color(255, 120, 30));
            ring.setOutlineThickness(3.f * scale);
            window.draw(ring);

            for (int i = 0; i < 6; i++)
            {
                float a = (float)i / 6.f * 6.2832f;
                float tx = cx + std::cos(a) * 14.f * scale;
                float ty = cy + std::sin(a) * 14.f * scale;
                sf::CircleShape spike(4.f * scale, 3);
                spike.setOrigin(4.f * scale, 4.f * scale);
                spike.setPosition(tx, ty);
                spike.setRotation(a * 57.3f);
                spike.setFillColor(sf::Color(255, 80, 20));
                window.draw(spike);
            }
            break;
        }
        case ItemType::FrostShard:
        {
            // 3 cyan diamond shards
            for (int i = -1; i <= 1; i++)
            {
                sf::ConvexShape shard(4);
                float ox = (float)i * 9.f * scale;
                shard.setPoint(0, sf::Vector2f(0.f, -10.f * scale));
                shard.setPoint(1, sf::Vector2f(4.f * scale, 0.f));
                shard.setPoint(2, sf::Vector2f(0.f, 10.f * scale));
                shard.setPoint(3, sf::Vector2f(-4.f * scale, 0.f));
                shard.setPosition(cx + ox, cy);
                shard.setFillColor(sf::Color(100, 220, 255));
                shard.setOutlineColor(sf::Color(200, 240, 255));
                shard.setOutlineThickness(1.f);
                window.draw(shard);
            }
            break;
        }
        case ItemType::ThunderStrike:
        {
            // yellow lightning bolt zigzag
            sf::ConvexShape bolt(6);
            float s = scale;
            bolt.setPoint(0, sf::Vector2f(-3.f * s, -14.f * s));
            bolt.setPoint(1, sf::Vector2f(5.f * s, -4.f * s));
            bolt.setPoint(2, sf::Vector2f(-1.f * s, -2.f * s));
            bolt.setPoint(3, sf::Vector2f(6.f * s, 14.f * s));
            bolt.setPoint(4, sf::Vector2f(-4.f * s, 4.f * s));
            bolt.setPoint(5, sf::Vector2f(1.f * s, 2.f * s));
            bolt.setPosition(cx, cy);
            bolt.setFillColor(sf::Color(255, 240, 60));
            bolt.setOutlineColor(sf::Color(255, 200, 0));
            bolt.setOutlineThickness(1.f);
            window.draw(bolt);
            break;
        }
        case ItemType::ShadowDash:
        {
            // purple arrow/chevron
            sf::ConvexShape arrow(5);
            float s = scale;
            arrow.setPoint(0, sf::Vector2f(0.f, -14.f * s));
            arrow.setPoint(1, sf::Vector2f(10.f * s, 0.f));
            arrow.setPoint(2, sf::Vector2f(6.f * s, 0.f));
            arrow.setPoint(3, sf::Vector2f(0.f, 10.f * s));
            arrow.setPoint(4, sf::Vector2f(-10.f * s, 0.f));
            arrow.setPosition(cx, cy);
            arrow.setFillColor(sf::Color(160, 60, 220));
            arrow.setOutlineColor(sf::Color(200, 120, 255));
            arrow.setOutlineThickness(1.f);
            window.draw(arrow);

            // trail dots
            for (int i = 1; i <= 3; i++)
            {
                sf::CircleShape dot(2.f * s);
                dot.setOrigin(2.f * s, 2.f * s);
                dot.setPosition(cx, cy + (6.f + i * 5.f) * s);
                dot.setFillColor(sf::Color(120, 40, 180, 200 - i * 50));
                window.draw(dot);
            }
            break;
        }
        case ItemType::BarrierShield:
        {
            // blue hexagonal shield
            sf::CircleShape hex(13.f * scale, 6);
            hex.setOrigin(13.f * scale, 13.f * scale);
            hex.setPosition(cx, cy);
            hex.setFillColor(sf::Color(40, 80, 200, 120));
            hex.setOutlineColor(sf::Color(100, 160, 255));
            hex.setOutlineThickness(2.5f * scale);
            window.draw(hex);

            sf::CircleShape inner(6.f * scale, 6);
            inner.setOrigin(6.f * scale, 6.f * scale);
            inner.setPosition(cx, cy);
            inner.setFillColor(sf::Color(80, 140, 255, 100));
            window.draw(inner);
            break;
        }
        case ItemType::VortexPull:
        {
            for (int i = 0; i < 3; i++)
            {
                float r = (12.f - i * 3.f) * scale;
                sf::CircleShape ring(r, 16);
                ring.setOrigin(r, r);
                ring.setPosition(cx, cy);
                ring.setFillColor(sf::Color::Transparent);
                ring.setOutlineColor(sf::Color(200, 60, 255, 200 - i * 50));
                ring.setOutlineThickness(2.f * scale);
                window.draw(ring);
            }
            sf::CircleShape core(3.f * scale);
            core.setOrigin(3.f * scale, 3.f * scale);
            core.setPosition(cx, cy);
            core.setFillColor(sf::Color(255, 100, 255));
            window.draw(core);
            break;
        }
        case ItemType::MirrorClone:
        {
            for (int m = -1; m <= 1; m += 2)
            {
                float ox = (float)m * 8.f * scale;
                sf::RectangleShape body(sf::Vector2f(8.f * scale, 16.f * scale));
                body.setOrigin(4.f * scale, 8.f * scale);
                body.setPosition(cx + ox, cy);
                body.setFillColor(sf::Color(180, 180, 220, m > 0 ? 120 : 220));
                body.setOutlineColor(sf::Color(220, 220, 255));
                body.setOutlineThickness(1.f);
                window.draw(body);
            }
            break;
        }
        case ItemType::ChainLightning:
        {
            sf::ConvexShape bolt(6);
            float s = scale;
            bolt.setPoint(0, sf::Vector2f(-5.f * s, -12.f * s));
            bolt.setPoint(1, sf::Vector2f(3.f * s, -4.f * s));
            bolt.setPoint(2, sf::Vector2f(-2.f * s, -2.f * s));
            bolt.setPoint(3, sf::Vector2f(7.f * s, 12.f * s));
            bolt.setPoint(4, sf::Vector2f(-1.f * s, 4.f * s));
            bolt.setPoint(5, sf::Vector2f(2.f * s, 2.f * s));
            bolt.setPosition(cx, cy);
            bolt.setFillColor(sf::Color(100, 200, 255));
            bolt.setOutlineColor(sf::Color(180, 230, 255));
            bolt.setOutlineThickness(1.5f);
            window.draw(bolt);

            for (int i = 0; i < 3; i++)
            {
                float a = (float)i / 3.f * 6.2832f;
                sf::CircleShape spark(2.f * s);
                spark.setOrigin(2.f * s, 2.f * s);
                spark.setPosition(cx + std::cos(a) * 12.f * s, cy + std::sin(a) * 12.f * s);
                spark.setFillColor(sf::Color(150, 220, 255));
                window.draw(spark);
            }
            break;
        }
        default: break;
        }
    }
};
