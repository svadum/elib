#include <elib/time/timer.h>

#include <array>
#include <numeric>
#include <utility>

#include <elib/time/system_clock.h>
#include <elib/time/elapsed_timer.h>

namespace elib::time
{
  constexpr Timer::Id emptyTimerId = std::numeric_limits<Timer::Id>::max();

  struct TimerHandle
  {
    bool registered{false};
    bool active{false};
    bool singleShot{false};
    config::TimerInterval interval{0};
    ElapsedTimer<Timer::Clock> elapsed;
    Timer::OnTimeout callback;
  };

  using Timers = std::array<TimerHandle, config::maxTimerNum>;
  Timers timers{};

  TimerHandle &getHandle(std::size_t index)
  {
    static TimerHandle invalid{};

    if (index >= timers.size())
    {
      invalid.registered = false;
      return invalid;
    }

    return timers[index];
  }

  Timer::Id registerTimerImpl(config::TimerInterval interval, Timer::OnTimeout callback, bool singleShot = false)
  {
    // NOTE: make sure Timer::Id type can represent entire timers range
    static_assert(config::maxTimerNum <= std::numeric_limits<Timer::Id>::max());

    for (std::size_t index{0}; index < timers.size(); index++)
    {
      TimerHandle &timer = timers[index];
      if (!timer.registered)
      {
        timer.callback = std::move(callback);
        timer.interval = interval;
        timer.singleShot = singleShot;
        timer.registered = true;

        return static_cast<Timer::Id>(index);
      }
    }

    return emptyTimerId;
  }

  Timer Timer::registerTimer(config::TimerInterval interval, OnTimeout callback)
  {
    const Timer::Id tid = registerTimerImpl(interval, std::move(callback));

    return Timer{tid};
  }

  bool Timer::singleShot(config::TimerInterval interval, OnTimeout callback)
  {
    const Timer::Id tid = registerTimerImpl(interval, std::move(callback), true);
    const bool success = tid != emptyTimerId;

    if (success)
    {
      getHandle(tid).active = true;
    }

    return success;
  }

  void Timer::processTimers()
  {
    static std::size_t currentIndex = 0;
    const std::size_t initialIndex = currentIndex;

    do
    {
      TimerHandle &currentTimer = timers[currentIndex];

      if (currentTimer.registered &&
          currentTimer.callback &&
          currentTimer.active &&
          currentTimer.elapsed.elapsed(currentTimer.interval))
      {
        currentTimer.callback();
        currentTimer.elapsed.reset();

        if (currentTimer.singleShot)
        {
          currentTimer.registered = false;
          currentTimer.active = false;
        }
        return;
      }

      if (++currentIndex >= timers.size())
      {
        currentIndex = 0;
      }
    } while (currentIndex != initialIndex);
  }

  void Timer::unregisterTimers()
  {
    for (TimerHandle& timer : timers)
      timer = TimerHandle{};
  }


  void unregisterTimer(Timer::Id id)
  {
    if (id >= timers.size())
      return;

    getHandle(id) = TimerHandle{};
  }

  Timer::Timer()
    : m_id{emptyTimerId}
  {

  }

  Timer::Timer(Id id)
    : m_id{id}
  {

  }

  Timer::~Timer()
  {
    unregister();
  }

  Timer::Timer(Timer &&other)
      : m_id{std::exchange(other.m_id, emptyTimerId)}
  {
  }

  Timer &Timer::operator=(Timer &&other)
  {
    if (this != &other)
    {
      unregister();
      m_id = std::exchange(other.m_id, emptyTimerId);
    }

    return *this;
  }

  void Timer::setInterval(config::TimerInterval interval)
  {
    getHandle(m_id).interval = interval;
  }

  config::TimerInterval Timer::interval() const
  {
    return getHandle(m_id).interval;
  }

  void Timer::setCallback(OnTimeout callback)
  {
    if (valid())
      getHandle(m_id).callback = std::move(callback);
  }

  Timer::Id Timer::id() const
  {
    return m_id;
  }

  void Timer::start()
  {
    TimerHandle &timer = getHandle(m_id);
    timer.active = true;
    timer.elapsed.start();
  }

  void Timer::stop()
  {
    getHandle(m_id).active = false;
  }

  bool Timer::running() const
  {
    if (valid())
      return getHandle(m_id).active;

    return false;
  }

  bool Timer::valid() const
  {
    if (m_id != emptyTimerId)
      return getHandle(m_id).registered;

    return false;
  }

  void Timer::unregister()
  {
    unregisterTimer(m_id);
    m_id = emptyTimerId;
  }
}