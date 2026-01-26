/////////////////////////////////////////////////////////////
//          Copyright Vadym Senkiv 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
/////////////////////////////////////////////////////////////

#pragma once

namespace elib
{
  template <typename Action>
  class scope_exit
  {
  public:
    explicit scope_exit(Action&& action) noexcept
      : m_active{true}
      , m_action(std::forward<Action>(action))
    {
    }

    scope_exit(const scope_exit&) = delete;
    scope_exit& operator=(const scope_exit&) = delete;

    scope_exit(scope_exit&& other) noexcept
      : m_action(std::move(other.m_action))
    {
      other.m_active = false;
    }

    scope_exit& operator=(scope_exit&& other) noexcept
    {
      if (this != &other)
      {
        m_action = std::move(other.m_action);
        other.m_active = false;
      }
      return *this;
    }

    ~scope_exit() noexcept
    {
      if (m_active)
        m_action();
    }

  private:
    bool m_active{false};
    Action m_action;
  };
}