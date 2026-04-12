#include <SFML/Graphics.hpp>
#include "Game.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Transform Arena", sf::Style::Default);
    window.setFramerateLimit(60);

    sf::View gameView(sf::FloatRect(0.f, 0.f, 800.f, 600.f));

    auto updateViewport = [&]()
    {
        float winW = (float)window.getSize().x;
        float winH = (float)window.getSize().y;
        float targetRatio = 800.f / 600.f;
        float windowRatio = winW / winH;

        float vpW = 1.f, vpH = 1.f, vpX = 0.f, vpY = 0.f;
        if (windowRatio > targetRatio)
        {
            vpW = targetRatio / windowRatio;
            vpX = (1.f - vpW) / 2.f;
        }
        else
        {
            vpH = windowRatio / targetRatio;
            vpY = (1.f - vpH) / 2.f;
        }
        gameView.setViewport(sf::FloatRect(vpX, vpY, vpW, vpH));
        window.setView(gameView);
    };

    updateViewport();

    Game game(window);
    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::Resized)
                updateViewport();

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11)
            {
                static bool fullscreen = false;
                fullscreen = !fullscreen;
                if (fullscreen)
                {
                    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
                    window.create(desktop, "Transform Arena", sf::Style::Fullscreen);
                }
                else
                    window.create(sf::VideoMode(800, 600), "Transform Arena", sf::Style::Default);
                window.setFramerateLimit(60);
                updateViewport();
            }

            game.handleEvent(event);
        }

        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        game.update(dt);

        window.clear(sf::Color::Black);
        game.draw();
        window.display();
    }

    return 0;
}
