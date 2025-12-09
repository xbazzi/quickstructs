#include <atomic>
#include <cstdint>
#include <thread>
#include <x86intrin.h>

class Mutex {
public:
    void lock()
    {
        // Fast path — try once
        if (!flag.test_and_set(std::memory_order_acquire))
            return;

        // Contended path: spin a few times before yielding
        for (;;) {
            // Short spin phase: cheap relaxed reads to avoid cache thrash
            for (int i = 0; i < 64; ++i) {
                if (!flag.test(std::memory_order_relaxed)) {
                    if (!flag.test_and_set(std::memory_order_acquire))
                        return; // acquired lock
                }
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                // Optional architecture pause hint (x86: _mm_pause())
            }

            // Give the CPU/scheduler a break
            std::this_thread::yield();
        }
    }

    void unlock()
    {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

static_assert(sizeof(Mutex) <= 4, "Mutex must be <= 4 bytes");
