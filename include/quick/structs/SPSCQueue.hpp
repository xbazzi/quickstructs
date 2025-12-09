#pragma once

// C++ Includes
#include <x86intrin.h>

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

// FastInAHurry Includes

namespace quick::structs {

#if defined(__cpp_lib_hardware_interference_size)
using cacheline_t =
    std::integral_constant<std::uint64_t,
                           std::hardware_destructive_interference_size>;
#else
constexpr std::uint16_t CACHE_LINE_SIZE_BYTES{64};
using cacheline_t =
    std::integral_constant<std::uint64_t, CACHE_LINE_SIZE_BYTES>;
#endif

/// @brief  Lock free single-producer, single-consumer queue (constexpr
/// constructed)
/// @attention Not liable for damages if you have more than ONE
/// consumer/producer
///            pair of threads accessing this queue. Undefined behavior, data
///            races, all the good things...
/// @tparam T No array types pls
/// @tparam CapacityPow2 Capacity should be power of two for logical indexing
///
/// @todo I should make it so that old values are overwritten
/// @todo Maintain pointers (handles) to preallocated buckets for
/// pre-constructed
///       slots (instead of doing malloc and placement new on every push).
/// @todo Use `tcmalloc` instead
template <class T, std::uint64_t CapacityPow2>
class SPSCQueue {
  static_assert(!std::is_array_v<T>,
                "SPSCQueue does not support array element types");
  static_assert((CapacityPow2 & (CapacityPow2 - 1)) == 0,
                "Capacity must be a power of two");
  static constexpr std::uint64_t kCapacity = CapacityPow2;
  static constexpr std::uint64_t kMask = kCapacity - 1;

  alignas(cacheline_t::value) std::atomic<std::uint64_t> m_head{
      0};  // written by producer, read by consumer
  alignas(cacheline_t::value) std::atomic<std::uint64_t> m_tail{
      0};  // written by consumer, read by producer

  // Ensure storage is aligned both to cache-line boundaries (for
  // padding/padding avoidance) and to the element alignment so placement-new
  // on T is safe.
  alignas(cacheline_t::value) alignas(
      alignof(T)) std::byte m_storage[kCapacity * sizeof(T)];

  // Helpers to index into ring without branching
  static T* slot_ptr(std::byte* base, std::uint64_t idx) noexcept {
    return std::launder(reinterpret_cast<T*>(base + (idx & kMask) * sizeof(T)));
  }

 public:
  constexpr SPSCQueue() = default;
  SPSCQueue(const SPSCQueue&) = delete;
  SPSCQueue& operator=(const SPSCQueue&) = delete;

  ~SPSCQueue() {
    // Drain any constructed-but-not-popped elements to run destructors.
    // NOTE: destructor assumes no other threads are concurrently using the
    // queue. Calling the destructor while producer/consumer are active is
    // undefined behavior. We use acquire loads to ensure we observe the
    // latest published head/tail if called during shutdown synchronization.
    auto tail = m_tail.load(std::memory_order_acquire);
    auto head = m_head.load(std::memory_order_acquire);
    while (tail != head) {
      std::destroy_at(slot_ptr(m_storage, tail));
      ++tail;
    }
  }

  /// @brief Copy push. Try moving instead of you can. Calls `emplace(arg)`.
  /// @note Non-copyable types are fine as long as they are
  /// movable/constructible.
  /// @warning Push by value+move is often a tad faster than const& for small
  /// trivially movable T.
  /// @param x
  /// @return Success or failure as a bool.
  bool push(const T& x) { return emplace(x); }

  /// @brief Move push. Just calls `emplace(std::move(item))`.
  /// @return Success or failure as a bool.
  /// @param x
  /// @return
  bool push(T&& x) { return emplace(std::move(x)); }

  /// @brief Actually constructs objects in the queue.
  /// @tparam ...Args
  /// @param ...args
  /// @return Success or failure as a bool.
  template <class... Args>
  bool emplace(Args&&... args) {
    // Producer thread only mutates m_head
    std::uint64_t head = m_head.load(std::memory_order_relaxed);
    // Read consumer's tail with acquire to observe element reclamation
    std::uint64_t tail = m_tail.load(std::memory_order_acquire);

    // Full if producer is kCapacity ahead of consumer
    if ((head - tail) == kCapacity) [[unlikely]]
      return false;

    T* p = slot_ptr(m_storage, head);
    std::construct_at(p, std::forward<Args>(args)...);

    // Publish the new element: release pairs with consumer's acquire
    m_head.store(head + 1, std::memory_order_release);
    return true;
  }

  /// @brief Pop into the passed argument (by reference) to prevent moves
  bool pop(T& out) {
    // Consumer thread only mutates m_tail
    std::uint64_t tail = m_tail.load(std::memory_order_relaxed);
    // Read producer's head with acquire to observe the constructed element
    std::uint64_t head = m_head.load(std::memory_order_acquire);

    if (head == tail) [[unlikely]]
      return false;  // empty

    T* p = slot_ptr(m_storage, tail);
    out = std::move(*p);
    std::destroy_at(p);

    // Release to publish reclamation to producer
    m_tail.store(tail + 1, std::memory_order_release);
    return true;
  }

  bool empty() const noexcept {
    // Acquire not strictly required here for SPSC fast-path introspection,
    // but we’ll use acquire on head to avoid surprising reorders.
    auto tail = m_tail.load(std::memory_order_relaxed);
    auto head = m_head.load(std::memory_order_acquire);
    return head == tail;
  }

  bool full() const noexcept {
    auto head = m_head.load(std::memory_order_relaxed);
    auto tail = m_tail.load(std::memory_order_acquire);
    return (head - tail) == kCapacity;
  }

  std::uint64_t size() const noexcept {
    auto head = m_head.load(std::memory_order_acquire);
    auto tail = m_tail.load(std::memory_order_relaxed);
    return head - tail;
  }

  static constexpr std::uint64_t capacity() noexcept { return kCapacity; }
};
/// @example spsc_queue_example.cc
/// An example in UniquePtr (delete this )

}  // End namespace quick::structs
