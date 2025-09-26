/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

#include <cstdlib>
#include <chrono>

namespace elib
{
  namespace time::config
  {
    namespace defaults
    {
      // System clock configuration (1 ms. clock by default)
      using SystemClockRep = std::uint32_t;
      using SystemClockPeriod = std::milli;

      // Timer configuration
      using TimerInterval = std::chrono::milliseconds;

      inline constexpr std::size_t maxTimerNum = 10; // maximum active registered timers
    }

    using namespace defaults;
  }
}

// Users can override Embedded Library configuration values
// by means of creating a "elib.tweaks.h" file.
//
// User has to:
// 1. Add path of a folder with the "elib.tweaks.h" to the compiler
//    include paths.
//
// NOTE: more details about library tweaks approach here:
//       https://vector-of-bool.github.io/2020/10/04/lib-configuration.html

#if __has_include("elib.tweaks.h")
#include "elib.tweaks.h"
#endif
