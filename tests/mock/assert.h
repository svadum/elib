#pragma once
#include <trompeloeil.hpp>
#include <elib/assert.h>

namespace mock
{
  class assert_mock {
  public:
    static assert_mock& instance()
    {
      static assert_mock assertMock;

      return assertMock;
    }
  
    MAKE_MOCK3(onError, void(const char*, int, const char*));

  private:
    assert_mock() = default;
    assert_mock(assert_mock&) = delete;
    assert_mock& operator=(assert_mock&) = delete;
    assert_mock(assert_mock&&) = delete;
    assert_mock& operator=(assert_mock&&) = delete;
  };
}

namespace elib
{
  void on_error(const char* file, int line, const char* message);
}