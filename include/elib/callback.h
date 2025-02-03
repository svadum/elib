#pragma once

#include <functional>

namespace elib
{
  class Callback
  {
  public:
    template<auto Func, typename Class>
    void connect(Class* instance)
    {
      m_callback.instance = instance;
      m_callback.func     = [](void* instance) {
        if (!instance)
          return;

        std::invoke(Func, static_cast<Class*>(instance));
      };
    }

    template<void (*Func)()>
    void connect()
    {
      m_callback.instance = nullptr;
      m_callback.func     = [](void*) {
        Func();
      };
    }

    void operator()()
    {
      if (m_callback.func)
      {
        m_callback.func(m_callback.instance);
      }
    }

  private:
    struct ErasedCallback
    {
      using Func = void (*)(void*);

      void* instance{nullptr};
      Func func{nullptr};
    };

    ErasedCallback m_callback{};
  };
}
