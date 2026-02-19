#include <catch2/catch_test_macros.hpp>
#include <elib/list.h>
#include <vector>
#include <numeric> // For std::iota

// Test Object: Cannot be default constructed
struct ComplexObject {
    int id;
    std::string data;
    
    ComplexObject(int i, std::string s) : id(i), data(std::move(s)) {}
    // No default constructor
    
    // Need comparison operator for tests
    bool operator==(const ComplexObject& other) const {
        return id == other.id && data == other.data;
    }
};

TEST_CASE("elib::list: Basic construction and state", "[list]") {
    elib::list<int, 5> l;

    REQUIRE(l.empty());
    REQUIRE_FALSE(l.full());
    REQUIRE(l.size() == 0);
    REQUIRE(l.max_size() == 5);
}

TEST_CASE("elib::list: Push and Pop operations", "[list]") {
    elib::list<int, 3> l;

    SECTION("Push/Pop Back") {
        REQUIRE_FALSE(l.pop_back()); // Pop from empty

        REQUIRE(l.push_back(1));
        REQUIRE(l.back() == 1);
        REQUIRE(l.front() == 1);
        REQUIRE(l.size() == 1);

        REQUIRE(l.push_back(2));
        REQUIRE(l.back() == 2);
        REQUIRE(l.size() == 2);

        REQUIRE(l.pop_back());
        REQUIRE(l.back() == 1);
        REQUIRE(l.size() == 1);

        REQUIRE(l.pop_back());
        REQUIRE(l.empty());
    }

    SECTION("Push/Pop Front") {
        REQUIRE_FALSE(l.pop_front()); // Pop from empty

        REQUIRE(l.push_front(1));
        REQUIRE(l.front() == 1);
        REQUIRE(l.back() == 1);
        REQUIRE(l.size() == 1);

        REQUIRE(l.push_front(2));
        REQUIRE(l.front() == 2);
        REQUIRE(l.size() == 2);

        REQUIRE(l.pop_front());
        REQUIRE(l.front() == 1);
        REQUIRE(l.size() == 1);

        REQUIRE(l.pop_front());
        REQUIRE(l.empty());
    }

    SECTION("Mixed Push/Pop") {
        l.push_back(10);  // [10]
        l.push_front(5);  // [5, 10]
        l.push_back(20);  // [5, 10, 20]
        REQUIRE(l.size() == 3);
        REQUIRE(l.front() == 5);
        REQUIRE(l.back() == 20);

        l.pop_front();    // [10, 20]
        REQUIRE(l.front() == 10);
        l.pop_back();     // [10]
        REQUIRE(l.front() == 10);
        REQUIRE(l.back() == 10);
        REQUIRE(l.size() == 1);
    }
}

TEST_CASE("elib::list: Capacity limits", "[list]") {
    elib::list<int, 2> l;
    
    REQUIRE(l.push_back(1));
    REQUIRE(l.push_back(2));
    REQUIRE(l.size() == 2);
    REQUIRE(l.full());

    // Pushing to a full list should fail
    REQUIRE_FALSE(l.push_back(3));
    REQUIRE_FALSE(l.push_front(3));
    REQUIRE(l.size() == 2); // Size should not change
    REQUIRE(l.front() == 1);
    REQUIRE(l.back() == 2);
}

TEST_CASE("elib::list: Iteration", "[list]") {
    elib::list<int, 4> l;
    l.push_back(10);
    l.push_back(20);
    l.push_back(30);

    SECTION("Forward iteration") {
        std::vector<int> expected = {10, 20, 30};
        int i = 0;
        for (const auto& val : l) {
            REQUIRE(val == expected[i++]);
        }
        REQUIRE(i == 3);
    }

    SECTION("Reverse iteration") {
        std::vector<int> expected = {30, 20, 10};
        int i = 0;
        for (auto it = --l.end();; --it) {
            REQUIRE(*it == expected[i++]);
            if (it == l.begin()) break;
        }
        REQUIRE(i == 3);
    }

    SECTION("Const iteration") {
        const auto& const_l = l;
        std::vector<int> expected = {10, 20, 30};
        
        // Using range-based for loop on const object
        int i = 0;
        for (const auto& val : const_l) {
            REQUIRE(val == expected[i++]);
        }
        REQUIRE(i == 3);

        // Using std::equal with cbegin/cend
        REQUIRE(std::equal(const_l.cbegin(), const_l.cend(), expected.cbegin(), expected.cend()));
    }
}

TEST_CASE("elib::list: `insert` method", "[list]") {
    elib::list<int, 5> l;

    SECTION("Insert into empty list") {
        auto it = l.insert(l.begin(), 10);
        REQUIRE(l.size() == 1);
        REQUIRE(l.front() == 10);
        REQUIRE(*it == 10);
        REQUIRE(it == l.begin());
    }

    SECTION("Insert at the beginning") {
        l.push_back(20);
        l.push_back(30);
        auto it = l.insert(l.begin(), 10);
        REQUIRE(l.size() == 3);
        REQUIRE(l.front() == 10);
        REQUIRE(*it == 10);
        REQUIRE(it == l.begin());
        REQUIRE(*std::next(it) == 20);
    }

    SECTION("Insert at the end") {
        l.push_back(10);
        l.push_back(20);
        auto it = l.insert(l.end(), 30);
        REQUIRE(l.size() == 3);
        REQUIRE(l.back() == 30);
        REQUIRE(*it == 30);

        const bool valid_it = std::next(it) == l.end();
        REQUIRE(valid_it);
    }

    SECTION("Insert in the middle") {
        l.push_back(10);
        l.push_back(30);
        auto it = l.insert(std::next(l.begin()), 20);
        REQUIRE(l.size() == 3);
        REQUIRE(*it == 20);
        REQUIRE(l.front() == 10);
        REQUIRE(l.back() == 30);
        REQUIRE(*std::next(it) == 30);
    }

    SECTION("Insert when full") {
        l.push_back(1);
        l.push_back(2);
        l.push_back(3);
        l.push_back(4);
        l.push_back(5);
        REQUIRE(l.full());
        auto it = l.insert(l.begin(), 99);
        REQUIRE(it == l.end()); // Should fail and return end()
        REQUIRE(l.size() == 5); // Size should not change
    }
}

TEST_CASE("elib::list: `erase` method", "[list]") {
    elib::list<int, 5> l;
    l.push_back(10);
    l.push_back(20);
    l.push_back(30);
    l.push_back(40);

    SECTION("Erase from the middle") {
        auto it = l.erase(std::next(l.begin())); // Erase 20
        REQUIRE(l.size() == 3);
        REQUIRE(*it == 30); // Iterator should now point to 30
        REQUIRE(l.front() == 10);
        REQUIRE(l.back() == 40);
    }

    SECTION("Erase the first element") {
        auto it = l.erase(l.begin()); // Erase 10
        REQUIRE(l.size() == 3);
        REQUIRE(*it == 20); // Iterator should now point to 20
        REQUIRE(it == l.begin());
        REQUIRE(l.front() == 20);
    }

    SECTION("Erase the last element") {
        auto it = l.erase(std::prev(l.end())); // Erase 40
        REQUIRE(l.size() == 3);
        REQUIRE(it == l.end()); // Iterator should now be end()
        REQUIRE(l.back() == 30);
    }

    SECTION("Erase until empty") {
        l.erase(l.begin());
        l.erase(l.begin());
        l.erase(l.begin());
        l.erase(l.begin());
        REQUIRE(l.empty());
        REQUIRE(l.size() == 0);
    }

    SECTION("Erase invalid iterators") {
        auto it = l.erase(l.end()); // Erasing end() is invalid
        REQUIRE(it == l.end());
        REQUIRE(l.size() == 4);

        elib::list<int, 5> empty_list;
        it = empty_list.erase(empty_list.begin()); // Erasing from empty
        REQUIRE(it == empty_list.end());
    }
}

TEST_CASE("elib::list: `clear` method", "[list]") {
    elib::list<int, 5> l;

    SECTION("Clear an empty list") {
        l.clear();
        REQUIRE(l.empty());
        REQUIRE(l.size() == 0);
    }

    SECTION("Clear a full list") {
        l.push_back(1);
        l.push_back(2);
        l.push_back(3);
        l.clear();
        REQUIRE(l.empty());
        REQUIRE(l.size() == 0);
        
        // Should be able to use the list again
        REQUIRE(l.push_back(100));
        REQUIRE(l.size() == 1);
        REQUIRE(l.front() == 100);
    }
}

TEST_CASE("elib::list: Copy and Move semantics", "[list]") {
    elib::list<int, 5> l1;
    l1.push_back(1);
    l1.push_back(2);
    l1.push_back(3);

    SECTION("Copy constructor") {
        elib::list<int, 5> l2 = l1;
        REQUIRE(l2.size() == 3);
        REQUIRE(std::equal(l1.begin(), l1.end(), l2.begin(), l2.end()));

        // Modify copy, original should be unchanged
        l2.pop_back();
        REQUIRE(l2.size() == 2);
        REQUIRE(l1.size() == 3);
    }

    SECTION("Copy assignment") {
        elib::list<int, 5> l2;
        l2.push_back(99);
        l2 = l1; // Assign
        REQUIRE(l2.size() == 3);
        REQUIRE(std::equal(l1.begin(), l1.end(), l2.begin(), l2.end()));

        // Self-assignment
        l1 = l1;
        REQUIRE(l1.size() == 3);
    }

    SECTION("Move constructor") {
        elib::list<int, 5> l2 = std::move(l1);
        REQUIRE(l2.size() == 3);
        REQUIRE(l2.front() == 1);
        REQUIRE(l1.empty()); // Original should be empty
    }

    SECTION("Move assignment") {
        elib::list<int, 5> l2;
        l2.push_back(99);
        l2 = std::move(l1);
        REQUIRE(l2.size() == 3);
        REQUIRE(l2.front() == 1);
        REQUIRE(l1.empty()); // Original should be empty
    }
}

TEST_CASE("elib::list: Complex types (Manual Storage)", "[list]") {
    elib::list<ComplexObject, 3> l;

    // This proves we aren't default constructing the pool
    // (If we were, this wouldn't compile)
    
    l.push_back({1, "One"});
    l.push_back({2, "Two"});

    REQUIRE(l.size() == 2);
    REQUIRE(l.front().data == "One");
    REQUIRE(l.back().data == "Two");

    l.insert(l.begin(), {-1, "Zero"});
    REQUIRE(l.size() == 3);
    REQUIRE(l.front().id == -1);

    l.pop_front();
    REQUIRE(l.front().data == "One");

    l.clear();
    REQUIRE(l.empty());
}

TEST_CASE("elib::list: Additional Constructors", "[list]") {
    SECTION("Initializer list constructor") {
        elib::list<int, 5> l = {10, 20, 30};
        
        REQUIRE(l.size() == 3);
        REQUIRE(l.front() == 10);
        REQUIRE(l.back() == 30);
        
        auto it = l.begin();
        REQUIRE(*it++ == 10);
        REQUIRE(*it++ == 20);
        REQUIRE(*it++ == 30);
        REQUIRE(it == l.end());
    }

    SECTION("Range constructor") {
        std::vector<int> vec = {5, 15, 25, 35};
        elib::list<int, 5> l(vec.begin(), vec.end());
        
        REQUIRE(l.size() == 4);
        REQUIRE(l.front() == 5);
        REQUIRE(l.back() == 35);
    }
}

TEST_CASE("elib::list: Emplace operations", "[list]") {
    // We use ComplexObject to ensure placement-new forwards arguments correctly
    elib::list<ComplexObject, 5> l;

    SECTION("emplace_back") {
        REQUIRE(l.emplace_back(1, "One"));
        REQUIRE(l.size() == 1);
        REQUIRE(l.back().id == 1);
        REQUIRE(l.back().data == "One");

        REQUIRE(l.emplace_back(2, "Two"));
        REQUIRE(l.size() == 2);
        REQUIRE(l.back().id == 2);
        REQUIRE(l.back().data == "Two");
    }

    SECTION("emplace_front") {
        REQUIRE(l.emplace_front(1, "One"));
        REQUIRE(l.size() == 1);
        REQUIRE(l.front().id == 1);
        REQUIRE(l.front().data == "One");
        
        REQUIRE(l.emplace_front(2, "Two"));
        REQUIRE(l.size() == 2);
        REQUIRE(l.front().id == 2);
        REQUIRE(l.front().data == "Two");
        
        // Check order
        REQUIRE(l.back().id == 1);
    }

    SECTION("emplace (insert in the middle)") {
        l.emplace_back(1, "One");
        l.emplace_back(3, "Three");

        // Insert before the second element
        auto it = l.begin();
        ++it; 

        auto emplaced_it = l.emplace(it, 2, "Two");
        
        REQUIRE(emplaced_it != l.end());
        REQUIRE(emplaced_it->id == 2);
        REQUIRE(l.size() == 3);

        // Verify sequence is now 1 -> 2 -> 3
        auto check_it = l.begin();
        REQUIRE(check_it->id == 1);
        ++check_it;
        REQUIRE(check_it->id == 2);
        ++check_it;
        REQUIRE(check_it->id == 3);
    }
    
    SECTION("emplace failure on full capacity") {
        elib::list<ComplexObject, 2> small_list;
        REQUIRE(small_list.emplace_back(1, "A"));
        REQUIRE(small_list.emplace_back(2, "B"));
        
        // 3rd emplace should fail
        REQUIRE_FALSE(small_list.emplace_back(3, "C"));
        REQUIRE_FALSE(small_list.emplace_front(0, "Z"));
        REQUIRE(small_list.emplace(small_list.begin(), 4, "D") == small_list.end());
        
        REQUIRE(small_list.size() == 2); // Unchanged
    }
}