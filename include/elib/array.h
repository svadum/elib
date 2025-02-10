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
#include <array>
#include <iterator>
#include <algorithm>

namespace elib
{

  template<typename Value, std::size_t Capacity>
  class array
  {
    static_assert(Capacity > std::size_t{0}, "Capacity must be greater than zero");

    using Data = std::array<Value, Capacity>;

  public:
    using iterator               = typename Data::iterator;
    using const_iterator         = typename Data::const_iterator;
    using reverse_iterator       = typename Data::reverse_iterator;
    using const_reverse_iterator = typename Data::const_reverse_iterator;

    using reference       = typename Data::reference;
    using const_reference = typename Data::const_reference;
    using pointer         = typename Data::pointer;
    using const_pointer   = typename Data::const_pointer;
    using value_type      = typename Data::value_type;
    using difference_type = typename Data::difference_type;
    using size_type       = typename Data::size_type;

    constexpr array()
      : m_data{}
      , m_end{m_data.begin()}
    {
    }

    constexpr array(const array& other) noexcept
    {
      *this = other;
    }

    constexpr array& operator=(const array& other) noexcept
    {
      if (this == &other)
        return *this;

      m_data = other.m_data;
      m_end  = other.m_end;

      return *this;
    }

    constexpr array(array&& other) noexcept
    {
      *this = std::move(other);
    }

    constexpr array& operator=(array&& other) noexcept
    {
      if (this == &other)
        return *this;

      clear();

      std::swap(m_data, other.m_data);
      std::swap(m_end, other.m_end);

      return *this;
    }

    constexpr array(const std::initializer_list<value_type> il)
      : m_data{}
    {
      if (il.size() > Capacity)
        return;

      auto it = begin();
      for (auto&& value : il)
      {
        *it = std::move(value);
        ++it;
      }

      m_end = it;
    }

    template<typename T, std::size_t N>
    constexpr array(const T (&array)[N])
      : m_data{}
    {
      static_assert(N <= Capacity, "Array size more than container capacity");

      m_end = m_data.begin() + N;
      std::copy(array, array + N, m_data.begin());
    }

    template<typename InputIterator>
    constexpr array(InputIterator first, InputIterator last)
      : m_data{}
    {
      const auto count = std::min(std::distance(first, last),
                                  static_cast<typename std::iterator_traits<InputIterator>::difference_type>(Capacity));

      std::copy_n(first, count, m_data.begin());

      m_end = m_data.begin() + count;
    }

    constexpr iterator begin() noexcept
    {
      return m_data.begin();
    }

    constexpr iterator end() noexcept
    {
      return m_end;
    }

    constexpr const_iterator begin() const noexcept
    {
      return m_data.begin();
    }

    constexpr const_iterator end() const noexcept
    {
      return m_end;
    }

    constexpr const_iterator cbegin() const noexcept
    {
      return m_data.cbegin();
    }

    constexpr const_iterator cend() const noexcept
    {
      return m_end;
    }

    constexpr reverse_iterator rbegin() noexcept
    {
      return reverse_iterator(m_end);
    }

    constexpr reverse_iterator rend() noexcept
    {
      return m_data.rend();
    }

    constexpr const_reverse_iterator rbegin() const noexcept
    {
      return const_reverse_iterator(m_end);
    }

    constexpr const_reverse_iterator rend() const noexcept
    {
      return m_data.rend();
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
      return const_reverse_iterator(m_end);
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
      return m_data.crend();
    }

    constexpr reference front() noexcept
    {
      return m_data.front();
    }

    constexpr const_reference front() const noexcept
    {
      return m_data.front();
    }

    constexpr reference back() noexcept
    {
      return *(end() - 1);
    }

    constexpr const_reference back() const noexcept
    {
      return *(end() - 1);
    }

    constexpr pointer data() noexcept
    {
      return m_data.data();
    }

    constexpr const_pointer data() const noexcept
    {
      return m_data.data();
    }

    constexpr size_type size() const noexcept
    {
      return m_end - m_data.begin();
    }

    constexpr size_type capacity() const noexcept
    {
      return Capacity;
    }

    constexpr bool empty() const noexcept
    {
      return !size();
    }

    constexpr bool full() const noexcept
    {
      return size() == Capacity;
    }

    constexpr iterator insert(iterator position, Value&& value)
    {
      return insert_item(position, value);
    }

    constexpr iterator insert(iterator position, const Value& value)
    {
      return insert_item(position, value);
    }

    template<class InputIt>
    constexpr iterator insert(iterator pos, InputIt first, InputIt last)
    {
      if (!is_from_this(pos))
        return end();

      const auto count             = std::distance(first, last);
      const auto availableCapacity = std::distance(m_end, m_data.end());

      if (count > availableCapacity)
        return end();

      if (std::distance(pos, m_end) > 0)
      {
        auto src  = pos;
        auto dest = std::next(pos, count);

        // move stored element
        while (src != m_end)
        {
          *dest = std::move(*src);

          ++src;
          ++dest;
        }
      }

      auto src = pos;
      while (first != last)
      {
        *src = *first;

        ++src;
        ++first;
      }

      std::advance(m_end, count);

      return pos;
    }

    template<typename InitValue>
    constexpr iterator insert(iterator pos, std::initializer_list<InitValue> ilist)
    {
      return insert(pos, ilist.begin(), ilist.end());
    }

    constexpr iterator erase(const_iterator position)
    {
      if (!is_from_this(position) || empty())
        return end();

      if (position == cend())
        return end();

      iterator mutable_pos = begin() + (position - cbegin());
      iterator src         = mutable_pos + 1;
      iterator dest        = mutable_pos;

      while (src != m_end)
      {
        *dest = std::move(*src);
        ++src;
        ++dest;
      }

      --m_end;

      return !empty() ? mutable_pos + 1 : m_end;
    }

    template<typename T>
    constexpr bool push_back(T&& value)
    {
      if (full())
        return false;

      *m_end = std::forward<T>(value);
      ++m_end;

      return true;
    }

    constexpr bool pop_back() noexcept
    {
      if (empty())
        return false;

      --m_end;

      return true;
    }

    constexpr void clear() noexcept
    {
      m_end = m_data.begin();
    }

    constexpr reference operator[](size_type pos) noexcept
    {
      return m_data[pos];
    }

    constexpr const_reference operator[](size_type pos) const noexcept
    {
      return m_data[pos];
    }

    constexpr reference at(size_type pos)
    {
      assert(pos < size());

      return m_data.at(pos);
    }

    constexpr const_reference at(size_type pos) const
    {
      assert(pos < size());

      return m_data.at(pos);
    }

  private:
    Data m_data;
    iterator m_end{m_data.begin()};

    template<typename Iterator>
    constexpr bool is_from_this(const Iterator& it)
    {
      return it >= begin() && it <= end();
    }

    template<typename ValueType>
    iterator insert_item(iterator position, ValueType&& value)
    {
      if (!is_from_this(position) || full())
        return end();

      // insert at end
      if (position == m_end)
      {
        *m_end = std::forward<Value>(value);
        ++m_end;

        return position;
      }

      auto src  = m_end;
      auto dest = m_end;

      while (src != position)
      {
        --src;
        *dest = std::move(*src);
        --dest;
      }

      *position = std::forward<Value>(value);
      ++m_end;

      return src;
    }
  };
}
