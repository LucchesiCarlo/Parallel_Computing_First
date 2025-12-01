//
// Created by carlo on 01/12/25.
//

#ifndef FIRST_ASSIGNMENT_HELPERSSOA_H
#define FIRST_ASSIGNMENT_HELPERSSOA_H
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

void getParameters(int argc, char **argv, int &n, double &seconds, int &threads);

void initializeBoidsSOA(const Boids &boids, sf::CircleShape *shapes, int N, int WIDTH, int HEIGHT,
                        float MAX_SPEED, float MIN_SPEED, long seed = -1);

void printBoidSOA(Boids boid, int i, sf::Shape &shape, sf::RenderWindow &window);

#pragma omp declare simd
inline float squareDistanceSOA(const Boids &a, const int i, const int j) {
    return static_cast<float>(std::pow((a.x[i] - a.x[j]), 2) + std::pow(a.y[i] - a.y[j], 2));
}

inline float squareDistanceSOA_no_simd(const Boids &a, const int i, const int j) {
    return static_cast<float>(std::pow((a.x[i] - a.x[j]), 2) + std::pow(a.y[i] - a.y[j], 2));
}

#endif //FIRST_ASSIGNMENT_HELPERSSOA_H