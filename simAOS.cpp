//
// Created by carlo on 30/11/25.
//

#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <SFML/Graphics.hpp>
#include "src/AOS/framegenAOS.h"

int main(int argc, char **argv) {
#ifdef PADDING
    std::cout << "Padding Enabled" << "\n";
#endif
    ExpParams exp;

    getParametersSim(argc, argv, exp.N, exp.ITER, exp.THREADS);

    Boid *boids = new Boid[exp.N];
    Boid *nextBoids = new Boid[exp.N];
    /*
     * Considering that boids number is constant, is better for performance to initialize all circle at once and only
     * update their positions.
     */
    auto shapes = new sf::CircleShape[exp.N];
    initializeBoidsAOS(boids, shapes, exp.N, exp.WIDTH, exp.HEIGHT, exp.MAX_SPEED, exp.MIN_SPEED);

    const auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < exp.ITER; i++) {
        generateFrame(boids, nextBoids, exp);
    }
    const auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<double> >(end - start);

    FILE *output;
    if (argc > 3) {
        output = fopen(argv[3], "w");
    } else {
        output = fopen("output.csv", "w");
    }
    if (output == nullptr) {
        std::cerr << "Could not open file due to an error." << std::endl;
    } else {
        fprintf(output, "Time\n%f\n", duration.count());
        std::cout << "Time taken: " << duration.count() << "\n";
        fclose(output);
    }

    delete[] shapes;
    delete[] boids;
    delete[] nextBoids;
}
