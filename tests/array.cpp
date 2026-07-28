#include <catch2/catch_test_macros.hpp>
#include <elib/array.h>

TEST_CASE("elib::array default construction", "[array]") {
    elib::array<int, 5> arr;

    REQUIRE(arr.size() == 0);
    REQUIRE(arr.capacity() == 5);
    REQUIRE(arr.empty() == true);
}

TEST_CASE("elib::array push_back and size", "[array]") {
    elib::array<int, 5> arr;

    REQUIRE(arr.push_back(10) == true);
    REQUIRE(arr.size() == 1);
    REQUIRE(arr[0] == 10);

    REQUIRE(arr.push_back(20) == true);
    REQUIRE(arr.size() == 2);
    REQUIRE(arr[1] == 20);
}

TEST_CASE("elib::array pop_back", "[array]") {
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

TEST_CASE("elib::array insert", "[array]") {
    elib::array<int, 4> arr;

    arr.push_back(1);
    arr.push_back(3);
    arr.insert(arr.begin() + 1, 2);

    REQUIRE(arr.size() == 3);
    REQUIRE(arr[0] == 1);
    REQUIRE(arr[1] == 2);
    REQUIRE(arr[2] == 3);
}

TEST_CASE("elib::array erase", "[array]") {
    elib::array<int, 4> arr;

    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);

    arr.erase(arr.begin() + 1);

    REQUIRE(arr.size() == 2);
    REQUIRE(arr[0] == 1);
    REQUIRE(arr[1] == 3);
}

TEST_CASE("elib::array at() method", "[array]") {
    elib::array<int, 3> arr;

    arr.push_back(10);
    arr.push_back(20);
    arr.push_back(30);

    REQUIRE(arr.at(0) == 10);
    REQUIRE(arr.at(1) == 20);
    REQUIRE(arr.at(2) == 30);
}

TEST_CASE("elib::array data() method", "[array]") {
    elib::array<int, 3> arr;

    arr.push_back(100);
    arr.push_back(200);

    int* ptr = arr.data();

    REQUIRE(ptr[0] == 100);
    REQUIRE(ptr[1] == 200);
}

TEST_CASE("elib::array full() method", "[array]") {
    elib::array<int, 2> arr;

    REQUIRE(arr.full() == false);

    arr.push_back(1);
    arr.push_back(2);

    REQUIRE(arr.full() == true);
}

TEST_CASE("elib::array erase last", "[array]") {
    elib::array<int, 4> arr;

    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);

    arr.erase(arr.end() - 1);

    REQUIRE(arr.size() == 2);
    REQUIRE(arr[0] == 1);
    REQUIRE(arr[1] == 2);
}

TEST_CASE("elib::array erase first", "[array]") {
    elib::array<int, 4> arr;

    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);

    arr.erase(arr.begin());

    REQUIRE(arr.size() == 2);
    REQUIRE(arr[0] == 2);
    REQUIRE(arr[1] == 3);
}

TEST_CASE("elib::array resize operations", "[array]") {
    SECTION("Resize smaller reduces size but keeps capacity") {
        elib::array<int, 5> arr;
        arr.push_back(10);
        arr.push_back(20);
        arr.push_back(30);

        REQUIRE(arr.resize(1) == true);
        REQUIRE(arr.size() == 1);
        REQUIRE(arr.capacity() == 5);
        REQUIRE(arr[0] == 10);
    }

    SECTION("Resize larger zero-fills new slots") {
        elib::array<int, 5> arr;
        arr.push_back(10);

        REQUIRE(arr.resize(3) == true);
        REQUIRE(arr.size() == 3);
        REQUIRE(arr[0] == 10);
        REQUIRE(arr[1] == 0); // Must be value-initialized (zero-filled)
        REQUIRE(arr[2] == 0); // Must be value-initialized (zero-filled)
    }

    SECTION("Resize to capacity is successful") {
        elib::array<int, 2> arr;

        REQUIRE(arr.resize(2) == true);
        REQUIRE(arr.size() == 2);
        REQUIRE(arr.full() == true);
    }

    SECTION("Resize past capacity fails and does not change size") {
        elib::array<int, 3> arr;
        arr.push_back(42);

        REQUIRE(arr.resize(5) == false);
        REQUIRE(arr.size() == 1); // State remains unchanged on failure
        REQUIRE(arr[0] == 42);
    }

    SECTION("Resize to zero acts as clear") {
        elib::array<int, 4> arr;
        arr.push_back(1);
        arr.push_back(2);

        REQUIRE(arr.resize(0) == true);
        REQUIRE(arr.empty() == true);
        REQUIRE(arr.size() == 0);
    }
}