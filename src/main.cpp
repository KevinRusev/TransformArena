#include <SFML/Graphics.hpp>
#include "Game.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Transform Arena", sf::Style::Default);
    window.setFramerateLimit(60);

    Game game(window);
    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11)
            {
                static bool fullscreen = false;
                fullscreen = !fullscreen;
                if (fullscreen)
                    window.create(sf::VideoMode::getDesktopMode(), "Transform Arena", sf::Style::Fullscreen);
                else
                    window.create(sf::VideoMode(800, 600), "Transform Arena", sf::Style::Default);
                window.setFramerateLimit(60);
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
