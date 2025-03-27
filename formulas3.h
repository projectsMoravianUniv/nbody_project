#ifndef FORMULAS3_H
#define FORMULAS3_H

#include <math.h>

#define G 6.6743015e-11
#define SOFTENING 1e-9
#define BLOCK_SIZE 64

typedef struct {
    double x[BLOCK_SIZE];
    double y[BLOCK_SIZE];
    double z[BLOCK_SIZE];
} Positions;
// this function calculates the forces
inline static double* calculateForces(double* forces, Positions* positions, double* masses, size_t n)
{
    // this is the main loop that calculates the forces
    // for each body in the system
    for (size_t i = 0; i < n; i++)
    {
        
        for (size_t j = i + 1; j < n; j++)
        { 
            double dx = positions[j/BLOCK_SIZE * 3].x[j%BLOCK_SIZE] - positions[i/BLOCK_SIZE * 3].x[i%BLOCK_SIZE];
            double dy = positions[j/BLOCK_SIZE * 3 + 1].y[j%BLOCK_SIZE] - positions[i/BLOCK_SIZE * 3 + 1].y[i%BLOCK_SIZE];
            double dz = positions[j/BLOCK_SIZE * 3 + 2].z[j%BLOCK_SIZE] - positions[i/BLOCK_SIZE * 3 + 2].z[i%BLOCK_SIZE];               
            double r = sqrt((dx * dx) + (dy * dy) + (dz * dz) + SOFTENING);
            double force = G * masses[i] * masses[j] / (r * r * r);

            forces[i*3] += dx * force;
            forces[i*3 + 1] += dy * force;
            forces[i*3 + 2] += dz * force;
            
            forces[j*3] -= dx * force;
            forces[j*3 + 1] -= dy * force;
            forces[j*3 + 2] -= dz * force;

        }
    }
    return forces;
}

// this function calculates the velocities
inline static Positions* calculateVelocities(Positions* velocities, double* forces, double* masses, size_t n, double time_step)
{
    for (size_t i = 0; i < n; i++)
    {
        velocities[i/BLOCK_SIZE * 3].x[i%BLOCK_SIZE] += forces[i * 3] / masses[i] * time_step;
        velocities[i/BLOCK_SIZE * 3 + 1].y[i%BLOCK_SIZE] += forces[i * 3 + 1] / masses[i] * time_step;
        velocities[i/BLOCK_SIZE * 3 + 2].z[i%BLOCK_SIZE] += forces[i * 3 + 2] / masses[i] * time_step;

        forces[i * 3] = 0;
        forces[i * 3 + 1] = 0;
        forces[i * 3 + 2] = 0;
    }
    return velocities;
}

// this function calculates the positions
inline static Positions* calculatePositions(Positions* positions, Positions* velocities, size_t n, double time_step)
{
    for (size_t i = 0; i < n; i++)
    {
        positions[i/BLOCK_SIZE * 3].x[i%BLOCK_SIZE] += velocities[i/BLOCK_SIZE * 3].x[i%BLOCK_SIZE] * time_step;
        positions[i/BLOCK_SIZE * 3 + 1].y[i%BLOCK_SIZE] += velocities[i/BLOCK_SIZE * 3 + 1].y[i%BLOCK_SIZE] * time_step;
        positions[i/BLOCK_SIZE * 3 + 2].z[i%BLOCK_SIZE] += velocities[i/BLOCK_SIZE * 3 + 2].z[i%BLOCK_SIZE] * time_step;
    }
    return positions;
}

#endif // FORMULAS3_H