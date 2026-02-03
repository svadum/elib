#include <elib/kernel.h>
#include <elib/config.h>
#include <elib/time/timer.h>
#include <array>

namespace elib::kernel
{
  std::array<ITask*, kernel::config::maxTaskNum + event::config::maxEventLoopNum> tasks{};

  bool registerTask(ITask& task)
  {
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

  void unregisterTask(ITask& task)
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

  std::size_t taskMaxNum()
  {
    return tasks.max_size();
  }
  
  void impl::moveTask(ITask& from, ITask& to)
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

  void processTasks()
  {
    static std::size_t currentIndex = 0;
    const std::size_t initialIndex = currentIndex;

    do
    {
      auto& task = tasks[currentIndex];

      // advance current index
      if (++currentIndex >= tasks.size())
      {
        currentIndex = 0;
      }

      if (task)
      {
        return task->run();
      }
    } while (currentIndex != initialIndex);
  }

  void processAll()
  {
    elib::time::Timer::processTimers();

    processTasks();
  }
}