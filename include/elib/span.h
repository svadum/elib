/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

#include <elib/external/span-lite/span.hpp>

namespace elib
{
  // NOTE: span-lite will switch to std::span if it's available
  template<class T, size_t Extent = nonstd::dynamic_extent>
  using span = nonstd::span<T, Extent>;
}
