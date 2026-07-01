/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

#include <utility>

namespace elib
{
  template <typename Range, typename Predicate, typename Action>
  void find_if_and_do(Range&& range, Predicate pred, Action action)
  {
    for (auto&& element : range)
    {
      if (pred(element))
      {
        action(std::forward<decltype(element)>(element));
        return;
      }
    }
  }
}