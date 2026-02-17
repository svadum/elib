/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

#include <chrono>
#include "elib/config.h"

namespace elib::time
{
  class system_clock
  {
  public:
    using period                    = config::system_clock_period;
    using rep                       = config::system_clock_rep;
    using duration                  = std::chrono::duration<rep, period>;
    using time_point                = std::chrono::time_point<system_clock>;
    static constexpr bool is_steady = false;

    struct duration_from
    {
      time_point start;
      duration interval;
    };

    static void increment();
    static void set(rep reps);
    static void reset();

    static rep ticks() noexcept;

    static time_point now() noexcept;

    static duration_from duration_from_now(duration interval);

    static bool has_passed(const duration_from& duration);
    static bool has_passed(const time_point& start, rep ticks);
  };
}

