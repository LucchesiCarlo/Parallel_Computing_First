//
// Created by carlo on 01/12/25.
//
#include <omp.h>
#include <immintrin.h>
#include "framegenSOA.h"


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

void generateFrame(Boids &boids, Boids &nextBoids, sf::CircleShape *shapes, const ExpParams &exp) {
    omp_set_num_threads(exp.THREADS);
#pragma omp parallel default(none) shared(exp, boids, nextBoids, shapes)
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

            __m256 current_x = _mm256_set1_ps(boids.x[i]);
            __m256 current_y = _mm256_set1_ps(boids.y[i]);

            float protectSquare = exp.PROTECT * exp.PROTECT;
            float visibleSquare = exp.VISIBLE * exp.VISIBLE;
            __m256 protectDist = _mm256_set1_ps(protectSquare);
            __m256 visibleDist = _mm256_set1_ps(visibleSquare);
            int j = 0;
            for (; j <= exp.N - 8; j += 8) {
                __m256 x_positions = _mm256_load_ps(&(boids.x[j]));
                __m256 y_positions = _mm256_load_ps(&(boids.y[j]));
                __m256 x_vel = _mm256_load_ps(&(boids.vx[j]));
                __m256 y_vel = _mm256_load_ps(&(boids.vy[j]));

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
                const float distance = squareDistanceSOA(boids, i, j);
                bool protect = (distance < protectDist[0]);
                bool visible = (distance < visibleDist[0]) - protect;

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
            shapes[i].setPosition({boids.x[i], boids.y[i]});
        }
    } //End Parallel
}

void createBoidsSOA(Boids &boids, const int N) {
    if (N <= 0)
        return;
    boids.x = (float *) _mm_malloc(N * sizeof(float), 32);
    boids.y = (float *) _mm_malloc(N * sizeof(float), 32);
    boids.vx = (float *) _mm_malloc(N * sizeof(float), 32);
    boids.vy = (float *) _mm_malloc(N * sizeof(float), 32);
}

void deleteBoidsSOA(Boids &boids) {
    _mm_free(boids.x);
    _mm_free(boids.y);
    _mm_free(boids.vx);
    _mm_free(boids.vy);

    boids.x = nullptr;
    boids.y = nullptr;
    boids.vx = nullptr;
    boids.vy = nullptr;
}
