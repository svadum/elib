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
  class ElapsedTimer
  {
  public:
    using ClockType = Clock;
    using Duration  = typename Clock::duration;

    ElapsedTimer()  = default;
    ~ElapsedTimer() = default;

    bool isActive() const
    {
      return m_isActive;
    }

    void start()
    {
      m_isActive   = true;
      m_startPoint = Clock::now();
    }

    void stop()
    {
      m_isActive   = false;
      m_startPoint = {};
    }

    void reset()
    {
      m_startPoint = Clock::now();
    }

    Duration elapsed() const
    {
      return Clock::now() - m_startPoint;
    }

    bool elapsed(Duration duration) const
    {
      return elapsed() >= duration;
    }

  private:
    typename Clock::time_point m_startPoint{};
    bool m_isActive{};
  };
}
