#pragma once

#include <elib/kernel.h>
#include <elib/assert.h>

namespace elib
{
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
}