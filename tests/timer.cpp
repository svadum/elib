#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include <cstdint>
#include <chrono>

#include "elib/time/system_clock.h"
#include "elib/time/timer.h"

using namespace elib::time;
using namespace std::chrono;

namespace {
    // Helper to mock the system clock and increment time
    void advanceTime(std::size_t ms)
    {
        for (std::size_t tick = 0; tick < ms; tick++)
        {
            SystemClock::increment();

            for (std::size_t timerCount = 0; timerCount < config::maxTimerNum; timerCount++)
            {
              // make sure all timers are processed
              Timer::processTimers();
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
        SystemClock::reset();
        Timer::unregisterTimers();
    }
};

CATCH_REGISTER_LISTENER(TimerListener);


TEST_CASE("Timer: Register Timer")
{
    bool callbackExecuted = false;
    auto timer = Timer::registerTimer(milliseconds{100}, [&]() {
        callbackExecuted = true;
    });

    REQUIRE(timer.valid());
    REQUIRE_FALSE(timer.running());

    timer.start();
    REQUIRE(timer.running());

    advanceTime(99);
    REQUIRE_FALSE(callbackExecuted);

    advanceTime(1);
    REQUIRE(callbackExecuted);

    timer.stop();
    REQUIRE_FALSE(timer.running());
}

TEST_CASE("Timer: Single-Shot Timer")
{
    int counter = 0;
    bool singleShotCreated = Timer::singleShot(milliseconds{50}, [&]() {
        counter++;
    });

    REQUIRE(singleShotCreated);

    advanceTime(49);
    REQUIRE(counter == 0);

    advanceTime(1);
    REQUIRE(counter == 1);

    advanceTime(50);
    REQUIRE(counter == 1); // Should not trigger again
}

TEST_CASE("Timer: Periodic Timer")
{
    int counter = 0;
    auto timer = Timer::registerTimer(milliseconds{30}, [&]() {
        counter++;
    });

    REQUIRE(timer.valid());
    timer.start();

    advanceTime(29);
    REQUIRE(counter == 0);

    advanceTime(1);
    REQUIRE(counter == 1);

    advanceTime(30);
    REQUIRE(counter == 2);

    advanceTime(30);
    REQUIRE(counter == 3);

    timer.stop();
    advanceTime(30);
    REQUIRE(counter == 3); // Timer stopped, no more increments
}

TEST_CASE("Timer: Change Interval While Running")
{
    int counter = 0;
    auto timer = Timer::registerTimer(milliseconds{50}, [&]() {
        counter++;
    });

    REQUIRE(timer.valid());
    timer.start();

    advanceTime(50);
    REQUIRE(counter == 1);

    timer.setInterval(milliseconds{30});
    REQUIRE(timer.interval() == milliseconds{30});

    advanceTime(30);
    REQUIRE(counter == 2);

    advanceTime(30);
    REQUIRE(counter == 3);
}

TEST_CASE("Timer: Unregister Timer")
{
    int counter = 0;
    auto timer = Timer::registerTimer(milliseconds{40}, [&]() {
        counter++;
    });

    REQUIRE(timer.valid());
    timer.start();

    advanceTime(40);
    REQUIRE(counter == 1);

    timer.unregister();
    REQUIRE_FALSE(timer.valid());

    advanceTime(40);
    REQUIRE(counter == 1); // Timer unregistered, no more increments
}

TEST_CASE("Timer: Destructor Unregisters Timer")
{
    int counter = 0;
    {
        auto timer = Timer::registerTimer(milliseconds{50}, [&]() {
            counter++;
        });
        REQUIRE(timer.valid());
        timer.start();
    } // Timer goes out of scope and should unregister itself

    advanceTime(50);
    REQUIRE(counter == 0); // Timer should not trigger
}

TEST_CASE("Timer: Multiple Timers")
{
    int timer1Counter = 0;
    int timer2Counter = 0;

    auto timer1 = Timer::registerTimer(milliseconds{20}, [&]() {
        timer1Counter++;
    });

    auto timer2 = Timer::registerTimer(milliseconds{50}, [&]() {
        timer2Counter++;
    });

    REQUIRE(timer1.valid());
    REQUIRE(timer2.valid());

    timer1.start();
    timer2.start();

    advanceTime(20);
    REQUIRE(timer1Counter == 1);
    REQUIRE(timer2Counter == 0);

    advanceTime(30);
    REQUIRE(timer1Counter == 2);
    REQUIRE(timer2Counter == 1);

    advanceTime(50);
    REQUIRE(timer1Counter == 5);
    REQUIRE(timer2Counter == 2);
}

TEST_CASE("Timer: Exhaust Timer Slots")
{
    std::vector<Timer> timers;
    for (std::size_t i = 0; i < config::maxTimerNum; ++i)
    {
        auto timer = Timer::registerTimer(milliseconds{10}, []() {});
        REQUIRE(timer.valid());

        // keep timers from destroyin and unregistering
        timers.push_back(std::move(timer));
    }

    // Exceeding the maximum number of timers should fail
    auto extraTimer = Timer::registerTimer(milliseconds{10}, []() {});
    REQUIRE_FALSE(extraTimer.valid());
}
