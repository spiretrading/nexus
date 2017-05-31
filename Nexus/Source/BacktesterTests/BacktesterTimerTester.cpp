#include "Nexus/BacktesterTests/BacktesterTimerTester.hpp"
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/Backtester/BacktesterServiceClients.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace std;

void BacktesterTimerTester::TestExpiry() {
  ptime startTime{date{2016, 5, 6}, seconds(0)};
  TestEnvironment testEnvironment;
  testEnvironment.Open();
  auto testServiceClients = MakeVirtualServiceClients<TestServiceClients>(
    Initialize(Ref(testEnvironment)));
  BacktesterEnvironment backtesterEnvironment{startTime,
    Ref(*testServiceClients)};
  backtesterEnvironment.Open();
  BacktesterServiceClients backtesterServiceClients{
    Ref(backtesterEnvironment)};
  backtesterServiceClients.Open();
  auto timer = backtesterServiceClients.BuildTimer(seconds(1));
  RoutineTaskQueue routines;
  auto expectedTimestamp = startTime + seconds(1);
  Mutex queryCompleteMutex;
  ConditionVariable testCompleteCondition;
  optional<bool> testSucceeded;
  timer->GetPublisher().Monitor(routines.GetSlot<Timer::Result>(
    [&] (Timer::Result result) {
      boost::lock_guard<Mutex> lock{queryCompleteMutex};
      auto timestamp = backtesterServiceClients.GetTimeClient().GetTime();
      if(timestamp == expectedTimestamp &&
          result == Timer::Result::EXPIRED) {
        testSucceeded = true;
        testCompleteCondition.notify_one();
      } else {
        testSucceeded = false;
        testCompleteCondition.notify_one();
      }
    }));
  timer->Start();
  boost::unique_lock<Mutex> lock{queryCompleteMutex};
  while(!testSucceeded.is_initialized()) {
    testCompleteCondition.wait(lock);
  }
  CPPUNIT_ASSERT(*testSucceeded);
}

void BacktesterTimerTester::TestCancel() {
  ptime startTime{date{2016, 5, 6}, seconds(0)};
  TestEnvironment testEnvironment;
  testEnvironment.Open();
  auto testServiceClients = MakeVirtualServiceClients<TestServiceClients>(
    Initialize(Ref(testEnvironment)));
  BacktesterEnvironment backtesterEnvironment{startTime,
    Ref(*testServiceClients)};
  backtesterEnvironment.Open();
  BacktesterServiceClients backtesterServiceClients{
    Ref(backtesterEnvironment)};
  backtesterServiceClients.Open();
  auto timerA = backtesterServiceClients.BuildTimer(seconds(1));
  auto timerB = backtesterServiceClients.BuildTimer(seconds(2));
  RoutineTaskQueue routines;
  auto expectedTimestamp = startTime + seconds(1);
  Mutex queryCompleteMutex;
  ConditionVariable testCompleteCondition;
  optional<bool> testSucceeded;
  timerA->GetPublisher().Monitor(routines.GetSlot<Timer::Result>(
    [&] (Timer::Result result) {
      auto timestamp = backtesterServiceClients.GetTimeClient().GetTime();
      CPPUNIT_ASSERT(timestamp == expectedTimestamp);
      CPPUNIT_ASSERT(result == Timer::Result::EXPIRED);
      timerB->Cancel();
    }));
  timerB->GetPublisher().Monitor(routines.GetSlot<Timer::Result>(
    [&] (Timer::Result result) {
      auto timestamp = backtesterServiceClients.GetTimeClient().GetTime();
      CPPUNIT_ASSERT(timestamp == expectedTimestamp);
      CPPUNIT_ASSERT(result == Timer::Result::CANCELED);
      boost::lock_guard<Mutex> lock{queryCompleteMutex};
      testSucceeded = true;
      testCompleteCondition.notify_one();
    }));
  routines.Push(
    [&] {
      timerB->Start();
      timerA->Start();
    });
  boost::unique_lock<Mutex> lock{queryCompleteMutex};
  while(!testSucceeded.is_initialized()) {
    testCompleteCondition.wait(lock);
  }
  CPPUNIT_ASSERT(*testSucceeded);
}
