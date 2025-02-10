/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

// NOTE: pragma once is used instead preprocessor guard
//       in order to avoid conflicts between different header versions
#pragma once

#include <cstddef>
#include <utility>
#include <new>

namespace elib::memory
{
  template<typename T>
  class aligned_storage
  {
  public:
    aligned_storage() = default;

    aligned_storage(const aligned_storage&) = delete;
    aligned_storage& operator=(const aligned_storage&) = delete;

    aligned_storage(aligned_storage&&) = delete;
    aligned_storage& operator=(aligned_storage&&) = delete;

    ~aligned_storage()
    {
      destroy();
    }

    template<typename... Args>
    T& construct(Args&&... args)
    {
      if (constructed)
        destroy();

      constructed = true;
      new (&storage) T{std::forward<Args>(args)...};

      return get();
    }

    void destroy()
    {
      if (constructed)
      {
        constructed = false;
        get().~T();
      }
    }

    bool is_constructed() const noexcept
    {
      return constructed;
    }

    T& get() noexcept
    {
      return *std::launder(reinterpret_cast<T*>(&storage));
    }

    const T& get() const noexcept
    {
      return *std::launder(reinterpret_cast<const T*>(&storage));
    }

  private:
    bool constructed{false};
    alignas(T) std::byte storage[sizeof(T)]{};
  };
}