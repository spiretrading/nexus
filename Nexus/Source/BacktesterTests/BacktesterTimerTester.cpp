#include "Nexus/BacktesterTests/BacktesterTimerTester.hpp"
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/Backtester/BacktesterTimer.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace std;

void BacktesterTimerTester::TestExpiry() {
  ptime startTime{date{2016, 5, 6}, seconds(0)};
  BacktesterEventHandler eventHandler{startTime};
  eventHandler.Open();
  BacktesterTimer timer{seconds(1), Ref(eventHandler)};
  RoutineTaskQueue routines;
  auto expectedTimestamp = startTime + seconds(1);
  Mutex queryCompleteMutex;
  ConditionVariable testCompleteCondition;
  optional<bool> testSucceeded;
  timer.GetPublisher().Monitor(routines.GetSlot<Timer::Result>(
    [&] (Timer::Result result) {
      boost::lock_guard<Mutex> lock{queryCompleteMutex};
      auto timestamp = eventHandler.GetTime();
      if(timestamp == expectedTimestamp &&
          result == Timer::Result::EXPIRED) {
        testSucceeded = true;
        testCompleteCondition.notify_one();
      } else {
        testSucceeded = false;
        testCompleteCondition.notify_one();
      }
    }));
  timer.Start();
  boost::unique_lock<Mutex> lock{queryCompleteMutex};
  while(!testSucceeded.is_initialized()) {
    testCompleteCondition.wait(lock);
  }
  CPPUNIT_ASSERT(*testSucceeded);
}

void BacktesterTimerTester::TestCancel() {
  ptime startTime{date{2016, 5, 6}, seconds(0)};
  BacktesterEventHandler eventHandler{startTime};
  eventHandler.Open();
  BacktesterTimer timerA{seconds(1), Ref(eventHandler)};
  BacktesterTimer timerB{seconds(2), Ref(eventHandler)};
  RoutineTaskQueue routines;
  auto expectedTimestamp = startTime + seconds(1);
  Mutex queryCompleteMutex;
  ConditionVariable testCompleteCondition;
  optional<bool> testSucceeded;
  timerA.GetPublisher().Monitor(routines.GetSlot<Timer::Result>(
    [&] (Timer::Result result) {
      auto timestamp = eventHandler.GetTime();
      CPPUNIT_ASSERT(timestamp == expectedTimestamp);
      CPPUNIT_ASSERT(result == Timer::Result::EXPIRED);
      timerB.Cancel();
    }));
  timerB.GetPublisher().Monitor(routines.GetSlot<Timer::Result>(
    [&] (Timer::Result result) {
      auto timestamp = eventHandler.GetTime();
      CPPUNIT_ASSERT(timestamp == expectedTimestamp);
      CPPUNIT_ASSERT(result == Timer::Result::CANCELED);
      boost::lock_guard<Mutex> lock{queryCompleteMutex};
      testSucceeded = true;
      testCompleteCondition.notify_one();
    }));
  routines.Push(
    [&] {
      timerB.Start();
      timerA.Start();
    });
  boost::unique_lock<Mutex> lock{queryCompleteMutex};
  while(!testSucceeded.is_initialized()) {
    testCompleteCondition.wait(lock);
  }
  CPPUNIT_ASSERT(*testSucceeded);
}
