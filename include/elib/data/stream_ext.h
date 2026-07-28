/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

/**
 * @file stream_ext.h
 * @brief Extension module providing stream operations for contiguous containers and string views.
 *
 * This module extends elib::data streams to seamlessly support std::string_view,
 * elib::span, and elib::array using highly optimized bulk memory transfers, while
 * strictly enforcing trivial type constraints for safe binary serialization.
 */

#pragma once

#include <cstdint>
#include <string_view>
#include <type_traits>
#include <elib/stream.h>
#include <elib/array.h>
#include <elib/span.h>

namespace elib::data
{
  namespace detail
  {
    /**
     * @brief Computes the total byte size of dynamically populated contiguous containers.
     * @tparam ContiguousContainer A container providing `value_type` and `size()`.
     * @param container The container instance.
     * @return Total size in bytes.
     */
    template <typename ContiguousContainer>
    constexpr std::size_t size_bytes(const ContiguousContainer& container) noexcept
    {
      using value_type = typename ContiguousContainer::value_type;
      return container.size() * sizeof(value_type);
    }
  }

  /**
   * @brief Writes a string view to the output stream.
   * @param os The target output stream.
   * @param sv The string view to write.
   * @return Reference to the output stream.
   */
  inline output_stream& operator<<(output_stream& os, std::string_view sv)
  {
    os.write(sv.data(), sv.size());
    return os;
  }

  /**
   * @brief Writes a span of trivial types to the output stream.
   * @tparam T The underlying type of the span.
   * @tparam Extent The static extent of the span.
   * @param os The target output stream.
   * @param sp The span to write.
   * @pre Type T must be an arithmetic type or an enum.
   * @return Reference to the output stream.
   */
  template<typename T, std::size_t Extent>
  inline data::output_stream& operator<<(data::output_stream& os, span<T, Extent> sp)
  {
    static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>, "Stream operations require trivial types");
    os.write(sp.data(), detail::size_bytes(sp));
    return os;
  }

  /**
   * @brief Reads from the input stream into a span of trivial types.
   * @tparam T The underlying type of the span.
   * @tparam Extent The static extent of the span.
   * @param is The source input stream.
   * @param sp The span to read into.
   * @pre Type T must be an arithmetic type or an enum.
   * @return Reference to the input stream.
   */
  template<typename T, std::size_t Extent>
  inline data::input_stream& operator>>(data::input_stream& is, span<T, Extent> sp)
  {
    static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>, "Stream operations require trivial types");
    is.read(sp.data(), detail::size_bytes(sp));
    return is;
  }

  /**
   * @brief Writes the currently populated elements of an elib::array to the output stream.
   * @note Writes only size() elements, not the full capacity.
   * @tparam Value The underlying type of the array.
   * @tparam Capacity The maximum capacity of the array.
   * @param os The target output stream.
   * @param arr The array to write.
   * @pre Type Value must be an arithmetic type or an enum.
   * @return Reference to the output stream.
   */
  template<typename Value, std::size_t Capacity>
  inline data::output_stream& operator<<(data::output_stream& os, const array<Value, Capacity>& arr)
  {
    static_assert(std::is_arithmetic_v<Value> || std::is_enum_v<Value>, "Stream operations require trivial types");
    os.write(arr.data(), detail::size_bytes(arr));
    return os;
  }

  /**
   * @brief Reads from the input stream into the currently populated elements of an elib::array.
   * @note Reads only into the existing populated size() of the array, not the full capacity.
   *       Empty arrays must be resized or populated prior to reading.
   * @tparam Value The underlying type of the array.
   * @tparam Capacity The maximum capacity of the array.
   * @param is The source input stream.
   * @param arr The array to read into.
   * @pre Type Value must be an arithmetic type or an enum.
   * @return Reference to the input stream.
   */
  template<typename Value, std::size_t Capacity>
  inline data::input_stream& operator>>(data::input_stream& is, array<Value, Capacity>& arr)
  {
    static_assert(std::is_arithmetic_v<Value> || std::is_enum_v<Value>, "Stream operations require trivial types");
    is.read(arr.data(), detail::size_bytes(arr));
    return is;
  }

  /**
   * @brief Returns a span covering the data that has been processed so far,
   * represents the successfully serialized bytes.
   *
   * @param stream The stream to evaluate.
   * @return A span of the processed bytes.
   */
  inline span<output_stream::byte> serialized(output_stream& stream)
  {
    return {stream.data(), stream.pos()};
  }

  /**
   * @brief Returns a span covering the remaining available buffer space
   * left to write into.
   *
   * @param stream The stream to evaluate.
   * @return A span of the remaining buffer space.
   */
  inline span<output_stream::byte> remaining(output_stream& stream)
  {
    return {stream.data() + stream.pos(), stream.capacity() - stream.pos()};
  }

    /**
   * @brief Returns a span covering the data that has been processed so far,
   * represents the bytes that have been read.
   *
   * @param stream The stream to evaluate.
   * @return A span of the processed bytes.
   */
  inline span<input_stream::byte> serialized(input_stream& stream)
  {
    return {stream.data(), stream.pos()};
  }

  /**
   * @brief Returns a span covering the remaining unread data remaining in the buffer
   *
   * @param stream The stream to evaluate.
   * @return A span of the remaining buffer space.
   */
  inline span<input_stream::byte> remaining(input_stream& stream)
  {
    return {stream.data() + stream.pos(), stream.capacity() - stream.pos()};
  }
}