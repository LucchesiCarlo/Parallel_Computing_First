#include <chrono>
#include <cmath>
#include <iostream>
#include <list>
#include <optional>
#include <random>
#include <SFML/Graphics.hpp>
#include "helpers.cpp"

struct Boid {
    float x = 0;
    float y = 0;

    float vx = 0;
    float vy = 0;
};

void printBoid(Boid boid, sf::Shape &shape, sf::RenderWindow &window);

inline float squareDistance(Boid a, Boid b);

int main(int argc, char **argv) {
    constexpr unsigned WIDTH = 1600;
    constexpr unsigned HEIGHT = 800;
    constexpr float MARGIN = 75;
    constexpr unsigned FPS = 60;
    constexpr float MAX_SPEED = 6.f;
    constexpr float MIN_SPEED = 3.f;
    constexpr float VISIBLE = 40;
    constexpr float PROTECT = 10;
    constexpr float AVOID = 0.02;
    constexpr float MATCH = 0.02;
    constexpr float CENTER = 0.00005;
    constexpr float TURN = 0.05;
    constexpr float EPSILON = 0.001;

    int n;
    double seconds;
    int threads;

    get_parameters(argc, argv, n, seconds, threads);
    const auto N = n;
    const double SECONDS = seconds;

    const unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);

    std::unique_ptr<Boid[]> boids(new Boid[N]);
    std::unique_ptr<Boid[]> nextBoids(new Boid[N]);
    /*
     * Considering that boids number is constant, is better for performance to initialize all circle at once and only
     * update their positions.
     */
    std::unique_ptr<sf::CircleShape[]> shapes(new sf::CircleShape[N]);
    std::list<double> values;

    for (int i = 0; i < N; i++) {
        boids[i].x = static_cast<float>(generator() % WIDTH);
        boids[i].y = static_cast<float>(generator() % HEIGHT);
        boids[i].vx = static_cast<float>(generator() % (static_cast<int>(MAX_SPEED - MIN_SPEED))) + MIN_SPEED;
        boids[i].vy = static_cast<float>(generator() % (static_cast<int>(MAX_SPEED - MIN_SPEED))) + MIN_SPEED;

        sf::CircleShape circle(1);
        circle.setFillColor(sf::Color::Black);
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(1.f);
        circle.setOrigin(circle.getGeometricCenter());
        shapes[i] = circle;
    }

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Boids");
    window.setFramerateLimit(FPS);

    const auto start = std::chrono::high_resolution_clock::now();
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color::Black);
        for (int i = 0; i < N; i++) {
            printBoid(boids[i], shapes[i], window);
        }
        window.display();

        auto start_frame = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; i++) {
            float close_dx = 0;
            float close_dy = 0;

            int neighbours = 0;
            float velX = 0;
            float velY = 0;
            float posX = 0;
            float posY = 0;
            for (int j = 0; j < N; j++) {
                if (i == j)
                    continue;
                const float distance = squareDistance(boids[i], boids[j]);
                if (distance < PROTECT * PROTECT) {
                    close_dx += boids[i].x - boids[j].x;
                    close_dy += boids[i].y - boids[j].y;
                } else if (distance < VISIBLE * VISIBLE) {
                    velX += boids[j].vx;
                    velY += boids[j].vy;
                    posX += boids[j].x;
                    posY += boids[j].y;
                    neighbours++;
                }
            }
            if (neighbours > 0) {
                velX = velX / static_cast<float>(neighbours);
                velY = velY / static_cast<float>(neighbours);
                posX = posX / static_cast<float>(neighbours);
                posY = posY / static_cast<float>(neighbours);
            }
            nextBoids[i].x = boids[i].x;
            nextBoids[i].y = boids[i].y;

            nextBoids[i].vx = close_dx * AVOID + (velX - boids[i].vx) * MATCH + (posX - boids[i].x) * CENTER + boids[i].
                              vx;
            nextBoids[i].vy = close_dy * AVOID + (velY - boids[i].vy) * MATCH + (posY - boids[i].y) * CENTER + boids[i].
                              vy;

            if (nextBoids[i].x < MARGIN) {
                nextBoids[i].vx += TURN;
            } else if (nextBoids[i].x > WIDTH - MARGIN) {
                nextBoids[i].vx -= TURN;
            }
            if (nextBoids[i].y < MARGIN) {
                nextBoids[i].vy += TURN;
            } else if (nextBoids[i].y > HEIGHT - MARGIN) {
                nextBoids[i].vy -= TURN;
            }

            const auto speed = static_cast<float>(sqrt(pow(boids[i].vx, 2) + pow(boids[i].vy, 2)));
            if (speed < EPSILON) {
                nextBoids[i].vy = MIN_SPEED;
            } else if (speed < MIN_SPEED) {
                nextBoids[i].vx *= MIN_SPEED / speed;
                nextBoids[i].vy *= MIN_SPEED / speed;
            } else if (speed > MAX_SPEED) {
                nextBoids[i].vx *= MAX_SPEED / speed;
                nextBoids[i].vy *= MAX_SPEED / speed;
            }
        }
        boids.swap(nextBoids);
        for (int i = 0; i < N; i++) {
            boids[i].x += boids[i].vx;
            boids[i].y += boids[i].vy;

            if (boids[i].x < 0) {
                boids[i].x = 0;
                boids[i].vx = 0;
            } else if (boids[i].x > WIDTH) {
                boids[i].x = WIDTH;
                boids[i].vx = 0;
            }
            if (boids[i].y < 0) {
                boids[i].y = 0;
                boids[i].vy = 0;
            } else if (boids[i].y > HEIGHT) {
                boids[i].y = HEIGHT;
                boids[i].vy = 0;
            }
        }
        auto end_frame = std::chrono::high_resolution_clock::now();
        auto frame = std::chrono::duration_cast<std::chrono::duration<double> >(end_frame - start_frame).count();
        values.push_back(frame);

        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<double> >(now - start);
        if (duration.count() > SECONDS) {
            window.close();
        }
    }

    FILE *output;
    if (argc > 3) {
        output = fopen(argv[3], "w");
    } else {
        output = fopen("output.txt", "w");
    }
    if (output == nullptr) {
        std::cerr << "Could not open file due to an error." << std::endl;
    } else {
        for (auto value: values) {
            fprintf(output, "%f,", value);
        }
        fclose(output);
    }
}

void printBoid(const Boid boid, sf::Shape &shape, sf::RenderWindow &window) {
    shape.setPosition({boid.x, boid.y});
    window.draw(shape);
}

inline float squareDistance(const Boid a, const Boid b) {
    return static_cast<float>(pow((a.x - b.x), 2) + pow(a.y - b.y, 2));
}
