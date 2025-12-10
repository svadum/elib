/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

namespace elib
{
  // helper for a vistior pattern
  template<class... Ts>
  struct overloaded : Ts... {
    using Ts::operator()...;
  };

  template<class... Ts>
  overloaded(Ts...) -> overloaded<Ts...>;
}