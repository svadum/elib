#include <elib/time/timer.h>

#include <array>
#include <numeric>
#include <utility>

#include <elib/assert.h>
#include <elib/time/system_clock.h>
#include <elib/time/elapsed_timer.h>

namespace elib::time
{
  constexpr timer::id_type empty_timer_id = std::numeric_limits<timer::id_type>::max();

  struct timer_handle
  {
    bool registered{false};
    bool active{false};
    bool single_shot{false};
    config::time_interval interval{0};
    elapsed_timer<timer::clock> elapsed;
    timer::on_timeout callback;
  };

  std::array<timer_handle, config::max_timer_num> timers{};

  timer_handle &get_handle(std::size_t index)
  {
    static timer_handle invalid{};

    ELIB_ASSERT_DEBUG(index < timers.size(), "elib::time::Timer: timer ID out of bounds!");

    if (index >= timers.size())
    {
      invalid.registered = false;
      return invalid;
    }

    return timers[index];
  }

  timer::id_type registerTimerImpl(config::time_interval interval, timer::on_timeout callback, bool singleShot = false)
  {
    // NOTE: make sure timer::id_type type can represent entire timers range
    static_assert(config::max_timer_num <= std::numeric_limits<timer::id_type>::max());

    for (std::size_t index{0}; index < timers.size(); index++)
    {
      timer_handle &timer = timers[index];
      if (!timer.registered)
      {
        timer.callback = std::move(callback);
        timer.interval = interval;
        timer.single_shot = singleShot;
        timer.registered = true;

        return static_cast<timer::id_type>(index);
      }
    }

    ELIB_ASSERT_DEBUG(false, "elib::time::timer: maximum active timer number overflow! Increase elib::config::max_timer_num");

    return empty_timer_id;
  }

  timer timer::register_timer(config::time_interval interval, on_timeout callback)
  {
    const timer::id_type tid = registerTimerImpl(interval, std::move(callback));

    return timer{tid};
  }

  bool timer::single_shot(config::time_interval interval, on_timeout callback)
  {
    const timer::id_type tid = registerTimerImpl(interval, std::move(callback), true);
    const bool success = tid != empty_timer_id;

    if (success)
    {
      get_handle(tid).active = true;
    }

    return success;
  }

  void timer::process_timers()
  {
    static std::size_t curr_index = 0;
    const std::size_t init_index = curr_index;

    do
    {
      timer_handle &curr_timer = timers[curr_index];

      // advance current index
      if (++curr_index >= timers.size())
      {
        curr_index = 0;
      }

      if (curr_timer.registered &&
          curr_timer.callback &&
          curr_timer.active &&
          curr_timer.elapsed.elapsed(curr_timer.interval))
      {
        curr_timer.callback();
        curr_timer.elapsed.reset();

        if (curr_timer.single_shot)
        {
          curr_timer.registered = false;
          curr_timer.active = false;
        }
        return;
      }
    } while (curr_index != init_index);
  }

  void timer::unregister_timers()
  {
    for (timer_handle& timer : timers)
      timer = timer_handle{};
  }


  void unregister_timer(timer::id_type id)
  {
    if (id >= timers.size())
      return;

    get_handle(id) = timer_handle{};
  }

  timer::timer()
    : id_{empty_timer_id}
  {

  }

  timer::timer(id_type id)
    : id_{id}
  {

  }

  timer::~timer()
  {
    unregister();
  }

  timer::timer(timer &&other)
      : id_{std::exchange(other.id_, empty_timer_id)}
  {
  }

  timer &timer::operator=(timer &&other)
  {
    if (this != &other)
    {
      unregister();
      id_ = std::exchange(other.id_, empty_timer_id);
    }

    return *this;
  }

  void timer::set_interval(config::time_interval interval)
  {
    get_handle(id_).interval = interval;
  }

  config::time_interval timer::interval() const
  {
    return get_handle(id_).interval;
  }

  void timer::set_callback(on_timeout callback)
  {
    if (valid())
      get_handle(id_).callback = std::move(callback);
  }

  timer::id_type timer::id() const
  {
    return id_;
  }

  void timer::start()
  {
    timer_handle &thandle = get_handle(id_);
    thandle.active = true;
    thandle.elapsed.start();
  }

  void timer::stop()
  {
    get_handle(id_).active = false;
  }

  bool timer::running() const
  {
    if (valid())
      return get_handle(id_).active;

    return false;
  }

  bool timer::valid() const
  {
    if (id_ != empty_timer_id)
      return get_handle(id_).registered;

    return false;
  }

  void timer::unregister()
  {
    unregister_timer(id_);
    id_ = empty_timer_id;
  }
}