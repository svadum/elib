/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

#include <elib/external/expected-lite/expected.hpp>

namespace elib
{
  // NOTE: expected-lite will switch to std::expected if it's available
  template<class T, class E>
  using expected = nonstd::expected<T, E>;
}
