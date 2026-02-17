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
      : active_{true}
      , action_(std::forward<Action>(action))
    {
    }

    scope_exit(const scope_exit&) = delete;
    scope_exit& operator=(const scope_exit&) = delete;

    scope_exit(scope_exit&& other) noexcept
      : action_(std::move(other.action_))
    {
      other.active_ = false;
    }

    scope_exit& operator=(scope_exit&& other) noexcept
    {
      if (this != &other)
      {
        action_ = std::move(other.action_);
        other.active_ = false;
      }
      return *this;
    }

    ~scope_exit() noexcept
    {
      if (active_)
        action_();
    }

  private:
    bool active_{false};
    Action action_;
  };
}