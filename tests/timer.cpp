#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <cstdint>
#include <chrono>

#include <elib/time/system_clock.h>
#include <elib/time/timer.h>

#include "mock/assert.h"

using namespace elib;
using namespace std::chrono;

namespace {
    // Helper to mock the system clock and increment time
    void advance_time(std::size_t ms)
    {
        for (std::size_t tick = 0; tick < ms; tick++)
        {
            elib::time::system_clock::increment();

            for (std::size_t timerCount = 0; timerCount < time::config::max_timer_num; timerCount++)
            {
              // make sure all timers are processed
              time::timer::process_timers();
            }
        }
    }
}


class TimerListener : public Catch::EventListenerBase
{
public:
    using EventListenerBase::EventListenerBase; // Inherit constructors

    virtual void testCaseStarting(const Catch::TestCaseInfo &) override
    {
        elib::time::system_clock::reset();
        time::timer::unregister_timers();
    }
};

CATCH_REGISTER_LISTENER(TimerListener);


TEST_CASE("elib::time::Timer: Register Timer", "[time][timer]")
{
    bool callbackExecuted = false;
    auto timer = time::timer::register_timer(milliseconds{100}, [&]() {
        callbackExecuted = true;
    });

    REQUIRE(timer.valid());
    REQUIRE_FALSE(timer.running());

    timer.start();
    REQUIRE(timer.running());

    advance_time(99);
    REQUIRE_FALSE(callbackExecuted);

    advance_time(1);
    REQUIRE(callbackExecuted);

    timer.stop();
    REQUIRE_FALSE(timer.running());
}

TEST_CASE("elib::time::Timer: Single-Shot Timer", "[time][timer]")
{
    int counter = 0;
    bool singleShotCreated = time::timer::single_shot(milliseconds{50}, [&]() {
        counter++;
    });

    REQUIRE(singleShotCreated);

    advance_time(49);
    REQUIRE(counter == 0);

    advance_time(1);
    REQUIRE(counter == 1);

    advance_time(50);
    REQUIRE(counter == 1); // Should not trigger again
}

TEST_CASE("elib::time::Timer: Periodic Timer", "[time][timer]")
{
    int counter = 0;
    auto timer = time::timer::register_timer(milliseconds{30}, [&]() {
        counter++;
    });

    REQUIRE(timer.valid());
    timer.start();

    advance_time(29);
    REQUIRE(counter == 0);

    advance_time(1);
    REQUIRE(counter == 1);

    advance_time(30);
    REQUIRE(counter == 2);

    advance_time(30);
    REQUIRE(counter == 3);

    timer.stop();
    advance_time(30);
    REQUIRE(counter == 3); // Timer stopped, no more increments
}

TEST_CASE("elib::time::Timer: Change Interval While Running", "[time][timer]")
{
    int counter = 0;
    auto timer = time::timer::register_timer(milliseconds{50}, [&]() {
        counter++;
    });

    REQUIRE(timer.valid());
    timer.start();

    advance_time(50);
    REQUIRE(counter == 1);

    timer.set_interval(milliseconds{30});
    REQUIRE(timer.interval() == milliseconds{30});

    advance_time(30);
    REQUIRE(counter == 2);

    advance_time(30);
    REQUIRE(counter == 3);
}

TEST_CASE("elib::time::Timer: Unregister Timer", "[time][timer]")
{
    int counter = 0;
    auto timer = time::timer::register_timer(milliseconds{40}, [&]() {
        counter++;
    });

    REQUIRE(timer.valid());
    timer.start();

    advance_time(40);
    REQUIRE(counter == 1);

    timer.unregister();
    REQUIRE_FALSE(timer.valid());

    advance_time(40);
    REQUIRE(counter == 1); // Timer unregistered, no more increments
}

TEST_CASE("elib::time::Timer: Destructor Unregisters Timer", "[time][timer]")
{
    int counter = 0;
    {
        auto timer = time::timer::register_timer(milliseconds{50}, [&]() {
            counter++;
        });
        REQUIRE(timer.valid());
        timer.start();
    } // Timer goes out of scope and should unregister itself

    advance_time(50);
    REQUIRE(counter == 0); // Timer should not trigger
}

TEST_CASE("elib::time::Timer: Multiple Timers", "[time][timer]")
{
    int timer1Counter = 0;
    int timer2Counter = 0;

    auto timer1 = time::timer::register_timer(milliseconds{20}, [&]() {
        timer1Counter++;
    });

    auto timer2 = time::timer::register_timer(milliseconds{50}, [&]() {
        timer2Counter++;
    });

    REQUIRE(timer1.valid());
    REQUIRE(timer2.valid());

    timer1.start();
    timer2.start();

    advance_time(20);
    REQUIRE(timer1Counter == 1);
    REQUIRE(timer2Counter == 0);

    advance_time(30);
    REQUIRE(timer1Counter == 2);
    REQUIRE(timer2Counter == 1);

    advance_time(50);
    REQUIRE(timer1Counter == 5);
    REQUIRE(timer2Counter == 2);
}

TEST_CASE("elib::time::Timer: Exhaust Timer Slots", "[time][timer]")
{
    std::vector<time::timer> timers;
    for (std::size_t i = 0; i < time::config::max_timer_num; ++i)
    {
        auto timer = time::timer::register_timer(milliseconds{10}, []() {});
        REQUIRE(timer.valid());

        // keep timers from destroyin and unregistering
        timers.push_back(std::move(timer));
    }

    using namespace trompeloeil;

   // Only expect the assertion if we are in a Debug build!
#if !defined(NDEBUG) && !defined(ELIB_CONFIG_NO_ASSERTS)
    REQUIRE_CALL(mock::assert_mock::instance(), onError(_, _, _))
            .WITH(std::string(_3).find("elib::time::timer") != std::string::npos);
#endif
    // Exceeding the maximum number of timers should fail
    auto extraTimer = time::timer::register_timer(milliseconds{10}, []() {});
    REQUIRE_FALSE(extraTimer.valid());
}

TEST_CASE("elib::time::Timer: Update Callback via setCallback", "[time][timer]")
{
    bool firstCallbackExecuted = false;
    bool secondCallbackExecuted = false;

    auto timer = time::timer::register_timer(milliseconds{50}, [&]() {
        firstCallbackExecuted = true;
    });

    timer.start();

    // Change the callback before the timer fires
    timer.set_callback([&]() {
        secondCallbackExecuted = true;
    });

    advance_time(50);

    CHECK_FALSE(firstCallbackExecuted);
    CHECK(secondCallbackExecuted);
}

TEST_CASE("elib::time::Timer: setCallback preserves Timer state", "[time][timer]")
{
    int counter = 0;
    auto timer = time::timer::register_timer(milliseconds{50}, [&]() { counter++; });

    timer.start();
    advance_time(25); // Timer is halfway through its interval

    // Update callback mid-run
    timer.set_callback([&]() { counter += 10; });

    advance_time(25); // Complete the original 50ms
    CHECK(counter == 10); // Should have triggered the NEW callback

    advance_time(50);
    CHECK(counter == 20); // Periodic behavior should still work
}

TEST_CASE("elib::time::Timer: setCallback handles Move Scenario", "[time][timer]")
{
    // This simulates what happens inside SerialSensorAPI move operations
    int value = 0;

    struct mock_owner {
        int id;
        time::timer timer;
        int& externalValue;

        mock_owner(int id, int& val) : id(id), externalValue(val) {
            timer = time::timer::register_timer(milliseconds{10}, [this]() {
                externalValue = this->id;
            });
            timer.start();
        }

        // Manual move simulating the SerialSensorAPI fix
        mock_owner(mock_owner&& other) noexcept 
            : id(other.id), timer(std::move(other.timer)), externalValue(other.externalValue) 
        {
            timer.set_callback([this]() {
                externalValue = this->id;
            });
        }
    };

    {
        mock_owner owner1(42, value);

        // Move owner1 to owner2. owner1's address is now invalid for the callback.
        mock_owner owner2(std::move(owner1));

        advance_time(10);

        // If setCallback worked, 'value' should be 42 (from owner2's context)
        // and not a crash or garbage from owner1's old address.
        CHECK(value == 42);
    }
}

TEST_CASE("elib::time::Timer: setCallback on Invalid Timer", "[time][timer]")
{
    time::timer timer; // Unregistered/Invalid
    REQUIRE_FALSE(timer.valid());

    // This should not crash or cause side effects
    timer.set_callback([]() { /* empty */ });
}