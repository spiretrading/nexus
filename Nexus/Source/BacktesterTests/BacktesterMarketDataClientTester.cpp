#include "Nexus/BacktesterTests/BacktesterMarketDataClientTester.hpp"
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterServiceClients.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
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

void BacktesterMarketDataClientTester::TestRealTimeQuery() {
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
  BacktesterEnvironment backtesterEnvironment{startTime};
  backtesterEnvironment.Open();
  BacktesterServiceClients backtesterServiceClients{
    Ref(backtesterEnvironment)};
  backtesterServiceClients.Open();
  RoutineTaskQueue routines;
  auto& marketDataClient = backtesterServiceClients.GetMarketDataClient();
  auto query = BuildRealTimeWithSnapshotQuery(security);
  auto expectedTimestamp = startTime;
  auto finalTimestamp = startTime + seconds(COUNT - 1);
  Mutex queryCompleteMutex;
  ConditionVariable queryCompleteCondition;
  optional<bool> testSucceeded;
  marketDataClient.QueryBboQuotes(query, routines.GetSlot<SequencedBboQuote>(
    [&] (const SequencedBboQuote& bboQuote) {
      boost::lock_guard<Mutex> lock{queryCompleteMutex};
      if(bboQuote->m_timestamp != expectedTimestamp) {
        testSucceeded = false;
        queryCompleteCondition.notify_one();
      } else if(expectedTimestamp == finalTimestamp) {
        testSucceeded = true;
        queryCompleteCondition.notify_one();
      } else {
        expectedTimestamp = expectedTimestamp + seconds(1);
      }
    }));
  boost::unique_lock<Mutex> lock{queryCompleteMutex};
  while(!testSucceeded.is_initialized()) {
    queryCompleteCondition.wait(lock);
  }
  CPPUNIT_ASSERT(*testSucceeded);
}
