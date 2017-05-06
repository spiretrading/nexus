#include "Nexus/BacktesterTests/BacktesterMarketDataClientTester.hpp"
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterServiceClients.hpp"
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
  testEnvironment.Open();
  Security security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
  for(auto i = 0; i < 100; ++i) {
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
  RoutineTaskQueue routines;
  auto& marketDataClient = backtesterServiceClients.GetMarketDataClient();
  auto query = BuildRealTimeWithSnapshotQuery(security);
  auto expectedSequence = Beam::Queries::Sequence{
    static_cast<Beam::Queries::Sequence::Ordinal>(1)};
  marketDataClient.QueryBboQuotes(query, routines.GetSlot<SequencedBboQuote>(
    [&] (const SequencedBboQuote& bboQuote) {
      CPPUNIT_ASSERT(bboQuote.GetSequence() == expectedSequence);
      expectedSequence = Increment(expectedSequence);
    }));
}
