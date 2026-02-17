/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

namespace elib::time
{
  // NOTE: Pay attention to possible overflows during long intervals with precise clocks
  template<typename Clock>
  class elapsed_timer
  {
  public:
    using clock_type = Clock;
    using duration_type  = typename Clock::duration;

    elapsed_timer()  = default;
    ~elapsed_timer() = default;

    bool isActive() const
    {
      return is_active_;
    }

    void start()
    {
      is_active_   = true;
      start_point_ = Clock::now();
    }

    void stop()
    {
      is_active_   = false;
      start_point_ = {};
    }

    void reset()
    {
      start_point_ = Clock::now();
    }

    duration_type elapsed() const
    {
      return Clock::now() - start_point_;
    }

    bool elapsed(duration_type duration) const
    {
      return elapsed() >= duration;
    }

  private:
    typename Clock::time_point start_point_{};
    bool is_active_{};
  };
}
