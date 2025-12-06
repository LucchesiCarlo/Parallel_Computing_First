//
// Created by carlo on 01/12/25.
//

#include <chrono>
#include <cmath>
#include <iostream>
#include <optional>
#include <random>
#include <SFML/Graphics.hpp>

#include "src/SOA/framegenSOA.h"
#include "src/SOA/helpersSOA.h"

int main(int argc, char **argv) {
#ifdef _OPENMP
    std::cout << "OPEN_MP available" << "\n";
#endif
    ExpParams exp;

    getParametersGUI(argc, argv, exp.N, exp.SEC, exp.THREADS);

    Boids boids{};
    Boids nextBoids{};

    createBoidsSOA(boids, exp.N);
    createBoidsSOA(nextBoids, exp.N);
    /*
     * Considering that boids number is constant, is better for performance to initialize all circle at once and only
     * update their positions.
     */
    auto shapes = new sf::CircleShape[exp.N];
    std::vector<double> values;
    std::vector<double> sequential;

    initializeBoidsSOA(boids, shapes, exp.N, exp.WIDTH, exp.HEIGHT, exp.MAX_SPEED, exp.MIN_SPEED);

    sf::RenderWindow window(sf::VideoMode({exp.WIDTH, exp.HEIGHT}), "Boids");
    window.setFramerateLimit(exp.FPS);

    const auto start = std::chrono::high_resolution_clock::now();
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        auto start_frame = std::chrono::high_resolution_clock::now();

        generateFrame(boids, nextBoids, exp);

        auto start_seq = std::chrono::high_resolution_clock::now();
        window.clear(sf::Color::Black);
        /* Even if the update of shapes positions are independent from each other, parallelizing is not beneficial.
         * Considering the big size of sf::CircleShape (344byte) it will clog the cache and consume a lor of memory
         * bandwidth. Then, considering that only the master thread can draw them on the canvas (SFML is not thread safe)
         * now we must move all the objects again.
         */
        for (int i = 0; i < exp.N; i++) {
            shapes[i].setPosition({boids.x[i], boids.y[i]});
            window.draw(shapes[i]);
        }
        auto end_seq = std::chrono::high_resolution_clock::now();

        auto end_frame = std::chrono::high_resolution_clock::now();
        auto frame = std::chrono::duration_cast<std::chrono::duration<double> >(end_frame - start_frame).count();
        auto seq_frame = std::chrono::duration_cast<std::chrono::duration<double> >(end_seq - start_seq).count();
        values.push_back(frame);
        sequential.push_back(seq_frame);

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
        fprintf(output, "Frame, Sequential\n");
        for (int i = 0; i < values.size(); i++) {
            fprintf(output, "%f,%f\n", values[i], sequential[i]);
        }
        fclose(output);
    }
    std::cout << "Avg frame generation " << std::accumulate(values.begin(), values.end(), 0.) / static_cast<
        double>(sequential.size()) << std::endl;
    std::cout << "Avg sequential section " << std::accumulate(sequential.begin(), sequential.end(), 0.) / static_cast<
        double>(sequential.size()) << std::endl;

    delete[] shapes;
    deleteBoidsSOA(boids);
    deleteBoidsSOA(nextBoids);
}