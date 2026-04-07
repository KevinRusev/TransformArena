#include <SFML/Graphics.hpp>
#include "Game.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Transform Arena");
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
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                window.close();

            game.handleEvent(event);
        }

        float dt = clock.restart().asSeconds();

        game.update(dt);
        game.draw();
        window.display();
    }

    return 0;
}
