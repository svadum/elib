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

    constexpr array()
      : data_{}
      , end_{data_.begin()}
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

      data_ = other.data_;
      end_  = other.end_;

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

      std::swap(data_, other.data_);
      std::swap(end_, other.end_);

      return *this;
    }

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

    template<typename T, std::size_t N>
    constexpr array(const T (&array)[N])
      : data_{}
    {
      static_assert(N <= Capacity, "Array size more than container capacity");

      end_ = data_.begin() + N;
      std::copy(array, array + N, data_.begin());
    }

    template<typename InputIterator>
    constexpr array(InputIterator first, InputIterator last)
      : data_{}
    {
      const auto count = std::min(std::distance(first, last),
                                  static_cast<typename std::iterator_traits<InputIterator>::difference_type>(Capacity));

      std::copy_n(first, count, data_.begin());

      end_ = data_.begin() + count;
    }

    constexpr iterator begin() noexcept
    {
      return data_.begin();
    }

    constexpr iterator end() noexcept
    {
      return end_;
    }

    constexpr const_iterator begin() const noexcept
    {
      return data_.begin();
    }

    constexpr const_iterator end() const noexcept
    {
      return end_;
    }

    constexpr const_iterator cbegin() const noexcept
    {
      return data_.cbegin();
    }

    constexpr const_iterator cend() const noexcept
    {
      return end_;
    }

    constexpr reverse_iterator rbegin() noexcept
    {
      return reverse_iterator(end_);
    }

    constexpr reverse_iterator rend() noexcept
    {
      return data_.rend();
    }

    constexpr const_reverse_iterator rbegin() const noexcept
    {
      return const_reverse_iterator(end_);
    }

    constexpr const_reverse_iterator rend() const noexcept
    {
      return data_.rend();
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
      return const_reverse_iterator(end_);
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
      return data_.crend();
    }

    constexpr reference front() noexcept
    {
      return data_.front();
    }

    constexpr const_reference front() const noexcept
    {
      return data_.front();
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
      return data_.data();
    }

    constexpr const_pointer data() const noexcept
    {
      return data_.data();
    }

    constexpr size_type size() const noexcept
    {
      return end_ - data_.begin();
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
      const auto availableCapacity = std::distance(end_, data_.end());

      if (count > availableCapacity)
        return end();

      if (std::distance(pos, end_) > 0)
      {
        auto src  = pos;
        auto dest = std::next(pos, count);

        // move stored element
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

    template<typename T>
    constexpr bool push_back(T&& value)
    {
      if (full())
        return false;

      *end_ = std::forward<T>(value);
      ++end_;

      return true;
    }

    constexpr bool pop_back() noexcept
    {
      if (empty())
        return false;

      --end_;

      return true;
    }

    constexpr void clear() noexcept
    {
      end_ = data_.begin();
    }

    constexpr reference operator[](size_type pos) noexcept
    {
      return data_[pos];
    }

    constexpr const_reference operator[](size_type pos) const noexcept
    {
      return data_[pos];
    }

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

      // insert at end
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
