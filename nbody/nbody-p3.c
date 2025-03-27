/**
 * Runs a simulation of the n-body problem in 3D.
 * 
 * To compile the program:
 *   gcc -Wall -fopenmp -O3 -march=native nbody-p3.c matrix.c util.c -o nbody-p3 -lm
 * 
 * To run the program:
 *   ./nbody-p3 time-step total-time outputs-per-body input.npy output.npy [opt: num-threads]
 * where:
 *   - time-step is the amount of time between steps (Δt, in seconds)
 *   - total-time is the total amount of time to simulate (in seconds)
 *   - outputs-per-body is the number of positions to output per body
 *   - input.npy is the file describing the initial state of the system (below)
 *   - output.npy is the output of the program (see below)
 *   - last argument is an optional number of threads (a reasonable default is
 *     chosen if not provided)
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
 * AUTHORS: Austin Leibensperger and Saul Sanchez
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <omp.h>

#include "matrix.h"
#include "util.h"


#define BLOCK_SIZE 32
#include "formulap3.h"


int main(int argc, const char* argv[]) {
    // parse arguments
    if (argc != 6 && argc != 7) { fprintf(stderr, "usage: %s time-step total-time outputs-per-body input.npy output.npy [num-threads]\n", argv[0]); return 1; }
    double time_step = atof(argv[1]), total_time = atof(argv[2]);
    if (time_step <= 0 || total_time <= 0 || time_step > total_time) { fprintf(stderr, "time-step and total-time must be positive with total-time > time-step\n"); return 1; }
    size_t num_outputs = atoi(argv[3]);
    if (num_outputs <= 0) { fprintf(stderr, "outputs-per-body must be positive\n"); return 1; }
    size_t num_threads = argc == 7 ? atoi(argv[6]) : get_num_cores_affinity()/2; // TODO: you may choose to adjust the default value
    if (num_threads <= 0) { fprintf(stderr, "num-threads must be positive\n"); return 1; }
    Matrix* input = matrix_from_npy_path(argv[4]);
    if (input == NULL) { perror("error reading input"); return 1; }
    if (input->cols != 7) { fprintf(stderr, "input.npy must have 7 columns\n"); return 1; }
    size_t n = input->rows;
    if (n == 0) { fprintf(stderr, "input.npy must have at least 1 row\n"); return 1; }
    if (num_threads > n) { num_threads = n; }
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
    //   num_threads  number of threads to use
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
        forces[i * 3] = 0;
        forces[i * 3 + 1] = 0;
        forces[i * 3 + 2] = 0;
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
    #pragma omp parallel default(none) firstprivate(positions, velocities, masses, forces, n, output) shared(time_step, output_steps, num_steps) num_threads(num_threads)
    for (size_t step = 1; step < num_steps; step++) {
        // compute time step
        calculateForces(forces, positions, masses, n);
        calculateVelocities(velocities, forces, masses, n, time_step);
        calculatePositions(positions, velocities, n, time_step);

        //if (step % 8) {
            //printf("%zu velocities: %g %g %g\n", step, velocities[0].x[1], velocities[1].y[1], velocities[2].z[1]);
            //printf("%zu positions: %g %g %g\n", step, positions[0].x[1], positions[1].y[1], positions[2].z[1]);
        //}
        //if (step > 512) { break;}

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
