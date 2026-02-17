/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <cstring>
#include <type_traits>

namespace elib::data
{
  template<bool readOnly = true>
  class stream_base
  {
  public:
    using byte = std::conditional_t<readOnly, const std::uint8_t, std::uint8_t>;
    using byte_pointer = byte*;

    constexpr stream_base(byte_pointer array, std::size_t size)
      : bytes_{array}
      , limit_{size}
    {
    }

    constexpr std::size_t pos() const { return pos_; }
    constexpr bool overflow() const { return overflow_; }

    constexpr bool seek(std::size_t pos)
    {
      // handle empty buffer
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
    template<typename T>
    using enable_if_trivial = std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, T>;

    template<typename T, std::size_t N>
    static constexpr std::size_t size_bytes(const std::array<T, N>&) noexcept { return N * sizeof(T); }
    template<typename T, std::size_t N>
    static constexpr std::size_t size_bytes(const T(&)[N]) noexcept { return N * sizeof(T); }

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

    constexpr void increment(std::size_t n) { pos_ += n; }
    inline byte_pointer current_address() { return bytes_ + pos_;}
  };

  class output_stream : public stream_base<false>
  {
  public:
    template<typename T, std::size_t N, typename = enable_if_trivial<T>>
    constexpr output_stream(std::array<T, N>& array)
      : stream_base{reinterpret_cast<byte_pointer>(array.data()), size_bytes(array)}
    {
    }

    template<typename T, std::size_t N, typename = enable_if_trivial<T>>
    constexpr output_stream(T (&array)[N])
      : stream_base{reinterpret_cast<byte_pointer>(array), size_bytes(array)}
    {
    }

    constexpr output_stream(byte_pointer array, std::size_t size)
      : stream_base{array, size}
    {

    }

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

  class input_stream : public stream_base<true>
  {
  public:
    template<typename T, std::size_t N, typename = enable_if_trivial<T>>
    constexpr input_stream(const std::array<T, N>& array)
      : stream_base{reinterpret_cast<byte_pointer>(array.data()), size_bytes(array)}
    {
    }

    template<typename T, std::size_t N, typename = enable_if_trivial<T>>
    constexpr input_stream(const T (&array)[N])
      : stream_base{reinterpret_cast<byte_pointer>(array), size_bytes(array)}
    {
    }

    constexpr input_stream(byte_pointer array, std::size_t size)
      : stream_base{array, size}
    {

    }

    template<typename T, typename = enable_if_trivial<T>>
    T read()
    {
      T value{};
      *this >> value;

      return value;
    }

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
      else if constexpr (std::is_same_v<T, float>)
      {
        value = *(reinterpret_cast<const float*>(current_address()));
      }
      else
      {
        std::memcpy(&value, current_address(), vsize);
      }
      increment(vsize);

      return true;
    }

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