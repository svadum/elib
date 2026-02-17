#include <catch2/catch_test_macros.hpp>
#include <elib/memory.h>
#include <string>
#include <type_traits>

namespace
{
  struct test_type
  {
    int a;
    std::string str;

    test_type(int x, std::string s)
      : a(x)
      , str(std::move(s))
    {
    }

    ~test_type() { destroyed = true; }

    static bool destroyed;
  };

  bool test_type::destroyed = false;

} // anonymous namespace

TEST_CASE("elib::aligned_storage: Default constructed storage is empty", "[aligned_storage]")
{
  elib::memory::aligned_storage<test_type> storage;

  REQUIRE_FALSE(storage.is_constructed());
}

TEST_CASE("elib::aligned_storage: Construct an object", "[aligned_storage]")
{
  elib::memory::aligned_storage<test_type> storage;

  auto& obj = storage.construct(42, "Hello");

  REQUIRE(storage.is_constructed());
  REQUIRE(obj.a == 42);
  REQUIRE(obj.str == "Hello");

  const auto& const_obj = storage.get();
  REQUIRE(const_obj.a == 42);
  REQUIRE(const_obj.str == "Hello");
}

TEST_CASE("elib::aligned_storage: Destroy object", "[aligned_storage]")
{
  elib::memory::aligned_storage<test_type> storage;

  storage.construct(99, "DestroyMe");
  REQUIRE(storage.is_constructed());

  storage.destroy();
  REQUIRE_FALSE(storage.is_constructed());
}

TEST_CASE("elib::aligned_storage: Destroy calls destructor", "[aligned_storage]")
{
  test_type::destroyed = false;
  {
    elib::memory::aligned_storage<test_type> storage;
    storage.construct(10, "CheckDestructor");

    REQUIRE(storage.is_constructed());
  } // Goes out of scope, should call destructor

  REQUIRE(test_type::destroyed);
}

TEST_CASE("elib::aligned_storage: Construct multiple times", "[aligned_storage]")
{
  elib::memory::aligned_storage<test_type> storage;

  storage.construct(1, "First");
  REQUIRE(storage.get().a == 1);
  REQUIRE(storage.get().str == "First");

  storage.construct(2, "Second");
  REQUIRE(storage.get().a == 2);
  REQUIRE(storage.get().str == "Second");

  REQUIRE(storage.is_constructed());
}

TEST_CASE("elib::aligned_storage: Construct and get()", "[aligned_storage]")
{
  elib::memory::aligned_storage<int> storage;

  storage.construct(123);
  REQUIRE(storage.is_constructed());
  REQUIRE(storage.get() == 123);
}

TEST_CASE("elib::aligned_storage: Access get() without constructing", "[aligned_storage]")
{
  elib::memory::aligned_storage<int> storage;

  REQUIRE_FALSE(storage.is_constructed());

  // Undefined behavior if accessed without checking is_constructed()
  // We don't test actual call because it's the user's responsibility to check
}

TEST_CASE("elib::aligned_storage: Correctly aligns large structures", "[aligned_storage]")
{
  struct alignas(64) LargeAligned
  {
    double data[8];
  };

  REQUIRE(std::alignment_of_v<LargeAligned> == 64);

  elib::memory::aligned_storage<LargeAligned> storage;
  storage.construct();

  REQUIRE(storage.is_constructed());
  REQUIRE(reinterpret_cast<std::uintptr_t>(&storage.get()) % 64 == 0);
}

TEST_CASE("elib::aligned_storage: Works with non-copyable and non-movable types", "[aligned_storage]")
{
  struct non_movable
  {
    int value;
    non_movable(int v)
      : value(v)
    {
    }
    non_movable(const non_movable&)            = delete;
    non_movable& operator=(const non_movable&) = delete;
  };

  elib::memory::aligned_storage<non_movable> storage;
  storage.construct(7);

  REQUIRE(storage.get().value == 7);
}

TEST_CASE("elib::aligned_storage: Works with trivial types", "[aligned_storage]")
{
  elib::memory::aligned_storage<int> storage;

  storage.construct(42);
  REQUIRE(storage.get() == 42);
}

TEST_CASE("elib::aligned_storage: Works with large objects", "[aligned_storage]")
{
  struct large_obj
  {
    char buffer[1024];
  };

  elib::memory::aligned_storage<large_obj> storage;
  storage.construct();

  REQUIRE(storage.is_constructed());
}

TEST_CASE("elib::aligned_storage: Properly destroys object in destructor", "[aligned_storage]")
{
  test_type::destroyed = false;
  {
    elib::memory::aligned_storage<test_type> storage;
    storage.construct(55, "DestructorTest");

    REQUIRE(storage.is_constructed());
  }

  REQUIRE(test_type::destroyed);
}
