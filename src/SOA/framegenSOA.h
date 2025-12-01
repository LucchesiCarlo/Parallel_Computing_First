//
// Created by carlo on 01/12/25.
//

#ifndef FIRST_ASSIGNMENT_FRAMEGENSOA_H
#define FIRST_ASSIGNMENT_FRAMEGENSOA_H
#include <SFML/Graphics.hpp>
#include "helpersSOA.h"

struct ExpParams {
    unsigned WIDTH = 1600;
    unsigned HEIGHT = 800;
    float MARGIN = 75;
    unsigned FPS = 60;
    float MAX_SPEED = 6.f;
    float MIN_SPEED = 3.f;
    float VISIBLE = 40;
    float PROTECT = 10;
    float AVOID = 0.02;
    float MATCH = 0.02;
    float CENTER = 0.00005;
    float TURN = 0.05;
    float EPSILON = 0.001;
    int N = 1000;
    int ITER = 1000;
    double SEC = 10.;
    int THREADS = 8;
};


void createBoidsSOA(Boids &boids, int N);

void generateFrame(Boids &boids, Boids &nextBoids, sf::CircleShape *shapes, const ExpParams &exp);

void deleteBoidsSOA(Boids &boids);
#endif //FIRST_ASSIGNMENT_FRAMEGENSOA_H
