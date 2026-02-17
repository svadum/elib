#include <elib/task.h>

namespace elib
{
  task::task()
  {
    [[maybe_unused]] const bool result = kernel::register_task(*this);

    ELIB_ASSERT(result, "elib::task: unable to register task! Try to increase maximum number of active tasks.");
  }

  task::~task()
  {
    kernel::unregister_task(*this);
  }

  task::task(task&& other)
  {
    kernel::impl::move_task(other, *this);
  }

  task& task::operator=(task&& other)
  {
    if (this != &other)
    {
      kernel::unregister_task(*this);
      kernel::impl::move_task(other, *this);
    }

    return *this;
  }


  manual_task::~manual_task()
  {
    kernel::unregister_task(*this);
  }

  manual_task::manual_task(manual_task&& other)
  {
    kernel::impl::move_task(other, *this);
  }

  manual_task& manual_task::operator=(manual_task&& other)
  {
    if (this != &other)
    {
      kernel::unregister_task(*this);
      kernel::impl::move_task(other, *this);
    }

    return *this;
  }

  bool manual_task::start()
  {
    const bool result = kernel::register_task(*this);

    return result;
  }

  void manual_task::stop()
  {
    kernel::unregister_task(*this);
  }
}