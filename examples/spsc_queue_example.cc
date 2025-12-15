// C++ Includes
#include <cstdint>
#include <iostream>
#include <thread>

// QuickLib Includes
#include "quick/structs/SPSCQueue.hh"

namespace fiah
{
static void ExampleSPSCQueue()
{
    using ElementType = double;

    // 1 KB queue
    constexpr uint16_t N = (1 << 10) / sizeof(ElementType);
    quick::structs::SPSCQueue<ElementType, N> queue;
    static_assert(queue.capacity() == N);

    std::thread prod([&] {
        for (int i = 0; i < 100000; ++i)
        {
            while (!queue.emplace(static_cast<ElementType>(i)))
            {
            }
        }
    });
    std::thread cons([&] {
        ElementType v;
        std::uint64_t cnt = 0;
        while (cnt < 100000)
        {
            if (queue.pop(v))
            {
                ++cnt;
            }
        }
        std::cout << "Got: " << cnt << " items\n";
    });

    prod.join();
    cons.join();
}
} // End namespace fiah

int main()
{
    fiah::ExampleSPSCQueue();
    return 0;
}