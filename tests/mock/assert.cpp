#include "assert.h"

void elib::onError(const char* file, int line, const char* message)
{
  mock::AssertMock::instance().onError(file, line, message);
}