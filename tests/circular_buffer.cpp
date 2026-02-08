#include <catch2/catch_test_macros.hpp>
#include <elib/circular_buffer.h>
#include <algorithm>
#include <iterator>
#include <vector>

TEST_CASE("elib::circular_buffer: Default construction", "[circular_buffer]") {
    elib::circular_buffer<int, 4> buf;

    REQUIRE(buf.empty());
    REQUIRE_FALSE(buf.full());
    REQUIRE(buf.size() == 0);
    REQUIRE(buf.capacity() == 4);
}

TEST_CASE("elib::circular_buffer: Push and pop elements", "[circular_buffer]") {
    elib::circular_buffer<int, 4> buf;

    REQUIRE(buf.push(1));
    REQUIRE(buf.push(2));
    REQUIRE(buf.push(3));

    REQUIRE(buf.size() == 3);
    REQUIRE(buf.front() == 1);
    REQUIRE(buf.back() == 3);

    REQUIRE(buf.pop());
    REQUIRE(buf.front() == 2);
    REQUIRE(buf.size() == 2);
}

TEST_CASE("elib::circular_buffer: Push until full", "[circular_buffer]") {
    elib::circular_buffer<int, 3> buf;

    REQUIRE(buf.push(10));
    REQUIRE(buf.push(20));
    REQUIRE(buf.push(30));

    REQUIRE(buf.full());
    REQUIRE_FALSE(buf.push(40)); // should fail

    REQUIRE(buf.front() == 10);
    REQUIRE(buf.back() == 30);
}

TEST_CASE("elib::circular_buffer: Push over old elements", "[circular_buffer]") {
    elib::circular_buffer<int, 3> buf;

    buf.push(1);
    buf.push(2);
    buf.push(3);

    REQUIRE(buf.full());

    buf.push_over(4); // should overwrite oldest element
    REQUIRE(buf.size() == 3);
    REQUIRE(buf.front() == 2);
    REQUIRE(buf.back() == 4);
}

TEST_CASE("elib::circular_buffer: Iterator traversal works", "[circular_buffer]") {
    elib::circular_buffer<int, 5> buf;

    buf.push(1);
    buf.push(2);
    buf.push(3);

    std::vector<int> values;
    std::copy(buf.begin(), buf.end(), std::back_inserter(values));

    REQUIRE(values == std::vector<int>{1, 2, 3});
}

TEST_CASE("elib::circular_buffer: Iterator wrap-around works", "[circular_buffer]") {
    elib::circular_buffer<int, 5> buf;

    buf.push(10);
    buf.push(20);
    buf.push(30);
    buf.pop(); // remove 10
    buf.pop(); // remove 20

    buf.push(40);
    buf.push(50);
    buf.push(60);
    buf.push(70);

    std::vector<int> values;
    std::copy(buf.begin(), buf.end(), std::back_inserter(values));

    REQUIRE(values == std::vector<int>{30, 40, 50, 60, 70});
}

TEST_CASE("elib::circular_buffer: Initializer list construction", "[circular_buffer]") {
    elib::circular_buffer<int, 5> buf{1, 2, 3};

    REQUIRE(buf.size() == 3);
    REQUIRE(buf.front() == 1);
    REQUIRE(buf.back() == 3);
}

TEST_CASE("elib::circular_buffer: Array construction", "[circular_buffer]") {
    int arr[] = {10, 20, 30};
    elib::circular_buffer<int, 5> buf(arr);

    REQUIRE(buf.size() == 3);
    REQUIRE(buf.front() == 10);
    REQUIRE(buf.back() == 30);
}

TEST_CASE("elib::circular_buffer: Move semantics", "[circular_buffer]") {
    elib::circular_buffer<int, 5> buf;
    buf.push(1);
    buf.push(2);

    elib::circular_buffer<int, 5> buf2 = std::move(buf);

    REQUIRE(buf2.size() == 2);
    REQUIRE(buf2.front() == 1);
    REQUIRE(buf2.back() == 2);
}

TEST_CASE("elib::circular_buffer: std::next works", "[circular_buffer]") {
    elib::circular_buffer<int, 5> buf;
    buf.push(100);
    buf.push(200);
    buf.push(300);

    auto it = std::next(buf.begin(), 1);
    REQUIRE(*it == 200);
}
