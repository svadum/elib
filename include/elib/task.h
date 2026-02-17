#pragma once

#include <elib/kernel.h>
#include <elib/assert.h>

namespace elib
{
  /**
   * @brief Standard Task.
   * Automatically registers itself on construction and unregisters on destruction.
   */
  class task : public kernel::task_base
  {
  public:
    task();
    virtual ~task() override;

    task(const task&) = delete;
    task& operator=(const task&) = delete;

    task(task&& other);
    task& operator=(task&& other);
  };

  /**
   * @brief Manual Task.
   * Does NOT register on construction. Must be manually started and stopped.
   * Useful for transient tasks or tasks that shouldn't run immediately at startup.
   */
  class manual_task : public kernel::task_base
  {
  public:
    manual_task() = default;
    virtual ~manual_task() override;

    manual_task(const manual_task&)            = delete;
    manual_task& operator=(const manual_task&) = delete;

    manual_task(manual_task&& other);
    manual_task& operator=(manual_task&& other);

    /**
     * @brief Registers the task with the kernel.
     * @return true if successful, false if registry is full.
     */
    bool start();

    /**
     * @brief Unregisters the task from the kernel.
     */
    void stop();
  };

  /**
   * @brief Generic Adapter Task.
   * Wraps any class that has a `process()` method into an elib::Task.
   *
   * @tparam Handler A class type that implements `void process()`.
   */
  template<typename Handler>
  class generic_task : public manual_task
  {
  public:
    /**
     * @brief Constructs the adapter task.
     * @param handler Reference to the object to adapt.
     * @param autoStart If true (default), attempts to register immediately.
     */
    generic_task(Handler& handler, bool autoStart = true)
      : handler_{handler}
    {
      if (autoStart)
      {
        [[maybe_unused]] const bool result = start();

        ELIB_ASSERT(result, "elib::GenericTask: unable to start task! Try to increase maximum number of active tasks.");
      }
    }

    virtual void run() override
    {
      handler_.process();
    }

  private:
    Handler& handler_;
  };
}