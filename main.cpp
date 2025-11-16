#include <optional>
#include <SFML/Graphics.hpp>

int main(int argc, char **argv) {
    constexpr unsigned WIDTH = 800;
    constexpr unsigned HEIGHT = 600;

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Boids");

    window.setFramerateLimit(60);
    // run the program as long as the window is open
    auto circle = sf::CircleShape(100);

    circle.setFillColor(sf::Color::Black);
    circle.setOutlineColor(sf::Color::White);
    circle.setOutlineThickness(3.f);
    circle.setPosition({WIDTH / 2., HEIGHT / 2.});
    circle.setOrigin(circle.getGeometricCenter());

    while (window.isOpen()) {
        // check all the window's events that were triggered since the last iteration of the loop
        while (const std::optional event = window.pollEvent()) {
            // "close requested" event: we close the window
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color::Black);
        window.draw(circle);
        window.display();
    }
}
