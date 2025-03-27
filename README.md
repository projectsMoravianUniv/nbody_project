# N-Body Problem Simulation

<script type="text/javascript" src="https://polyfill.io/v3/polyfill.min.js?features=es6"></script>
<script type="text/javascript" id="MathJax-script" async
  src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>

## Project Overview

This project implements an efficient simulation of n bodies in a 3D space under gravitational influence. The simulation follows Newton’s Law of Universal Gravitation, the Superposition Principle, and Newton’s Second Law of Motion.

## Physics Background

The motion of each body is influenced by the gravitational force exerted by all other bodies. The fundamental equations used include:

- **Newton’s Law of Universal Gravitation**:
  \[
  F_{ij} = G \frac{m_i m_j}{r_{ij}^2}
  \]
  where:
  - \( G = 6.6743015 \times 10^{-11} \) Nm²/kg² (gravitational constant)
  - \( r_{ij} = \sqrt{(x_j - x_i)^2 + (y_j - y_i)^2 + (z_j - z_i)^2 + \epsilon} \)
    - Distance between bodies, with softening factor \( \epsilon = 10^{-9} \) to prevent division by zero.

- **Superposition Principle**: The net force acting on a body is the sum of all individual gravitational forces.
- **Newton’s Second Law**: Acceleration is determined as:
  \[
  a_x = \frac{F_x}{m_i}, \quad a_y = \frac{F_y}{m_i}, \quad a_z = \frac{F_z}{m_i}
  \]
- **Leapfrog Integration**: Used for numerical integration to update velocities and positions efficiently.

## Program Variants

Four different implementations of the N-Body simulation are provided:

1. `nbody-s`: Serial implementation using a naive approach.
2. `nbody-s3`: Serial implementation utilizing Newton’s Third Law for optimization.
3. `nbody-p`: Parallel implementation of the naive approach.
4. `nbody-p3`: Parallel implementation using Newton’s Third Law for efficiency.

## Command-Line Arguments

Each program follows the same command-line interface:

```
./nbody-<version> <time-step> <total-time> <outputs-per-body> <input.npy> <output.npy> [num-threads]
```

- **time-step**: Floating-point number (>0) representing Δt in seconds.
- **total-time**: Floating-point number (≥ Δt) representing the total simulation time.
- **outputs-per-body**: Number of recorded positions per body.
- **input.npy**: Input file containing initial data (mass, position, velocity).
- **output.npy**: Output file storing simulation results.
- **num-threads** (optional): Number of threads for parallel execution.

## Input and Output Format

**Input: `input.npy`**

An `n x 7` matrix where each row contains:

- Mass (kg)
- Initial x, y, z position (m)
- Initial vx, vy, vz velocity (m/s)

**Output: `output.npy`**

A `num_outputs x 3n` matrix storing the positions of all bodies over time.

## Performance Optimization

To meet performance benchmarks, several optimizations are used:

- **Avoid redundant calculations**: Compute \( F_{ij} \) only for \( j < i \) using Newton’s Third Law.
- **Efficient memory usage**: Avoid dynamic memory allocation inside loops.
- **Loop unrolling & vectorization**: Improve computation efficiency.
- **Parallelization**: Use OpenMP for multi-threading in `nbody-p` and `nbody-p3`.
- **Minimize function call overhead**: Use inline static functions.

## Benchmark Requirements

The program must achieve the following execution times on Expanse using `-Ofast`:

| Dataset     | Serial (s/s3) | Parallel (p/p3) |
|------------|--------------|----------------|
| random100  | ≤ 0.1 sec    | ≤ 0.1 sec      |
| random1000 | ≤ 8 sec      | ≤ 1.5 sec      |
| random10000| ≤ 750 sec    | ≤ 20 sec       |

**Extra Credit Targets**

| Dataset     | Serial (s/s3) | Parallel (p/p3) |
|------------|--------------|----------------|
| random100  | ≤ 0.05 sec   | ≤ 0.015 sec    |
| random1000 | ≤ 4.5 sec    | ≤ 0.4 sec      |
| random10000| ≤ 450 sec    | ≤ 15 sec       |

## Analysis and Reporting

 `analysis.md` covers the following things:

1. Speedup of parallel vs. serial implementations at different sizes.
2. Parallel efficiency using Amdahl’s Law.
3. Crossover points where different implementations outperform each other.
4. Plots and visualizations for performance trends.

## Running Tests and Visualization

**Example test run:**

```
python3 compare_npy.py my_output.npy expected_output.npy
```

**Visualization:**

```
python3 visualize.py my_output.npy
```

## Notes

- Start with small examples, validate correctness before scaling.
- Avoid unnecessary computation in performance-critical loops.
- Do not commit `.npy` files to the repository.

## Author

[Austin Leibensperger, Saul Sanchez, Professor Jeff Bush]