/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

/**
 * @file list.h
 * @brief A fixed-capacity, doubly-linked list for embedded systems.
 */
#pragma once

#include <cstddef>
#include <array>
#include <iterator>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <elib/assert.h>

namespace elib
{
  /**
   * @brief A fixed-capacity, doubly-linked list designed for embedded systems.
   * @tparam T The type of the elements to be stored.
   * @tparam Capacity The maximum number of elements the list can hold.
   *
   * @details
   * `elib::list` provides a `std::list`-like interface but avoids dynamic memory allocation.
   * All nodes are stored in an internal, fixed-size pool (`std::array`), making it suitable
   * for environments where the heap is unavailable or its use is restricted.
   *
   * It supports types that are not default-constructible through a manual lifetime management strategy.
   */
  template<typename T, std::size_t Capacity>
  class list
  {
    static_assert(Capacity > 0, "elib::list: Capacity must be greater than zero");

    // --- Storage Strategy ---
    // If T is trivial (like int), we use direct storage (simple assignment).
    // If T is complex (std::string) or not default-constructible, we use a union (manual lifetime).
    static constexpr bool is_simple_value = std::is_trivially_destructible_v<T> && 
                                            std::is_default_constructible_v<T> && 
                                            std::is_copy_assignable_v<T>;
    
    /// @brief Direct storage for trivially constructible/destructible types.
    struct direct_storage
    {
      T value;

      template<typename... Args>
      void construct(Args&&... args)
      {
        value = T(std::forward<Args>(args)...);
      }

      void destroy() {} // No-op for trivial types
    };

    /// @brief Manual lifetime management for complex or non-default-constructible types.
    struct manual_storage
    {
      union { T value; }; // Anonymous union prevents default construction

      manual_storage() {}  // Do nothing ctor
      ~manual_storage() {} // Do nothing dtor

      template<typename... Args>
      void construct(Args&&... args)
      {
        new (&value) T(std::forward<Args>(args)...);
      }

      void destroy()
      {
        value.~T();
      }
    };

    using storage_type = std::conditional_t<is_simple_value, direct_storage, manual_storage>;

    /// @brief Internal node structure for the linked list.
    struct node_type
    {
      node_type* prev{nullptr};
      node_type* next{nullptr};
      storage_type storage;

      void link_next(node_type* next)
      {
        this->next = next;

        if (next) {
          next->prev = this;
        }
      }

      void link_previous(node_type* prev)
      {
        this->prev = prev;

        if (prev) {
          prev->next = this;
        }
      }

      void link(node_type* previous, node_type* next)
      {
        link_previous(previous);
        link_next(next);
      }

      void unlink()
      {
        if (next) {
          next->prev = prev;
        }

        if (prev) {
          prev->next = next;
        }

        next = nullptr;
        prev = nullptr;
      }
    };

    /// @brief A fixed-size pool allocator for list nodes.
    struct node_pool
    {
      node_pool()
      {
        reset();
      }

      node_type* allocate()
      {
        if (!free_head)
          return nullptr;

        node_type* node = free_head;
        free_head = free_head->next;

        return node;
      }

      void deallocate(node_type* node)
      {
        if (node < pool.data() || node >= pool.data() + Capacity)
          return;

        node->next = free_head;
        free_head = node;
      }

      void reset()
      {
        for (std::size_t i = 0; i < pool.size() - 1; i++)
        {
          pool[i].next = &pool[i + 1];
        }
        pool[pool.size() - 1].next = nullptr;
        free_head = &pool.front();
      }

      bool is_belong_to_this(node_type* node) const
      {
        return node >= pool.data() && node < pool.data() + Capacity;
      }

      node_type* free_head{nullptr};
      std::array<node_type, Capacity> pool{};
    };

    /**
     * @brief A base template for the list's bidirectional iterators.
     * @tparam ValueType The type of the value the iterator points to (T or const T).
     */
    template<typename ValueType>
    struct iterator_base
    {
      using iterator_category = std::bidirectional_iterator_tag;
      using reference         = ValueType&;
      using pointer           = ValueType*;
      using value_type        = std::decay_t<ValueType>;
      using difference_type   = std::ptrdiff_t;

      node_type* node;
      node_type* tail;

      /**
       * @brief Dereferences the iterator to access the element.
       */
      reference operator*() { return node->storage.value; }
      /**
       * @brief Dereferences the iterator to access a member of the element.
       */
      pointer operator->() { return &(node->storage.value); }

      iterator_base& operator++()
      {
        if (node) node = node->next;
        return *this;
      }

      iterator_base operator++(int)
      {
        iterator_base tmp = *this;
        ++(*this);
        return tmp;
      }

      iterator_base& operator--()
      {
        if (node) node = node->prev;
        else      node = tail; // Decrementing end() goes to tail
        return *this;
      }

      iterator_base operator--(int)
      {
        iterator_base tmp = *this;
        --(*this);
        return tmp;
      }
      bool operator==(const iterator_base& other)
      {
        return node == other.node;
      }

      bool operator!=(const iterator_base& other)
      {
        return !(*this == other);
      }

      /**
       * @brief Compares two iterators for equality.
       */
      friend bool operator==(const iterator_base& lhs, const iterator_base& rhs)
      {
        return lhs.node == rhs.node;
      }

      /**
       * @brief Compares two iterators for inequality.
       */
      friend bool operator!=(const iterator_base& lhs, const iterator_base& rhs)
      {
        return lhs.node != rhs.node;
      }

      operator iterator_base<const ValueType>() const
      {
        return iterator_base<const ValueType>{node, tail};
      }
    };

    template<typename ValueType>
    bool is_belong_to_this(const iterator_base<ValueType>& it) const
    {
      return it.node == nullptr /* end */ || pool_.is_belong_to_this(it.node);
    }

    node_type* head_{nullptr};
    node_type* tail_{nullptr};
    std::size_t size_{};
    node_pool pool_;

  public:
    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using iterator        = iterator_base<T>;
    using const_iterator  = iterator_base<const T>;
    using size_type       = std::size_t;

    /**
     * @brief Default constructor. Creates an empty list.
     */
    list() = default;

    /**
     * @brief Constructs the list with the contents of the initializer list.
     * @param init Initializer list to initialize the elements of the list with.
     */
    list(std::initializer_list<T> init)
    {
      for (auto&& value : init)
        emplace_back(std::move(value));
    }

    /**
     * @brief Constructs the list with the contents of the range [first, last).
     * @tparam InputIt Input iterator type.
     * @param first The beginning of the range to copy from.
     * @param last The end of the range to copy from.
     */
    template< class InputIt >
    list(InputIt first, InputIt last)
    {
      std::for_each(first, last, [this](const auto& value){
        emplace_back(value);
      });
    }

    /**
     * @brief Copy constructor. Creates a deep copy of another list.
     * @param other The list to copy from.
     */
    list(const list& other)
    {
      *this = other;
    }

    /**
     * @brief Copy assignment operator.
     * @details Clears the current list and performs a deep copy of the other list's elements.
     * @param other The list to copy from.
     * @return A reference to this list.
     */
    list& operator=(const list& other)
    {
      if (this == &other)
        return *this;

      clear();

      for (const auto& value : other)
        emplace_back(value);

      return *this;
    }

    /**
     * @brief Move constructor.
     * @details Since the node pool is internal, this performs a deep copy and clears the source list.
     * It is not an O(1) operation.
     * @param other The list to move from.
     */
    list(list&& other)
    {
      *this = std::move(other);
      other.clear();
    }

    /**
     * @brief Move assignment operator.
     * @details Since the node pool is internal, this performs a deep copy and clears the source list.
     * It is not an O(1) operation.
     * @param other The list to move from.
     * @return A reference to this list.
     */
    list& operator=(list&& other)
    {
      if (this == &other)
        return *this;

      clear();

      for (auto&& value : other)
        emplace_back(std::move(value));

      other.clear();

      return *this;
    }

    /**
     * @brief Destructor. Clears the list and destroys all elements.
     */
    ~list()
    {
      clear();
    }

    /**
     * @brief Appends an element to the end of the list.
     * @param value The value to add.
     * @return `true` if the element was added successfully, `false` if the list was full.
     */
    bool push_back(const T& value)
    {
      return emplace_back(value);
    }

    bool push_back(T&& value)
    {
      return emplace_back(std::move(value));
    }

    /**
     * @brief Removes the last element from the list.
     * @return `true` if an element was removed, `false` if the list was empty.
     */
    bool pop_back()
    {
      if (!tail_) // list is empty
        return false;

      node_type* old_tail = tail_;
      if (old_tail->prev) {
        tail_ = old_tail->prev;
        tail_->next = nullptr;
      } else {
        head_ = nullptr;
        tail_ = nullptr;
      }
      --size_;

      old_tail->storage.destroy();
      pool_.deallocate(old_tail);

      return true;
    }

    /**
     * @brief Prepends an element to the beginning of the list.
     * @param value The value to add.
     * @return `true` if the element was added successfully, `false` if the list was full.
     */
    bool push_front(const T& value)
    {
      return emplace_front(value);
    }

    bool push_front(T&& value)
    {
      return emplace_front(std::move(value));
    }

    /**
     * @brief Removes the first element from the list.
     * @return `true` if an element was removed, `false` if the list was empty.
     */
    bool pop_front()
    {
      if (!head_)
        return false;

      node_type* old_head = head_;
      if (old_head->next) {
        head_ = old_head->next;
        head_->prev = nullptr;
      } else {
        head_ = nullptr;
        tail_ = nullptr;
      }
      --size_;

      old_head->storage.destroy();
      pool_.deallocate(old_head);

      return true;
    }

    /**
     * @brief Inserts an element before the given position.
     * @param it Iterator before which the new element will be inserted.
     * @param value The value of the element to insert.
     * @return An iterator pointing to the newly inserted element, or `end()` if the
     *         list is full or the iterator is invalid.
     */
    iterator insert(const_iterator it, const T& value)
    {
      return emplace(it, value);
    }

    iterator insert(const_iterator it, T&& value)
    {
      return emplace(it, std::move(value));
    }


    /**
     * @brief Erases the element at the specified position.
     * @param it Iterator to the element to remove.
     * @return An iterator pointing to the element that followed the erased element,
     *         or `end()` if the last element was erased or the iterator was invalid.
     */
    iterator erase(iterator it)
    {
      if (empty() || it == end() || !is_belong_to_this(it))
        return end();

      node_type* node = it.node;
      node_type* following = it.node->next;

      if (node == head_) head_ = node->next;
      if (node == tail_) tail_ = node->prev;

      it.node->unlink();
      --size_;
      node->storage.destroy();
      pool_.deallocate(node);

      return iterator{following, tail_};
    }

    /**
     * @brief Inserts a new element into the container directly before `pos`.
     * @details The element is constructed in-place using placement-new, avoiding extra copy/move operations.
     * @tparam Args Types of arguments to forward to the constructor of the element.
     * @param pos Iterator before which the content will be inserted.
     * @param args Arguments to forward to the constructor of the element.
     * @return Iterator pointing to the emplaced element, or `end()` if the list is full or iterator is invalid.
     */
    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
      if (full() || !is_belong_to_this(pos))
        return end();

      node_type* node = pos.node;
      node_type* new_node = pool_.allocate();
      if (!new_node)
        return end();

      new_node->storage.construct(std::forward<Args>(args)...);
      if (node) {
        new_node->link_previous(node->prev);
      } else {
        new_node->link_previous(tail_);
      }

      new_node->link_next(node);
      ++size_;

      if (!new_node->prev) {
        head_ = new_node;
      }

      if (!new_node->next) {
        tail_ = new_node;
      }

      if (!head_) {
        head_ = new_node;
        tail_ = new_node;
      }

      return iterator{new_node, tail_};
    }

    /**
     * @brief Appends a new element to the end of the container.
     * @details The element is constructed in-place using placement-new at the back of the list.
     * @tparam Args Types of arguments to forward to the constructor of the element.
     * @param args Arguments to forward to the constructor of the element.
     * @return `true` if the element was successfully emplaced, `false` if the list was full.
     */
    template<typename... Args>
    bool emplace_front(Args&&... args)
    {
      return emplace(cbegin(), std::forward<Args>(args)...) != end();
    }

    /**
     * @brief Prepends a new element to the beginning of the container.
     * @details The element is constructed in-place using placement-new at the front of the list.
     * @tparam Args Types of arguments to forward to the constructor of the element.
     * @param args Arguments to forward to the constructor of the element.
     * @return `true` if the element was successfully emplaced, `false` if the list was full.
     */
    template<typename... Args>
    bool emplace_back(Args&&... args)
    {
      return emplace(cend(), std::forward<Args>(args)...) != end();
    }

    /**
     * @brief Accesses the last element.
     * @pre The list must not be empty.
     * @return A reference to the last element.
     */
    reference back()
    {
      ELIB_ASSERT(tail_, "elib::list::back: list is empty");

      return tail_->storage.value;
    }

    /**
     * @brief Accesses the last element.
     * @pre The list must not be empty.
     * @return A const reference to the last element.
     */
    const_reference back() const
    {
      ELIB_ASSERT(tail_, "elib::list::back: list is empty");

      return tail_->storage.value;
    }

    /**
     * @brief Accesses the first element.
     * @pre The list must not be empty.
     * @return A reference to the first element.
     */
    reference front()
    {
      ELIB_ASSERT(tail_, "elib::list::front: list is empty");

      return head_->storage.value;
    }

    /**
     * @brief Accesses the first element.
     * @pre The list must not be empty.
     * @return A const reference to the first element.
     */
    const_reference front() const
    {
      ELIB_ASSERT(tail_, "elib::list::front: list is empty");

      return head_->storage.value;
    }

    /**
     * @brief Returns an iterator to the first element of the list.
     * @return Iterator to the first element.
     */
    iterator begin()
    {
      return iterator{head_, tail_};
    }

    /**
     * @brief Returns a const iterator to the first element of the list.
     * @return Const iterator to the first element.
     */
    const_iterator begin() const
    {
      return const_iterator{head_, tail_};
    }

    /**
     * @brief Returns a const iterator to the first element of the list.
     * @return Const iterator to the first element.
     */
    const_iterator cbegin() const
    {
      return const_iterator{head_, tail_};
    }

    /**
     * @brief Returns an iterator to the element following the last element of the list.
     * @return Iterator to the element following the last element.
     */
    iterator end()
    {
      return iterator{nullptr, tail_};
    }

    /**
     * @brief Returns a const iterator to the element following the last element of the list.
     * @return Const iterator to the element following the last element.
     */
    const_iterator end() const
    {
      return const_iterator{nullptr, tail_};
    }

    /**
     * @brief Returns a const iterator to the element following the last element of the list.
     * @return Const iterator to the element following the last element.
     */
    const_iterator cend() const
    {
      return const_iterator{nullptr, tail_};
    }

    /**
     * @brief Returns the number of elements in the list.
     * @return The number of elements.
     */
    size_type size() const
    {
      return size_;
    }

    /**
     * @brief Returns the maximum number of elements the list can hold.
     * @return The capacity of the list.
     */
    static size_type max_size()
    {
      return Capacity;
    }

    /**
     * @brief Checks if the list has no elements.
     * @return `true` if the list is empty, `false` otherwise.
     */
    bool empty() const
    {
      return size_ == 0;
    }

    /**
     * @brief Checks if the list has reached its maximum capacity.
     * @return `true` if the list is full, `false` otherwise.
     */
    bool full() const
    {
      return size_ == Capacity;
    }

    /**
     * @brief Erases all elements from the list.
     */
    void clear()
    {
      node_type* node = head_;
      while (node)
      {
        // NOTE: save next as it's not valid after node deallocation
        node_type* next = node->next;
        node->storage.destroy();
        pool_.deallocate(node); 
        node = next;
      }

      head_ = nullptr;
      tail_ = nullptr;
      size_ = {};
    }
  };
}
