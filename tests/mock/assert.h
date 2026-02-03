#pragma once
#include <trompeloeil.hpp>
#include <elib/assert.h>

namespace mock
{
  class AssertMock {
  public:
    static AssertMock& instance()
    {
      static AssertMock assertMock;

      return assertMock;
    }
  
    MAKE_MOCK3(onError, void(const char*, int, const char*));

  private:
    AssertMock() = default;
    AssertMock(AssertMock&) = delete;
    AssertMock& operator=(AssertMock&) = delete;
    AssertMock(AssertMock&&) = delete;
    AssertMock& operator=(AssertMock&&) = delete;
  };
}

namespace elib
{
  void onError(const char* file, int line, const char* message);
}