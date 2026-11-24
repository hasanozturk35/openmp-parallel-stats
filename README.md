# OpenMP Parallel Statistics

A small C benchmark that computes **sum**, **mean** and **maximum** over an integer array, both serially and in parallel with OpenMP, and measures the speedup.

## What it does

1. **Serial vs parallel**: computes the sum, mean and max of a 10,000-element array both ways and checks that the results match (so it would catch a race condition).
2. **Thread scaling**: times a parallel sum over 10,000,000 elements with 1, 2, 4 and 8 threads and prints a speedup table.
3. **`critical` vs `reduction`**: computes the max twice, once guarded by `#pragma omp critical` and once with `reduction(max:m)`, to show the cost of lock contention.

## Build & run

Requires GCC with OpenMP support (on Windows, MinGW-w64 through MSYS2).

```bash
gcc -fopenmp -O2 -o parallel_stats parallel_stats.c
./parallel_stats
```

To override the default thread count:

```bash
OMP_NUM_THREADS=4 ./parallel_stats          # Linux / macOS
$env:OMP_NUM_THREADS=4; .\parallel_stats    # Windows PowerShell
```

## Sample output

```
--- TOPLAM ---
Seri    toplam = 4966614
Paralel toplam = 4966614
Kontrol        : ESIT - dogru

  Thread | Sure (s)   | Speedup
  -------|------------|--------
       1 |   0.004000 |    1.00x
       2 |   0.003000 |    1.33x
       4 |   0.001000 |    4.00x
       8 |   0.002000 |    2.00x

critical  maksimum = 999  (sure: 0.307000 s)
reduction maksimum = 999  (sure: 0.002000 s)
reduction, critical'dan 153.5x daha hizli
```

## Takeaways

- `reduction` gives every thread a private accumulator and merges them once at the end, so the loop runs without any locking.
- `critical` serializes every iteration, which makes it about two orders of magnitude slower than `reduction`, even with 4 threads.
- On small inputs (N = 10,000), thread start-up costs more than the work itself, so speedup only shows up on large arrays.
- Timings are single runs with a millisecond-resolution timer on Windows, so the speedup numbers are noisy.

> Program output and comments are in Turkish.
