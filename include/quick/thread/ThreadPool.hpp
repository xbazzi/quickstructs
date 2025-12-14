// C++ Includes
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <format>
#include <functional>
#include <future>
#include <print>
#include <queue>
#include <ranges>
#include <thread>

#include "quick/utils/Timer.hh"

namespace quick::thread
{

namespace
{
thread_local int t_thread_id = -1;
}

class ThreadPool
{
  public:
    ThreadPool() = default;
    ThreadPool(std::size_t num_threads);

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;
    ~ThreadPool();

    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>>;
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

ThreadPool::ThreadPool(std::size_t num_threads = std::thread::hardware_concurrency()) : m_num_threads{num_threads}
{
    quick::utils::Timer timer{"ThreadPool ctor"};
    using namespace std::chrono_literals;
    // auto stop_token = m_stop_source.get_token();
    m_workers.reserve(num_threads);
    auto range = std::views::iota(0, static_cast<int>(num_threads));
    std::ranges::for_each(range, [this](int thread_id) {
        m_workers.emplace_back([this, thread_id](std::stop_token stoken) {
            while (!stoken.stop_requested() and !m_stopping.load(std::memory_order_acquire))
            {
                /// @todo Investigate how this blocks (spin? futex?)
                m_semaphore.acquire();

                /// @todo do we need the empty task check? we are doing it
                /// in the scoped block anyway.
                if (stoken.stop_requested() && m_tasks.empty())
                    break;

                std::move_only_function<void()> task;
                {
                    std::scoped_lock<std::mutex> lock(m_mutex);
                    if (m_tasks.empty())
                        continue;
                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }

                t_thread_id = thread_id;
                task();
            }
        });
    });
}

std::size_t ThreadPool::get_num_threads() const noexcept
{
    return m_num_threads;
}

std::size_t ThreadPool::get_num_active_tasks() const noexcept
{
    std::scoped_lock<std::mutex> lock(m_mutex);
    return m_tasks.size();
}

std::string ThreadPool::get_thread_id() const noexcept
{
    return std::format("{}", t_thread_id);
}

template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>>
{
    using Ret = std::invoke_result_t<F, Args...>;
    auto task = std::packaged_task<Ret()>(std::bind_front(std::forward<F>(f), std::forward<Args>(args)...));
    auto fut = task.get_future();
    {
        std::scoped_lock lock(m_mutex);
        m_tasks.emplace(std::move(task));
    }
    m_semaphore.release();
    return fut;
}

ThreadPool::~ThreadPool()
{
    m_stopping.store(true, std::memory_order_release);
    std::ranges::for_each(m_workers, [this](auto &thread) {
        m_semaphore.release();
        thread.request_stop();
    });
}
} // End namespace quick::thread
