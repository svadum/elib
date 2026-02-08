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
 * themselves to the central Kernel registry.
 *
 * Key Features:
 * - **Zero Dynamic Allocation**: Uses static arrays and circular buffers.
 * - **Automatic Registration**: Inherits from elib::Task; registers on construction, unregisters on destruction.
 * - **Move Semantics**: Safe to return EventLoops from factory functions (Kernel registry is auto-updated).
 * - **Tunable Scheduling**: Supports processing single events (latency focus) or event bursts (throughput focus).
 *
 * Usage Example:
 * @code
 * #include <elib/event_loop.h>
 * * struct MotorEvent { int speed; };
 * * void setup() {
 * static elib::EventLoop<MotorEvent, 16> motorLoop;
 * * // 1. Configure Logic
 * motorLoop.setHandler([](const MotorEvent& e) {
 * hal::set_pwm(e.speed);
 * });
 *
 * // 2. Optional: Allow processing up to 5 events per kernel tick to clear bursts
 * motorLoop.setMaxEventsPerCall(5);
 *
 * // 3. Push Event (safe from ISR)
 * motorLoop.push({100});
 * }
 * @endcode
 */

#pragma once

#include <elib/task.h>
#include <elib/circular_buffer.h>
#include <elib/config.h> // for maxEventPerCallNum
#include <functional>
#include <algorithm> // for std::clamp

namespace elib
{
  /**
   * @brief A type-safe event queue and processor.
   * * @tparam Event The type of data to store in the queue.
   * @tparam EventQueueSize The capacity of the internal static buffer.
   */
  template<typename Event, std::size_t EventQueueSize>
  class EventLoop final : public elib::Task
  {
  public:
    using Handler = std::function<void(const Event&)>;

    /**
     * @brief Constructs an EventLoop and registers it with the elib::Kernel.
     * @note If the Kernel registry is full, this will trigger an ELIB_ASSERT (if enabled).
     */
    EventLoop() = default;

    // --------------------------------------------------------
    // Move Semantics
    // --------------------------------------------------------
    
    /**
     * @brief Move Constructor.
     * Transfers ownership of the event queue and handler.
     * Automatically updates the Kernel registry to point to the new object address.
     */
    EventLoop(EventLoop&& other) noexcept 
      : Task(std::move(other))
      , m_handler(std::move(other.m_handler))
      , m_maxEventsPerCall(other.m_maxEventsPerCall)
      , m_events(std::move(other.m_events))
    {
    }

    /**
     * @brief Move Assignment Operator.
     * Unregisters the current object, moves data from 'other', and patches the Kernel registry.
     */
    EventLoop& operator=(EventLoop&& other) noexcept
    {
      if (this != &other)
      {
        Task::operator=(std::move(other));
        m_handler = std::move(other.m_handler);
        m_maxEventsPerCall = other.m_maxEventsPerCall;
        m_events = std::move(other.m_events);
      }
      return *this;
    }

    // --------------------------------------------------------
    // Public API
    // --------------------------------------------------------

    /**
     * @brief Sets the callback to be executed when an event is processed.
     * @param handler The function to call. 
     * @note If `nullptr` (or empty std::function) is passed, a default no-op handler 
     * is set to prevent runtime crashes.
     */
    void setHandler(Handler handler)
    {
      m_handler = handler ? std::move(handler) : emptyHandler;
    }

    /**
     * @brief Configures the "Burst Mode" for this loop.
     * * Controls how many events are popped and processed during a single execution
     * slice of the Kernel.
     * * @param count Number of events [1, config::maxEventPerCallNum].
     * - **1 (Default)**: Best for system responsiveness (fair sharing).
     * - **>1**: Best for clearing backlogs quickly, but consumes more CPU time per slice.
     * * @note The value is automatically clamped to the global safety limit.
     */
    void setMaxEventsPerCall(std::size_t count)
    {
      m_maxEventsPerCall = std::clamp(count, std::size_t{1}, event::config::maxEventPerCallNum);
    }

    /**
     * @brief Returns the configured maximum number of events processed per call.
     */
    std::size_t maxEventsPerCall() const
    {
      return m_maxEventsPerCall;
    }

    // --------------------------------------------------------
    // Queue Access
    // --------------------------------------------------------

    /**
     * @brief Pushes an event into the queue.
     * @return true if successful, false if the queue is full.
     */
    bool push(const Event& event)
    {
      return m_events.push(event);
    }

    /**
     * @brief Pushes an event into the queue (Move semantics).
     * @return true if successful, false if the queue is full.
     */
    bool push(Event&& event)
    {
      return m_events.push(std::move(event));
    }

    /**
     * @brief Pushes an event, overwriting the oldest one if the queue is full.
     */
    void pushOver(const Event& event)
    {
      m_events.push_over(event);
    }

    /**
     * @brief Pushes an event, overwriting the oldest one if the queue is full (Move semantics).
     */
    void pushOver(Event&& event)
    {
      m_events.push_over(std::move(event));
    }

    /**
     * @brief Discards all pending events in the queue.
     */
    void clear()
    {
      m_events.clear();
    }

    bool empty() const { return m_events.empty(); }
    bool full() const { return m_events.full(); }
    std::size_t size() const { return m_events.size(); }
    std::size_t capacity() const { return m_events.capacity(); }

    // --------------------------------------------------------
    // ITask Interface Implementation
    // --------------------------------------------------------
    
    /**
     * @brief Kernel entry point.
     * Processes up to `maxEventsPerCall()` events from the queue.
     * @note Users should not call this manually; let `elib::kernel::processAll()` drive it.
     */
    void run() override
    {
      if (m_events.empty())
        return;

      for (std::size_t count = m_maxEventsPerCall; count > 0 && !m_events.empty(); --count)
      {
        Event pending = std::move(m_events.front());
        m_events.pop();

        m_handler(pending);
      }
    }

  private:
    static void emptyHandler(const Event&) {}

    Handler m_handler{emptyHandler};
    std::size_t m_maxEventsPerCall{1}; 
    circular_buffer<Event, EventQueueSize> m_events;
  };
}