/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

/**
 * @file stream.h
 * @brief Zero-allocation, bare-metal memory stream module.
 *
 * Provides highly efficient input and output stream wrappers around raw memory buffers
 * and static arrays. Supports fundamental types while preventing out-of-bounds access.
 */

#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace elib::data
{
  /**
   * @brief Base class for memory streams managing buffer state and bounds.
   * @tparam readOnly True if the underlying buffer should be accessed via const pointers.
   */
  template<bool readOnly = true>
  class stream_base
  {
  public:
    using byte = std::conditional_t<readOnly, const std::uint8_t, std::uint8_t>;
    using byte_pointer = byte*;

    /**
     * @brief Constructs a stream base with a buffer and its size.
     * @param buffer Pointer to the start of the memory buffer.
     * @param size The maximum size of the buffer in bytes.
     */
    constexpr stream_base(byte_pointer buffer, std::size_t size)
      : bytes_{buffer}
      , limit_{size}
    {
    }

    /// @return The underlying byte buffer pointer.
    constexpr byte_pointer data() { return bytes_; }
    /// @return The underlying const byte buffer pointer.
    constexpr byte_pointer data() const { return bytes_; }

    /// @return The current cursor position in bytes.
    constexpr std::size_t pos() const { return pos_; }

    /// @return The total capacity (maximum size) of the underlying buffer.
    constexpr std::size_t capacity() const { return limit_; }

    /// @return True if a read/write operation attempted to exceed the buffer limit.
    constexpr bool overflow() const { return overflow_; }

    /**
     * @brief Adjusts the current read/write cursor position.
     * @param pos The new byte offset to seek to.
     * @return True if the seek was successful, false if it exceeds the limit.
     */
    constexpr bool seek(std::size_t pos)
    {
      if (!pos && !limit_)
      {
        overflow_ = false;
        return true;
      }

      if (pos >= limit_)
        return false;

      pos_ = pos;

      if (overflow_)
        overflow_ = false;

      return true;
    }

  private:
    byte_pointer bytes_{nullptr};
    bool overflow_{};
    std::size_t pos_{};
    std::size_t limit_{};

  protected:
    /// Trait enabling templates only for fundamental arithmetic types and enums.
    template<typename T>
    using enable_if_trivial = std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, T>;

    /// @return Total bytes in a std::array.
    template<typename T, std::size_t N>
    static constexpr std::size_t size_bytes(const std::array<T, N>&) noexcept { return N * sizeof(T); }

    /// @return Total bytes in a C-style array.
    template<typename T, std::size_t N>
    static constexpr std::size_t size_bytes(const T(&)[N]) noexcept { return N * sizeof(T); }

    /**
     * @brief Verifies if the incoming operation exceeds the limit and sets the overflow flag.
     * @param size Bytes requested for the operation.
     * @return True if overflowed, false otherwise.
     */
    constexpr bool set_overflow(std::size_t size) noexcept
    {
      if ((pos_ + size) > limit_)
        overflow_ = true;

      return overflow_;
    }

    template<typename T, std::size_t N>
    constexpr bool set_overflow(const std::array<T, N>&) noexcept { return set_overflow(N * sizeof(T)); }

    template<typename T, std::size_t N>
    constexpr bool set_overflow(const T(&)[N]) noexcept { return set_overflow(N * sizeof(T)); }

    /// @brief Advances the cursor position.
    constexpr void increment(std::size_t n) { pos_ += n; }

    /// @return Memory address of the current cursor position.
    inline byte_pointer current_address() { return bytes_ + pos_;}
  };

  /**
   * @brief Stream for writing binary data into an underlying mutable memory buffer.
   */
  class output_stream : public stream_base<false>
  {
  public:
    /**
     * @brief Constructs an output stream over a std::array.
     * @tparam T Underlying array type.
     * @tparam N Array capacity.
     */
    template<typename T, std::size_t N, typename = enable_if_trivial<T>>
    constexpr output_stream(std::array<T, N>& array)
      : stream_base{reinterpret_cast<byte_pointer>(array.data()), size_bytes(array)}
    {
    }

    /**
     * @brief Constructs an output stream over a C-style array.
     */
    template<typename T, std::size_t N, typename = enable_if_trivial<T>>
    constexpr output_stream(T (&array)[N])
      : stream_base{reinterpret_cast<byte_pointer>(array), size_bytes(array)}
    {
    }

    /**
     * @brief Constructs an output stream over a raw memory block.
     */
    constexpr output_stream(byte_pointer array, std::size_t size)
      : stream_base{array, size}
    {
    }

    /**
     * @brief Writes an opaque block of memory into the stream.
     * @param data Pointer to the source data block.
     * @param size Size of the data block in bytes.
     * @return True if successful, false on overflow.
     */
    bool write(const void* data, std::size_t size)
    {
      if (!size) return true;

      if (set_overflow(size))
        return false;

      std::memcpy(current_address(), data, size);
      increment(size);

      return true;
    }

    /**
     * @brief Writes a single trivial value into the stream.
     */
    template<typename T, typename = enable_if_trivial<T>>
    bool write(T value)
    {
      constexpr auto vsize = sizeof(T);
      if (set_overflow(vsize))
        return false;

      if constexpr (vsize == sizeof(byte))
        *current_address() = static_cast<byte>(value);
      else
        std::memcpy(current_address(), &value, vsize);

      increment(vsize);
      return true;
    }

    /**
     * @brief Writes a C-style array of trivial types into the stream.
     */
    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    bool write(const T(&array)[size])
    {
      if (set_overflow(array))
        return false;

      const std::size_t underlyingSize = size_bytes(array);
      std::memcpy(current_address(), array, underlyingSize);
      increment(underlyingSize);
      return true;
    }

    /**
     * @brief Writes a std::array of trivial types into the stream.
     */
    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    bool write(const std::array<T, size>& array)
    {
      if (set_overflow(array))
        return false;

      const std::size_t underlyingSize = size_bytes(array);
      std::memcpy(current_address(), array.data(), underlyingSize);
      increment(underlyingSize);
      return true;
    }

    // Stream operator overloads for output

    template<typename T, typename = enable_if_trivial<T>>
    output_stream& operator<<(T value)
    {
      static_cast<void>(write(value));
      return *this;
    }

    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    output_stream& operator<<(const T(&array)[size])
    {
      static_cast<void>(write(array));
      return *this;
    }

    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    output_stream& operator<<(const std::array<T, size>& array)
    {
      static_cast<void>(write(array));
      return *this;
    }
  };

  /**
   * @brief Stream for reading binary data from an underlying read-only memory buffer.
   */
  class input_stream : public stream_base<true>
  {
  public:
    /**
     * @brief Constructs an input stream over a read-only std::array.
     */
    template<typename T, std::size_t N, typename = enable_if_trivial<T>>
    constexpr input_stream(const std::array<T, N>& array)
      : stream_base{reinterpret_cast<byte_pointer>(array.data()), size_bytes(array)}
    {
    }

    /**
     * @brief Constructs an input stream over a read-only C-style array.
     */
    template<typename T, std::size_t N, typename = enable_if_trivial<T>>
    constexpr input_stream(const T (&array)[N])
      : stream_base{reinterpret_cast<byte_pointer>(array), size_bytes(array)}
    {
    }

    /**
     * @brief Constructs an input stream over a read-only raw memory block.
     */
    constexpr input_stream(byte_pointer array, std::size_t size)
      : stream_base{array, size}
    {
    }

    /**
     * @brief Reads an opaque block of memory from the stream.
     * @param data Pointer to the destination data block.
     * @param size Size of the data block in bytes to read.
     * @return True if successful, false on overflow.
     */
    bool read(void* data, std::size_t size)
    {
      if (!size) return true;

      if (set_overflow(size))
        return false;

      std::memcpy(data, current_address(), size);
      increment(size);

      return true;
    }

    /**
     * @brief Extracts and returns a single trivial value from the stream.
     */
    template<typename T, typename = enable_if_trivial<T>>
    T read()
    {
      T value{};
      *this >> value;
      return value;
    }

    /**
     * @brief Reads a single trivial value from the stream into the provided reference.
     */
    template<typename T, typename = enable_if_trivial<T>>
    bool read(T& value)
    {
      constexpr auto vsize = sizeof(T);
      if (set_overflow(vsize))
        return false;

      if constexpr (vsize == sizeof(byte))
      {
        value = static_cast<T>(*current_address());
      }
      else
      {
        std::memcpy(&value, current_address(), vsize);
      }
      increment(vsize);
      return true;
    }

    /**
     * @brief Reads from the stream into a C-style array of trivial types.
     */
    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    bool read(T(&array)[size])
    {
      if (set_overflow(array))
        return false;

      const std::size_t underlyingSize = size_bytes(array);
      std::memcpy(array, current_address(), underlyingSize);
      increment(underlyingSize);
      return true;
    }

    /**
     * @brief Reads from the stream into a std::array of trivial types.
     */
    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    bool read(std::array<T, size>& array)
    {
      if (set_overflow(array))
        return false;

      const std::size_t underlyingSize = size_bytes(array);
      std::memcpy(array.data(), current_address(), underlyingSize);
      increment(underlyingSize);
      return true;
    }

    // Stream operator overloads for input

    template<typename T, typename = enable_if_trivial<T>>
    input_stream& operator>>(T& value)
    {
      static_cast<void>(read(value));
      return *this;
    }

    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    input_stream& operator>>(T(&array)[size])
    {
      static_cast<void>(read(array));
      return *this;
    }

    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    input_stream& operator>>(std::array<T, size>& array)
    {
      static_cast<void>(read(array));
      return *this;
    }
  };
}