#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;

namespace {
  const auto TEST_SECURITY = Security("TST", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());

  auto StoreBboQuote(LocalHistoricalDataStore& dataStore, Money bidPrice,
      Quantity bidQuantity, Money askPrice, Quantity askQuantity,
      const ptime& timestamp, const Beam::Queries::Sequence& sequence) {
    auto quote = SequencedSecurityBboQuote(SecurityBboQuote(BboQuote(
      Quote(bidPrice, bidQuantity, Side::BID),
      Quote(askPrice, askQuantity, Side::ASK), timestamp), TEST_SECURITY),
      sequence);
    dataStore.Store(quote);
    return quote;
  }

  void TestBboQuoteQuery(LocalHistoricalDataStore& dataStore,
      const Beam::Queries::Range& range, const SnapshotLimit& limit,
      const std::vector<SequencedSecurityBboQuote>& expectedResult) {
    auto query = SecurityMarketDataQuery();
    query.SetIndex(TEST_SECURITY);
    query.SetRange(range);
    query.SetSnapshotLimit(limit);
    auto queryResult = dataStore.LoadBboQuotes(query);
    auto transformedExpectedResult = std::vector<SequencedBboQuote>(
      expectedResult.begin(), expectedResult.end());
    REQUIRE(transformedExpectedResult == queryResult);
  }
}

TEST_SUITE("LocalHistoricalDataStore") {
  TEST_CASE("store_and_load_bbo_quote") {
    auto dataStore = LocalHistoricalDataStore();
    auto sequence = Beam::Queries::Sequence(5);
    auto timeClient = IncrementalTimeClient();
    auto bboQuoteA = StoreBboQuote(dataStore, Money::ONE, 100,
      Money::ONE + Money::CENT, 100, timeClient.GetTime(), sequence);
    sequence = Increment(sequence);
    auto bboQuoteB = StoreBboQuote(dataStore, Money::ONE, 200,
      Money::ONE + Money::CENT, 200, timeClient.GetTime(), sequence);
    sequence = Increment(sequence);
    auto bboQuoteC = StoreBboQuote(dataStore, Money::ONE, 300,
      Money::ONE + Money::CENT, 300, timeClient.GetTime(), sequence);
    TestBboQuoteQuery(dataStore, Beam::Queries::Range::Total(),
      SnapshotLimit::Unlimited(), {bboQuoteA, bboQuoteB, bboQuoteC});
    TestBboQuoteQuery(dataStore, Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 0),
      std::vector<SequencedSecurityBboQuote>());
    TestBboQuoteQuery(dataStore, Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 1), {bboQuoteA});
    TestBboQuoteQuery(dataStore, Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 2), {bboQuoteA, bboQuoteB});
    TestBboQuoteQuery(dataStore, Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 3),
      {bboQuoteA, bboQuoteB, bboQuoteC});
    TestBboQuoteQuery(dataStore, Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::HEAD, 4),
      {bboQuoteA, bboQuoteB, bboQuoteC});
    TestBboQuoteQuery(dataStore, Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 0),
      std::vector<SequencedSecurityBboQuote>());
    TestBboQuoteQuery(dataStore, Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 1), {bboQuoteC});
    TestBboQuoteQuery(dataStore, Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 2), {bboQuoteB, bboQuoteC});
    TestBboQuoteQuery(dataStore, Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 3),
      {bboQuoteA, bboQuoteB, bboQuoteC});
    TestBboQuoteQuery(dataStore, Beam::Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 4),
      {bboQuoteA, bboQuoteB, bboQuoteC});
  }
}
