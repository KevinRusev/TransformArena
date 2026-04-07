#include <SFML/Graphics.hpp>
#include "Player.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Transform Arena");
    window.setFramerateLimit(60);

    Player player(400.f, 300.f);
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
        }

        float dt = clock.restart().asSeconds();
        player.handleInput(dt);
        player.update(dt);

        window.clear(sf::Color(18, 18, 28));
        player.draw(window);
        window.display();
    }

    return 0;
}
