#include "Nexus/BacktesterTests/BacktesterTimerTester.hpp"
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterServiceClients.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::Tests;
using namespace std;

void BacktesterTimerTester::TestExpiry() {
  ptime startTime{date{2016, 5, 6}, seconds(0)};
  TestEnvironment testEnvironment;
  testEnvironment.Open();
  Security security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
  auto COUNT = 3;
  for(auto i = 0; i < COUNT; ++i) {
    auto bboQuote = MakeSequencedValue(MakeIndexedValue(
      BboQuote{Quote{Money::ONE, 100, Side::BID},
      Quote{Money::ONE, 100, Side::ASK}, startTime + seconds(i)}, security),
      Beam::Queries::Sequence{
      static_cast<Beam::Queries::Sequence::Ordinal>(i)});
    testEnvironment.GetMarketDataEnvironment().GetDataStore().Store(bboQuote);
  }
  TestServiceClients serviceClients{Ref(testEnvironment)};
  serviceClients.Open();
  BacktesterEnvironment backtesterEnvironment{
    Ref(serviceClients.GetMarketDataClient()), startTime};
  backtesterEnvironment.Open();
  BacktesterServiceClients backtesterServiceClients{
    Ref(backtesterEnvironment)};
  backtesterServiceClients.Open();
  auto timer = backtesterServiceClients.BuildTimer(seconds(1));
  RoutineTaskQueue routines;
  auto expectedTimestamp = startTime + seconds(1);
  Mutex queryCompleteMutex;
  ConditionVariable queryCompleteCondition;
  optional<bool> testSucceeded;
  timer->GetPublisher().Monitor(routines.GetSlot<Timer::Result>(
    [&] (Timer::Result result) {
      boost::lock_guard<Mutex> lock{queryCompleteMutex};
      auto timestamp = backtesterServiceClients.GetTimeClient().GetTime();
      if(timestamp == expectedTimestamp &&
          result == Timer::Result::EXPIRED) {
        testSucceeded = true;
        queryCompleteCondition.notify_one();
      } else {
        testSucceeded = false;
        queryCompleteCondition.notify_one();
      }
    }));
  timer->Start();
  auto& marketDataClient = backtesterServiceClients.GetMarketDataClient();
  auto query = BuildRealTimeWithSnapshotQuery(security);
  marketDataClient.QueryBboQuotes(query, std::make_shared<Queue<BboQuote>>());
  boost::unique_lock<Mutex> lock{queryCompleteMutex};
  while(!testSucceeded.is_initialized()) {
    queryCompleteCondition.wait(lock);
  }
  CPPUNIT_ASSERT(*testSucceeded);
}
