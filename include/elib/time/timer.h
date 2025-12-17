/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <functional>
#include <elib/config.h>
#include <elib/time/system_clock.h>

namespace elib::time
{
  class Timer
  {
  public:
    using Clock = SystemClock;
    using Id = std::uint32_t;
    using OnTimeout = std::function<void()>;

    static Timer registerTimer(config::TimerInterval interval, OnTimeout callback);
    static bool singleShot(config::TimerInterval interval, OnTimeout callback);
    static void processTimers();
    static void unregisterTimers();

    Timer();
    ~Timer();

    Timer(const Timer &) = delete;
    Timer &operator=(const Timer &) = delete;

    Timer(Timer &&other);
    Timer &operator=(Timer &&other);

    void setInterval(config::TimerInterval interval);
    config::TimerInterval interval() const;

    void setCallback(OnTimeout callback);

    Id id() const;

    void start();
    void stop();

    void unregister();

    bool running() const;
    bool valid() const;

  private:
    explicit Timer(Id id);

    Id m_id;
  };
}