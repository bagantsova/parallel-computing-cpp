# parallel-computing-cpp

Concurrency primitives and measurements in C++20: parallel Monte Carlo
integration with `std::packaged_task` and `std::future`, and thread-safe stack
and queue implementations benchmarked against Boost.Lockfree.

Written as coursework at MIPT (Applied Mathematics and Physics) and cleaned up
for a portable toolchain.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Requires a C++20 compiler and CMake 3.16. Boost is optional: without it the
lock-free comparison benchmarks are skipped and `monte_carlo_pi` still builds.

## Monte Carlo integration

Estimates pi by rejection sampling inside the unit circle, sequentially and
then across `std::thread::hardware_concurrency()` threads.

- work split into equal blocks, one `std::packaged_task` per thread, results
  collected through `std::future`
- a scope guard joins every thread on destruction, so an exception in the
  collection loop cannot leave a thread detached
- each worker seeds its generator from `std::this_thread::get_id()`, so the
  streams do not correlate

```bash
./build/monte_carlo_pi
```

Measured on two cores, 10^7 samples:

| | estimate | time |
|---|---:|---:|
| sequential | 3.14248 | 189 ms |
| parallel | 3.14220 | 95 ms |

A 1.99x speedup on two cores, which is what a problem with no shared writes
and no synchronisation inside the loop should give.

## Thread-safe containers

`threadsafe_stack.hpp` and `threadsafe_queue.hpp` are lock-based containers
with the interface hazards designed out — `pop` returns the value rather than
exposing a separate `top`/`pop` pair that cannot be made atomic from outside,
and the copy happens under the lock.

The benchmarks push and pop from several threads at once and compare against
`boost::lockfree::stack` and `boost::lockfree::queue`, so the cost of the mutex
is measured against a lock-free implementation rather than asserted.

```bash
./build/threadsafe_stack_bench      # requires Boost
./build/threadsafe_queue_bench
```

## Layout

```
src/         monte_carlo_pi.cpp, the two benchmarks
include/     timer.hpp, threadsafe_stack.hpp, threadsafe_queue.hpp
```

## Licence

MIT. See [LICENSE](LICENSE).
