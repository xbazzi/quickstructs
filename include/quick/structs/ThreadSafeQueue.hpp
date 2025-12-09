// C++ Includes
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace quick::structs {

/// @brief Generic thread-safe locking queue, backed by std::queue
/// This is really just a std::dequeue under the hood. We should make
/// our own container underlying type (like in SPSCQueue.hh)
/// @attention Prefer the lock-free SPSCQueue instead
/// @tparam T No array types pls
template <typename T>
class ThreadSafeQueue {
 private:
  std::queue<T> _queue;
  std::mutex _mutex;
  std::condition_variable _cv;

 public:
  void push(T value) {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _queue.push(std::move(value));
    }
    _cv.notify_one();
  }

  bool empty() { return _queue.empty(); }

  T wait_and_pop() {
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [&] { return !_queue.empty(); });
    T val = std::move(_queue.front());
    _queue.pop();
    return val;
  }
};
}  // namespace quick::structs