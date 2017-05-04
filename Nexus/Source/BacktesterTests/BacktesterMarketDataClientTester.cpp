#include "Nexus/BacktesterTests/BacktesterMarketDataClientTester.hpp"
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterMarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::Queries;
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
  Security security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
  for(auto i = 0; i < 100; ++i) {
    auto bboQuote = MakeSequencedValue(MakeIndexedValue(
      BboQuote{Quote{Money::ONE, 100, Side::BID},
      Quote{Money::ONE, 100, Side::ASK}, startTime + seconds(i)}, security),
      Beam::Queries::Sequence{
      static_cast<Beam::Queries::Sequence::Ordinal>(i)});
    testEnvironment.GetMarketDataEnvironment().GetDataStore().Store(bboQuote);
  }
  testEnvironment.Open();
  TestServiceClients serviceClients{Ref(testEnvironment)};
  serviceClients.Open();
  BacktesterEnvironment backtesterEnvironment{
    Ref(serviceClients.GetMarketDataClient()), startTime};
  backtesterEnvironment.Open();
  BacktesterMarketDataClient marketDataClient{Ref(backtesterEnvironment)};
  marketDataClient.Open();
  auto query = BuildRealTimeWithSnapshotQuery(security);
  auto queue = std::make_shared<Queue<SequencedBboQuote>>();
  marketDataClient.QueryBboQuotes(query, queue);
}
