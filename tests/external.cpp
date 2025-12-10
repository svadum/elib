#include <catch2/catch_test_macros.hpp>

#include <array>

#include <elib/optional.h>
#include <elib/expected.h>
#include <elib/span.h>
#include <elib/function_ref.h>
#include <elib/inplace_function.h>


TEST_CASE("elib::span availble & compiles", "[elib::external]") {
  std::array<int, 10> array{};
  elib::span<int> span(array);

  REQUIRE(span.size() == array.size());
}

TEST_CASE("elib::optional availble & compiles", "[elib::external]") {
  elib::optional<int> optional{};
  REQUIRE_FALSE(optional.has_value());

  optional = 1;
  REQUIRE(optional.has_value());
}

TEST_CASE("elib::expected availble & compiles", "[elib::external]") {
  elib::expected<int, double> expected{1};
  REQUIRE(expected.has_value());
  REQUIRE(expected);

  expected = elib::unexpected<double>{3.14};
  REQUIRE_FALSE(expected);
  REQUIRE_FALSE(expected.has_value());
}

TEST_CASE("elib::function_ref availble & compiles", "[elib::external]") {
  const auto callback = [](int& i) { i++; };

  elib::function_ref<void(int&)> function{callback};

  int number = 0;
  function(number);
  REQUIRE(number == 1);
}

TEST_CASE("elib::inplace_function availble & compiles", "[elib::external]") {
  const auto callback = [](int& i) { i++; };

  elib::inplace_function<void(int&), 4> function{callback};

  int number = 0;
  function(number);
  REQUIRE(number == 1);
}

