#include <elib/task.h>

namespace elib
{
  Task::Task()
  {
    [[maybe_unused]] const bool result = kernel::registerTask(*this);

    ELIB_ASSERT(result, "elib::Task: unable to register task! Try to increase maximum number of active tasks.");
  }

  Task::~Task()
  {
    kernel::unregisterTask(*this);
  }

  Task::Task(Task&& other)
  {
    kernel::impl::moveTask(other, *this);
  }

  Task& Task::operator=(Task&& other)
  {
    if (this != &other)
    {
      kernel::unregisterTask(*this);
      kernel::impl::moveTask(other, *this);
    }

    return *this;
  }


  ManualTask::~ManualTask()
  {
    kernel::unregisterTask(*this);
  }

  ManualTask::ManualTask(ManualTask&& other)
  {
    kernel::impl::moveTask(other, *this);
  }

  ManualTask& ManualTask::operator=(ManualTask&& other)
  {
    if (this != &other)
    {
      kernel::unregisterTask(*this);
      kernel::impl::moveTask(other, *this);
    }

    return *this;
  }

  bool ManualTask::start()
  {
    const bool result = kernel::registerTask(*this);

    return result;
  }

  void ManualTask::stop()
  {
    kernel::unregisterTask(*this);
  }
}