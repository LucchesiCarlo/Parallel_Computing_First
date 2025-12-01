#include <chrono>
#include <cmath>
#include <iostream>
#include <list>
#include <optional>
#include <random>
#include <SFML/Graphics.hpp>
#include <omp.h>
#include "src/helpersSOA.h"

void printBoid(Boids boid, int i, sf::Shape &shape, sf::RenderWindow &window);

inline float squareDistance(const Boids &a, int i, int j);

int main(int argc, char **argv) {
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

    Boids boids{};
    Boids nextBoids{};

    createBoidsSOA(boids, N);
    createBoidsSOA(nextBoids, N);
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

                int neighbours = 0;
                float velX = 0;
                float velY = 0;
                float posX = 0;
                float posY = 0;
                for (int j = 0; j < N; j++) {
                    if (i == j)
                        continue;
                    const float distance = squareDistanceSOA_no_simd(boids, i, j);
                    if (distance < PROTECT * PROTECT) {
                        close_dx += boids.x[i] - boids.x[j];
                        close_dy += boids.y[i] - boids.y[j];
                    } else if (distance < VISIBLE * VISIBLE) {
                        velX += boids.vx[j];
                        velY += boids.vy[j];
                        posX += boids.x[j];
                        posY += boids.y[j];
                        neighbours++;
                    }
                }
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

    delete[] boids.x;
    delete[] boids.y;
    delete[] boids.vx;
    delete[] boids.vy;

    delete[] nextBoids.x;
    delete[] nextBoids.y;
    delete[] nextBoids.vx;
    delete[] nextBoids.vy;
}