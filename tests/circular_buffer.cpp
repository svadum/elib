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

    REQUIRE(buf.push_back(1));
    REQUIRE(buf.push_back(2));
    REQUIRE(buf.push_back(3));

    REQUIRE(buf.size() == 3);
    REQUIRE(buf.front() == 1);
    REQUIRE(buf.back() == 3);

    REQUIRE(buf.pop_front());
    REQUIRE(buf.front() == 2);
    REQUIRE(buf.size() == 2);
}

TEST_CASE("elib::circular_buffer: Push until full", "[circular_buffer]") {
    elib::circular_buffer<int, 3> buf;

    REQUIRE(buf.push_back(10));
    REQUIRE(buf.push_back(20));
    REQUIRE(buf.push_back(30));

    REQUIRE(buf.full());
    REQUIRE_FALSE(buf.push_back(40)); // should fail

    REQUIRE(buf.front() == 10);
    REQUIRE(buf.back() == 30);
}

TEST_CASE("elib::circular_buffer: Push over old elements", "[circular_buffer]") {
    elib::circular_buffer<int, 3> buf;

    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    REQUIRE(buf.full());

    buf.push_over(4); // should overwrite oldest element
    REQUIRE(buf.size() == 3);
    REQUIRE(buf.front() == 2);
    REQUIRE(buf.back() == 4);
}

TEST_CASE("elib::circular_buffer: Iterator traversal works", "[circular_buffer]") {
    elib::circular_buffer<int, 5> buf;

    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    std::vector<int> values;
    std::copy(buf.begin(), buf.end(), std::back_inserter(values));

    REQUIRE(values == std::vector<int>{1, 2, 3});
}

TEST_CASE("elib::circular_buffer: Iterator wrap-around works", "[circular_buffer]") {
    elib::circular_buffer<int, 5> buf;

    buf.push_back(10);
    buf.push_back(20);
    buf.push_back(30);
    buf.pop_front(); // remove 10
    buf.pop_front(); // remove 20

    buf.push_back(40);
    buf.push_back(50);
    buf.push_back(60);
    buf.push_back(70);

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
    buf.push_back(1);
    buf.push_back(2);

    elib::circular_buffer<int, 5> buf2 = std::move(buf);

    REQUIRE(buf2.size() == 2);
    REQUIRE(buf2.front() == 1);
    REQUIRE(buf2.back() == 2);
}

TEST_CASE("elib::circular_buffer: std::next works", "[circular_buffer]") {
    elib::circular_buffer<int, 5> buf;
    buf.push_back(100);
    buf.push_back(200);
    buf.push_back(300);

    auto it = std::next(buf.begin(), 1);
    REQUIRE(*it == 200);
}

TEST_CASE("elib::circular_buffer: pop back", "[circular_buffer]") {
    elib::circular_buffer<int, 3> buf;
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    REQUIRE(buf.size() == 3);
    REQUIRE(buf.back() == 3);

    REQUIRE(buf.pop_back());
    REQUIRE(buf.back() == 2);

    REQUIRE(buf.pop_back());
    REQUIRE(buf.back() == 1);

    REQUIRE(buf.pop_back());
    REQUIRE_FALSE(buf.pop_back());
}

TEST_CASE("elib::circular_buffer: push front", "[circular_buffer]") {
    elib::circular_buffer<int, 3> buf;

    REQUIRE(buf.push_front(1));
    REQUIRE(buf.front() == 1);

    REQUIRE(buf.push_front(2));
    REQUIRE(buf.front() == 2);

    REQUIRE(buf.push_front(3));
    REQUIRE(buf.front() == 3);

    REQUIRE(buf.size() == 3);

    buf.pop_front();
    REQUIRE(buf.push_front(4));
    REQUIRE(buf.front() == 4);
}

TEST_CASE("elib::circular_buffer: push front & pop back", "[circular_buffer]") {
    elib::circular_buffer<int, 3> buf;

    REQUIRE(buf.push_front(1));
    REQUIRE(buf.push_front(2));
    REQUIRE(buf.push_front(3));

    REQUIRE(buf.size() == 3);
    REQUIRE(buf.back() == 1);

    REQUIRE(buf.pop_back());
    REQUIRE(buf.back() == 2);
    REQUIRE(buf.size() == 2);

    REQUIRE(buf.pop_back());
    REQUIRE(buf.back() == 3);
    REQUIRE(buf.size() == 1);

    REQUIRE(buf.pop_back());
    REQUIRE(buf.empty());
}

TEST_CASE("elib::circular_buffer: Iterator bidirectional traversal", "[circular_buffer]") {
    elib::circular_buffer<int, 5> buf;
    buf.push_back(10);
    buf.push_back(20);
    buf.push_back(30);

    auto it = buf.end();
    
    REQUIRE(it == buf.end());
    
    --it;
    REQUIRE(*it == 30);
    
    --it;
    REQUIRE(*it == 20);
    
    --it;
    REQUIRE(*it == 10);
    
    REQUIRE(it == buf.begin());
    
    // Going back up
    ++it;
    REQUIRE(*it == 20);
}

TEST_CASE("elib::circular_buffer: Reverse Iteration", "[circular_buffer]") {
    elib::circular_buffer<int, 5> buf;
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    std::vector<int> reverse_values;
    for (auto it = std::make_reverse_iterator(buf.end()); it != std::make_reverse_iterator(buf.begin()); ++it) {
        reverse_values.push_back(*it);
    }

    REQUIRE(reverse_values == std::vector<int>{3, 2, 1});
}

TEST_CASE("elib::circular_buffer: push_front wrap-around", "[circular_buffer]") {
    elib::circular_buffer<int, 3> buf;
    
    REQUIRE(buf.push_front(1)); // [1]
    REQUIRE(buf.push_front(2)); // [2, 1]
    REQUIRE(buf.push_front(3)); // [3, 2, 1]
    
    REQUIRE(buf.full());
    REQUIRE(buf.front() == 3);
    REQUIRE(buf.back() == 1);
    
    // Verify order
    auto it = buf.begin();
    REQUIRE(*it++ == 3);
    REQUIRE(*it++ == 2);
    REQUIRE(*it++ == 1);
    REQUIRE(it == buf.end());
}