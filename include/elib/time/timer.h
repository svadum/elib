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
  class timer
  {
  public:
    using clock = system_clock;
    using id_type = std::uint32_t;
    using on_timeout = std::function<void()>;

    static timer register_timer(config::time_interval interval, on_timeout callback);
    static bool single_shot(config::time_interval interval, on_timeout callback);
    static void process_timers();
    static void unregister_timers();

    timer();
    ~timer();

    timer(const timer &) = delete;
    timer &operator=(const timer &) = delete;

    timer(timer &&other);
    timer &operator=(timer &&other);

    void set_interval(config::time_interval interval);
    config::time_interval interval() const;

    void set_callback(on_timeout callback);

    id_type id() const;

    void start();
    void stop();

    void unregister();

    bool running() const;
    bool valid() const;

  private:
    explicit timer(id_type id);

    id_type id_;
  };
}