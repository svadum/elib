#include <catch2/catch_test_macros.hpp>
#include <elib/array.h>

TEST_CASE("array default construction", "[array]") {
    elib::array<int, 5> arr;

    REQUIRE(arr.size() == 0);
    REQUIRE(arr.capacity() == 5);
    REQUIRE(arr.empty() == true);
}

TEST_CASE("array push_back and size", "[array]") {
    elib::array<int, 5> arr;

    REQUIRE(arr.push_back(10) == true);
    REQUIRE(arr.size() == 1);
    REQUIRE(arr[0] == 10);

    REQUIRE(arr.push_back(20) == true);
    REQUIRE(arr.size() == 2);
    REQUIRE(arr[1] == 20);
}

TEST_CASE("array pop_back", "[array]") {
    elib::array<int, 3> arr;

    arr.push_back(5);
    arr.push_back(15);
    arr.push_back(25);

    REQUIRE(arr.pop_back() == true);
    REQUIRE(arr.size() == 2);
    REQUIRE(arr.back() == 15);

    REQUIRE(arr.pop_back() == true);
    REQUIRE(arr.pop_back() == true);
    REQUIRE(arr.pop_back() == false);
    REQUIRE(arr.empty() == true);
}

TEST_CASE("array insert", "[array]") {
    elib::array<int, 4> arr;

    arr.push_back(1);
    arr.push_back(3);
    arr.insert(arr.begin() + 1, 2);

    REQUIRE(arr.size() == 3);
    REQUIRE(arr[0] == 1);
    REQUIRE(arr[1] == 2);
    REQUIRE(arr[2] == 3);
}

TEST_CASE("array erase", "[array]") {
    elib::array<int, 4> arr;

    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);

    arr.erase(arr.begin() + 1);

    REQUIRE(arr.size() == 2);
    REQUIRE(arr[0] == 1);
    REQUIRE(arr[1] == 3);
}

TEST_CASE("array at() method", "[array]") {
    elib::array<int, 3> arr;

    arr.push_back(10);
    arr.push_back(20);
    arr.push_back(30);

    REQUIRE(arr.at(0) == 10);
    REQUIRE(arr.at(1) == 20);
    REQUIRE(arr.at(2) == 30);
}

TEST_CASE("array data() method", "[array]") {
    elib::array<int, 3> arr;

    arr.push_back(100);
    arr.push_back(200);

    int* ptr = arr.data();

    REQUIRE(ptr[0] == 100);
    REQUIRE(ptr[1] == 200);
}

TEST_CASE("array full() method", "[array]") {
    elib::array<int, 2> arr;

    REQUIRE(arr.full() == false);

    arr.push_back(1);
    arr.push_back(2);

    REQUIRE(arr.full() == true);
}

TEST_CASE("array erase last", "[array]") {
    elib::array<int, 4> arr;

    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);

    arr.erase(arr.end() - 1);

    REQUIRE(arr.size() == 2);
    REQUIRE(arr[0] == 1);
    REQUIRE(arr[1] == 2);
}

TEST_CASE("array erase first", "[array]") {
    elib::array<int, 4> arr;

    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);

    arr.erase(arr.begin());

    REQUIRE(arr.size() == 2);
    REQUIRE(arr[0] == 2);
    REQUIRE(arr[1] == 3);
}