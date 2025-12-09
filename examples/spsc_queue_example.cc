// C++ Includes
#include <thread>
#include <cstdint>

// FastInAHurry Includes
#include "fiah/structs/SPSCQueue.hh"

namespace fiah {
static void ExampleSPSCQueue()
{
    using ElementType = double;

    // 1 KB queue
    constexpr uint16_t N = (1 << 10) / sizeof(ElementType);
    constexpr fiah::structs::SPSCQueue<ElementType, N> queue; 
    static_asset(queue.capacit)

    std::thread prod([&]{
        for (int i = 0; i < 100000; ++i) {
        while (!q.emplace()) { /* spin/yield if you want */ }
        }
    });

    std::thread cons([&]{
        int v;
        std::uint64_t cnt = 0;
        while (cnt < 100000) 
        {
            if (q.pop(v)) 
            { 
                ++cnt; 
            }
        }
        std::cout << "Got: " << cnt << " items\n";
    });

    prod.join(); cons.join();
}
} // End namespace fiah