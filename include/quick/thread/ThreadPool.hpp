// C++ Includes
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <format>  // do we need?
#include <functional>
#include <future>
#include <print>
#include <queue>
#include <ranges>
#include <thread>

namespace quick::thread {

namespace {
thread_local int t_thread_id = -1;
}

class ThreadPool {
 public:
  ThreadPool() = default;
  ThreadPool(std::size_t num_threads);

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;
  ~ThreadPool();

  template <class F, class... Args>
  auto enqueue(F&& f, Args&&... args)
      -> std::future<std::invoke_result_t<F, Args...>>;
  std::string get_thread_id() const noexcept;
  std::size_t get_num_active_tasks() const noexcept;
  std::size_t get_num_threads() const noexcept;

 private:
  std::size_t m_num_threads{0};
  std::queue<std::move_only_function<void()>> m_tasks;
  std::vector<std::jthread> m_workers;
  mutable std::mutex m_mutex;
  std::counting_semaphore<> m_semaphore{0};
  std::atomic<std::size_t> m_active_tasks{0};
  std::atomic_bool m_stopping{false};
};
}  // End namespace quick::thread