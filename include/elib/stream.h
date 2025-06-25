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
  class StreamBase
  {
  public:
    using byte = std::conditional_t<readOnly, const std::uint8_t, std::uint8_t>;
    using byte_pointer = byte*;

    constexpr StreamBase(byte_pointer array, std::size_t size)
      : m_bytes{array}
      , m_limit{size}
    {
    }

    constexpr std::size_t pos() const { return m_pos; }
    constexpr bool overflow() const { return m_overflow; }

    constexpr bool seek(std::size_t pos)
    {
      // handle empty buffer
      if (!pos && !m_limit)
      {
        m_overflow = false;
        return true;
      }

      if (pos >= m_limit)
        return false;

      m_pos = pos;

      if (m_overflow)
        m_overflow = false;

      return true;
    }

  private:
    byte_pointer m_bytes{nullptr};
    bool m_overflow{};
    std::size_t m_pos{};
    std::size_t m_limit{};

  protected:
    template<typename T>
    using enable_if_trivial = std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, T>;

    template<typename T, std::size_t N>
    static constexpr std::size_t sizeBytes(const std::array<T, N>&) noexcept { return N * sizeof(T); }
    template<typename T, std::size_t N>
    static constexpr std::size_t sizeBytes(const T(&)[N]) noexcept { return N * sizeof(T); }

    constexpr bool setOverflow(std::size_t size) noexcept
    {
      if ((m_pos + size) > m_limit)
        m_overflow = true;

      return m_overflow;
    }
    template<typename T, std::size_t N>
    constexpr bool setOverflow(const std::array<T, N>&) noexcept { return setOverflow(N * sizeof(T)); }
    template<typename T, std::size_t N>
    constexpr bool setOverflow(const T(&)[N]) noexcept { return setOverflow(N * sizeof(T)); }

    constexpr void increment(std::size_t n) { m_pos += n; }
    inline byte_pointer currentAddress() { return m_bytes + m_pos;}
  };

  class OutputStream : public StreamBase<false>
  {
  public:
    template<typename T, std::size_t N, typename = enable_if_trivial<T>>
    constexpr OutputStream(std::array<T, N>& array)
      : StreamBase{reinterpret_cast<byte_pointer>(array.data()), sizeBytes(array)}
    {
    }

    template<typename T, std::size_t N, typename = enable_if_trivial<T>>
    constexpr OutputStream(T (&array)[N])
      : StreamBase{reinterpret_cast<byte_pointer>(array), sizeBytes(array)}
    {
    }

    constexpr OutputStream(byte_pointer array, std::size_t size)
      : StreamBase{array, size}
    {

    }

    template<typename T, typename = enable_if_trivial<T>>
    bool write(T value)
    {
      constexpr auto vsize = sizeof(T);
      if (setOverflow(vsize))
        return false;

      if constexpr (vsize == sizeof(byte))
        *currentAddress() = static_cast<byte>(value);
      else
        std::memcpy(currentAddress(), &value, vsize);

      increment(vsize);

      return true;
    }

    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    bool write(const T(&array)[size])
    {
      if (setOverflow(array))
        return false;

      const std::size_t underlyingSize = sizeBytes(array);
      std::memcpy(currentAddress(), array, underlyingSize);
      increment(underlyingSize);

      return true;
    }

    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    bool write(const std::array<T, size>& array)
    {
      if (setOverflow(array))
        return false;

      const std::size_t underlyingSize = sizeBytes(array);
      std::memcpy(currentAddress(), array.data(), underlyingSize);
      increment(underlyingSize);

      return true;
    }

    template<typename T, typename = enable_if_trivial<T>>
    OutputStream& operator<<(T value)
    {
      static_cast<void>(write(value));

      return *this;
    }

    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    OutputStream& operator<<(const T(&array)[size])
    {
      static_cast<void>(write(array));

      return *this;
    }

    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    OutputStream& operator<<(const std::array<T, size>& array)
    {
      static_cast<void>(write(array));

      return *this;
    }
  };

  class InputStream : public StreamBase<true>
  {
  public:
    template<typename T, std::size_t N, typename = enable_if_trivial<T>>
    constexpr InputStream(const std::array<T, N>& array)
      : StreamBase{reinterpret_cast<byte_pointer>(array.data()), sizeBytes(array)}
    {
    }

    template<typename T, std::size_t N, typename = enable_if_trivial<T>>
    constexpr InputStream(const T (&array)[N])
      : StreamBase{reinterpret_cast<byte_pointer>(array), sizeBytes(array)}
    {
    }

    constexpr InputStream(byte_pointer array, std::size_t size)
      : StreamBase{array, size}
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
      if (setOverflow(vsize))
        return false;

      if constexpr (vsize == sizeof(byte))
      {
        value = static_cast<T>(*currentAddress());
      }
      else if constexpr (std::is_same_v<T, float>)
      {
        value = *(reinterpret_cast<const float*>(currentAddress()));
      }
      else
      {
        std::memcpy(&value, currentAddress(), vsize);
      }
      increment(vsize);

      return true;
    }

    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    bool read(T(&array)[size])
    {
      if (setOverflow(array))
        return false;

      const std::size_t underlyingSize = sizeBytes(array);
      std::memcpy(array, currentAddress(), underlyingSize);
      increment(underlyingSize);

      return true;
    }

    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    bool read(std::array<T, size>& array)
    {
      if (setOverflow(array))
        return false;

      const std::size_t underlyingSize = sizeBytes(array);
      std::memcpy(array.data(), currentAddress(), underlyingSize);
      increment(underlyingSize);

      return true;
    }

    template<typename T, typename = enable_if_trivial<T>>
    InputStream& operator>>(T& value)
    {
      static_cast<void>(read(value));

      return *this;
    }

    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    InputStream& operator>>(T(&array)[size])
    {
      static_cast<void>(read(array));

      return *this;
    }


    template<typename T, std::size_t size, typename = enable_if_trivial<T>>
    InputStream& operator>>(std::array<T, size>& array)
    {
      static_cast<void>(read(array));

      return *this;
    }
  };
}