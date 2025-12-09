// Write your solution here
// C++20 for C++
// /////////////////////////////////////////////////////////////////////////
// If C++: Your code is automatically compiled with a precompiled header. //
// 99% of includes / packages are already added for you.                  //
// You do NOT need to add your own includes here.                         //
// /////////////////////////////////////////////////////////////////////////

#include <atomic>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
// #include <mutex>

namespace quick {

template <class T>
struct default_deleter {
    void operator()(T* ptr) const noexcept
    {
        static_assert(sizeof(T) > 0, "Cannot delete incomplete type.");
        if (ptr) {
            std::destroy_at(ptr);
            ::operator delete(ptr);
        }
    }
};

template <class T, class Deleter = default_deleter<T>>
struct control_block : private Deleter {
    static_assert(!(std::is_array_v<T> || std::is_bounded_array_v<T>), "No arr");

    std::atomic_uint64_t m_count { 1 };
    T* p_obj { nullptr };

    control_block(const Deleter& custom_deleter = Deleter {}) noexcept(std::is_nothrow_copy_constructible_v<Deleter>)
        : Deleter { custom_deleter }
    {
    }

    void init(T* ptr) noexcept
    {
        p_obj = ptr;
    }

    void add_ref() noexcept
    {
        m_count.fetch_add(1, std::memory_order_acq_rel);
    }

    void release()
    {
        if (m_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            if (p_obj) {
                static_cast<Deleter>(*this)(p_obj);
                p_obj = nullptr;
            }

            std::destroy_at(this);
            ::operator delete(this);
        }
    }

    uint64_t use_count() const noexcept
    {
        return m_count.load(std::memory_order_acquire);
    }
};

template <class T, class Deleter = default_deleter<T>>
class shared_ptr {
private:
    using Pointer_T = T*;
    using Pointer_CB = control_block<T, Deleter>*;
    using CB = control_block<T, Deleter>;

    Pointer_T p_obj;
    Pointer_CB p_cb;

public:
    shared_ptr()
        : p_obj { nullptr }
        , p_cb { nullptr }
    {
    }

    template <class U = Deleter>
        requires std::default_initializable<U>
    shared_ptr(Pointer_T pointer)
        : p_obj { pointer }
    {
        p_cb = static_cast<Pointer_CB>(::operator new(sizeof(CB)));
        std::construct_at(p_cb);
        p_cb->init(p_obj);
    }

    shared_ptr(Pointer_T pointer, const Deleter& d)
        : p_obj { pointer }
    {
        p_cb = static_cast<Pointer_CB>(::operator new(sizeof(CB)));
        std::construct_at(p_cb, d);
        p_cb->init(p_obj);
    }

    shared_ptr(const shared_ptr& other) noexcept
        : p_obj { other.p_obj }
        , p_cb { other.p_cb }
    {
        if (p_cb) {
            p_cb->add_ref();
        }
    }

    shared_ptr& operator=(const shared_ptr& other) noexcept
    {
        if (this != &other) {
            if (p_cb) {
                p_cb->release();
            }
            p_obj = other.p_obj;
            p_cb = other.p_cb;
            if (p_cb) {
                p_cb->add_ref();
            }
        }
        return *this;
    }

    shared_ptr(shared_ptr&& other) noexcept
        : p_obj { other.p_obj }
        , p_cb { other.p_cb }
    {
        other.p_obj = nullptr;
        other.p_cb = nullptr;
    }

    shared_ptr& operator=(shared_ptr&& other) noexcept
    {
        if (this != &other) {
            if (p_cb) {
                p_cb->release();
            }
            p_obj = other.p_obj;
            p_cb = other.p_cb;

            other.p_obj = nullptr;
            other.p_cb = nullptr;
        }
        return *this;
    }

    ~shared_ptr()
    {
        reset();
    }

    void reset() noexcept
    {
        if (p_cb) {
            p_cb->release();
            p_cb = nullptr;
        }
        p_obj = nullptr;
    }

    void reset(T* pointer, const Deleter& d)
    {
        if (p_cb) {
            p_cb->release();
        }
        p_obj = pointer;
        if (p_obj != nullptr) {
            p_cb = static_cast<Pointer_CB>(::operator new(sizeof(CB)));
            std::construct_at(p_cb, d);
            p_cb->init(p_obj);
        } else {
            p_cb = nullptr;
        }
    }

    size_t use_count() const
    {
        return p_cb ? p_cb->use_count() : 0;
    }

    T* operator->() const
    {
        return p_obj;
    }
    T& operator*() const
    {
        return *p_obj;
    }
    operator bool() const noexcept
    {
        return p_obj != nullptr;
    }
};
} // namespace quick

int main()
{
    int p = 5;
    int* x = &p;
    auto deleter = [&](int*) { };
    quick::shared_ptr<int, decltype(deleter)> intptr(x, deleter);
    std::cout << intptr.use_count() << " ";
    quick::shared_ptr<int, decltype(deleter)> anotherone(std::move(intptr));
    std::cout << anotherone.use_count() << " ";
    intptr.reset();
    std::cout << anotherone.use_count() << " ";
    anotherone.reset();

    std::cout << anotherone.use_count() << " ";
}