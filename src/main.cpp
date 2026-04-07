#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Transform Game");
    window.setFramerateLimit(60);

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

        float deltaTime = clock.restart().asSeconds();

        window.clear(sf::Color(30, 30, 40));

        // TODO: game logic and rendering goes here

        window.display();
    }

    return 0;
}
