# 1. What is the speedup of the naïve parallel program compared to the serial program?
- At a size of random100 the fastest time for serial was 0.042883

- At a size of random1000 the fastest time for serial was 4.192066

- At a size of random10000 the fastest time for serial was 421.080992

- At a size of random100 the fastest time for parallel was 0.038205 with 8 threads

- At a size of random1000 the fastest time for parallel was 0.497687 with 32 threads

- At a size of random10000 the fastest time for parallel was 8.506878 with 64 threads

We see that the parallel program runs faster than the serial for all these sizes. Optimizing the number of threads used in our parallel program allows us to run our program with the higher performances.

- P size random100 =  (0.042883 - 0.038205) / 0.042883 * 100 = 10.90875
- P size random1000 =  (4.192066 - 0.497687) / 4.192066 * 100 = 88.12788
- P size random1000 =  (421.080992 - 8.506878) / 421.080992 * 100 = 97.97975

There was no possible crossover point where our serial program was faster than our parallel program on expanse. Locally these differences may vary, potentially having a crossover point.


# 2. Repeat the above but for the 3rd-law program.
- At a size of random100 the fastest time for serial 3rd law was 0.037075

- At a size of random1000 the fastest time for serial 3rd law was 3.613398

- At a size of random10000 the fastest time for serial 3rd law 362.094913

- At a size of random100 the fastest time for parallel 3rd law 0.089737 with 128 threads

- At a size of random1000 the fastest time for parallel 3rd law 1.425298 with 128 threads

- At a size of random10000 the fastest time for parallel 3rd law 41.683292 with 256 threads

This time we see that serial 3rd law was faster than parallel 3rd law for a size of random100 however, for sizes of random1000 and random1000 the parallel 3rd law wad faster than the serial. Again we optimized the number of threads to receive higher performances.

- P size random100 =  (0.037075 - 0.089737) / 0.037075 * 100 = -142.0418
- P size random1000 =  (3.613398 - 1.425298) / 3.613398 * 100 = 60.55519
- P size random1000 =  (362.094913 - 41.683292) / 362.094913 * 100 = 88.4883

We see that only for a size of random100 our serial 3rd law was faster than our parallel on expanse. Giving us a negative parallelism value.

# 3. Compare your naïve program to your serial program. For what sizes is the naïve
program faster and when is the 3rd-law program faster? What is the crossover point?

The serial 3rd-Law program is faster for all sizes of random100, random1000, and random10000 than the naïve serial program because of this there are no crossover points.

Plots are located in a HTML file.


