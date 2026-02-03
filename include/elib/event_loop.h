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
#include <elib/task.h>
#include <elib/assert.h>
#include <elib/circular_buffer.h>

namespace elib
{
  namespace event
  {
    enum class ProcessStrategy
    {
      SingleEvent, // one event per call
      AllEvents    // all pending events per call
    };
  }

  template<typename Event, std::size_t EventQueueSize>
  class EventLoop : public Task
  {
  public:
    using Handler = std::function<void(const Event&)>;

    EventLoop() = default;

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    EventLoop(EventLoop&& other)
      : Task(std::move(other))
      , m_processStrategy{other.m_processStrategy}
      , m_handler{std::move(other.m_handler)}
      , m_events{std::move(other.m_events)}
    {

    }

    EventLoop& operator=(EventLoop&& other)
    {
      if (this != &other)
      {
        Task::operator=(std::move(other));

        m_processStrategy = other.m_processStrategy;
        m_handler = std::move(other.m_handler);
        m_events = std::move(other.m_events);
      }

      return *this;
    }

    void setHandler(Handler handler)
    {
      m_handler = handler ? std::move(handler) : emptyHandler;
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

    void run() override
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

      m_handler(pending);
    }

    void processAll()
    {
      for (int count = event::config::maxEventPerCallNum; count > 0 && !m_events.empty(); --count)
      {
        processSingle();
      }
    }

  private:
    static void emptyHandler(const Event&) {}

    event::ProcessStrategy m_processStrategy{event::ProcessStrategy::SingleEvent};
    Handler m_handler{emptyHandler};
    circular_buffer<Event, EventQueueSize> m_events{};
  };
}