#include <chrono>
#include <cmath>
#include <iostream>
#include <list>
#include <optional>
#include <random>
#include <SFML/Graphics.hpp>
#include <omp.h>
#include "helpersAOS.cpp"

int main(int argc, char **argv) {
#ifdef PADDING
    std::cout << "Padding Enabled" << "\n";
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

    Boid *boids = new Boid[N];
    Boid *nextBoids = new Boid[N];
    /*
     * Considering that boids number is constant, is better for performance to initialize all circle at once and only
     * update their positions.
     */
    std::unique_ptr<sf::CircleShape[]> shapes(new sf::CircleShape[N]);
    std::list<double> values;

    initializeBoidsAOS(boids, shapes.get(), N, WIDTH, HEIGHT, MAX_SPEED, MIN_SPEED);

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
            printBoidAOS(boids[i], shapes[i], window);
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
                    const float distance = squareDistanceAOS_no_simd(boids, i, j);
                    if (distance < PROTECT * PROTECT) {
                        close_dx += boids[i].x - boids[j].x;
                        close_dy += boids[i].y - boids[j].y;
                    } else if (distance < VISIBLE * VISIBLE) {
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

                nextBoids[i].vx = close_dx * AVOID + (velX - boids[i].vx) * MATCH + (posX - boids[i].x) * CENTER + boids
                                  [i].
                                  vx;
                nextBoids[i].vy = close_dy * AVOID + (velY - boids[i].vy) * MATCH + (posY - boids[i].y) * CENTER + boids
                                  [i].
                                  vy;

                if (nextBoids[i].x < MARGIN) {
                    nextBoids[i].vx += TURN;
                } else if (nextBoids[i].x > WIDTH - MARGIN) {
                    nextBoids[i].vx -= TURN;
                }
                if (nextBoids[i].y < MARGIN) {
                    nextBoids[i].vy += TURN;
                } else if (nextBoids[i].y > HEIGHT - MARGIN) {
                    nextBoids[i].vy -= TURN;
                }

                const auto speed = static_cast<float>(sqrt(pow(boids[i].vx, 2) + pow(boids[i].vy, 2)));
                if (speed < EPSILON) {
                    nextBoids[i].vy = MIN_SPEED;
                } else if (speed < MIN_SPEED) {
                    nextBoids[i].vx *= MIN_SPEED / speed;
                    nextBoids[i].vy *= MIN_SPEED / speed;
                } else if (speed > MAX_SPEED) {
                    nextBoids[i].vx *= MAX_SPEED / speed;
                    nextBoids[i].vy *= MAX_SPEED / speed;
                }
            }
#pragma omp single
            std::swap(boids, nextBoids);

#pragma omp for nowait
            for (int i = 0; i < N; i++) {
                boids[i].x += boids[i].vx;
                boids[i].y += boids[i].vy;

                if (boids[i].x < 0) {
                    boids[i].x = 0;
                    boids[i].vx = 0;
                } else if (boids[i].x > WIDTH) {
                    boids[i].x = WIDTH;
                    boids[i].vx = 0;
                }
                if (boids[i].y < 0) {
                    boids[i].y = 0;
                    boids[i].vy = 0;
                } else if (boids[i].y > HEIGHT) {
                    boids[i].y = HEIGHT;
                    boids[i].vy = 0;
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

    delete[] boids;
    delete[] nextBoids;
}
