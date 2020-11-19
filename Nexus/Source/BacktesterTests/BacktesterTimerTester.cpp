#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <doctest/doctest.h>
#include "Nexus/Backtester/ActiveBacktesterEvent.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/Backtester/BacktesterTimer.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("BacktesterTimer") {
  TEST_CASE("expiry") {
    auto startTime = time_from_string("2016-05-06 00:00:00");
    auto eventHandler = BacktesterEventHandler(startTime);
    auto timer = BacktesterTimer(seconds(1), Ref(eventHandler));
    auto routines = RoutineTaskQueue();
    auto expectedTimestamp = startTime + seconds(1);
    auto queryCompleteMutex = Mutex();
    auto testCompleteCondition = ConditionVariable();
    auto testSucceeded = optional<bool>();
    timer.GetPublisher().Monitor(routines.GetSlot<Timer::Result>(
      [&] (Timer::Result result) {
        auto lock = lock_guard(queryCompleteMutex);
        auto timestamp = eventHandler.GetTime();
        if(timestamp == expectedTimestamp && result == Timer::Result::EXPIRED) {
          testSucceeded = true;
          testCompleteCondition.notify_one();
        } else {
          testSucceeded = false;
          testCompleteCondition.notify_one();
        }
      }));
    timer.Start();
    eventHandler.Add(std::make_shared<ActiveBacktesterEvent>(
      time_from_string("2016-05-07 00:00:00")));
    auto lock = unique_lock(queryCompleteMutex);
    while(!testSucceeded) {
      testCompleteCondition.wait(lock);
    }
    REQUIRE(*testSucceeded);
  }

  TEST_CASE("cancel") {
    auto startTime = time_from_string("2016-05-06 00:00:00");
    auto eventHandler = BacktesterEventHandler(startTime);
    auto timerA = BacktesterTimer(seconds(1), Ref(eventHandler));
    auto timerB = BacktesterTimer(seconds(2), Ref(eventHandler));
    auto routines = RoutineTaskQueue();
    auto expectedTimestamp = startTime + seconds(1);
    auto queryCompleteMutex = Mutex();
    auto testCompleteCondition = ConditionVariable();
    auto testSucceeded = optional<bool>();
    timerA.GetPublisher().Monitor(routines.GetSlot<Timer::Result>(
      [&] (Timer::Result result) {
        auto timestamp = eventHandler.GetTime();
        REQUIRE(timestamp == expectedTimestamp);
        REQUIRE(result == Timer::Result::EXPIRED);
        timerB.Cancel();
      }));
    timerB.GetPublisher().Monitor(routines.GetSlot<Timer::Result>(
      [&] (Timer::Result result) {
        auto timestamp = eventHandler.GetTime();
        REQUIRE(timestamp == expectedTimestamp);
        REQUIRE(result == Timer::Result::CANCELED);
        auto lock = lock_guard(queryCompleteMutex);
        testSucceeded = true;
        testCompleteCondition.notify_one();
      }));
    routines.Push(
      [&] {
        timerB.Start();
        timerA.Start();
        eventHandler.Add(std::make_shared<ActiveBacktesterEvent>(
          time_from_string("2016-05-07 00:00:00")));
      });
    auto lock = unique_lock(queryCompleteMutex);
    while(!testSucceeded) {
      testCompleteCondition.wait(lock);
    }
    REQUIRE(*testSucceeded);
  }
}
