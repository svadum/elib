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
  class SystemClock
  {
  public:
    using period                    = config::SystemClockPeriod;
    using rep                       = config::SystemClockRep;
    using duration                  = std::chrono::duration<rep, period>;
    using time_point                = std::chrono::time_point<SystemClock>;
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

    static duration_from durationFromNow(duration interval);

    static bool hasPassed(const duration_from& duration);
    static bool hasPassed(const time_point& start, rep ticks);
  };
}

