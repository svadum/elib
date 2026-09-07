/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <array>
#include <algorithm>
#include <type_traits>

namespace elib
{
  namespace detail
  {
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
      template<typename UBuffer, typename UTraits>
      friend class circular_buffer_iterator;

    public:
      using iterator_category = std::random_access_iterator_tag;
      using reference         = typename Traits::reference;
      using pointer           = typename Traits::pointer;
      using value_type        = typename Traits::value_type;
      using difference_type   = typename Traits::difference_type;
      using size_type         = typename Traits::size_type;

      constexpr circular_buffer_iterator() = default;

      constexpr circular_buffer_iterator(Buffer* buffer, size_type idx)
        : buffer_{buffer}
        , idx_{idx}
      {
      }

      constexpr circular_buffer_iterator(const circular_buffer_iterator& it) = default;

      template<typename UBuffer, typename UTraits,
               typename = std::enable_if_t<std::is_convertible_v<UBuffer*, Buffer*>>>
      constexpr circular_buffer_iterator(const circular_buffer_iterator<UBuffer, UTraits>& it)
        : buffer_{it.buffer_}
        , idx_{it.idx_}
      {
      }

      constexpr circular_buffer_iterator& operator=(const circular_buffer_iterator& it) = default;

      constexpr reference operator*() const
      {
        return buffer_->data_[(buffer_->first_idx_ + idx_) % buffer_->capacity()];
      }

      constexpr pointer operator->() const { return &(operator*()); }

      constexpr circular_buffer_iterator& operator++()
      {
        if (idx_ != buffer_->size()) ++idx_;
        return *this;
      }

      constexpr circular_buffer_iterator operator++(int)
      {
        auto tmp = *this;
        ++(*this);
        return tmp;
      }

      constexpr circular_buffer_iterator& operator--()
      {
        if (idx_ != 0) --idx_;
        return *this;
      }

      constexpr circular_buffer_iterator operator--(int)
      {
        auto tmp = *this;
        --(*this);
        return tmp;
      }

      // --- Random Access Arithmetic Operators ---
      constexpr circular_buffer_iterator& operator+=(difference_type n)
      {
        if (n >= 0) idx_ += n;
        else idx_ -= (-n);
        return *this;
      }

      constexpr circular_buffer_iterator& operator-=(difference_type n)
      {
        if (n >= 0) idx_ -= n;
        else idx_ += (-n);
        return *this;
      }

      constexpr circular_buffer_iterator operator+(difference_type n) const
      {
        auto tmp = *this;
        return tmp += n;
      }

      friend constexpr circular_buffer_iterator operator+(difference_type n, const circular_buffer_iterator& it)
      {
        return it + n;
      }

      constexpr circular_buffer_iterator operator-(difference_type n) const
      {
        auto tmp = *this;
        return tmp -= n;
      }

      // --- Distance Operator ---
      template<typename UBuffer, typename UTraits>
      constexpr difference_type operator-(const circular_buffer_iterator<UBuffer, UTraits>& other) const
      {
        return static_cast<difference_type>(idx_) - static_cast<difference_type>(other.idx_);
      }

      // --- Offset Dereference Operator ---
      constexpr reference operator[](difference_type n) const
      {
        return *(*this + n);
      }

      // --- Relational and Equality Operators ---
      template<typename UBuffer, typename UTraits>
      constexpr bool operator==(const circular_buffer_iterator<UBuffer, UTraits>& it) const
      {
        return buffer_ == it.buffer_ && idx_ == it.idx_;
      }

      template<typename UBuffer, typename UTraits>
      constexpr bool operator!=(const circular_buffer_iterator<UBuffer, UTraits>& it) const
      {
        return !(*this == it);
      }

      template<typename UBuffer, typename UTraits>
      constexpr bool operator<(const circular_buffer_iterator<UBuffer, UTraits>& it) const
      {
        return idx_ < it.idx_;
      }

      template<typename UBuffer, typename UTraits>
      constexpr bool operator>(const circular_buffer_iterator<UBuffer, UTraits>& it) const
      {
        return idx_ > it.idx_;
      }

      template<typename UBuffer, typename UTraits>
      constexpr bool operator<=(const circular_buffer_iterator<UBuffer, UTraits>& it) const
      {
        return idx_ <= it.idx_;
      }

      template<typename UBuffer, typename UTraits>
      constexpr bool operator>=(const circular_buffer_iterator<UBuffer, UTraits>& it) const
      {
        return idx_ >= it.idx_;
      }

    private:
      Buffer* buffer_{nullptr};
      size_type idx_{0};
    };
  }

  /**
   * @brief A fixed-capacity circular buffer container.
   *
   * @tparam Value The type of elements stored in the buffer.
   * @tparam Capacity The maximum number of elements the buffer can hold.
   */
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

    /**
     * @brief Default constructor. Initializes an empty buffer.
     */
    constexpr circular_buffer() = default;

    /**
     * @brief Copy constructor.
     */
    constexpr circular_buffer(const circular_buffer& other) noexcept = default;

    /**
     * @brief Copy assignment operator.
     */
    constexpr circular_buffer& operator=(const circular_buffer& other) noexcept = default;

    /**
     * @brief Move constructor.
     */
    constexpr circular_buffer(circular_buffer&& other) noexcept
      : data_{std::move(other.data_)}
      , first_idx_{other.first_idx_}
      , size_{other.size_}
    {
      other.clear();
    }

    /**
     * @brief Move assignment operator.
     */
    constexpr circular_buffer& operator=(circular_buffer&& other) noexcept
    {
      if (this == &other)
        return *this;

      data_ = std::move(other.data_);
      first_idx_ = other.first_idx_;
      size_ = other.size_;
      other.clear();

      return *this;
    }

    /**
     * @brief Constructs the buffer with an initializer list.
     */
    constexpr circular_buffer(const std::initializer_list<value_type> il)
      : data_{}
      , first_idx_{0}
      , size_{0}
    {
      if (il.size() > Capacity)
        return;

      auto it = data_.begin();
      for (auto&& value : il)
      {
        *it = std::move(value);
        ++it;
      }
      size_ = il.size();
    }

    /**
     * @brief Constructs the buffer from a C-style array.
     */
    template<typename T, std::size_t N>
    constexpr circular_buffer(const T (&array)[N])
      : data_{}
      , first_idx_{0}
      , size_{0}
    {
      static_assert(N <= Capacity, "Array size more than container capacity");

      std::copy(array, array + N, data_.begin());
      size_ = N;
    }

    /**
     * @brief Constructs the buffer from a pointer and size.
     */
    constexpr circular_buffer(value_type* data, std::size_t size)
      : data_{}
      , first_idx_{0}
      , size_{0}
    {
      if (!data || !size || size > Capacity)
        return;

      std::copy(data, data + size, data_.begin());
      size_ = size;
    }

    /**
     * @brief Returns an iterator to the beginning.
     */
    constexpr iterator begin() { return iterator(this, 0); }

    /**
     * @brief Returns an iterator to the end.
     */
    constexpr iterator end() { return iterator(this, size_); }

    /**
     * @brief Returns a const iterator to the beginning.
     */
    constexpr const_iterator begin() const { return const_iterator(this, 0); }

    /**
     * @brief Returns a const iterator to the end.
     */
    constexpr const_iterator end() const { return const_iterator(this, size_); }

    /**
     * @brief Returns a const iterator to the beginning.
     */
    constexpr const_iterator cbegin() const { return begin(); }

    /**
     * @brief Returns a const iterator to the end.
     */
    constexpr const_iterator cend() const { return end(); }

    /**
     * @brief Access the first element.
     */
    constexpr reference front() { return data_[first_idx_]; }

    /**
     * @brief Access the first element (const).
     */
    constexpr const_reference front() const { return data_[first_idx_]; }

    /**
     * @brief Access the last element.
     */
    constexpr reference back() {
      return data_[size_ > 0 ? (first_idx_ + size_ - 1) % Capacity : first_idx_];
    }

    /**
     * @brief Access the last element (const).
     */
    constexpr const_reference back() const {
      return data_[size_ > 0 ? (first_idx_ + size_ - 1) % Capacity : first_idx_];
    }

    /**
     * @brief Returns the number of elements in the buffer.
     */
    constexpr size_type size() const { return size_; }

    /**
     * @brief Returns the capacity of the buffer.
     */
    constexpr size_type capacity() const { return Capacity; }

    /**
     * @brief Checks if the buffer is empty.
     */
    constexpr bool empty() const { return size_ == 0; }

    /**
     * @brief Checks if the buffer is full.
     */
    constexpr bool full() const { return size_ == Capacity; }

    /**
     * @brief Adds an element to the end of the buffer.
     * @deprecated Use push_back() instead.
     */
    template<typename T> [[deprecated("use push_back() instead")]]
    constexpr bool push(T&& value)
    {
      return push_back(std::forward<T>(value));
    }

    /**
     * @brief Adds an element to the end of the buffer.
     * @return true if successful, false if buffer is full.
     */
    template<typename T>
    constexpr bool push_back(T&& value)
    {
      if (full())
        return false;

      data_[(first_idx_ + size_) % Capacity] = std::forward<T>(value);
      ++size_;

      return true;
    }

    /**
     * @brief Removes the last element.
     * @return true if successful, false if buffer is empty.
     */
    constexpr bool pop_back()
    {
      if (empty())
        return false;

      --size_;
      return true;
    }

    /**
     * @brief Adds an element to the end, overwriting the oldest if full.
     */
    template<typename T>
    constexpr void push_over(T&& value)
    {
      if (full())
        pop_front();

      push_back(std::forward<T>(value));
    }

    /**
     * @brief Adds an element to the front of the buffer.
     * @return true if successful, false if buffer is full.
     */
    template<typename T>
    constexpr bool push_front(T&& value)
    {
      if (full())
        return false;

      first_idx_ = (first_idx_ > 0) ? first_idx_ - 1 : Capacity - 1;
      data_[first_idx_] = std::forward<T>(value);
      ++size_;

      return true;
    }

    /**
     * @brief Removes the first element.
     * @return true if successful, false if buffer is empty.
     */
    constexpr bool pop_front()
    {
      if (empty())
        return false;

      first_idx_ = (first_idx_ + 1) % Capacity;
      --size_;

      return true;
    }

    /**
     * @brief Removes the first element.
     * @deprecated Use pop_front() instead.
     */
    [[deprecated("use pop_front() instead")]]
    constexpr bool pop()
    {
      return pop_front();
    }

    /**
     * @brief Inserts an element at the specified position.
     * @return Iterator pointing to the inserted value, or end() if the buffer is full.
     */
    template<typename T>
    constexpr iterator insert(const_iterator pos, T&& value)
    {
      if (full())
        return end();

      const difference_type index = std::distance(cbegin(), pos);
      const auto elements_after = static_cast<difference_type>(size() - index);

      if (index < elements_after)
      {
        first_idx_ = (first_idx_ > 0) ? first_idx_ - 1 : Capacity - 1;
        ++size_;

        auto target = std::next(begin(), index);
        std::move(std::next(begin()), std::next(target), begin());
        *target = std::forward<T>(value);

        return target;
      }
      else
      {
        ++size_;
        auto target = std::next(begin(), index);
        std::move_backward(target, std::prev(end()), end());
        *target = std::forward<T>(value);

        return target;
      }
    }

    /**
     * @brief Removes the element at the specified position.
     * @return Iterator following the removed element.
     */
    constexpr iterator erase(const_iterator pos)
    {
      if (pos == cend())
        return end();

      const difference_type index = std::distance(cbegin(), pos);
      const auto elements_after = static_cast<difference_type>(size() - 1 - index);
      auto target = std::next(begin(), index);

      if (index < elements_after)
      {
        std::move_backward(begin(), target, std::next(target));
        pop_front();
        return std::next(begin(), index);
      }
      else
      {
        std::move(std::next(target), end(), target);
        pop_back();
        return std::next(begin(), index);
      }
    }

    /**
     * @brief Clears the buffer.
     */
    constexpr void clear()
    {
      size_  = 0;
      first_idx_ = 0;
    }

  private:
    storage data_{};
    size_type first_idx_{0};
    size_type size_{0};

    friend iterator;
    friend const_iterator;
  };
}