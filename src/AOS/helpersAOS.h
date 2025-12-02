//
// Created by carlo on 30/11/25.
//

#ifndef FIRST_ASSIGNMENT_HELPERSAOS_H
#define FIRST_ASSIGNMENT_HELPERSAOS_H

#include <SFML/Graphics.hpp>
#ifdef PADDING
#define CACHE_LINE_SIZE 64
struct alignas(CACHE_LINE_SIZE) Boid {
    float x = 0;
    float y = 0;

    float vx = 0;
    float vy = 0;

    char padding[CACHE_LINE_SIZE - sizeof(float) * 4];
};
#else
struct Boid {
    float x = 0;
    float y = 0;

    float vx = 0;
    float vy = 0;
};
#endif

void getParametersGUI(int argc, char **argv, int &n, double &seconds, int &threads);

void getParametersSim(int argc, char **argv, int &n, int &iter, int &threads);

void initializeBoidsAOS(Boid *boids, sf::CircleShape *shapes, int N, int WIDTH, int HEIGHT,
                        float MAX_SPEED, float MIN_SPEED, long seed = -1);

#pragma omp declare simd
inline float squareDistanceAOS(const Boid *a, const int i, const int j) {
    const float dx = a[i].x - a[j].x;
    const float dy = a[i].y - a[j].y;
    return dx * dx + dy * dy;
}

inline float squareDistanceAOS_no_simd(const Boid *a, const int i, const int j) {
    const float dx = a[i].x - a[j].x;
    const float dy = a[i].y - a[j].y;
    return dx * dx + dy * dy;
}

void printBoidAOS(Boid boid, sf::Shape &shape, sf::RenderWindow &window);

#endif //FIRST_ASSIGNMENT_HELPERSAOS_H