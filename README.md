*This project has been created as part of the 42 curriculum by <login1>.*

# Codexion

## Description

Codexion simulates coders sharing a circular co-working hub with a limited pool of
USB dongles. Each coder cycles through **compiling** (requires two dongles held
simultaneously), **debugging**, and **refactoring**. A coder who fails to start
compiling within `time_to_burnout` milliseconds of their last compile (or of the
simulation start) burns out. The goal is to model resource-sharing between
concurrent threads using mutexes and condition variables while avoiding deadlock,
starvation, and burnout — under two different arbitration policies (FIFO / EDF).

## Instructions

```sh
make
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

Example:

```sh
./codexion 5 800 200 100 100 5 20 fifo
```

`scheduler` must be exactly `fifo` or `edf`.

## Resources

- TODO: link the POSIX threads man pages / tutorials you actually read
  (pthread_create, pthread_mutex, pthread_cond).
- TODO: link any dining-philosophers writeups you used.
- **AI usage**: TODO — describe honestly which parts (if any) were drafted with
  AI assistance, for which specific tasks, and how you verified/rewrote them.
  Per the subject's AI Instructions chapter, you must be able to explain every
  line during the defense.

## Blocking cases handled

- **Deadlock (Coffman conditions)**: a coder never holds one dongle while
  waiting for the other. Both dongles for a coder are granted atomically by a
  single arbitrator (`try_dispatch`, protected by `table_lock`); a request sits
  in a priority queue until *both* its dongles are simultaneously free. This
  removes the hold-and-wait / circular-wait conditions that cause the classic
  dining-philosophers deadlock.
- **Starvation / liveness**: under `edf`, the coder closest to burning out is
  always granted its dongles first among competing requests whose resources
  are free, which keeps any single coder from being perpetually skipped as
  long as the parameters are feasible. Under `fifo`, arrival order is
  strictly respected via a monotonic sequence counter (not wall-clock
  timestamps, to avoid ties).
- **Dongle cooldown**: a released dongle stores a `cooldown_until` timestamp;
  it is excluded from `try_dispatch` until that time has passed.
- **Precise burnout detection**: a dedicated monitor thread polls every
  0.5 ms and logs `burned out` as soon as `now - last_compile_start` exceeds
  `time_to_burnout`, comfortably inside the 10 ms tolerance.
- **Log serialization**: all logging goes through `log_state`, which holds a
  single `print_lock` mutex for the full read-timestamp + printf, so lines
  from different threads can never interleave.
- **Special case, 1 coder**: with a single coder there is only one dongle on
  the table (per the subject), so the two "hands" resolve to the same
  dongle index. `try_dispatch` explicitly refuses to grant when
  `left == right`, since physically holding one dongle twice is impossible —
  that coder is expected to burn out.

## Thread synchronization mechanisms

- `table_lock` (`pthread_mutex_t`) + `table_cond` (`pthread_cond_t`): protect
  the dongle array and the request heap. A coder wanting to compile pushes a
  request into the heap and calls `pthread_cond_timedwait` in a loop, waking
  periodically (or whenever another thread broadcasts, e.g. on release) to
  re-attempt dispatch. This avoids busy-waiting while still reacting quickly
  to time-based events like cooldown expiry.
- `state_lock`: protects each coder's `last_compile_start` and
  `compiles_done`, which are written by the coder thread and read by the
  monitor thread — preventing a torn/half-updated read from causing a false
  burnout report or a missed one.
- `print_lock`: serializes all `printf` calls (see above).
- `stop` flag (`volatile sig_atomic_t`): a single-writer-many-reader boolean
  set once by the monitor thread when the simulation should end (burnout or
  all coders reached `number_of_compiles_required`). Kept lock-free
  intentionally, since it only ever transitions 0 → 1 and every reader only
  needs the latest value to stop promptly — avoids a lock-ordering cycle
  that would otherwise exist between `table_lock` and a hypothetical
  `stop_lock`.
