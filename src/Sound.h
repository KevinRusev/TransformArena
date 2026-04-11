#pragma once
#include <SFML/Audio.hpp>
#include <cmath>
#include <vector>
#include <cstdlib>

class SoundSystem
{
public:
    enum SoundID { HIT, DASH, SHOOT, SLAM, COIN, BOSS_HIT, PORTAL, DEATH, TRANSFORM, BUY, COUNT };

    SoundSystem()
    {
        generateHit();
        generateDash();
        generateShoot();
        generateSlam();
        generateCoin();
        generateBossHit();
        generatePortal();
        generateDeath();
        generateTransform();
        generateBuy();

        for (int i = 0; i < COUNT; i++)
            sounds[i].setBuffer(buffers[i]);
    }

    void play(SoundID id)
    {
        if (id < 0 || id >= COUNT) return;
        sounds[id].stop();
        sounds[id].play();
    }

    void setVolume(SoundID id, float vol)
    {
        if (id >= 0 && id < COUNT)
            sounds[id].setVolume(vol);
    }

private:
    sf::SoundBuffer buffers[COUNT];
    sf::Sound sounds[COUNT];

    static constexpr unsigned RATE = 44100;
    static constexpr float PI = 3.14159265f;

    void loadBuffer(SoundID id, const std::vector<sf::Int16>& samples)
    {
        buffers[id].loadFromSamples(samples.data(), samples.size(), 1, RATE);
    }

    void generateHit()
    {
        int len = (int)(RATE * 0.06f);
        std::vector<sf::Int16> s(len);
        for (int i = 0; i < len; i++)
        {
            float t = (float)i / RATE;
            float env = 1.f - (float)i / len;
            float noise = ((float)(std::rand() % 20000) - 10000) / 10000.f;
            s[i] = (sf::Int16)(noise * env * 8000);
        }
        loadBuffer(HIT, s);
    }

    void generateDash()
    {
        int len = (int)(RATE * 0.12f);
        std::vector<sf::Int16> s(len);
        for (int i = 0; i < len; i++)
        {
            float t = (float)i / RATE;
            float env = 1.f - (float)i / len;
            float freq = 200.f + 600.f * ((float)i / len);
            s[i] = (sf::Int16)(std::sin(2.f * PI * freq * t) * env * 6000);
        }
        loadBuffer(DASH, s);
    }

    void generateShoot()
    {
        int len = (int)(RATE * 0.08f);
        std::vector<sf::Int16> s(len);
        for (int i = 0; i < len; i++)
        {
            float t = (float)i / RATE;
            float env = 1.f - (float)i / len;
            float freq = 800.f - 400.f * ((float)i / len);
            s[i] = (sf::Int16)(std::sin(2.f * PI * freq * t) * env * 5000);
        }
        loadBuffer(SHOOT, s);
    }

    void generateSlam()
    {
        int len = (int)(RATE * 0.2f);
        std::vector<sf::Int16> s(len);
        for (int i = 0; i < len; i++)
        {
            float t = (float)i / RATE;
            float env = 1.f - (float)i / len;
            env *= env;
            float freq = 80.f + 30.f * std::sin(t * 20.f);
            float noise = ((float)(std::rand() % 10000) - 5000) / 10000.f;
            s[i] = (sf::Int16)((std::sin(2.f * PI * freq * t) * 0.7f + noise * 0.3f) * env * 10000);
        }
        loadBuffer(SLAM, s);
    }

    void generateCoin()
    {
        int len = (int)(RATE * 0.15f);
        std::vector<sf::Int16> s(len);
        int half = len / 2;
        for (int i = 0; i < len; i++)
        {
            float t = (float)i / RATE;
            float env = 1.f - (float)i / len;
            float freq = (i < half) ? 1200.f : 1600.f;
            s[i] = (sf::Int16)(std::sin(2.f * PI * freq * t) * env * 4000);
        }
        loadBuffer(COIN, s);
    }

    void generateBossHit()
    {
        int len = (int)(RATE * 0.25f);
        std::vector<sf::Int16> s(len);
        for (int i = 0; i < len; i++)
        {
            float t = (float)i / RATE;
            float env = 1.f - (float)i / len;
            env *= env;
            float noise = ((float)(std::rand() % 20000) - 10000) / 10000.f;
            s[i] = (sf::Int16)((std::sin(2.f * PI * 60.f * t) * 0.6f + noise * 0.4f) * env * 12000);
        }
        loadBuffer(BOSS_HIT, s);
    }

    void generatePortal()
    {
        int len = (int)(RATE * 0.35f);
        std::vector<sf::Int16> s(len);
        for (int i = 0; i < len; i++)
        {
            float t = (float)i / RATE;
            float env = std::sin(PI * (float)i / len);
            float freq = 400.f + 300.f * std::sin(t * 15.f);
            s[i] = (sf::Int16)(std::sin(2.f * PI * freq * t) * env * 5000);
        }
        loadBuffer(PORTAL, s);
    }

    void generateDeath()
    {
        int len = (int)(RATE * 0.5f);
        std::vector<sf::Int16> s(len);
        for (int i = 0; i < len; i++)
        {
            float t = (float)i / RATE;
            float env = 1.f - (float)i / len;
            float freq = 400.f - 300.f * ((float)i / len);
            float noise = ((float)(std::rand() % 10000) - 5000) / 10000.f;
            s[i] = (sf::Int16)((std::sin(2.f * PI * freq * t) * 0.6f + noise * 0.4f) * env * 8000);
        }
        loadBuffer(DEATH, s);
    }

    void generateTransform()
    {
        int len = (int)(RATE * 0.12f);
        std::vector<sf::Int16> s(len);
        for (int i = 0; i < len; i++)
        {
            float t = (float)i / RATE;
            float env = 1.f - (float)i / len;
            float freq = 300.f + 500.f * ((float)i / len);
            float val = std::sin(2.f * PI * freq * t) + 0.5f * std::sin(2.f * PI * freq * 1.5f * t);
            s[i] = (sf::Int16)(val * env * 3500);
        }
        loadBuffer(TRANSFORM, s);
    }

    void generateBuy()
    {
        int len = (int)(RATE * 0.2f);
        std::vector<sf::Int16> s(len);
        int third = len / 3;
        for (int i = 0; i < len; i++)
        {
            float t = (float)i / RATE;
            float env = 1.f - (float)i / len;
            float freq;
            if (i < third) freq = 800.f;
            else if (i < third * 2) freq = 1000.f;
            else freq = 1200.f;
            s[i] = (sf::Int16)(std::sin(2.f * PI * freq * t) * env * 4000);
        }
        loadBuffer(BUY, s);
    }
};
