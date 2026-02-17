/**
 * @file kernel.h
 * @brief Cooperative round-robin scheduler (Kernel) for bare-metal systems.
 *
 * The Kernel is the heart of the library's execution model. It maintains a
 * central registry of all active "Schedulable Entities" (Tasks) and distributes
 * CPU time among them fairly.
 *
 * ## Architecture
 * - **task_base**: The raw interface for any object that executes code.
 * - **task**: The RAII base class that automatically registers/unregisters with the kernel.
 * - **event_loop**: A specialized Task that processes asynchronous events.
 *
 * ## Scheduling Policy
 * The kernel uses a **Cooperative Round-Robin** strategy:
 * 1. **High Priority**: System Timers are processed first in every `process_all()` cycle.
 * 2. **Normal Priority**: Tasks are executed sequentially. `process_all()` runs
 * EXACTLY ONE task slice per call to ensure responsiveness.
 *
 * ## Usage Example
 * @code
 * // 1. Define a custom polling task
 * class led_blinker : public elib::task {
 * void run() override {
 * // Non-blocking logic runs here
 * if (timer_expired) toggle_led();
 * }
 * };
 *
 * // 2. Instantiate objects (automatically registered)
 * led_blinker blinker;
 * elib::event_loop<int, 4> loop;
 *
 * // 3. Super Loop
 * int main() {
 * setup();
 * while (true) {
 * // Drives Timers -> Blinker -> Loop -> Timers -> ...
 * elib::kernel::process_all();
 * }
 * }
 * @endcode
 */

#pragma once

#include <cstddef>

namespace elib::kernel
{
  /**
   * @brief Abstract interface for any executable unit in the system.
   * @note Users generally inherit from elib::Task, not ITask directly.
   */
  class task_base
  {
  public:
    virtual ~task_base() = default;

    /**
     * @brief The execution body of the task.
     * @warning Must be non-blocking. Any delay here stalls the entire system.
     */
    virtual void run() = 0;
  };

  /**
   * @brief Registers a task object in the global execution registry.
   * @param task Reference to the task.
   * @return true if registered successfully, false if registry is full.
   */
  bool register_task(task_base& task);

  /**
   * @brief Removes a task from the global execution registry.
   * @note Safe to call even if the task was not currently registered.
   */
  void unregister_task(task_base& task);

  /**
   * @brief Returns the maximum number of concurrent tasks allowed.
   * Defined by elib::kernel::config::maxTaskNum + elib::event::config::maxEventLoopNum.
   */
  std::size_t task_max_num();

  /**
   * @brief Manually runs the next task in the round-robin sequence.
   * @note Generally called internally by processAll().
   */
  void process_tasks();

  /**
   * @brief The Master System Driver.
   *
   * This function should be called continuously in the application's main loop.
   * It performs the following steps:
   * 1. Updates System Timers (elib::time::processTimers).
   * 2. Runs the next available Task in the registry.
   */
  void process_all();

  namespace impl
  {
    /**
     * @brief Internal helper to update the registry when a Task object moves in memory.
     * @param from The old memory address.
     * @param to The new memory address.
     */
    void move_task(task_base& from, task_base& to);
  }
}