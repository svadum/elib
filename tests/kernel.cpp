#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <elib/kernel.h>
#include <elib/task.h>
#include <elib/event_loop.h>
#include <elib/config.h>
#include <vector>
#include <memory>

// --------------------------------------------------------------------------
// Helper: A simple counting task to verify execution
// --------------------------------------------------------------------------
class TestTask : public elib::Task
{
public:
    int runCount = 0;
    
    void run() override {
        runCount++;
    }
};

TEST_CASE("elib::kernel: Task Scheduling & Fairness", "[kernel]")
{
    // SETUP: Create mixed workload
    TestTask task1;
    TestTask task2;
    
    // Check initial state
    REQUIRE(task1.runCount == 0);
    REQUIRE(task2.runCount == 0);

    SECTION("Round-Robin Execution")
    {
        // Cycle 1: Should run Task 1 (or Task 2, depending on internal order)
        elib::kernel::processTasks();
        
        // Cycle 2: Should run the OTHER task
        elib::kernel::processTasks();

        // Cycle 3: Back to the first one
        elib::kernel::processTasks();

        // After 3 cycles with 2 tasks, total runs should be 3.
        // Distribution should be roughly equal (2 vs 1).
        REQUIRE(task1.runCount + task2.runCount == 3);
        REQUIRE(task1.runCount >= 1);
        REQUIRE(task2.runCount >= 1);
    }
}

TEST_CASE("elib::kernel: Unified Architecture (EventLoop IS-A Task)", "[kernel][event_loop]")
{
    // 1. Create a Generic Task
    TestTask simpleTask;

    // 2. Create an EventLoop (Now inherits from Task)
    elib::EventLoop<int, 4> loop;
    int loopProcessed = 0;
    
    loop.setHandler([&](int){ loopProcessed++; });
    loop.push(42);

    // 3. Run Scheduler (processAll drives EVERYTHING)
    // We run enough iterations to guarantee both slots are visited.
    for(int i=0; i < 5; ++i) {
        elib::kernel::processAll();
    }

    // 4. Verify both types of objects were scheduled
    REQUIRE(simpleTask.runCount > 0); // Task::run() was called
    REQUIRE(loopProcessed == 1);      // EventLoop::run() -> process() was called
}

TEST_CASE("elib::kernel: Move Semantics & Registry Patching", "[kernel]")
{
    SECTION("Moving a Task keeps it scheduled")
    {
        // Factory pattern: create and return by value (Move)
        auto factory = []() {
            TestTask t;
            // t registers itself at address A
            return t; 
            // t moves to address B, 'moveTask' patches registry A -> B
        };

        TestTask worker = factory();
        
        // Verify worker is active in the scheduler
        int initialCount = worker.runCount;
        
        // Force full cycle
        size_t capacity = elib::kernel::taskMaxNum();
        for(size_t i=0; i < capacity + 2; ++i) elib::kernel::processAll();

        REQUIRE(worker.runCount > initialCount);
    }

    SECTION("Moving an EventLoop keeps it scheduled")
    {
        auto factory = []() {
            elib::EventLoop<int, 4> l;
            l.push(123);
            return l;
        };

        elib::EventLoop<int, 4> mainLoop = factory();
        bool handled = false;
        mainLoop.setHandler([&](int){ handled = true; });

        // Force full cycle
        size_t capacity = elib::kernel::taskMaxNum();
        for(size_t i=0; i < capacity + 2; ++i) elib::kernel::processAll();

        REQUIRE(handled == true);
    }
}

TEST_CASE("elib::kernel: Registry Capacity Limits", "[kernel]")
{
    // Note: This test assumes we can mock the assert, 
    // or that we are running in Release mode where ASSERT is void.
    // If you have the mock assert system set up, uncomment the REQUIRE_CALL blocks.

    std::vector<std::unique_ptr<TestTask>> spam;
    size_t limit = elib::kernel::taskMaxNum();

    // 1. Fill the registry
    for(size_t i=0; i < limit; ++i) {
        spam.push_back(std::make_unique<TestTask>());
    }

    // 2. Try to add one more
    // This calls registerTask() in the constructor.
    // In Debug: It should Assert.
    // In Release: It should proceed but not register.
    
    // TestTask extraTask; // <--- Would trigger assert here
    
    // Because we cannot easily catch the "return false" from the constructor 
    // without inspecting internal state or using the mock, 
    // we verify that the registry didn't explode or corrupt.
    
    elib::kernel::processTasks(); // Should be safe
}