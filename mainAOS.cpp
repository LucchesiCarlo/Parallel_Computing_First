//
// Created by carlo on 30/11/25.
//

#include <chrono>
#include <cmath>
#include <iostream>
#include <list>
#include <optional>
#include <random>
#include <SFML/Graphics.hpp>
#include "src/AOS/framegenAOS.h"

int main(int argc, char **argv) {
#ifdef PADDING
    std::cout << "Padding Enabled" << "\n";
#endif
    ExpParams exp;

    getParametersGUI(argc, argv, exp.N, exp.SEC, exp.THREADS);

    const unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);

    Boid *boids = new Boid[exp.N];
    Boid *nextBoids = new Boid[exp.N];
    /*
     * Considering that boids number is constant, is better for performance to initialize all circle at once and only
     * update their positions.
     */
    sf::CircleShape shapes[exp.N];
    std::list<double> values;

    for (int i = 0; i < exp.N; i++) {
        boids[i].x = static_cast<float>(generator() % exp.WIDTH);
        boids[i].y = static_cast<float>(generator() % exp.HEIGHT);
        boids[i].vx = static_cast<float>(generator() % (static_cast<int>(exp.MAX_SPEED - exp.MIN_SPEED))) + exp.
                      MIN_SPEED;
        boids[i].vy = static_cast<float>(generator() % (static_cast<int>(exp.MAX_SPEED - exp.MIN_SPEED))) + exp.
                      MIN_SPEED;

        sf::CircleShape circle(1);
        circle.setFillColor(sf::Color::Black);
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(1.f);
        circle.setOrigin(circle.getGeometricCenter());
        shapes[i] = circle;
    }

    sf::RenderWindow window(sf::VideoMode({exp.WIDTH, exp.HEIGHT}), "Boids");
    window.setFramerateLimit(exp.FPS);

    const auto start = std::chrono::high_resolution_clock::now();
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        auto start_frame = std::chrono::high_resolution_clock::now();

        generateFrame(boids, nextBoids, shapes, exp);
        window.clear(sf::Color::Black);
        for (int i = 0; i < exp.N; i++) {
            window.draw(shapes[i]);
        }

        auto end_frame = std::chrono::high_resolution_clock::now();
        auto frame = std::chrono::duration_cast<std::chrono::duration<double> >(end_frame - start_frame).count();
        values.push_back(frame);

        window.display();
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<double> >(now - start);
        if (duration.count() > exp.SEC) {
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

    delete[] boids;
    delete[] nextBoids;
}