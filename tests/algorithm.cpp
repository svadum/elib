#include <catch2/catch_test_macros.hpp>
#include <elib/algorithm.h>


TEST_CASE("find_if_and_do basic behavior", "[algorithm]") {
    std::vector<int> numbers = {1, 3, 5, 8, 9};
    int execution_count = 0;

    SECTION("Executes action exactly once when element is found") {
        int found_value = 0;

        elib::find_if_and_do(numbers,
            [](int n) { return n == 5; },
            [&](int n) {
                execution_count++;
                found_value = n;
            }
        );

        REQUIRE(execution_count == 1);
        REQUIRE(found_value == 5);
    }

    SECTION("Does nothing when element is not found") {
        elib::find_if_and_do(numbers,
            [](int n) { return n == 99; },
            [&](int) { execution_count++; }
        );

        REQUIRE(execution_count == 0);
    }
}

TEST_CASE("find_if_and_do stops at the first match and allows mutation", "[algorithm]") {
    std::vector<int> numbers = {1, 4, 6, 8, 9};
    int execution_count = 0;

    // Predicate matches multiple elements (4, 6, 8)
    elib::find_if_and_do(numbers,
        [](int n) { return n % 2 == 0; },
        [&](int& n) {
            execution_count++;
            n = 42; // Mutate the found element
        }
    );

    REQUIRE(execution_count == 1); // Action should only run once
    REQUIRE(numbers[1] == 42);     // The first match was modified
    REQUIRE(numbers[2] == 6);      // Subsequent matches were untouched
}

TEST_CASE("find_if_and_do works with const collections", "[algorithm]") {
    const std::vector<std::string> words = {"apple", "banana", "cherry"};
    bool found_banana = false;

    elib::find_if_and_do(words,
        [](const std::string& s) { return s == "banana"; },
        [&](const std::string&) { found_banana = true; }
    );

    REQUIRE(found_banana == true);
}

TEST_CASE("find_if_and_do successfully handles std::vector<bool> proxies", "[algorithm]") {
    // std::vector<bool> is a notorious edge case because iterators return proxy objects,
    // not real references. This tests our auto&& forwarding logic.
    std::vector<bool> flags = {false, false, true, false};

    elib::find_if_and_do(flags,
        [](bool b) { return b == true; },
        // auto&& correctly binds to the temporary proxy object
        [](auto&& proxy) {
            proxy = false; // Flip it back to false
        }
    );

    REQUIRE(flags[0] == false);
    REQUIRE(flags[1] == false);
    REQUIRE(flags[2] == false); // Successfully mutated via proxy
    REQUIRE(flags[3] == false);
}