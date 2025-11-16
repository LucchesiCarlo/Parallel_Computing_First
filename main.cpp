#include <optional>
#include <random>
#include <chrono>
#include <SFML/Graphics.hpp>

struct Boid {
    float x;
    float y;

    float xv = 0;
    float yv = 0;
};

void printBoid(Boid boid, sf::RenderWindow& window);

int main(int argc, char **argv) {
    constexpr unsigned WIDTH = 800;
    constexpr unsigned HEIGHT = 600;
    constexpr unsigned FPS = 60;
    constexpr float MAX_SPEED = 10.f;
    constexpr int N = 100;
    constexpr float VISIBLE = 20;
    constexpr float PROTECT = 10;
    constexpr float AVOID = 10;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);

    std::unique_ptr<Boid[]> boids(new Boid[N]);

    for(int i = 0; i < N; i++) {
        boids[i].x = static_cast<float>(generator() % WIDTH);
        boids[i].y = static_cast<float>(generator() % HEIGHT);
    }

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Boids");
    window.setFramerateLimit(FPS);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color::Black);
        for (int i = 0; i < N; i++) {
            printBoid(boids[i], window);
        }
        window.display();
    }
}

void printBoid(Boid boid, sf::RenderWindow& window) {
    sf::CircleShape circle(5);
    circle.setFillColor(sf::Color::Black);
    circle.setOutlineColor(sf::Color::White);
    circle.setOutlineThickness(1.f);
    circle.setPosition({boid.x, boid.y});
    circle.setOrigin(circle.getGeometricCenter());

    window.draw(circle);
}
