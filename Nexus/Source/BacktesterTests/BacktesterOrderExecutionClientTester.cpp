#include "Nexus/BacktesterTests/BacktesterOrderExecutionClientTester.hpp"
#include <Beam/Queues/StateQueue.hpp>
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
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;
using namespace std;

void BacktesterOrderExecutionClientTester::TestOrderSubmissionAndFill() {
  ptime startTime{date{2016, 5, 6}, seconds(0)};
  TestEnvironment testEnvironment;
  testEnvironment.Open();
  Security security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
  for(auto i = 0; i < 100; ++i) {
    auto bboQuote = MakeSequencedValue(MakeIndexedValue(
      BboQuote{Quote{Money::ONE - i * Money::CENT, 100, Side::BID},
      Quote{Money::ONE  - i * Money::CENT, 100, Side::ASK},
      startTime + seconds(i)}, security), Beam::Queries::Sequence{
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
  auto& marketDataClient = backtesterServiceClients.GetMarketDataClient();
  auto& orderExecutionClient =
    backtesterServiceClients.GetOrderExecutionClient();
  RoutineTaskQueue routines;
  auto bboQuoteQueue = std::make_shared<StateQueue<BboQuote>>();
  Mutex queryCompleteMutex;
  ConditionVariable queryCompleteCondition;
  optional<bool> testSucceeded;
  routines.Push(
    [&] {
      auto bboQuery = BuildRealTimeWithSnapshotQuery(security);
      marketDataClient.QueryBboQuotes(bboQuery, bboQuoteQueue);
      auto fields = OrderFields::BuildLimitOrder(security, Side::BID, 100,
        90 * Money::CENT);
      auto& order = orderExecutionClient.Submit(fields);
      order.GetPublisher().Monitor(
        routines.GetSlot<ExecutionReport>(
        [=] (const ExecutionReport& report) {
          std::cout << bboQuoteQueue->Top().m_timestamp << std::endl;
          std::cout << report.m_status << std::endl;
        }));
      boost::lock_guard<Mutex> lock{queryCompleteMutex};
      testSucceeded = true;
      queryCompleteCondition.notify_one();
    });
  boost::unique_lock<Mutex> lock{queryCompleteMutex};
  while(!testSucceeded.is_initialized()) {
    queryCompleteCondition.wait(lock);
  }
  CPPUNIT_ASSERT(*testSucceeded);
}
