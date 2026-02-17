#pragma once

#include <functional>

namespace elib
{
  class callback
  {
  public:
    template<auto Func, typename Class>
    void connect(Class* instance)
    {
      callback_.instance = instance;
      callback_.func     = [](void* instance) {
        if (!instance)
          return;

        std::invoke(Func, static_cast<Class*>(instance));
      };
    }

    template<void (*Func)()>
    void connect()
    {
      callback_.instance = nullptr;
      callback_.func     = [](void*) {
        Func();
      };
    }

    void operator()()
    {
      if (callback_.func)
      {
        callback_.func(callback_.instance);
      }
    }

  private:
    struct ErasedCallback
    {
      using Func = void (*)(void*);

      void* instance{nullptr};
      Func func{nullptr};
    };

    ErasedCallback callback_{};
  };
}
