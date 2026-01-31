#include <catch2/catch_test_macros.hpp>
#include <elib/event_loop.h>
#include <elib/config.h>

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
    elib::event::processEventLoops();

    // After processing
    REQUIRE(call_count == 1);
    REQUIRE(received_id == 1);
    REQUIRE(received_val == 100);
}

  SECTION("Buffer clears after processing")
  {
    loop.push({1, 100});
    elib::event::processEventLoops();
    REQUIRE(call_count == 1);

    // Call again - buffer should be empty, handler should not run
    elib::event::processEventLoops();
    REQUIRE(call_count == 1);
  }
}

TEST_CASE("elib::event_loop processing strategies", "[event_loop]")
{
  elib::EventLoop<int, 32> loop;
  int process_count = 0;
  
  loop.setHandler([&](const int&) {
      process_count++;
  });

  SECTION("Strategy: SingleEvent (Default)")
  {
      loop.setProcessStrategy(elib::event::ProcessStrategy::SingleEvent);

      // Push 3 events
      loop.push(1);
      loop.push(2);
      loop.push(3);

      // First pass: Should process only 1
      elib::event::processEventLoops();
      REQUIRE(process_count == 1);

      // Second pass: Should process 2nd
      elib::event::processEventLoops();
      REQUIRE(process_count == 2);
  }

  SECTION("Strategy: AllEvents")
  {
      loop.setProcessStrategy(elib::event::ProcessStrategy::AllEvents);

      // Push 5 events
      for(int i=0; i<5; ++i) loop.push(i);

      // One pass should clear them all
      elib::event::processEventLoops();
      REQUIRE(process_count == 5);
  }

  SECTION("Strategy: AllEvents respects maxEventPerCallNum cap")
  {
      loop.setProcessStrategy(elib::event::ProcessStrategy::AllEvents);
      
      // Push more than the config limit (default is 25 in your config)
      // Let's assume maxEventPerCallNum is 25. We push 30.
      const size_t limit = elib::event::config::maxEventPerCallNum;
      const size_t push_count = limit + 5;
      
      // Ensure our buffer is big enough for test
      // (If buffer is smaller than limit, this test tests buffer size, not call limit)
      // Since we defined EventLoop<int, 32>, and default limit is 25, this works.

      for(size_t i=0; i < push_count; ++i) {
          loop.push(static_cast<int>(i));
      }

      // Process
      elib::event::processEventLoops();

      // Should have processed exactly 'limit' events, leaving 5 remaining
      REQUIRE(process_count == limit);
      
      // Next call cleans up the rest
      elib::event::processEventLoops();
      REQUIRE(process_count == push_count);
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
      elib::event::processEventLoops();

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
      elib::event::processEventLoops();
      
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

      elib::event::processEventLoops();
      
      // One should be done, one should be pending
      bool caseA = (val1 == 10 && val2 == 0);
      bool caseB = (val1 == 0 && val2 == 20);
      REQUIRE((caseA || caseB));

      elib::event::processEventLoops();

      // Now both should be done
      REQUIRE(val1 == 10);
      REQUIRE(val2 == 20);
  }
}

TEST_CASE("elib::event_loop capacity limits", "[event_loop]")
{
  SECTION("Buffer overflow behavior")
  {
      elib::EventLoop<int, 2> tinyLoop;
      
      REQUIRE(tinyLoop.push(1) == true);
      REQUIRE(tinyLoop.push(2) == true);
      REQUIRE(tinyLoop.push(3) == false); // Should fail
  }

  SECTION("Registry overflow behavior")
  {
      // This test requires creating more loops than maxEventLoopNum.
      // We use std::vector to hold them to avoid stack overflow or manual management.
      std::vector<std::unique_ptr<elib::EventLoop<int, 1>>> spam;
      
      // Fill the registry
      for(size_t i=0; i < elib::event::config::maxEventLoopNum; ++i) {
          spam.push_back(std::make_unique<elib::EventLoop<int, 1>>());
      }

      // Try to create one more
      // Since constructor doesn't throw or return status, we can't easily check 
      // failure inside the class without inspecting private state or adding a valid() method.
      // However, we can ensure it doesn't crash.
      
      elib::EventLoop<int, 1> extraLoop;
      bool pushed = extraLoop.push(1); // Should still work locally
      REQUIRE(pushed == true);
      
      // But processing shouldn't crash
      elib::event::processEventLoops();
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
  tinyLoop.setProcessStrategy(elib::event::ProcessStrategy::AllEvents);
  
  tinyLoop.pushOver(3); // force 3. push_over works in circular order, so 1 will be overwritten
  tinyLoop.process();

  REQUIRE(processed[0] == 2); // 1 was overwritten, 2 become first processed
  REQUIRE(processed[1] == 3); // 3 was forced and processed
}