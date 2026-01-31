/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

/**
 * @file event_loop.h
 * @brief A lightweight, static-allocation event loop system for bare-metal embedded C++.
 *
 * This library provides a type-safe Producer-Consumer mechanism.
 * Modules can declare their own private EventLoops, which automatically register
 * themselves to a central system registry.
 *
 * Key Features:
 * - **Zero Dynamic Allocation**: Uses static arrays and circular buffers.
 * - **Automatic Registration**: EventLoops register on construction and unregister on destruction.
 * - **Move Semantics**: Safe to return EventLoops from factory functions (registry is auto-updated).
 * - **Fair Scheduling**: The central processEventLoops() driver uses a round-robin strategy
 * to ensure no single module starves the system or high-priority timers.
 *
 * Usage Example:
 * @code
 * #include <elib/event_loop.h>
 * #include <variant>
 *
 * // 1. Define your Event types
 * struct MotorEvent { int speed; };
 * struct SensorEvent { float temperature; };
 * * // 2. Define a variant to hold any possible event for this loop
 * using SystemEvent = std::variant<MotorEvent, SensorEvent>;
 *
 * // 3. Declare the loop (Global or inside a Module class)
 * //    Queue size is 16. No heap allocation occurs.
 * elib::EventLoop<SystemEvent, 16> mainLoop;
 *
 * void setup() {
 * // 4. Attach the handler
 * mainLoop.setHandler([](const SystemEvent& e) {
 * std::visit([](auto&& arg) {
 * using T = std::decay_t<decltype(arg)>;
 * if constexpr (std::is_same_v<T, MotorEvent>) {
 * // Handle Motor: arg.speed
 * } else if constexpr (std::is_same_v<T, SensorEvent>) {
 * // Handle Sensor: arg.temperature
 * }
 * }, e);
 * });
 * }
 *
 * // 5. ISR or Application Logic (Producer)
 * void on_sensor_interrupt() {
 * // push() returns false if full. pushOver() overwrites oldest.
 * mainLoop.push(SensorEvent{24.5f}); 
 * }
 *
 * // 6. Super Loop (Consumer)
 * int main() {
 * setup();
 * while (true) {
 * // Processes one active loop per call to allow interleaving
 * // with other system tasks (like timers).
 * elib::event::processEventLoops();
 * }
 * }
 * @endcode
 */

#pragma once

#include <utility>
#include <functional>
#include <elib/config.h>
#include <elib/circular_buffer.h>

namespace elib
{
  namespace event
  {
    namespace impl
    {
      class IEventLoop
      {
      public:
        virtual ~IEventLoop() = default;

        virtual void process() = 0;
      };

      bool registerLoop(IEventLoop& loop);
      void unregisterLoop(IEventLoop& loop);

      void moveLoop(IEventLoop& from, IEventLoop& to);
    }

    enum class ProcessStrategy
    {
      SingleEvent, // one event per call
      AllEvents    // all pending events per call
    };

    void processEventLoops();
  }

  template<typename Event, std::size_t EventQueueSize>
  class EventLoop final : public event::impl::IEventLoop
  {
  public:
    using Handler = std::function<void(const Event&)>;

    EventLoop()
    {
      const bool result = event::impl::registerLoop(*this);

      (void)(result);
      // TODO: handle failed registration: abort? assert?, we don't use exception
      // add valid() method and let user decide what to do?
    }

    ~EventLoop()
    {
      event::impl::unregisterLoop(*this);
    }

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    EventLoop(EventLoop&& other)
      : m_processStrategy{other.m_processStrategy}
      , m_handler{std::move(other.m_handler)}
      , m_events{std::move(other.m_events)}
    {
      event::impl::moveLoop(other, *this);
    }

    EventLoop& operator=(EventLoop&& other)
    {
      if (this != &other)
      {
        event::impl::unregisterLoop(*this);

        m_processStrategy = other.m_processStrategy;
        m_handler = std::move(other.m_handler);
        m_events = std::move(other.m_events);

        event::impl::moveLoop(other, *this);
      }

      return *this;
    }

    void setHandler(Handler handler)
    {
      m_handler = std::move(handler);
    }

    void setProcessStrategy(event::ProcessStrategy strategy)
    {
      m_processStrategy = strategy;
    }

    event::ProcessStrategy processStrategy() const
    {
      return m_processStrategy;
    }

    bool push(const Event& event)
    {
      return m_events.push(event);
    }

    bool push(Event&& event)
    {
      return m_events.push(std::move(event));
    }

    void pushOver(const Event& event)
    {
      m_events.push_over(event);
    }

    void pushOver(Event&& event)
    {
      m_events.push_over(std::move(event));
    }

    void clear()
    {
      m_events.clear();
    }

    void process() override
    {
      if (m_events.empty())
        return;

      switch (m_processStrategy)
      {
      case event::ProcessStrategy::SingleEvent: processSingle(); break;
      case event::ProcessStrategy::AllEvents: processAll(); break;
      }
    }

  private:
    void processSingle()
    {
      Event pending = std::move(m_events.front());
      m_events.pop();

      onEvent(pending);
    }

    void processAll()
    {
      for (int count = event::config::maxEventPerCallNum; count > 0 && !m_events.empty(); --count)
      {
        processSingle();
      }
    }

    void onEvent(const Event& event)
    {
      if (m_handler)
        m_handler(event);
    }

  private:
    event::ProcessStrategy m_processStrategy{event::ProcessStrategy::SingleEvent};
    Handler m_handler;
    circular_buffer<Event, EventQueueSize> m_events;
  };
}