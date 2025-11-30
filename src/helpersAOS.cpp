//
// Created by carlo on 17/11/25.
//

#include <string>
#include <random>
#include <SFML/Graphics.hpp>
#include "helpersAOS.h"

void getParameters(const int argc, char **argv, int &n, double &seconds, int &threads) {
    constexpr int N = 1000;
    constexpr double SECONDS = 10.;
    constexpr int THREADS = 10;

    n = N;
    if (argc > 1) {
        try {
            n = std::stoi(argv[1]);
            if (n < 1) {
                n = 1;
            }
        } catch (std::exception &) {
            n = N;
        }
    }
    seconds = SECONDS;
    if (argc > 2) {
        try {
            seconds = std::stod(argv[2]);
        } catch (std::exception &) {
            seconds = SECONDS;
        }
    }
    threads = THREADS;
    if (argc > 4) {
        try {
            threads = std::stoi(argv[4]);
            if (threads < 1) {
                threads = 1;
            }
        } catch (std::exception &) {
            threads = THREADS;
        }
    }
}

void initializeBoidsAOS(Boid *boids, sf::CircleShape *shapes, const int N, const int WIDTH, const int HEIGHT,
                        const float MAX_SPEED, const float MIN_SPEED, long seed) {
    if (seed == -1)
        seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    if (N <= 0)
        return;
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
}

void printBoidAOS(const Boid boid, sf::Shape &shape, sf::RenderWindow &window) {
    shape.setPosition({boid.x, boid.y});
    window.draw(shape);
}

