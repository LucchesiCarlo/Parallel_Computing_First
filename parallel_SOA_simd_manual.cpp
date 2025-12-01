#include <chrono>
#include <cmath>
#include <iostream>
#include <list>
#include <optional>
#include <random>
#include <SFML/Graphics.hpp>
#include <omp.h>
#include <immintrin.h>
#include "src/helpersSOA.h"

inline void horizontal_add_avx(__m256 a, __m256 b, float &res_a, float &res_b);

inline void horizontal_add_avx(__m256i a, __m256i b, int &res_a, int &res_b);

void aligned_boids_create(Boids &boids, int N);

void aligned_boids_delete(Boids &boids);

int main(int argc, char **argv) {
#ifdef PADDING
    std::cout << "Padding Enabled" << "\n";
#endif
#ifdef __AVX2__
    std::cout << "AVX2 supported!\n";
#endif
#ifdef _OPENMP
    std::cout << "OPEN_MP available" << "\n";
#endif
    constexpr unsigned WIDTH = 1600;
    constexpr unsigned HEIGHT = 800;
    constexpr float MARGIN = 75;
    constexpr unsigned FPS = 60;
    constexpr float MAX_SPEED = 6.f;
    constexpr float MIN_SPEED = 3.f;
    constexpr float VISIBLE = 40;
    constexpr float PROTECT = 10;
    constexpr float AVOID = 0.02;
    constexpr float MATCH = 0.02;
    constexpr float CENTER = 0.00005;
    constexpr float TURN = 0.05;
    constexpr float EPSILON = 0.001;

    int n;
    double seconds;
    int threads;

    getParameters(argc, argv, n, seconds, threads);
    const auto N = n;
    const double SECONDS = seconds;
    const int THREADS = threads;

    const unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);

    Boids boids{};
    Boids nextBoids{};

    aligned_boids_create(boids, N);
    aligned_boids_create(nextBoids, N);
    /*
     * Considering that boids number is constant, is better for performance to initialize all circle at once and only
     * update their positions.
     */
    std::unique_ptr<sf::CircleShape[]> shapes(new sf::CircleShape[N]);
    std::list<double> values;

    initializeBoidsSoa(boids, shapes.get(), N, WIDTH, HEIGHT, MAX_SPEED, MIN_SPEED);

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Boids");
    window.setFramerateLimit(FPS);

    const auto start = std::chrono::high_resolution_clock::now();
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color::Black);
        for (int i = 0; i < N; i++) {
            printBoidSOA(boids, i, shapes[i], window);
        }
        window.display();

        auto start_frame = std::chrono::high_resolution_clock::now();
        omp_set_num_threads(THREADS);
#pragma omp parallel default(none) shared(N, boids, nextBoids)
        {
            //Start Parallel
#pragma omp for schedule(runtime)
            for (int i = 0; i < N; i++) {
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

                float protectSquare = PROTECT * PROTECT;
                float visibleSquare = VISIBLE * VISIBLE;
                __m256 protectDist = _mm256_set1_ps(protectSquare);
                __m256 visibleDist = _mm256_set1_ps(visibleSquare);
                int j = 0;
                for (; j <= N - 8; j += 8) {
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

                for (; j < N; j++) {
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

                nextBoids.vx[i] = close_dx * AVOID + (velX - boids.vx[i]) * MATCH + (posX - boids.x[i]) * CENTER + boids
                                  .vx[i];
                nextBoids.vy[i] = close_dy * AVOID + (velY - boids.vy[i]) * MATCH + (posY - boids.y[i]) * CENTER + boids
                                  .vy[i];

                if (nextBoids.x[i] < MARGIN) {
                    nextBoids.vx[i] += TURN;
                } else if (nextBoids.x[i] > WIDTH - MARGIN) {
                    nextBoids.vx[i] -= TURN;
                }
                if (nextBoids.y[i] < MARGIN) {
                    nextBoids.vy[i] += TURN;
                } else if (nextBoids.y[i] > HEIGHT - MARGIN) {
                    nextBoids.vy[i] -= TURN;
                }

                const auto speed = static_cast<float>(sqrt(pow(boids.vx[i], 2) + pow(boids.vy[i], 2)));
                if (speed < EPSILON) {
                    nextBoids.vy[i] = MIN_SPEED;
                } else if (speed < MIN_SPEED) {
                    nextBoids.vx[i] *= MIN_SPEED / speed;
                    nextBoids.vy[i] *= MIN_SPEED / speed;
                } else if (speed > MAX_SPEED) {
                    nextBoids.vx[i] *= MAX_SPEED / speed;
                    nextBoids.vy[i] *= MAX_SPEED / speed;
                }
            }
#pragma omp single
            std::swap(boids, nextBoids);

#pragma omp for nowait
            for (int i = 0; i < N; i++) {
                boids.x[i] += boids.vx[i];
                boids.y[i] += boids.vy[i];

                if (boids.x[i] < 0) {
                    boids.x[i] = 0;
                    boids.vx[i] = 0;
                } else if (boids.x[i] > WIDTH) {
                    boids.x[i] = WIDTH;
                    boids.vx[i] = 0;
                }
                if (boids.y[i] < 0) {
                    boids.y[i] = 0;
                    boids.vy[i] = 0;
                } else if (boids.y[i] > HEIGHT) {
                    boids.y[i] = HEIGHT;
                    boids.vy[i] = 0;
                }
            }
        } //End Parallel
        auto end_frame = std::chrono::high_resolution_clock::now();
        auto frame = std::chrono::duration_cast<std::chrono::duration<double> >(end_frame - start_frame).count();
        values.push_back(frame);

        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<double> >(now - start);
        if (duration.count() > SECONDS) {
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
        for (auto value: values) {
            fprintf(output, "%f,", value);
        }
        fclose(output);
    }

    aligned_boids_delete(boids);
    aligned_boids_delete(nextBoids);
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

void aligned_boids_create(Boids &boids, const int N) {
    boids.x = (float *) _mm_malloc(N * sizeof(float), 32);
    boids.y = (float *) _mm_malloc(N * sizeof(float), 32);
    boids.vx = (float *) _mm_malloc(N * sizeof(float), 32);
    boids.vy = (float *) _mm_malloc(N * sizeof(float), 32);
}

void aligned_boids_delete(Boids &boids) {
    _mm_free(boids.x);
    _mm_free(boids.y);
    _mm_free(boids.vx);
    _mm_free(boids.vy);

    boids.x = nullptr;
    boids.y = nullptr;
    boids.vx = nullptr;
    boids.vy = nullptr;
}
