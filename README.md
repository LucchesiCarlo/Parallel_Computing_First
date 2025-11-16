# Parallel Computing First Assignment: Boids

This repository contains the project for the first assigment of the Parallel computing course.
The aim is to implement two programs that elaborate the same thing, one sequential and one parallel. Then measuring the 
difference in performance.

In implementing the parallel version will be used the OpenMP framework.

### Topic description
The problem of boids consist in implementing the control of independent agents, simulating the behaviour of a flock 
of birds. Those can be seen as a reactive agents that fuses 3 main behaviours:
 - **Separation**: two boids move away if they are too close each other.
 - **Alignment**: a boids tries to match the speed of nearby boids.
 - **Cohesion**: boids tends to mode to the center of mass of nearby boids.

Other type of behaviours can be added; in this specific case we will limit the maximum speed achieved and avoiding them 
to exit the visible screen.
