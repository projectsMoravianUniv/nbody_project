/**
 * Runs a simulation of the n-body problem in 3D.
 * 
 * To compile the program:
 *   gcc -Wall -O3 -march=native nbody-s.c matrix.c util.c -o nbody-s -lm
 * 
 * To run the program:
 *   ./nbody-s time-step total-time outputs-per-body input.npy output.npy
 * where:
 *   - time-step is the amount of time between steps (Δt, in seconds)
 *   - total-time is the total amount of time to simulate (in seconds)
 *   - outputs-per-body is the number of positions to output per body
 *   - input.npy is the file describing the initial state of the system (below)
 *   - output.npy is the output of the program (see below)
 * 
 * input.npy has a n-by-7 matrix with one row per body and the columns:
 *   - mass (in kg)
 *   - initial x, y, z position (in m)
 *   - initial x, y, z velocity (in m/s)
 * 
 * output.npy is generated and has a (outputs-per-body)-by-(3n) matrix with each
 * row containing the x, y, and z positions of each of the n bodies after a
 * given timestep.
 * 
 * See the PDF for implementation details and other requirements.
 * 
 * AUTHORS: Saul Sanchez, Austin Leibensperger
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "matrix.h"
#include "util.h"
#include "formulas.h"

// Gravitational Constant in N m^2 / kg^2 or m^3 / kg / s^2
#define G 6.6743015e-11

// Softening factor to reduce divide-by-near-zero effects
#define SOFTENING 1e-9

// block size
#define BLOCK_SIZE 64




//double newtonsLawUnivGrav(double m1, double m2, double r)
//{
    //return G * m1 * m2 / (r * r * r);
//}

// double distance(double x1, double y1, double z1, double x2, double y2, double z2)
// {
//     double dx = x2 - x1;
//     double dy = y2 - y1;
//     double dz = z2 - z1;
//     return sqrt((dx * dx) + (dy * dy) + (dz * dz) + SOFTENING);
// }

// double superPositionPrinciple(double m1, double m2, double r)
// {
//     return G * m1 * m2 / ((r * r) * r * r + (SOFTENING * SOFTENING));
// }

// dont need mass for the newtonsLawUnivGrav and distance functions

// double acceleration(double force, double mass)
// {
//     return force / mass;
// }

// double velocity(double acceleration, double time)
// {
//     return acceleration * time;
// }

// double position(double velocity, double time)
// {
//     return velocity * time;
// }


int main(int argc, const char* argv[]) {
    // parse arguments
    if (argc != 6 && argc != 7) { fprintf(stderr, "usage: %s time-step total-time outputs-per-body input.npy output.npy [num-threads]\n", argv[0]); return 1; }
    double time_step = atof(argv[1]), total_time = atof(argv[2]);
    if (time_step <= 0 || total_time <= 0 || time_step > total_time) { fprintf(stderr, "time-step and total-time must be positive with total-time > time-step\n"); return 1; }
    size_t num_outputs = atoi(argv[3]);
    if (num_outputs <= 0) { fprintf(stderr, "outputs-per-body must be positive\n"); return 1; }
    Matrix* input = matrix_from_npy_path(argv[4]);
    if (input == NULL) { perror("error reading input"); return 1; }
    if (input->cols != 7) { fprintf(stderr, "input.npy must have 7 columns\n"); return 1; }
    size_t n = input->rows;
    if (n == 0) { fprintf(stderr, "input.npy must have at least 1 row\n"); return 1; }
    size_t num_steps = (size_t)(total_time / time_step + 0.5);
    if (num_steps < num_outputs) { num_outputs = 1; }
    size_t output_steps = num_steps/num_outputs;
    num_outputs = (num_steps+output_steps-1)/output_steps;

    // variables available now:
    //   time_step    number of seconds between each time point
    //   total_time   total number of seconds in the simulation
    //   num_steps    number of time steps to simulate (more useful than total_time)
    //   num_outputs  number of times the position will be output for all bodies
    //   output_steps number of steps between each output of the position
    //   input        n-by-7 Matrix of input data
    //   n            number of bodies to simulate

    // start the clock
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);


    // inside main function, after the start clock
    Positions* positions = (Positions*)malloc(n * 3 * sizeof(Positions));
    Positions* velocities = (Positions*)malloc(n * 3 * sizeof(Positions));
    double* forces = (double*)malloc(n * 3 * sizeof(double));
    double* masses = (double*)malloc(n * sizeof(double));

    // initialize positions, velocities, and masses
    for (size_t i = 0; i < n; i++) {
        masses[i] = MATRIX_AT(input, i, 0);
        positions[i/BLOCK_SIZE * 3].x[i%BLOCK_SIZE] = MATRIX_AT(input, i, 1);
        positions[i/BLOCK_SIZE * 3 + 1].y[i%BLOCK_SIZE] = MATRIX_AT(input, i, 2);
        positions[i/BLOCK_SIZE * 3 + 2].z[i%BLOCK_SIZE] = MATRIX_AT(input, i, 3);
        velocities[i/BLOCK_SIZE * 3].x[i%BLOCK_SIZE] = MATRIX_AT(input, i, 4);
        velocities[i/BLOCK_SIZE * 3 + 1].y[i%BLOCK_SIZE] = MATRIX_AT(input, i, 5);
        velocities[i/BLOCK_SIZE * 3 + 2].z[i%BLOCK_SIZE] = MATRIX_AT(input, i, 6);
    }

    // create the output matrix

    Matrix* output = matrix_create_raw(num_outputs, 3*n);



    // save positions to row `0` of output
    for (size_t i = 0; i < n; i++) {
        MATRIX_AT(output, 0, i * 3 + 0) = MATRIX_AT(input, i, 1);
        MATRIX_AT(output, 0, i * 3 + 1) = MATRIX_AT(input, i, 2);
        MATRIX_AT(output, 0, i * 3 + 2) = MATRIX_AT(input, i, 3);
    }



    // run the simulation for each time step
    for (size_t step = 1; step < num_steps; step++) {
        // compute time step
        calculateForces(forces, positions, masses, n);
        calculateVelocities(velocities, forces, masses, n, time_step);
        calculatePositions(positions, velocities, n, time_step);
        // Periodically copy the positions to the output data
        if (step % output_steps == 0) {
            for (size_t i = 0; i < n; i++) {
                MATRIX_AT(output, step / output_steps, i * 3 + 0) = positions[i/BLOCK_SIZE * 3].x[i%BLOCK_SIZE];
                MATRIX_AT(output, step / output_steps, i * 3 + 1) = positions[i/BLOCK_SIZE * 3 + 1].y[i%BLOCK_SIZE];
                MATRIX_AT(output, step / output_steps, i * 3 + 2) = positions[i/BLOCK_SIZE * 3 + 2].z[i%BLOCK_SIZE];
            }
        }
    }

    if (num_steps % output_steps != 0) {
        // save positions to row 'num_outputs - 1' of the output matrix
        for (size_t i = 0; i < n; i++) {
            // apply a unary function to each element of the matrix
            MATRIX_AT(output, num_outputs - 1, i * 3 + 0) = positions[i/BLOCK_SIZE * 3].x[i%BLOCK_SIZE];
            MATRIX_AT(output, num_outputs - 1, i * 3 + 1) = positions[i/BLOCK_SIZE * 3 + 1].y[i%BLOCK_SIZE];
            MATRIX_AT(output, num_outputs - 1, i * 3 + 2) = positions[i/BLOCK_SIZE * 3 + 2].z[i%BLOCK_SIZE];
        }
    }

    // get the end and computation time
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = get_time_diff(&start, &end);
    printf("%f secs\n", time);

    // save results
    matrix_to_npy_path(argv[5], output);

    // cleanup
    free(positions);
    free(velocities);
    free(masses);
    free(forces);
    matrix_free(input);

    return 0;

}