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
  template<class T>
  using span = nonstd::span<T>;
}
