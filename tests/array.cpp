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

TEST_CASE("elib::array copy and move semantics", "[array]") {
    elib::array<int, 5> arr1{1, 2, 3};

    // Copy Constructor
    elib::array<int, 5> arr2(arr1);
    REQUIRE(arr2.size() == 3);
    REQUIRE(arr2[2] == 3);

    // Copy Assignment
    elib::array<int, 5> arr3;
    arr3 = arr1;
    REQUIRE(arr3.size() == 3);
    REQUIRE(arr3[0] == 1);

    // Move Constructor
    elib::array<int, 5> arr4(std::move(arr1));
    REQUIRE(arr4.size() == 3);
    REQUIRE(arr1.size() == 0); // Refactored version clears size on move

    // Move Assignment
    elib::array<int, 5> arr5;
    arr5 = std::move(arr2);
    REQUIRE(arr5.size() == 3);
    REQUIRE(arr2.size() == 0);
}

TEST_CASE("elib::array advanced constructors", "[array]") {
    SECTION("C-style array constructor") {
        int c_arr[] = {10, 20, 30};
        elib::array<int, 5> arr1(c_arr);
        REQUIRE(arr1.size() == 3);
        REQUIRE(arr1.front() == 10);
        REQUIRE(arr1.back() == 30);
    }

    SECTION("Iterator range constructor") {
        std::array<int, 3> std_arr = {4, 5, 6};
        elib::array<int, 5> arr2(std_arr.begin(), std_arr.end());
        REQUIRE(arr2.size() == 3);
        REQUIRE(arr2[1] == 5);
    }
}

TEST_CASE("elib::array range and initializer list insert", "[array]") {
    elib::array<int, 7> arr{1, 6, 7};

    SECTION("Iterator range insert") {
        std::array<int, 4> elems{2, 3, 4, 5};
        arr.insert(arr.begin() + 1, elems.begin(), elems.end());
        REQUIRE(arr.size() == 7);
        REQUIRE(arr[1] == 2);
        REQUIRE(arr[4] == 5);
        REQUIRE(arr[5] == 6);
    }

    SECTION("Initializer list insert") {
        arr.insert(arr.begin() + 1, {2, 3});
        REQUIRE(arr.size() == 5);
        REQUIRE(arr[1] == 2);
        REQUIRE(arr[2] == 3);
        REQUIRE(arr[3] == 6);
    }
}

TEST_CASE("elib::array iterators and clear", "[array]") {
    elib::array<int, 5> arr{1, 2, 3};

    SECTION("Reverse iterators") {
        REQUIRE(*arr.rbegin() == 3);
        REQUIRE(*(arr.rend() - 1) == 1);

        int sum = 0;
        for(auto it = arr.rbegin(); it != arr.rend(); ++it) {
            sum += *it;
        }
        REQUIRE(sum == 6);
    }

    SECTION("Clear method") {
        arr.clear();
        REQUIRE(arr.empty() == true);
        REQUIRE(arr.size() == 0);
        REQUIRE(arr.capacity() == 5);
    }
}

// Compile-time tests to guarantee the constexpr refactoring works
TEST_CASE("elib::array true constexpr evaluation", "[array]") {
    STATIC_REQUIRE([]() {
        elib::array<int, 5> arr{1, 2, 3};
        auto arr2 = arr; // test constexpr copy
        arr2.push_back(4); // test constexpr mutation
        arr2.pop_back();
        return arr2.size() == 3 && arr2.back() == 3;
    }());
}