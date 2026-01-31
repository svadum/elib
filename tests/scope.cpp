#include <catch2/catch_test_macros.hpp>
#include <elib/scope.h>


TEST_CASE("elib::scope_exit works correctly", "[scope]") {
  bool flag = false;
  {
    elib::scope_exit on_exit([&flag] { flag = true; });
  }
  REQUIRE(flag == true);
}