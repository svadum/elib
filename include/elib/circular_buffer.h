/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <algorithm>
#include <type_traits>

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

    template<typename pointer>
    constexpr void decrement(pointer& it, pointer storage_begin, pointer storage_end)
    {
      if (it != storage_begin) {
        --it;
        return;
      }

      it = storage_end - 1;
    }

    template<typename size_type>
    constexpr void increment(size_type& index, size_type capacity)
    {
      if (++index == capacity)
        index = 0;
    }

    template<typename size_type>
    constexpr void decrement(size_type& index, size_type capacity)
    {
      index = index > 0 ? index - 1 : capacity - 1;
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

    template<typename size_type, typename difference_type>
    constexpr size_type add(size_type index, difference_type n, size_type capacity)
    {
      const auto availableAtEnd = capacity - index;
      if (availableAtEnd > n)
        return index + n;

      return ((n - availableAtEnd) % capacity);
    }

    template<typename size_type, typename difference_type>
    constexpr size_type sub(size_type index, difference_type n, size_type capacity)
    {
      if (n > capacity)
        n = n % capacity;

      if (index >= n)
        return index - n;

      return capacity - (n - index);
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
        auto ptr = detail::add(static_cast<pointer>(buffer_->first_), idx_, 
                               buffer_->storage_begin(), buffer_->storage_end());
        return *(ptr);
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
      // (Templated to allow comparing iterator with const_iterator safely)
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
      size_type idx_{};
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
    circular_buffer(const circular_buffer& other) noexcept
    {
      *this = other;
    }

    /**
     * @brief Copy assignment operator.
     */
    circular_buffer& operator=(const circular_buffer& other) noexcept
    {
      if (this == &other)
        return *this;

      // TODO: copy only initialized values
      std::copy(other.data_.begin(), other.data_.end(), storage_begin());
      size_ = other.size_;

      first_ = storage_begin() + (other.first_ - other.storage_begin());
      last_  = storage_begin() + (other.last_ - other.storage_begin());

      return *this;
    }

    /**
     * @brief Move constructor.
     */
    circular_buffer(circular_buffer&& other) noexcept
    {
      *this = std::move(other);
    }

    /**
     * @brief Move assignment operator.
     */
    circular_buffer& operator=(circular_buffer&& other) noexcept
    {
      if (this == &other)
        return *this;

      clear();

      std::swap(data_, other.data_);
      std::swap(size_, other.size_);

      first_ = storage_begin() + (other.first_ - other.storage_begin());
      last_  = storage_begin() + (other.last_ - other.storage_begin());

      return *this;
    }

    /**
     * @brief Constructs the buffer with an initializer list.
     */
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

      size_ = il.size();
      last_ = detail::add(last_, size_, storage_begin(), storage_end());
    }

    /**
     * @brief Constructs the buffer from a C-style array.
     */
    template<typename T, std::size_t N>
    constexpr circular_buffer(const T (&array)[N])
      : circular_buffer()
    {
      static_assert(N <= Capacity, "Array size more than container capacity");

      size_ = N;
      last_ = detail::add(last_, N, storage_begin(), storage_end());
      std::copy(array, array + N, storage_begin());
    }

    /**
     * @brief Constructs the buffer from a pointer and size.
     */
    circular_buffer(value_type* data, std::size_t size)
      : circular_buffer()
    {
      if (!data || !size || size > Capacity)
        return;

      size_ = size;
      last_ = detail::add(last_, size, storage_begin(), storage_end());
      std::copy(data, data + size, storage_begin());
    }

    /**
     * @brief Returns an iterator to the beginning.
     */
    constexpr iterator begin()
    {
      return iterator(this, 0);
    }

    /**
     * @brief Returns an iterator to the end.
     */
    constexpr iterator end()
    {
      return iterator(this, size_);
    }

    /**
     * @brief Returns a const iterator to the beginning.
     */
    constexpr const_iterator begin() const
    {
      return const_iterator(this, 0);
    }

    /**
     * @brief Returns a const iterator to the end.
     */
    constexpr const_iterator end() const
    {
      return const_iterator(this, size_);
    }

    /**
     * @brief Returns a const iterator to the beginning.
     */
    constexpr const_iterator cbegin() const
    {
      return begin();
    }

    /**
     * @brief Returns a const iterator to the end.
     */
    constexpr const_iterator cend() const
    {
      return end();
    }

    /**
     * @brief Access the first element.
     */
    constexpr reference front()
    {
      return *first_;
    }

    /**
     * @brief Access the first element (const).
     */
    constexpr const_reference front() const
    {
      return *first_;
    }

    /**
     * @brief Access the last element.
     */
    constexpr reference back()
    {
      return *((last_ == storage_begin() ? storage_end() : last_) - 1);
    }

    /**
     * @brief Access the last element (const).
     */
    constexpr const_reference back() const
    {
      return *((last_ == storage_begin() ? storage_end() : last_) - 1);
    }

    /**
     * @brief Returns the number of elements in the buffer.
     */
    constexpr size_type size() const
    {
      return size_;
    }

    /**
     * @brief Returns the capacity of the buffer.
     */
    constexpr size_type capacity() const
    {
      return Capacity;
    }

    /**
     * @brief Checks if the buffer is empty.
     */
    constexpr bool empty() const
    {
      return !size();
    }

    /**
     * @brief Checks if the buffer is full.
     */
    constexpr bool full() const
    {
      return size() == Capacity;
    }

    /**
     * @brief Adds an element to the end of the buffer.
     * @deprecated Use push_back() instead.
     */
    template<typename T> [[deprecated("use push_back() instead")]]
    bool push(T&& value)
    {
      return push_back(std::forward<T>(value));
    }

    /**
     * @brief Adds an element to the end of the buffer.
     * @return true if successful, false if buffer is full.
     */
    template<typename T>
    bool push_back(T&& value)
    {
      if (full())
        return false;

      *last_ = std::forward<T>(value);
      detail::increment(last_, storage_begin(), storage_end());
      ++size_;

      return true;
    }

    /**
     * @brief Removes the last element.
     * @return true if successful, false if buffer is empty.
     */
    bool pop_back()
    {
      if (empty())
        return false;

      detail::decrement(last_, storage_begin(), storage_end());
      --size_;

      return true;
    }

    /**
     * @brief Adds an element to the end, overwriting the oldest if full.
     */
    template<typename T>
    void push_over(T&& value)
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
    bool push_front(T&& value)
    {
      if (full())
        return false;

      auto new_first = first_;
      detail::decrement(new_first, storage_begin(), storage_end());
      *new_first = std::forward<T>(value);
      first_ = new_first;
      ++size_;

      return true;
    }

    /**
     * @brief Removes the first element.
     * @return true if successful, false if buffer is empty.
     */
    bool pop_front()
    {
      if (empty())
        return false;

      detail::increment(first_, storage_begin(), storage_end());
      --size_;

      return true;
    }

    /**
     * @brief Removes the first element.
     * @deprecated Use pop_front() instead.
     */
    [[deprecated("use pop_front() instead")]]
    bool pop()
    {
      return pop_front();
    }

    /**
     * @brief Inserts an element at the specified position.
     * @return Iterator pointing to the inserted value, or end() if the buffer is full.
     */
    template<typename T>
    iterator insert(const_iterator pos, T&& value)
    {
      if (full())
        return end();

      const auto index = std::distance(cbegin(), pos);
      const auto elements_after = size() - index;

      if (index < elements_after) 
      {
        // Closer to the front: shift front elements left
        auto new_first = first_;
        detail::decrement(new_first, storage_begin(), storage_end());
        first_ = new_first;
        ++size_;

        auto target = std::next(begin(), index);
        // Move the displaced front elements
        std::move(std::next(begin()), std::next(target), begin());
        *target = std::forward<T>(value);
        return target;
      } 
      else 
      {
        // Closer to the back: shift back elements right
        detail::increment(last_, storage_begin(), storage_end());
        ++size_;

        auto target = std::next(begin(), index);
        // Move the displaced back elements
        std::move_backward(target, std::prev(end()), end());
        *target = std::forward<T>(value);
        return target;
      }
    }

    /**
     * @brief Removes the element at the specified position.
     * @return Iterator following the removed element.
     */
    iterator erase(const_iterator pos)
    {
      if (pos == cend())
        return end();

      const auto index = std::distance(cbegin(), pos);
      const auto elements_after = size() - 1 - index;
      auto target = std::next(begin(), index);

      if (index < elements_after) 
      {
        // Closer to the front: shift front elements right to fill gap
        std::move_backward(begin(), target, std::next(target));
        pop_front();
        return std::next(begin(), index);
      } 
      else 
      {
        // Closer to the back: shift back elements left to fill gap
        std::move(std::next(target), end(), target);
        pop_back();
        return std::next(begin(), index);
      }
    }

    /**
     * @brief Clears the buffer.
     */
    void clear()
    {
      size_  = 0;
      first_ = last_ = storage_begin();
    }

  private:
    storage data_{};

    pointer first_{data_.data()};
    pointer last_{data_.data()};

    size_type size_{0};

    friend iterator;
    friend const_iterator;

    constexpr pointer storage_begin()
    {
      return data_.data();
    }

    constexpr const_pointer storage_begin() const
    {
      return data_.data();
    }

    constexpr pointer storage_end()
    {
      return data_.data() + data_.size();
    }

    constexpr const_pointer storage_end() const
    {
      return data_.data() + data_.size();
    }
  };
}
