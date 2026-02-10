#pragma once

#include <elib/kernel.h>
#include <elib/assert.h>

namespace elib
{
  /**
   * @brief Standard Task.
   * Automatically registers itself on construction and unregisters on destruction.
   */
  class Task : public kernel::ITask
  {
  public:
    Task();
    virtual ~Task() override;

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other);
    Task& operator=(Task&& other);
  };

  /**
   * @brief Manual Task.
   * Does NOT register on construction. Must be manually started and stopped.
   * Useful for transient tasks or tasks that shouldn't run immediately at startup.
   */
  class ManualTask : public kernel::ITask
  {
  public:
    ManualTask() = default;
    virtual ~ManualTask() override;

    ManualTask(const ManualTask&)            = delete;
    ManualTask& operator=(const ManualTask&) = delete;

    ManualTask(ManualTask&& other);
    ManualTask& operator=(ManualTask&& other);

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
  class GenericTask : public ManualTask
  {
  public:
    /**
     * @brief Constructs the adapter task.
     * @param handler Reference to the object to adapt.
     * @param autoStart If true (default), attempts to register immediately.
     */
    GenericTask(Handler& handler, bool autoStart = true)
      : m_handler{handler}
    {
      if (autoStart)
      {
        [[maybe_unused]] const bool result = start();

        ELIB_ASSERT(result, "elib::GenericTask: unable to start task! Try to increase maximum number of active tasks.");
      }
    }

    virtual void run() override
    {
      m_handler.process();
    }

  private:
    Handler& m_handler;
  };
}