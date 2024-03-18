#ifndef FORMULAP_H
#define FORMULAP_H

#include <math.h> // Add the missing include directive for the "math.h" header file.

#define G 6.6743015e-11
#define SOFTENING 1e-9

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 64
#endif

typedef struct {
    double x[BLOCK_SIZE];
    double y[BLOCK_SIZE];
    double z[BLOCK_SIZE];
} Positions;



inline static double* calculateForces(double* forces, Positions* positions, double* masses, size_t n)
{
        #pragma omp for schedule(static, BLOCK_SIZE) 
        //#pragma omp parallel for default(none) firstprivate(n, positions, masses) shared(forces) schedule(static) reduction(+:forceX, forceY, forceZ)
        for (size_t i = 0; i < n; i++)
        {
            double forceX = 0;
            double forceY = 0;
            double forceZ = 0;
            for (size_t j = 0; j < n; j++)
            {
                if (i != j)
                {
                    double dx = positions[j/BLOCK_SIZE * 3].x[j%BLOCK_SIZE] - positions[i/BLOCK_SIZE * 3].x[i%BLOCK_SIZE];
                    double dy = positions[j/BLOCK_SIZE * 3 + 1].y[j%BLOCK_SIZE] - positions[i/BLOCK_SIZE * 3 + 1].y[i%BLOCK_SIZE];
                    double dz = positions[j/BLOCK_SIZE * 3 + 2].z[j%BLOCK_SIZE] - positions[i/BLOCK_SIZE * 3 + 2].z[i%BLOCK_SIZE];
                    double mj = masses[j];
                    double r = sqrt((dx * dx) + (dy * dy) + (dz * dz) + SOFTENING);
                    double force = G * mj / (r * r * r);
                    forceX += force * dx;
                    forceY += force * dy;
                    forceZ += force * dz;
                }
            }
            forces[i * 3] = forceX;
            forces[i * 3 + 1] = forceY;
            forces[i * 3 + 2] = forceZ;
        }
        return forces;
}

inline static Positions* calculateVelocities(Positions* velocities, double* forces, double* masses, size_t n, double time_step)
{
    #pragma omp for schedule(static, BLOCK_SIZE)
    for (size_t i = 0; i < n; i++)
    {
        velocities[i/BLOCK_SIZE * 3].x[i%BLOCK_SIZE] += forces[i * 3] * time_step;
        velocities[i/BLOCK_SIZE * 3 + 1].y[i%BLOCK_SIZE] += forces[i * 3 + 1] * time_step;
        velocities[i/BLOCK_SIZE * 3 + 2].z[i%BLOCK_SIZE] += forces[i * 3 + 2] * time_step;

    }
    return velocities;
}

inline static Positions* calculatePositions(Positions* positions, Positions* velocities, size_t n, double time_step)
{
    #pragma omp for schedule(static, BLOCK_SIZE)
    for (size_t i = 0; i < n; i++)
    {
        positions[i/BLOCK_SIZE * 3].x[i%BLOCK_SIZE] += velocities[i/BLOCK_SIZE * 3].x[i%BLOCK_SIZE] * time_step;
        positions[i/BLOCK_SIZE * 3 + 1].y[i%BLOCK_SIZE] += velocities[i/BLOCK_SIZE * 3 + 1].y[i%BLOCK_SIZE] * time_step;
        positions[i/BLOCK_SIZE * 3 + 2].z[i%BLOCK_SIZE] += velocities[i/BLOCK_SIZE * 3 + 2].z[i%BLOCK_SIZE] * time_step;
    }
    return positions;
}

#endif // FORMULAP_H