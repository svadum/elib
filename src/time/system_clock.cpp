#include "elib/time/system_clock.h"

namespace elib::time
{
  static volatile SystemClock::rep sysClockTick = 0;

  void SystemClock::increment()
  {
    ++sysClockTick;
  }

  void SystemClock::set(rep reps)
  {
    sysClockTick = reps;
  }

  void SystemClock::reset()
  {
    sysClockTick = 0;
  }

  SystemClock::rep SystemClock::ticks() noexcept
  {
    return sysClockTick;
  }

  SystemClock::time_point SystemClock::now() noexcept
  {
    return time_point{duration{sysClockTick}};
  }

  SystemClock::duration_from SystemClock::durationFromNow(duration interval)
  {
    return duration_from{now(), interval};
  }

  bool SystemClock::hasPassed(const duration_from& duration)
  {
    return (now() - duration.start) >= duration.interval;
  }

  bool SystemClock::hasPassed(const time_point& start, rep ticks)
  {
    return (now() - start).count() >= ticks;
  }
}
