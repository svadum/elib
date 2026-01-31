#include <elib/event_loop.h>
#include <elib/config.h>
#include <array>

namespace elib::event
{
  namespace impl
  {
    std::array<IEventLoop*, event::config::maxEventLoopNum> eventLoops{};

    bool registerLoop(IEventLoop& loop)
    {
      for (auto& slot : eventLoops)
      {
        if (slot == nullptr)
        {
          slot = &loop;
          return true;
        }
      }

      return false;
    }

    void unregisterLoop(IEventLoop& loop)
    {
      for (auto& slot : eventLoops)
      {
        if (slot == &loop)
        {
          slot = nullptr;
          return;
        }
      }
    }

    void moveLoop(IEventLoop& from, IEventLoop& to)
    {
      for (auto& slot : eventLoops)
      {
        if (slot == &from)
        {
          slot = &to;
          return;
        }
      }
    }
  }

  void processEventLoops()
  {
    static std::size_t currentIndex = 0;
    const std::size_t initialIndex = currentIndex;

    do
    {
      auto& eventLoop = impl::eventLoops[currentIndex];

      // advance current index
      if (++currentIndex >= impl::eventLoops.size())
      {
        currentIndex = 0;
      }

      if (eventLoop)
      {
        return eventLoop->process();
      }
    } while (currentIndex != initialIndex);
  }
}