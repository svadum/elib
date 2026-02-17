/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

// NOTE: pragma once is used instead preprocessor guard
//       in order to avoid conflicts between different header versions
#pragma once

#include <array>

namespace elib
{
  namespace detail
  {
    template<typename pointer>
    constexpr void increment(pointer& it, pointer storage_begin, pointer storage_end)
    {
      if (++it == storage_end)
        it = storage_begin;
    }

    template<typename pointer, typename difference_type>
    constexpr pointer add(pointer ptr, difference_type n, pointer storage_begin, pointer storage_end)
    {
      static_assert(std::is_pointer_v<pointer>, "circular_buffer::add (internal): Pointer must be a pointer type");

      const auto availableAtEnd = storage_end - ptr;
      if (availableAtEnd > n)
        return ptr + n;

      const auto capacity = std::distance(storage_begin, storage_end);
      return storage_begin + ((n - availableAtEnd) % capacity);
    }

    template<typename pointer, typename difference_type>
    constexpr pointer sub(pointer ptr, difference_type n, pointer storage_begin, pointer storage_end)
    {
      static_assert(std::is_pointer_v<pointer>, "circular_buffer::sub (internal): Pointer must be a pointer type");

      const auto capacity = std::distance(storage_begin, storage_end);
      if (n > capacity)
        n = n % capacity;

      const auto availableAtFront = ptr - storage_begin;
      if (availableAtFront >= n)
        return ptr - n;

      return storage_end - (n - availableAtFront);
    }

    template<typename Container>
    struct nonconst_traits
    {
      using reference       = typename Container::reference;
      using pointer         = typename Container::pointer;
      using value_type      = typename Container::value_type;
      using difference_type = typename Container::difference_type;
      using size_type       = typename Container::size_type;

      typedef nonconst_traits<Container> nonconst_self;
    };

    template<typename Container>
    struct const_traits
    {
      using reference       = typename Container::const_reference;
      using pointer         = typename Container::const_pointer;
      using value_type      = typename Container::value_type;
      using difference_type = typename Container::difference_type;
      using size_type       = typename Container::size_type;

      using nonconst_self = nonconst_traits<Container>;
    };

    template<typename Buffer, typename Traits>
    class circular_buffer_iterator
    {
      using nonconst_self = typename Traits::nonconst_self;

    public:
      using iterator_category = std::forward_iterator_tag;
      using reference         = typename Traits::reference;
      using pointer           = typename Traits::pointer;
      using value_type        = typename Traits::value_type;
      using difference_type   = typename Traits::difference_type;
      using size_type         = typename Traits::size_type;

      constexpr circular_buffer_iterator() = default;
      circular_buffer_iterator(Buffer* buffer, const pointer it)
        : m_buffer{buffer}
        , m_it{it}
      {
      }

      constexpr circular_buffer_iterator(const circular_buffer_iterator& it)
        : m_buffer{it.m_buffer}
        , m_it{it.m_it}
      {
      }

      constexpr circular_buffer_iterator(const nonconst_self& it)
        : m_buffer{it.m_buffer}
        , m_it{it.m_it}
      {
      }

      constexpr circular_buffer_iterator& operator=(const circular_buffer_iterator& it)
      {
        if (this == &it)
          return *this;

        m_buffer = it.m_buffer;
        m_it     = it.m_it;

        return *this;
      }

      constexpr reference operator*() const { return *m_it; }
      constexpr pointer operator->() const { return &(operator*()); };

      constexpr circular_buffer_iterator& operator++()
      {
        increment(m_it, m_buffer->storage_begin(), m_buffer->storage_end());
        if (m_it == m_buffer->m_last)
          m_it = nullptr;

        return *this;
      }

      constexpr circular_buffer_iterator operator++(int)
      {
        circular_buffer_iterator<Buffer, Traits> tmp = *this;
        ++*this;

        return tmp;
      }

      template<class Traits0>
      bool operator==(const circular_buffer_iterator<Buffer, Traits0>& it) const
      {
        return m_it == it.m_it;
      }

      template<class Traits0>
      bool operator!=(const circular_buffer_iterator<Buffer, Traits0>& it) const
      {
        return m_it != it.m_it;
      }

    private:
      Buffer* m_buffer{nullptr};
      pointer m_it{nullptr};
    };
  }

  template<typename Value, std::size_t Capacity>
  class circular_buffer
  {
    static_assert(Capacity > std::size_t{0}, "Capacity must be greater than zero");

    using storage = std::array<Value, Capacity>;

  public:
    using iterator       = detail::circular_buffer_iterator<circular_buffer, detail::nonconst_traits<circular_buffer>>;
    using const_iterator = detail::circular_buffer_iterator<const circular_buffer, detail::const_traits<circular_buffer>>;

    using reference       = typename storage::reference;
    using const_reference = typename storage::const_reference;
    using pointer         = typename storage::pointer;
    using const_pointer   = typename storage::const_pointer;
    using value_type      = typename storage::value_type;
    using difference_type = typename storage::difference_type;
    using size_type       = typename storage::size_type;

    constexpr circular_buffer() = default;

    circular_buffer(const circular_buffer& other) noexcept
    {
      *this = other;
    }

    circular_buffer& operator=(const circular_buffer& other) noexcept
    {
      if (this == &other)
        return *this;

      // TODO: copy only initialized values
      std::copy(other.m_data.begin(), other.m_data.end(), storage_begin());
      m_size = other.m_size;

      m_first = storage_begin() + (other.m_first - other.storage_begin());
      m_last  = storage_begin() + (other.m_last - other.storage_begin());

      return *this;
    }

    circular_buffer(circular_buffer&& other) noexcept
    {
      *this = std::move(other);
    }

    circular_buffer& operator=(circular_buffer&& other) noexcept
    {
      if (this == &other)
        return *this;

      clear();

      std::swap(m_data, other.m_data);
      std::swap(m_size, other.m_size);

      m_first = storage_begin() + (other.m_first - other.storage_begin());
      m_last  = storage_begin() + (other.m_last - other.storage_begin());

      return *this;
    }

    constexpr circular_buffer(const std::initializer_list<value_type> il)
      : circular_buffer()
    {
      if (il.size() > Capacity)
        return;

      auto start = storage_begin();
      for (auto&& value : il)
      {
        *start = std::move(value);
        ++start;
      }

      m_size = il.size();
      m_last = detail::add(m_last, m_size, storage_begin(), storage_end());
    }

    template<typename T, std::size_t N>
    constexpr circular_buffer(const T (&array)[N])
      : circular_buffer()
    {
      static_assert(N <= Capacity, "Array size more than container capacity");

      m_size = N;
      m_last = detail::add(m_last, N, storage_begin(), storage_end());
      std::copy(array, array + N, storage_begin());
    }

    circular_buffer(value_type* data, std::size_t size)
      : circular_buffer()
    {
      if (!data || !size || size > Capacity)
        return;

      m_size = size;
      m_last = detail::add(m_last, size, storage_begin(), storage_end());
      std::copy(data, data + size, storage_begin());
    }

    constexpr iterator begin()
    {
      return iterator(this, empty() ? nullptr : m_first);
    }

    constexpr iterator end()
    {
      return iterator(this, nullptr);
    }

    constexpr const_iterator begin() const
    {
      return const_iterator(this, empty() ? nullptr : m_first);
    }

    constexpr const_iterator end() const
    {
      return const_iterator(this, nullptr);
    }

    constexpr const_iterator cbegin() const
    {
      return begin();
    }

    constexpr const_iterator cend() const
    {
      return end();
    }

    constexpr reference front()
    {
      return *m_first;
    }

    constexpr const_reference front() const
    {
      return *m_first;
    }

    constexpr reference back()
    {
      return *((m_last == storage_begin() ? storage_end() : m_last) - 1);
    }

    constexpr const_reference back() const
    {
      return *((m_last == storage_begin() ? storage_end() : m_last) - 1);
    }

    constexpr size_type size() const
    {
      return m_size;
    }

    constexpr size_type capacity() const
    {
      return Capacity;
    }

    constexpr bool empty() const
    {
      return !size();
    }

    constexpr bool full() const
    {
      return size() == Capacity;
    }

    template<typename T>
    bool push(T&& value)
    {
      if (full())
        return false;

      *m_last = std::forward<T>(value);
      detail::increment(m_last, storage_begin(), storage_end());
      ++m_size;

      return true;
    }

    template<typename T>
    void push_over(T&& value)
    {
      if (full())
        pop();

      push(value);
    }

    bool pop()
    {
      if (empty())
        return false;

      detail::increment(m_first, storage_begin(), storage_end());
      --m_size;

      return true;
    }

    void clear()
    {
      m_size  = 0;
      m_first = m_last = storage_begin();
    }

  private:
    storage m_data{};

    pointer m_first{m_data.data()};
    pointer m_last{m_data.data()};

    size_type m_size{0};

    friend iterator;
    friend const_iterator;

    constexpr pointer storage_begin()
    {
      return m_data.data();
    }

    constexpr const_pointer storage_begin() const
    {
      return m_data.data();
    }

    constexpr pointer storage_end()
    {
      return m_data.data() + m_data.size();
    }

    constexpr const_pointer storage_end() const
    {
      return m_data.data() + m_data.size();
    }
  };
}
