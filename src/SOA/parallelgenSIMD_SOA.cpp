//
// Created by carlo on 01/12/25.
//
#include <omp.h>
#include "framegenSOA.h"

void generateFrame(Boids &boids, Boids &nextBoids, sf::CircleShape *shapes, const ExpParams &exp) {
    omp_set_num_threads(exp.THREADS);
#pragma omp parallel default(none) shared(exp, boids, nextBoids, shapes)
    {
        //Start Parallel
#pragma omp for schedule(runtime)
        for (int i = 0; i < exp.N; i++) {
            float close_dx = 0;
            float close_dy = 0;

            int neighbours = 0;
            float velX = 0;
            float velY = 0;
            float posX = 0;
            float posY = 0;

            const float squareProtect = exp.PROTECT * exp.PROTECT;
            const float squareVisible = exp.VISIBLE * exp.VISIBLE;
#pragma omp simd
            for (int j = 0; j < exp.N; j++) {
                const float distance = squareDistanceSOA(boids, i, j);
                bool protect = (distance < squareProtect);
                bool visible = (distance < squareVisible) - protect;

                close_dx += (boids.x[i] - boids.x[j]) * protect;
                close_dy += (boids.y[i] - boids.y[j]) * protect;

                velX += boids.vx[j] * visible;
                velY += boids.vy[j] * visible;
                posX += boids.x[j] * visible;
                posY += boids.y[j] * visible;
                neighbours += visible;
            }
            velX -= boids.vx[i];
            velY -= boids.vy[i];
            posX -= boids.x[i];
            posY -= boids.y[i];
            neighbours--;
            if (neighbours > 0) {
                velX = velX / static_cast<float>(neighbours);
                velY = velY / static_cast<float>(neighbours);
                posX = posX / static_cast<float>(neighbours);
                posY = posY / static_cast<float>(neighbours);
            }
            nextBoids.x[i] = boids.x[i];
            nextBoids.y[i] = boids.y[i];

            nextBoids.vx[i] = close_dx * exp.AVOID + (velX - boids.vx[i]) * exp.MATCH + (posX - boids.x[i]) * exp.CENTER
                              + boids
                              .vx[i];
            nextBoids.vy[i] = close_dy * exp.AVOID + (velY - boids.vy[i]) * exp.MATCH + (posY - boids.y[i]) * exp.CENTER
                              + boids
                              .vy[i];

            if (nextBoids.x[i] < exp.MARGIN) {
                nextBoids.vx[i] += exp.TURN;
            } else if (nextBoids.x[i] > exp.WIDTH - exp.MARGIN) {
                nextBoids.vx[i] -= exp.TURN;
            }
            if (nextBoids.y[i] < exp.MARGIN) {
                nextBoids.vy[i] += exp.TURN;
            } else if (nextBoids.y[i] > exp.HEIGHT - exp.MARGIN) {
                nextBoids.vy[i] -= exp.TURN;
            }

            const auto speed = static_cast<float>(sqrt(pow(boids.vx[i], 2) + pow(boids.vy[i], 2)));
            if (speed < exp.EPSILON) {
                nextBoids.vy[i] = exp.MIN_SPEED;
            } else if (speed < exp.MIN_SPEED) {
                nextBoids.vx[i] *= exp.MIN_SPEED / speed;
                nextBoids.vy[i] *= exp.MIN_SPEED / speed;
            } else if (speed > exp.MAX_SPEED) {
                nextBoids.vx[i] *= exp.MAX_SPEED / speed;
                nextBoids.vy[i] *= exp.MAX_SPEED / speed;
            }
        }
#pragma omp single
        std::swap(boids, nextBoids);

#pragma omp for nowait
        for (int i = 0; i < exp.N; i++) {
            boids.x[i] += boids.vx[i];
            boids.y[i] += boids.vy[i];

            if (boids.x[i] < 0) {
                boids.x[i] = 0;
                boids.vx[i] = 0;
            } else if (boids.x[i] > exp.WIDTH) {
                boids.x[i] = exp.WIDTH;
                boids.vx[i] = 0;
            }
            if (boids.y[i] < 0) {
                boids.y[i] = 0;
                boids.vy[i] = 0;
            } else if (boids.y[i] > exp.HEIGHT) {
                boids.y[i] = exp.HEIGHT;
                boids.vy[i] = 0;
            }
        }
    } //End Parallel
}

void createBoidsSOA(Boids &boids, const int N) {
    if (N <= 0)
        return;
    boids.x = new float[N];
    boids.y = new float[N];
    boids.vx = new float[N];
    boids.vy = new float[N];
}

void deleteBoidsSOA(Boids &boids) {
    delete[] boids.x;
    delete[] boids.y;
    delete[] boids.vx;
    delete[] boids.vy;
}
