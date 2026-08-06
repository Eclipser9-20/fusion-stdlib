// librandom — the Fusion `random` module. Fetched into /usr/local/Fusion/modules,
// unlocked with `import random`. LCG seeded once from the wall clock.
#include <stdint.h>
#include <sys/time.h>
static uint64_t _state = 0;
static uint64_t _next(void) {
    if (!_state) { struct timeval tv; gettimeofday(&tv, 0); _state = (uint64_t)tv.tv_sec * 1000000ull + (uint64_t)tv.tv_usec; }
    _state = _state * 6364136223846793005ull + 1442695040888963407ull;
    return _state >> 32;                       // high bits = good distribution
}
long randomTo(long count)            { if (count <= 0) return 0; return (long)(_next() % (uint64_t)count); }   // [0, count)
long randomRange(long low, long high){ if (high < low) return low; return low + randomTo(high - low + 1); }    // [low, high] inclusive
