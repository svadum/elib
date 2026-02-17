/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

namespace elib::time
{
  template<typename Clock>
  class deadline_timer
  {
  public:
    using tick     = typename Clock::rep;
    using duration_type = typename Clock::duration;

    deadline_timer()  = default;

    deadline_timer(tick deadline)
      : deadline_{Clock::now() + deadline}
    {
    }

    deadline_timer(duration_type deadline)
      : deadline_{Clock::now() + deadline}
    {
    }

    ~deadline_timer() = default;


    void setDeadline(tick Ticks)
    {
      deadline_ = Clock::now() + Ticks;
    }

    void setDeadline(duration_type deadline)
    {
      deadline_ = Clock::now() + deadline;
    }

    bool hasExpired() const
    {
      return deadline_ <= Clock::now();
    }

    duration_type remainingTime() const
    {
      return deadline_ - Clock::now();
    }

  private:
    typename Clock::time_point deadline_;
  };
}

