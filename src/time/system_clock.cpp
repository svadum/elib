#include <elib/time/system_clock.h>

namespace elib::time
{
  static volatile system_clock::rep sys_clock_tick = 0;

  void system_clock::increment()
  {
    ++sys_clock_tick;
  }

  void system_clock::set(rep reps)
  {
    sys_clock_tick = reps;
  }

  void system_clock::reset()
  {
    sys_clock_tick = 0;
  }

  system_clock::rep system_clock::ticks() noexcept
  {
    return sys_clock_tick;
  }

  system_clock::time_point system_clock::now() noexcept
  {
    return time_point{duration{sys_clock_tick}};
  }

  system_clock::duration_from system_clock::duration_from_now(duration interval)
  {
    return duration_from{now(), interval};
  }

  bool system_clock::has_passed(const duration_from& duration)
  {
    return (now() - duration.start) >= duration.interval;
  }

  bool system_clock::has_passed(const time_point& start, rep ticks)
  {
    return (now() - start).count() >= ticks;
  }
}
