//
// Created by carlo on 30/11/25.
//
#include <cmath>
#include <omp.h>
#include <immintrin.h>
#include <random>

#include "framegenAOS.h"

inline void horizontal_add_avx(__m256 a, __m256 b, float &res_a, float &res_b);

inline void horizontal_add_avx(__m256i a, __m256i b, int &res_a, int &res_b);

void generateFrame(Boid *&boids, Boid *&nextBoids, const ExpParams &exp) {
    omp_set_num_threads(exp.THREADS);
#pragma omp parallel default(none) shared(exp, boids, nextBoids)
    {
        //Start Parallel
#pragma omp for schedule(runtime)
        for (int i = 0; i < exp.N; i++) {
            float close_dx = 0;
            float close_dy = 0;
            __m256 _close_dx = _mm256_set1_ps(0);
            __m256 _close_dy = _mm256_set1_ps(0);

            int neighbours = 0;
            __m256i _neighbours = _mm256_set1_epi32(0);
            const __m256i _neighbourMask = _mm256_set1_epi32(1);
            float velX = 0;
            float velY = 0;
            float posX = 0;
            float posY = 0;
            __m256 _velX = _mm256_set1_ps(0);
            __m256 _velY = _mm256_set1_ps(0);
            __m256 _posX = _mm256_set1_ps(0);
            __m256 _posY = _mm256_set1_ps(0);

            __m256 current_x = _mm256_set1_ps(boids[i].x);
            __m256 current_y = _mm256_set1_ps(boids[i].y);

            float protectSquare = exp.PROTECT * exp.PROTECT;
            float visibleSquare = exp.VISIBLE * exp.VISIBLE;
            __m256 protectDist = _mm256_set1_ps(protectSquare);
            __m256 visibleDist = _mm256_set1_ps(visibleSquare);
            int j = 0;
            for (; j <= exp.N - 8; j += 8) {
                __m256 x_positions = {
                    boids[j].x, boids[j + 1].x, boids[j + 2].x, boids[j + 3].x, boids[j + 4].x, boids[j + 5].x,
                    boids[j + 6].x, boids[j + 7].x
                };
                __m256 y_positions = {
                    boids[j].y, boids[j + 1].y, boids[j + 2].y, boids[j + 3].y, boids[j + 4].y, boids[j + 5].y,
                    boids[j + 6].y, boids[j + 7].y
                };
                __m256 x_vel = {
                    boids[j].vx, boids[j + 1].vx, boids[j + 2].vx, boids[j + 3].vx, boids[j + 4].vx,
                    boids[j + 5].vx, boids[j + 6].vx, boids[j + 7].vx
                };
                __m256 y_vel = {
                    boids[j].vy, boids[j + 1].vy, boids[j + 2].vy, boids[j + 3].vy, boids[j + 4].vy,
                    boids[j + 5].vy, boids[j + 6].vy, boids[j + 7].vy
                };

                __m256 distanceX = _mm256_sub_ps(current_x, x_positions);
                __m256 distanceY = _mm256_sub_ps(current_y, y_positions);

                __m256 distance = _mm256_add_ps(distanceX * distanceX, distanceY * distanceY);
                auto protect = _mm256_cmp_ps(protectDist, distance, _CMP_GT_OQ);
                auto visible = _mm256_cmp_ps(visibleDist, distance, _CMP_GT_OQ);

                visible = _mm256_andnot_ps(protect, visible);

                _close_dx = _mm256_add_ps(_close_dx, _mm256_and_ps(distanceX, protect));
                _close_dy = _mm256_add_ps(_close_dy, _mm256_and_ps(distanceY, protect));

                _velX = _mm256_add_ps(_velX, _mm256_and_ps(x_vel, visible));
                _velY = _mm256_add_ps(_velY, _mm256_and_ps(y_vel, visible));
                _posX = _mm256_add_ps(_posX, _mm256_and_ps(x_positions, visible));
                _posY = _mm256_add_ps(_posY, _mm256_and_ps(y_positions, visible));
                _neighbours = _mm256_add_epi32(_neighbours,
                                               _mm256_and_si256(_mm256_castps_si256(visible), _neighbourMask));
            }

            float sum_a, sum_b;
            horizontal_add_avx(_close_dx, _close_dy, sum_a, sum_b);
            close_dx = sum_a;
            close_dy = sum_b;
            horizontal_add_avx(_velX, _velY, sum_a, sum_b);
            velX = sum_a;
            velY = sum_b;
            horizontal_add_avx(_posX, _posY, sum_a, sum_b);
            posX = sum_a;
            posY = sum_b;

            int sum;
            horizontal_add_avx(_neighbours, _neighbours, sum, sum);
            neighbours = sum;

            for (; j < exp.N; j++) {
                const float distance = squareDistanceAOS(boids, i, j);
                bool protect = (distance < protectDist[0]);
                bool visible = (distance < visibleDist[0]) - protect;

                close_dx += (boids[i].x - boids[j].x) * protect;
                close_dy += (boids[i].y - boids[j].y) * protect;

                velX += boids[j].vx * visible;
                velY += boids[j].vy * visible;
                posX += boids[j].x * visible;
                posY += boids[j].y * visible;
                neighbours += visible;
            }
            velX -= boids[i].vx;
            velY -= boids[i].vy;
            posX -= boids[i].x;
            posY -= boids[i].y;
            neighbours--;

            if (neighbours > 0) {
                velX = velX / static_cast<float>(neighbours);
                velY = velY / static_cast<float>(neighbours);
                posX = posX / static_cast<float>(neighbours);
                posY = posY / static_cast<float>(neighbours);
            }
            nextBoids[i].x = boids[i].x;
            nextBoids[i].y = boids[i].y;

            nextBoids[i].vx = close_dx * exp.AVOID + (velX - boids[i].vx) * exp.MATCH + (posX - boids[i].x) * exp.CENTER
                              + boids
                              [i].
                              vx;
            nextBoids[i].vy = close_dy * exp.AVOID + (velY - boids[i].vy) * exp.MATCH + (posY - boids[i].y) * exp.CENTER
                              + boids
                              [i].
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
#pragma omp single
        std::swap(boids, nextBoids);

#pragma omp for nowait
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
    } //End Parallel
}

inline void horizontal_add_avx(__m256 a, __m256 b, float &res_a, float &res_b) {
    auto partial = _mm256_hadd_ps(a, b);
    __m128 lower = _mm256_extractf128_ps(partial, 0);
    __m128 upper = _mm256_extractf128_ps(partial, 1);
    upper = upper + lower;
    __m128 result = _mm_hadd_ps(upper, upper);

    auto *mem = new float[4];
    _mm_storeu_ps(mem, result);
    res_a = mem[0];
    res_b = mem[1];
    delete[] mem;
}

inline void horizontal_add_avx(__m256i a, __m256i b, int &res_a, int &res_b) {
    auto partial = _mm256_hadd_epi32(a, b);
    __m128i lower = _mm256_extracti128_si256(partial, 0);
    __m128i upper = _mm256_extracti128_si256(partial, 1);
    upper = upper + lower;
    __m128i result = _mm_hadd_epi32(upper, upper);

    res_a = _mm_extract_epi32(result, 0);
    res_b = _mm_extract_epi32(result, 1);
}