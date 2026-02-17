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
 * - **Automatic Registration**: Inherits from elib::task; registers on construction, unregisters on destruction.
 * - **Move Semantics**: Safe to return EventLoops from factory functions (Kernel registry is auto-updated).
 * - **Tunable Scheduling**: Supports processing single events (latency focus) or event bursts (throughput focus).
 *
 * Usage Example:
 * @code
 * #include <elib/event_loop.h>
 * * struct motor_event { int speed; };
 * * void setup() {
 * static elib::event_loop<motor_event, 16> motor_loop;
 * * // 1. Configure Logic
 * motor_loop.set_handler([](const MotorEvent& e) {
 * hal::set_pwm(e.speed);
 * });
 *
 * // 2. Optional: Allow processing up to 5 events per kernel tick to clear bursts
 * motorLoop.set_max_events_per_call(5);
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
  class event_loop final : public elib::task
  {
  public:
    using event_type = Event;
    using handler_type = std::function<void(const Event&)>;

    /**
     * @brief Constructs an EventLoop and registers it with the elib::kernel.
     * @note If the Kernel registry is full, this will trigger an ELIB_ASSERT (if enabled).
     */
    event_loop() = default;

    // --------------------------------------------------------
    // Move Semantics
    // --------------------------------------------------------
    
    /**
     * @brief Move Constructor.
     * Transfers ownership of the event queue and handler.
     * Automatically updates the Kernel registry to point to the new object address.
     */
    event_loop(event_loop&& other) noexcept 
      : task(std::move(other))
      , handler_(std::move(other.handler_))
      , max_events_per_call_(other.max_events_per_call_)
      , events_(std::move(other.events_))
    {
    }

    /**
     * @brief Move Assignment Operator.
     * Unregisters the current object, moves data from 'other', and patches the Kernel registry.
     */
    event_loop& operator=(event_loop&& other) noexcept
    {
      if (this != &other)
      {
        task::operator=(std::move(other));
        handler_ = std::move(other.handler_);
        max_events_per_call_ = other.max_events_per_call_;
        events_ = std::move(other.events_);
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
    void set_handler(handler_type handler)
    {
      handler_ = handler ? std::move(handler) : empty_handler;
    }

    /**
     * @brief Configures the "Burst Mode" for this loop.
     * * Controls how many events are popped and processed during a single execution
     * slice of the Kernel.
     * * @param count Number of events [1, config::max_events_per_call].
     * - **1 (Default)**: Best for system responsiveness (fair sharing).
     * - **>1**: Best for clearing backlogs quickly, but consumes more CPU time per slice.
     * * @note The value is automatically clamped to the global safety limit.
     */
    void set_max_events_per_call(std::size_t count)
    {
      max_events_per_call_ = std::clamp(count, std::size_t{1}, event::config::max_event_per_call_num);
    }

    /**
     * @brief Returns the configured maximum number of events processed per call.
     */
    std::size_t max_events_per_call() const
    {
      return max_events_per_call_;
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
      return events_.push(event);
    }

    /**
     * @brief Pushes an event into the queue (Move semantics).
     * @return true if successful, false if the queue is full.
     */
    bool push(Event&& event)
    {
      return events_.push(std::move(event));
    }

    /**
     * @brief Pushes an event, overwriting the oldest one if the queue is full.
     */
    void push_over(const Event& event)
    {
      events_.push_over(event);
    }

    /**
     * @brief Pushes an event, overwriting the oldest one if the queue is full (Move semantics).
     */
    void push_over(Event&& event)
    {
      events_.push_over(std::move(event));
    }

    /**
     * @brief Discards all pending events in the queue.
     */
    void clear()
    {
      events_.clear();
    }

    bool empty() const { return events_.empty(); }
    bool full() const { return events_.full(); }
    std::size_t size() const { return events_.size(); }
    std::size_t capacity() const { return events_.capacity(); }

    // --------------------------------------------------------
    // ITask Interface Implementation
    // --------------------------------------------------------
    
    /**
     * @brief Kernel entry point.
     * Processes up to `maxEventsPerCall()` events from the queue.
     * @note Users should not call this manually; let `elib::kernel::process_all()` drive it.
     */
    void run() override
    {
      if (events_.empty())
        return;

      for (std::size_t count = max_events_per_call_; count > 0 && !events_.empty(); --count)
      {
        Event pending = std::move(events_.front());
        events_.pop();

        handler_(pending);
      }
    }

  private:
    static void empty_handler(const Event&) {}

    handler_type handler_{empty_handler};
    std::size_t max_events_per_call_{1}; 
    circular_buffer<Event, EventQueueSize> events_;
  };
}