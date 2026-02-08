#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <memory>
#include <elib/event_loop.h>
#include <elib/config.h>
#include "mock/assert.h"

// specific event type for testing
struct TestEvent {
    int id;
    int value;
};

TEST_CASE("elib::event_loop basic functionality", "[event_loop]")
{
  // Clear any static state if necessary, though destructors should handle cleanup
  // Setup
  elib::EventLoop<TestEvent, 8> loop;
  int received_id = 0;
  int received_val = 0;
  int call_count = 0;

  loop.setHandler([&](const TestEvent& e) {
      received_id = e.id;
      received_val = e.value;
      call_count++;
  });

  SECTION("Pushing and processing a single event")
  {
    bool pushed = loop.push({1, 100});
    REQUIRE(pushed == true);

    // Before processing
    REQUIRE(call_count == 0);

    // Process
    elib::kernel::processTasks();

    // After processing
    REQUIRE(call_count == 1);
    REQUIRE(received_id == 1);
    REQUIRE(received_val == 100);
}

  SECTION("Buffer clears after processing")
  {
    loop.push({1, 100});
    elib::kernel::processTasks();
    REQUIRE(call_count == 1);

    // Call again - buffer should be empty, handler should not run
    elib::kernel::processTasks();
    REQUIRE(call_count == 1);
  }
}

TEST_CASE("elib::event_loop max events per call", "[event_loop]")
{
  elib::EventLoop<int, 32> loop;
  int process_count = 0;
  
  loop.setHandler([&](const int&) {
      process_count++;
  });

  SECTION("default: 1 event per call")
  {
      // Push 3 events
      loop.push(1);
      loop.push(2);
      loop.push(3);

      // First pass: Should process only 1
      loop.run();
      REQUIRE(process_count == 1);

      // Second pass: Should process 2nd
      loop.run();
      REQUIRE(process_count == 2);

      // Second pass: Should process 2nd
      loop.run();
      REQUIRE(process_count == 3);
  }

  SECTION("custom: valid")
  {
    const std::size_t count = elib::event::config::maxEventPerCallNum / 2;
    loop.setMaxEventsPerCall(count);

    // Push 5 events
    for(int i=0; i<count; ++i) loop.push(i);

    // One pass should clear them all
    loop.run();
    REQUIRE(process_count == count);
  }

  SECTION("custom: invalid - under minimum")
  {
      loop.setMaxEventsPerCall(0); // invalid, should return to default: 1
      loop.push(1);
      loop.run();

      // pushed event must be processed
      REQUIRE(process_count == 1);
  }

  SECTION("custom: invalid - over limit")
  {
      const std::size_t count = elib::event::config::maxEventPerCallNum + 1;
      loop.setMaxEventsPerCall(count);
      
      for(size_t i=0; i < count; ++i) {
          loop.push(static_cast<int>(i));
      }

      // Process
      loop.run();

      // not more than limit is processed
      REQUIRE(process_count == elib::event::config::maxEventPerCallNum);
      
      // Next call cleans up the rest
      loop.run();
      REQUIRE(process_count == count);
  }
}

TEST_CASE("elib::event_loop move semantics", "[event_loop]")
{
  int received_val = 0;
  
  // Factory function lambda to simulate returning from function
  auto createLoop = [&]() {
      elib::EventLoop<int, 8> temp;
      temp.setHandler([&](const int& v){ received_val = v; });
      temp.push(42);
      return temp; // Triggers Move Constructor
  };

  SECTION("Move Constructor updates registry")
  {
      // 1. Create and move
      elib::EventLoop<int, 8> mainLoop = createLoop();

      // The 'temp' loop inside the lambda is destroyed.
      // We need to ensure 'mainLoop' is wired into the registry.

      REQUIRE(received_val == 0);

      // 2. Process
      // If the registry update failed, this would either crash (dangling pointer)
      // or do nothing (if pointer was nullified).
      mainLoop.run();

      REQUIRE(received_val == 42);
  }

  SECTION("Move Assignment updates registry")
  {
      elib::EventLoop<int, 8> loopA;
      {
          elib::EventLoop<int, 8> loopB;
          loopB.setHandler([&](const int& v){ received_val = v; });
          // loopB registers itself.
          
          // Move B into A. 
          // A should unregister its old self.
          // A should take B's place in the registry.
          // B should become invalid/empty.
          loopA = std::move(loopB); 
      } // loopB destructor runs here

      loopA.push(99);
      elib::kernel::processTasks();
      
      REQUIRE(received_val == 99);
  }
}

TEST_CASE("elib::event_loop multiple loops interaction", "[event_loop]")
{
  elib::EventLoop<int, 4> loop1;
  elib::EventLoop<int, 4> loop2;
  
  int val1 = 0;
  int val2 = 0;

  loop1.setHandler([&](const int& v){ val1 = v; });
  loop2.setHandler([&](const int& v){ val2 = v; });

  loop1.push(10);
  loop2.push(20);

  SECTION("Round robin fairness")
  {
      // Your implementation processes ONE loop per call to processEventLoops()
      // We don't know which one comes first (depends on registry slots), 
      // but we know calling it twice should handle both.

      elib::kernel::processTasks();
      
      // One should be done, one should be pending
      bool caseA = (val1 == 10 && val2 == 0);
      bool caseB = (val1 == 0 && val2 == 20);
      REQUIRE((caseA || caseB));

      elib::kernel::processTasks();

      // Now both should be done
      REQUIRE(val1 == 10);
      REQUIRE(val2 == 20);
  }
}

TEST_CASE("elib::event_loop capacity limits", "[event_loop]")
{
  using namespace trompeloeil;

  SECTION("Buffer overflow behavior")
  {
    elib::EventLoop<int, 2> tinyLoop;
    
    REQUIRE(tinyLoop.push(1) == true);
    REQUIRE(tinyLoop.push(2) == true);
    REQUIRE(tinyLoop.push(3) == false); // Should fail
  }

  SECTION("Registry overflow behavior")
  {
    // 1. Fill the registry
    std::vector<std::unique_ptr<elib::EventLoop<int, 1>>> spam;
    
    for(size_t i=0; i < elib::kernel::taskMaxNum(); ++i) {
        spam.push_back(std::make_unique<elib::EventLoop<int, 1>>());
    }

    // 2. Set Expectation
    // The constructor will trigger the assert. We expect it, match the message, 
    // and allow it to return void (suppressing the crash).
    REQUIRE_CALL(mock::AssertMock::instance(), onError(_, _, _));

    // 3. Trigger
    // This creates the extra loop. The assert fires, caught by mock, returns.
    elib::EventLoop<int, 1> extraLoop;

    // 4. Verify Local Behavior (Zombie Object)
    // It works locally...
    bool pushed = extraLoop.push(1); 
    REQUIRE(pushed == true);
    
    // 5. Verify System Safety
    // ...but is not in the global loop, so processing is safe.
    elib::kernel::processTasks();
  }
}

TEST_CASE("elib::event_loop push over capacity - force push", "[event_loop]")
{
  elib::EventLoop<int, 2> tinyLoop;
  REQUIRE(tinyLoop.push(1) == true);
  REQUIRE(tinyLoop.push(2) == true);
  REQUIRE(tinyLoop.push(3) == false);

  std::vector<int> processed;
  tinyLoop.setHandler([&](int e){ processed.push_back(e); });
  tinyLoop.setMaxEventsPerCall(3);
  
  tinyLoop.pushOver(3); // force 3. push_over works in circular order, so 1 will be overwritten
  tinyLoop.run();

  REQUIRE(processed[0] == 2); // 1 was overwritten, 2 become first processed
  REQUIRE(processed[1] == 3); // 3 was forced and processed
}