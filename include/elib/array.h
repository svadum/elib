/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

/**
 * @file array.h
 * @brief A static capacity vector-like container for bare-metal environments.
 *
 * Provides a dynamically sized array backed by fixed-size storage, preventing
 * heap allocations while offering standard container semantics.
 */

#pragma once

#include <cstddef>
#include <array>
#include <iterator>
#include <algorithm>
#include <cassert>

namespace elib
{
  /**
   * @brief A contiguous container with a fixed capacity but variable size.
   * @tparam Value The type of elements stored.
   * @tparam Capacity The maximum number of elements the array can hold[cite: 6].
   */
  template<typename Value, std::size_t Capacity>
  class array
  {
    static_assert(Capacity > std::size_t{0}, "Capacity must be greater than zero");

    using storage = std::array<Value, Capacity>;

  public:
    using iterator               = typename storage::iterator;
    using const_iterator         = typename storage::const_iterator;
    using reverse_iterator       = typename storage::reverse_iterator;
    using const_reverse_iterator = typename storage::const_reverse_iterator;

    using reference       = typename storage::reference;
    using const_reference = typename storage::const_reference;
    using pointer         = typename storage::pointer;
    using const_pointer   = typename storage::const_pointer;
    using value_type      = typename storage::value_type;
    using difference_type = typename storage::difference_type;
    using size_type       = typename storage::size_type;

    /** @brief Default constructor. Creates an empty array[cite: 6]. */
    constexpr array()
      : data_{}
      , end_{data_.begin()}
    {
    }

    /** @brief Copy constructor. */
    constexpr array(const array& other) noexcept
    {
      *this = other;
    }

    /** @brief Copy assignment operator. */
    constexpr array& operator=(const array& other) noexcept
    {
      if (this == &other)
        return *this;

      data_ = other.data_;
      end_  = other.end_;

      return *this;
    }

    /** @brief Move constructor. */
    constexpr array(array&& other) noexcept
    {
      *this = std::move(other);
    }

    /** @brief Move assignment operator. */
    constexpr array& operator=(array&& other) noexcept
    {
      if (this == &other)
        return *this;

      clear();

      std::swap(data_, other.data_);
      std::swap(end_, other.end_);

      return *this;
    }

    /**
     * @brief Constructs an array from an initializer list.
     * @note Elements exceeding capacity are silently ignored[cite: 6].
     */
    constexpr array(const std::initializer_list<value_type> il)
      : data_{}
    {
      if (il.size() > Capacity)
        return;

      auto it = begin();
      for (auto&& value : il)
      {
        *it = std::move(value);
        ++it;
      }

      end_ = it;
    }

    /**
     * @brief Constructs an array from a C-style array.
     * @pre The size of the C-array must not exceed Capacity[cite: 6].
     */
    template<typename T, std::size_t N>
    constexpr array(const T (&array)[N])
      : data_{}
    {
      static_assert(N <= Capacity, "Array size more than container capacity");

      end_ = data_.begin() + N;
      std::copy(array, array + N, data_.begin());
    }

    /**
     * @brief Constructs an array from an iterator range.
     * @note Copies at most Capacity elements[cite: 6].
     */
    template<typename InputIterator>
    constexpr array(InputIterator first, InputIterator last)
      : data_{}
    {
      const auto count = std::min(std::distance(first, last),
                                  static_cast<typename std::iterator_traits<InputIterator>::difference_type>(Capacity));

      std::copy_n(first, count, data_.begin());

      end_ = data_.begin() + count;
    }

    // --- Iterators ---
    constexpr iterator begin() noexcept { return data_.begin(); }
    constexpr iterator end() noexcept { return end_; }
    constexpr const_iterator begin() const noexcept { return data_.begin(); }
    constexpr const_iterator end() const noexcept { return end_; }
    constexpr const_iterator cbegin() const noexcept { return data_.cbegin(); }
    constexpr const_iterator cend() const noexcept { return end_; }
    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end_); }
    constexpr reverse_iterator rend() noexcept { return data_.rend(); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end_); }
    constexpr const_reverse_iterator rend() const noexcept { return data_.rend(); }
    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end_); }
    constexpr const_reverse_iterator crend() const noexcept { return data_.crend(); }

    // --- Element Access ---
    constexpr reference front() noexcept { return data_.front(); }
    constexpr const_reference front() const noexcept { return data_.front(); }
    constexpr reference back() noexcept { return *(end() - 1); }
    constexpr const_reference back() const noexcept { return *(end() - 1); }
    constexpr pointer data() noexcept { return data_.data(); }
    constexpr const_pointer data() const noexcept { return data_.data(); }

    // --- Capacity ---

    /** @return The number of elements currently stored. */
    constexpr size_type size() const noexcept { return end_ - data_.begin(); }

    /** @return The maximum number of elements the array can store. */
    constexpr size_type capacity() const noexcept { return Capacity; }

    /** @return True if the array contains no elements. */
    constexpr bool empty() const noexcept { return !size(); }

    /** @return True if the array has reached its capacity. */
    constexpr bool full() const noexcept { return size() == Capacity; }

    // --- Modifiers ---

    /**
     * @brief Resizes the array to contain exactly count elements.
     *
     * If the current size is greater than count, the container is reduced to its first count elements.
     * If the current size is less than count, additional zero-initialized elements are appended.
     *
     * @param count New size of the array.
     * @return True if the resize was successful, false if count exceeds capacity.
     */
    constexpr bool resize(size_type count) noexcept
    {
      if (count > Capacity)
        return false;

      auto new_end = data_.begin() + count;

      // Zero-fill new slots if expanding
      if (count > size())
      {
        std::fill(end_, new_end, value_type{});
      }

      end_ = new_end;
      return true;
    }

    /**
     * @brief Inserts an element at the specified position.
     * @return Iterator to the inserted element, or end() on failure[cite: 6].
     */
    constexpr iterator insert(iterator position, Value&& value) { return insert_item(position, value); }
    constexpr iterator insert(iterator position, const Value& value) { return insert_item(position, value); }

    template<class InputIt>
    constexpr iterator insert(iterator pos, InputIt first, InputIt last)
    {
      if (!is_from_this(pos))
        return end();

      const auto count             = std::distance(first, last);
      const auto availableCapacity = std::distance(end_, data_.end());

      if (count > availableCapacity)
        return end();

      if (std::distance(pos, end_) > 0)
      {
        auto src  = pos;
        auto dest = std::next(pos, count);

        while (src != end_)
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

      std::advance(end_, count);
      return pos;
    }

    template<typename InitValue>
    constexpr iterator insert(iterator pos, std::initializer_list<InitValue> ilist)
    {
      return insert(pos, ilist.begin(), ilist.end());
    }

    /**
     * @brief Erases the element at the specified position.
     * @return Iterator following the removed element[cite: 6].
     */
    constexpr iterator erase(const_iterator position)
    {
      if (!is_from_this(position) || empty())
        return end();

      if (position == cend())
        return end();

      iterator mutable_pos = begin() + (position - cbegin());
      iterator src         = mutable_pos + 1;
      iterator dest        = mutable_pos;

      while (src != end_)
      {
        *dest = std::move(*src);
        ++src;
        ++dest;
      }

      --end_;
      return !empty() ? mutable_pos + 1 : end_;
    }

    /**
     * @brief Appends an element to the end of the array.
     * @return True if inserted, false if the array is already full[cite: 6].
     */
    template<typename T>
    constexpr bool push_back(T&& value)
    {
      if (full())
        return false;

      *end_ = std::forward<T>(value);
      ++end_;

      return true;
    }

    /**
     * @brief Removes the last element from the array.
     * @return True if removed, false if empty[cite: 6].
     */
    constexpr bool pop_back() noexcept
    {
      if (empty())
        return false;

      --end_;
      return true;
    }

    /** @brief Clears all elements from the array. */
    constexpr void clear() noexcept { end_ = data_.begin(); }

    constexpr reference operator[](size_type pos) noexcept { return data_[pos]; }
    constexpr const_reference operator[](size_type pos) const noexcept { return data_[pos]; }

    constexpr reference at(size_type pos)
    {
      assert(pos < size());
      return data_.at(pos);
    }

    constexpr const_reference at(size_type pos) const
    {
      assert(pos < size());
      return data_.at(pos);
    }

  private:
    storage data_;
    iterator end_{data_.begin()};

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

      if (position == end_)
      {
        *end_ = std::forward<ValueType>(value);
        ++end_;
        return position;
      }

      auto src  = end_;
      auto dest = end_;

      while (src != position)
      {
        --src;
        *dest = std::move(*src);
        --dest;
      }

      *position = std::forward<ValueType>(value);
      ++end_;

      return src;
    }
  };
}