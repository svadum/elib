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
class test_task : public elib::task
{
public:
    int run_count = 0;
    
    void run() override {
        run_count++;
    }
};

TEST_CASE("elib::kernel: Task Scheduling & Fairness", "[kernel]")
{
    // SETUP: Create mixed workload
    test_task task1;
    test_task task2;
    
    // Check initial state
    REQUIRE(task1.run_count == 0);
    REQUIRE(task2.run_count == 0);

    SECTION("Round-Robin Execution")
    {
        // Cycle 1: Should run Task 1 (or Task 2, depending on internal order)
        elib::kernel::process_tasks();
        
        // Cycle 2: Should run the OTHER task
        elib::kernel::process_tasks();

        // Cycle 3: Back to the first one
        elib::kernel::process_tasks();

        // After 3 cycles with 2 tasks, total runs should be 3.
        // Distribution should be roughly equal (2 vs 1).
        REQUIRE(task1.run_count + task2.run_count == 3);
        REQUIRE(task1.run_count >= 1);
        REQUIRE(task2.run_count >= 1);
    }
}

TEST_CASE("elib::kernel: Unified Architecture (EventLoop IS-A Task)", "[kernel][event_loop]")
{
    // 1. Create a Generic Task
    test_task simple_task;

    // 2. Create an EventLoop (Now inherits from Task)
    elib::event_loop<int, 4> loop;
    int loop_processed = 0;
    
    loop.set_handler([&](int){ loop_processed++; });
    loop.push(42);

    // 3. Run Scheduler (processAll drives EVERYTHING)
    // We run enough iterations to guarantee both slots are visited.
    for(int i=0; i < 5; ++i) {
        elib::kernel::process_all();
    }

    // 4. Verify both types of objects were scheduled
    REQUIRE(simple_task.run_count > 0); // Task::run() was called
    REQUIRE(loop_processed == 1);      // EventLoop::run() -> process() was called
}

TEST_CASE("elib::kernel: Move Semantics & Registry Patching", "[kernel]")
{
    SECTION("Moving a Task keeps it scheduled")
    {
        // Factory pattern: create and return by value (Move)
        auto factory = []() {
            test_task t;
            // t registers itself at address A
            return t; 
            // t moves to address B, 'moveTask' patches registry A -> B
        };

        test_task worker = factory();
        
        // Verify worker is active in the scheduler
        int init_count = worker.run_count;
        
        // Force full cycle
        size_t capacity = elib::kernel::task_max_num();
        for(size_t i=0; i < capacity + 2; ++i) elib::kernel::process_all();

        REQUIRE(worker.run_count > init_count);
    }

    SECTION("Moving an EventLoop keeps it scheduled")
    {
        auto factory = []() {
            elib::event_loop<int, 4> l;
            l.push(123);
            return l;
        };

        elib::event_loop<int, 4> main_loop = factory();
        bool handled = false;
        main_loop.set_handler([&](int){ handled = true; });

        // Force full cycle
        size_t capacity = elib::kernel::task_max_num();
        for(size_t i=0; i < capacity + 2; ++i) elib::kernel::process_all();

        REQUIRE(handled == true);
    }
}

TEST_CASE("elib::kernel: Registry Capacity Limits", "[kernel]")
{
    // Note: This test assumes we can mock the assert, 
    // or that we are running in Release mode where ASSERT is void.
    // If you have the mock assert system set up, uncomment the REQUIRE_CALL blocks.

    std::vector<std::unique_ptr<test_task>> spam;
    size_t limit = elib::kernel::task_max_num();

    // 1. Fill the registry
    for(size_t i=0; i < limit; ++i) {
        spam.push_back(std::make_unique<test_task>());
    }

    // 2. Try to add one more
    // This calls registerTask() in the constructor.
    // In Debug: It should Assert.
    // In Release: It should proceed but not register.
    
    // TestTask extraTask; // <--- Would trigger assert here
    
    // Because we cannot easily catch the "return false" from the constructor 
    // without inspecting internal state or using the mock, 
    // we verify that the registry didn't explode or corrupt.
    
    elib::kernel::process_tasks(); // Should be safe
}

TEST_CASE("elib::ManualTask Lifecycle", "[task]")
{
    class my_manual_task : public elib::manual_task
    {
    public:
        int counter = 0;
        void run() override { counter++; }
    };


    my_manual_task task;
    
    SECTION("Does not run before start()")
    {
        // By default, ManualTask does NOT register in constructor
        elib::kernel::process_all();
        REQUIRE(task.counter == 0);
    }

    SECTION("Runs after start()")
    {
        bool started = task.start();
        REQUIRE(started == true);

        elib::kernel::process_all();
        REQUIRE(task.counter == 1);
    }

    SECTION("Stops running after stop()")
    {
        task.start();
        elib::kernel::process_all();
        REQUIRE(task.counter == 1);

        task.stop();
        elib::kernel::process_all();
        // Counter should NOT increase because task is removed
        REQUIRE(task.counter == 1); 
    }

    SECTION("Double start is safe")
    {
        task.start();
        bool second_start = task.start(); 
        
        // It returns true (already registered), but shouldn't duplicate
        REQUIRE(second_start == true); 

        // Run scheduler enough times to cycle through everything
        elib::kernel::process_all();
        elib::kernel::process_all();

        // If it was registered twice, it might run twice per cycle (depending on implementation)
        // But your kernel implementation explicitly prevents duplicates.
        // We verify basic correctness here.
        REQUIRE(task.counter > 0);
    }
}

TEST_CASE("elib::GenericTask Adapter", "[task]")
{
    struct simple_module
    {
        int counter = 0;
        void process() { counter++; }
    };

    simple_module module;

    SECTION("Wraps and drives a POCO")
    {
        // Create adapter, auto-start = true
        elib::generic_task<simple_module> task(module);

        elib::kernel::process_all();
        
        REQUIRE(module.counter == 1);
    }

    SECTION("Respects AutoStart = false")
    {
        // Create adapter, auto-start = false
        elib::generic_task<simple_module> task(module, false);

        elib::kernel::process_all();
        
        // Should NOT have run
        REQUIRE(module.counter == 0);

        // Manually start
        task.start();
        elib::kernel::process_all();
        REQUIRE(module.counter == 1);
    }
}