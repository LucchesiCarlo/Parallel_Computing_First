//
// Created by carlo on 30/11/25.
//
#include <cmath>
#include "helpersAOS.h"
#include "framegenAOS.h"

void generateFrame(Boid *&boids, Boid *&nextBoids, const ExpParams &exp) {
    for (int i = 0; i < exp.N; i++) {
        float close_dx = 0;
        float close_dy = 0;

        int neighbours = 0;
        float velX = 0;
        float velY = 0;
        float posX = 0;
        float posY = 0;
        for (int j = 0; j < exp.N; j++) {
            if (i == j)
                continue;
            const float distance = squareDistanceAOS(boids, i, j);
            if (distance < exp.PROTECT * exp.PROTECT) {
                close_dx += boids[i].x - boids[j].x;
                close_dy += boids[i].y - boids[j].y;
            } else if (distance < exp.VISIBLE * exp.VISIBLE) {
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

        nextBoids[i].vx = close_dx * exp.AVOID + (velX - boids[i].vx) * exp.MATCH + (posX - boids[i].x) * exp.CENTER +
                          boids[i].
                          vx;
        nextBoids[i].vy = close_dy * exp.AVOID + (velY - boids[i].vy) * exp.MATCH + (posY - boids[i].y) * exp.CENTER +
                          boids[i].
                          vy;

        if (nextBoids[i].x < exp.MARGIN) {
            nextBoids[i].vx += exp.TURN;
        } else if (nextBoids[i].x > exp.WIDTH - exp.MARGIN) {
            nextBoids[i].vx -= exp.TURN;
        }
        if (nextBoids[i].y < exp.MARGIN) {
            nextBoids[i].vy += exp.TURN;
        } else if (nextBoids[i].y > exp.HEIGHT - exp.MARGIN) {
            nextBoids[i].vy -= exp.TURN;
        }

        const auto speed = static_cast<float>(sqrt(pow(boids[i].vx, 2) + pow(boids[i].vy, 2)));
        if (speed < exp.EPSILON) {
            nextBoids[i].vy = exp.MIN_SPEED;
        } else if (speed < exp.MIN_SPEED) {
            nextBoids[i].vx *= exp.MIN_SPEED / speed;
            nextBoids[i].vy *= exp.MIN_SPEED / speed;
        } else if (speed > exp.MAX_SPEED) {
            nextBoids[i].vx *= exp.MAX_SPEED / speed;
            nextBoids[i].vy *= exp.MAX_SPEED / speed;
        }
    }
    std::swap(boids, nextBoids);

    for (int i = 0; i < exp.N; i++) {
        boids[i].x += boids[i].vx;
        boids[i].y += boids[i].vy;

        if (boids[i].x < 0) {
            boids[i].x = 0;
            boids[i].vx = 0;
        } else if (boids[i].x > exp.WIDTH) {
            boids[i].x = exp.WIDTH;
            boids[i].vx = 0;
        }
        if (boids[i].y < 0) {
            boids[i].y = 0;
            boids[i].vy = 0;
        } else if (boids[i].y > exp.HEIGHT) {
            boids[i].y = exp.HEIGHT;
            boids[i].vy = 0;
        }
    }
}
