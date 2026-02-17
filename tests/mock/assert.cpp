#include "assert.h"

void elib::on_error(const char* file, int line, const char* message)
{
  mock::assert_mock::instance().onError(file, line, message);
}