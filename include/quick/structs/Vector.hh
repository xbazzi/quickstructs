#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <type_traits>

namespace quick::structs
{
template <typename Element> class Vector
{
  private:
    std::uint64_t m_size{0};
    std::uint64_t m_cap{1};
    Element *p_arr{nullptr};

    static Element *_allocate(std::uint64_t alloc_size)
    {
        if (alloc_size == 0)
            return nullptr;
        return static_cast<Element *>(::operator new(sizeof(Element) * alloc_size));
    }

    void _grow_capacity(std::uint64_t new_cap)
    {
        Element *new_arr = _allocate(new_cap);
        std::size_t i = 0;
        try
        {
            for (; i < m_size; ++i)
                std::construct_at(new_arr + i, std::move_if_noexcept(p_arr[i]));
        }
        catch (...)
        {
            std::destroy_n(new_arr, i);
            ::operator delete(new_arr);
            throw;
        }

        std::destroy_n(p_arr, m_size);
        ::operator delete(p_arr);
        p_arr = new_arr;
        m_cap = new_cap;
    }

    void _destroy_and_free()
    {
        if (p_arr)
        {
            std::destroy_n(p_arr, m_size);
            ::operator delete(p_arr);
        }
        return;
    }

  public:
    Vector() : m_size{0}, p_arr{static_cast<Element *>(::operator new(sizeof(Element)))}
    {
    }

    Vector(std::uint64_t size) : m_size{size}
    {
        m_cap = std::max<std::uint64_t>(1ULL, size);
        p_arr = _allocate(m_cap);
        for (std::uint64_t i = 0; i < size; ++i)
        {
            std::construct_at(p_arr + i);
        }
    }

    ~Vector()
    {
        std::destroy_n(p_arr, m_size);
        ::operator delete(p_arr);
    }

    Element &operator[](std::size_t pos)
    {
        return *(p_arr + pos);
    }

    template <class T> void push_back(T &&element)
    {
        if (m_cap == 1) [[unlikely]]
            _grow_capacity(m_cap * 3);
        if (m_size >= m_cap)
            _grow_capacity(m_cap * 3);
        Element *inserter = p_arr + m_size;

        // "Tryhard version"
        // std::construct_at<std::remove_pointer_t<decltype(inserter)>>(inserter,
        // element);
        //
        // "Never touched a woman"
        // ::new (static_cast<void*>(inserter))
        //     std::remove_reference_t<Element>(std::forward<Element>(element));
        //
        // Touches grass
        std::construct_at(inserter, std::forward<Element>(element));
        ++m_size;
    }

    template <class... Args> void emplace_back(Args &&...args)
    {
        if (m_cap == 1) [[unlikely]]
            _grow_capacity(m_cap * 3);
        if (m_size >= m_cap)
            _grow_capacity(m_cap * 3);
        Element *inserter = p_arr + m_size;

        // Construct element in-place with forwarded arguments
        std::construct_at(inserter, std::forward<Args>(args)...);
        ++m_size;
    }

    const Element &at(std::size_t index) const
    {
        if (index >= m_size)
            throw std::out_of_range("u tripping");
        return *(static_cast<Element *>(p_arr + index));
    }

    std::size_t size() const
    {
        return m_size;
    }

    std::size_t capacity() const
    {
        return m_cap;
    }

    void shrink_to_fit()
    {
        if (m_cap > m_size)
            _grow_capacity(m_size);
    }

    void resize(uint64_t new_cap)
    {
        m_cap = new_cap;
    }

    void pop_back()
    {
        if (!m_size)
            return;
        --m_size;
        std::destroy_at(p_arr + m_size);
    }
};
} // namespace quick::structs
