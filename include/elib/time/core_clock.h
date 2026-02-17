/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

#include <chrono>

namespace elib::time
{
  // NOTE: core_clock is suitable for measuring relevantly small
  // time intervals because of its high precision (tick frequency)
  //
  // Pay attention to overflows, for example:
  // with coreFrequency = 170,000,000 32-bit counter will overflow
  // in ~25.26 seconds, so if you need to measure bigger intervals
  // of time choose less precise clock
  //
  // CycleCounter interface requirement:
  //
  //   core_clock::rep CycleCounter::get() noexpect
  //   {
  //     return your_current_cycle_counter_value;
  //   }

  template<std::size_t clockFrequency, typename CycleCounter>
  class core_clock
  {
  public:
    using period                    = std::ratio<1, clockFrequency>;
    using rep                       = std::uint32_t;
    using duration                  = std::chrono::duration<rep, period>;
    using time_point                = std::chrono::time_point<core_clock>;
    static constexpr bool is_steady = true;

    struct duration_from
    {
      time_point start;
      duration interval;
    };

    static time_point now() noexcept
    {
      return time_point{duration{CycleCounter::get()}};
    }

    static duration_from durationFromNow(duration duration)
    {
      return duration_from{now(), duration};
    }

    static bool hasPassed(const duration_from& duration)
    {
      return (now() - duration.start) >= duration.interval;
    }

    static bool hasPassed(const time_point& start, rep ticks)
    {
      return (now() - start).count() >= ticks;
    }

    static void delay(duration duration)
    {
      const auto delay = durationFromNow(duration);

      while (!hasPassed(delay));
    }

    static void delay(rep ticks)
    {
      delay(duration{ticks});
    }
  };
}