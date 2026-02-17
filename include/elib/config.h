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
      using system_clock_rep = std::uint32_t;
      using system_clock_period = std::milli;

      // Timer configuration
      using time_interval = std::chrono::milliseconds;

      inline constexpr std::size_t max_timer_num = 10;     // maximum active registered timers
    }

    using namespace defaults;
  }

  namespace event::config
  {
    inline constexpr std::size_t max_event_loop_num = 10;    // maximum active registered event loops
    inline constexpr std::size_t max_event_per_call_num = 25; // maximum amount of events that can be 
                                                          // processed during at a time
  }

  namespace kernel::config
  {
    inline constexpr std::size_t max_task_num = 10; // maximum active registered tasks
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
