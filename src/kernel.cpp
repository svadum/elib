#include <elib/kernel.h>
#include <elib/config.h>
#include <elib/time/timer.h>
#include <array>
#include <algorithm>

namespace elib::kernel
{
  std::array<task_base*, kernel::config::max_task_num + event::config::max_event_loop_num> tasks{};

  bool register_task(task_base& task)
  {
    auto it = std::find(tasks.begin(), tasks.end(), &task);
    if (it != tasks.end())
      return true; // task already registered, duplicates are not allowed

    // find empty slot
    for (auto& slot : tasks)
    {
      if (slot == nullptr)
      {
        slot = &task;
        return true;
      }
    }

    return false;
  }

  void unregister_task(task_base& task)
  {
    for (auto& slot : tasks)
    {
      if (slot == &task)
      {
        slot = nullptr;
        return;
      }
    }
  }

  std::size_t task_max_num()
  {
    return tasks.max_size();
  }
  
  void impl::move_task(task_base& from, task_base& to)
  {
    for (auto& slot : tasks)
    {
      if (slot == &from)
      {
        slot = &to;
        return;
      }
    }
  }

  void process_tasks()
  {
    static std::size_t current_index = 0;
    const std::size_t initial_index = current_index;

    do
    {
      auto& task = tasks[current_index];

      // advance current index
      if (++current_index >= tasks.size())
      {
        current_index = 0;
      }

      if (task)
      {
        return task->run();
      }
    } while (current_index != initial_index);
  }

  void process_all()
  {
    elib::time::timer::process_timers();

    process_tasks();
  }
}