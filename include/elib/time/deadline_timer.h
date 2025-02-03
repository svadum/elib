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
  class DeadlineTimer
  {
  public:
    using Tick     = typename Clock::rep;
    using Duration = typename Clock::duration;

    DeadlineTimer()  = default;

    DeadlineTimer(Tick deadline)
      : m_deadline{Clock::now() + deadline}
    {
    }

    DeadlineTimer(Duration deadline)
      : m_deadline{Clock::now() + deadline}
    {
    }

    ~DeadlineTimer() = default;


    void setDeadline(Tick Ticks)
    {
      m_deadline = Clock::now() + Ticks;
    }

    void setDeadline(Duration deadline)
    {
      m_deadline = Clock::now() + deadline;
    }

    bool hasExpired() const
    {
      return m_deadline <= Clock::now();
    }

    Duration remainingTime() const
    {
      return m_deadline - Clock::now();
    }

  private:
    typename Clock::time_point m_deadline;
  };
}

