#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace quick::memory {

/// @brief Default deleter that calls delete on the Pointer_T
template <typename T>
struct default_deleter {
  constexpr default_deleter() noexcept = default;

  /// @brief Allow construction from compatible deleters
  template <typename U,
            typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
  default_deleter(const default_deleter<U>&) noexcept {}

  void operator()(T* ptr) const noexcept {
    static_assert(sizeof(T) > 0, "Cannot delete incomplete type");
    delete ptr;
  }
};

/// @brief Custom unique Pointer_T implementation as an alternative to
/// std::UniquePtr
/// @tparam T The type being managed
/// @tparam Deleter The deleter type (defaults to default_deleter<T>)
///
/// This class provides RAII-style ownership of a dynamically allocated object.
/// It is move-only and cannot be copied. When the UniquePtr is destroyed or
/// reset, it calls the deleter on the managed object.
///
/// Example usage:
/// @code
/// ```
///   auto ptr = quick::memory::UniquePtr<int>(new int(42));
///   *ptr = 10;
///   int value = *ptr;
///
///   auto custom = quick::memory::UniquePtr<int, CustomDeleter>(new int(5),
///   CustomDeleter{});
/// ```
/// @endcode
///
/// @todo Implement custom allocator
/// @todo Implement array type
template <typename T, typename Deleter = default_deleter<T>>
class UniquePtr {
 private:
  // Arrays are not supported in this implementation
  static_assert(
      !std::is_array_v<T>,
      "Array types not supported. Use UniquePtr<T[]> specialization.");

  T* m_ptr;
  [[no_unique_address]] Deleter m_deleter;  // EBO

 public:
  using Pointer_T = T*;
  using Element_T = T;
  using Deleter_T = Deleter;

  // ============================================================================
  // Constructors
  // ============================================================================

  /// @brief Default constructor - creates an empty UniquePtr
  constexpr UniquePtr() noexcept : m_ptr(nullptr), m_deleter() {}

  /// @brief Nullptr constructor - creates an empty UniquePtr
  constexpr UniquePtr(std::nullptr_t) noexcept : m_ptr(nullptr), m_deleter() {}

  /// @brief Takes ownership of the given Pointer_T
  /// @param ptr Raw Pointer_T to take ownership of (can be nullptr)
  explicit UniquePtr(Pointer_T ptr) noexcept : m_ptr(ptr), m_deleter() {}

  /// @brief Takes ownership of Pointer_T with a copy of the deleter
  /// @param ptr Raw Pointer_T to take ownership of
  /// @param del Deleter to copy
  UniquePtr(Pointer_T ptr, const Deleter& del) noexcept(
      std::is_nothrow_copy_constructible_v<Deleter>)
      : m_ptr(ptr), m_deleter(del) {}

  /// @brief Takes ownership of Pointer_T with a moved deleter
  /// @param ptr Raw Pointer_T to take ownership of
  /// @param del Deleter to move
  UniquePtr(Pointer_T ptr, Deleter&& del) noexcept(
      std::is_nothrow_move_constructible_v<Deleter>)
      : m_ptr(ptr), m_deleter(std::move(del)) {}

  /// @brief Move constructor - transfers ownership from other
  /// @param other The UniquePtr to move from (will be left empty)
  UniquePtr(UniquePtr&& other) noexcept
      : m_ptr(other.m_ptr), m_deleter(std::forward<Deleter>(other.m_deleter)) {
    other.m_ptr = nullptr;
  }

  /// @brief Converting move constructor - allows derived-to-base conversions
  /// @tparam U Type that is convertible to T
  /// @tparam D Deleter type that is convertible to Deleter
  template <typename U, typename D,
            typename = std::enable_if_t<
                std::is_convertible_v<typename UniquePtr<U, D>::Pointer_T,
                                      Pointer_T> &&
                !std::is_array_v<U>>>
  UniquePtr(UniquePtr<U, D>&& other) noexcept
      : m_ptr(other.release()),
        m_deleter(std::forward<D>(other.get_deleter())) {}

  /// @brief Copy constructor is deleted (unique ownership)
  UniquePtr(const UniquePtr&) = delete;

  /// @brief Destructor - deletes the managed object if owned
  ~UniquePtr() { reset(); }

  // ============================================================================
  // Assignment operators
  // ============================================================================

  /// @brief Move assignment - transfers ownership from other
  /// @param other The UniquePtr to move from
  /// @return Reference to this
  UniquePtr& operator=(UniquePtr&& other) noexcept {
    if (this != &other) {
      reset(other.release());
      m_deleter = std::forward<Deleter>(other.m_deleter);
    }
    return *this;
  }

  /// @brief Converting move assignment
  template <typename U, typename D,
            typename = std::enable_if_t<
                std::is_convertible_v<typename UniquePtr<U, D>::Pointer_T,
                                      Pointer_T> &&
                std::is_assignable_v<Deleter&, D&&>>>
  UniquePtr& operator=(UniquePtr<U, D>&& other) noexcept {
    reset(other.release());
    m_deleter = std::forward<D>(other.get_deleter());
    return *this;
  }

  /// @brief Assign nullptr - deletes the managed object
  UniquePtr& operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  /// @brief Copy assignment is deleted (unique ownership)
  UniquePtr& operator=(const UniquePtr&) = delete;

  // ============================================================================
  // Modifiers
  // ============================================================================

  /// @brief Release ownership of the managed object
  /// @return The raw Pointer_T (caller is now responsible for deletion)
  [[nodiscard]] Pointer_T release() noexcept {
    return std::exchange(m_ptr, nullptr);
  }

  /// @brief Replace the managed object
  /// @param ptr New Pointer_T to manage (can be nullptr)
  void reset(Pointer_T ptr = nullptr) noexcept {
    Pointer_T old = std::exchange(m_ptr, ptr);
    if (old) {
      m_deleter(old);
    }
  }

  /// @brief Swap with another UniquePtr
  /// @param other The UniquePtr to swap with
  void swap(UniquePtr& other) noexcept {
    std::swap(m_ptr, other.m_ptr);
    std::swap(m_deleter, other.m_deleter);
  }

  // ============================================================================
  // Observers
  // ============================================================================

  /// @brief  *Read attention below!* Get the raw Pointer_T without releasing
  /// ownership
  /// @attention Deleted because y'all don't know how to act
  /// @return The managed raw Pointer_T (may be nullptr)
  [[nodiscard]] Pointer_T get() const noexcept = delete;
  // {
  //     return m_ptr;
  // }

  /// @brief Get a reference to the deleter
  [[nodiscard]] Deleter& get_deleter() noexcept { return m_deleter; }

  /// @brief Get a const reference to the deleter
  [[nodiscard]] const Deleter& get_deleter() const noexcept {
    return m_deleter;
  }

  /// @brief Check if the UniquePtr owns an object
  /// @return true if managing an object, false otherwise
  explicit operator bool() const noexcept { return m_ptr != nullptr; }

  // ============================================================================
  // Dereference operators
  // ============================================================================

  /// @brief Dereference the managed Pointer_T
  /// @return Reference to the managed object
  /// @warning Undefined behavior if the Pointer_T is nullptr
  [[nodiscard]] typename std::add_lvalue_reference_t<T> operator*()
      const noexcept {
    return *m_ptr;
  }

  /// @brief Access members of the managed object
  /// @return The managed raw Pointer_T
  /// @warning Undefined behavior if the Pointer_T is nullptr
  [[nodiscard]] Pointer_T operator->() const noexcept { return m_ptr; }
};

// ================================================================================
// Non-member functions
// ================================================================================

/// @brief Swap two UniquePtrs
template <typename T, typename D>
void swap(UniquePtr<T, D>& lhs, UniquePtr<T, D>& rhs) noexcept {
  lhs.swap(rhs);
}

/// @brief Create a UniquePtr with the given arguments
/// @tparam T Type to create
/// @tparam Args Constructor argument types
/// @param args Arguments to forward to T's constructor
/// @return UniquePtr managing the newly created object
template <typename T, typename... Args>
[[nodiscard]] UniquePtr<T> make_unique(Args&&... args) {
  return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

// ================================================================================
// Comparison operators
// ================================================================================

template <typename T1, typename D1, typename T2, typename D2>
[[nodiscard]] bool operator==(const UniquePtr<T1, D1>& lhs,
                              const UniquePtr<T2, D2>& rhs) noexcept {
  return lhs.m_ptr == rhs.m_ptr;
}

template <typename T1, typename D1, typename T2, typename D2>
[[nodiscard]] bool operator!=(const UniquePtr<T1, D1>& lhs,
                              const UniquePtr<T2, D2>& rhs) noexcept {
  return lhs.m_ptr != rhs.m_ptr;
}

template <typename T1, typename D1, typename T2, typename D2>
[[nodiscard]] bool operator<(const UniquePtr<T1, D1>& lhs,
                             const UniquePtr<T2, D2>& rhs) noexcept {
  using CT = std::common_type_t<typename UniquePtr<T1, D1>::Pointer_T,
                                typename UniquePtr<T2, D2>::Pointer_T>;
  return std::less<CT>{}(lhs.m_ptr, rhs.m_ptr);
}

template <typename T1, typename D1, typename T2, typename D2>
[[nodiscard]] bool operator<=(const UniquePtr<T1, D1>& lhs,
                              const UniquePtr<T2, D2>& rhs) noexcept {
  return !(rhs < lhs);
}

template <typename T1, typename D1, typename T2, typename D2>
[[nodiscard]] bool operator>(const UniquePtr<T1, D1>& lhs,
                             const UniquePtr<T2, D2>& rhs) noexcept {
  return rhs < lhs;
}

template <typename T1, typename D1, typename T2, typename D2>
[[nodiscard]] bool operator>=(const UniquePtr<T1, D1>& lhs,
                              const UniquePtr<T2, D2>& rhs) noexcept {
  return !(lhs < rhs);
}

// Comparisons with nullptr
template <typename T, typename D>
[[nodiscard]] bool operator==(const UniquePtr<T, D>& ptr,
                              std::nullptr_t) noexcept {
  return !ptr;
}

template <typename T, typename D>
[[nodiscard]] bool operator==(std::nullptr_t,
                              const UniquePtr<T, D>& ptr) noexcept {
  return !ptr;
}

template <typename T, typename D>
[[nodiscard]] bool operator!=(const UniquePtr<T, D>& ptr,
                              std::nullptr_t) noexcept {
  return static_cast<bool>(ptr);
}

template <typename T, typename D>
[[nodiscard]] bool operator!=(std::nullptr_t,
                              const UniquePtr<T, D>& ptr) noexcept {
  return static_cast<bool>(ptr);
}

}  // namespace quick::memory