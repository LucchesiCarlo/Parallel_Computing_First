//
// Created by carlo on 17/11/25.
//

#include <string>
#include <cmath>
#include <SFML/Graphics.hpp>

struct Boids {
    float *x;
    float *y;

    float *vx;
    float *vy;
};

void get_parameters(const int argc, char **argv, int &n, double &seconds, int &threads) {
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

void printBoidSOA(Boids boid, const int i, sf::Shape &shape, sf::RenderWindow &window) {
    shape.setPosition({boid.x[i], boid.y[i]});
    window.draw(shape);
}

void initialize_boids_soa(Boids& boids, const int N) {
    if (N <= 0)
        return;
    boids.x = new float[N];
    boids.y = new float[N];
    boids.vx = new float[N];
    boids.vy = new float[N];
}

#pragma omp declare simd
inline float squareDistanceSOA(const Boids &a, const int i, const int j) {
    return static_cast<float>(std::pow((a.x[i] - a.x[j]), 2) + std::pow(a.y[i] - a.y[j], 2));
}

void delete_boids_soa(const Boids& boids) {
    delete[] boids.x;
    delete[] boids.y;
    delete[] boids.vx;
    delete[] boids.vy;
}