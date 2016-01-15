#include "Nexus/MarketDataServiceTests/LocalHistoricalDataStoreTester.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace std;

namespace {
  Security TEST_SECURITY("TST", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
}

void LocalHistoricalDataStoreTester::setUp() {
  m_timeClient.Initialize();
}

void LocalHistoricalDataStoreTester::tearDown() {
  m_timeClient.Reset();
}

void LocalHistoricalDataStoreTester::TestStoreAndLoadBboQuote() {
  LocalHistoricalDataStore dataStore;
  Beam::Queries::Sequence sequence(5);
  SequencedSecurityBboQuote bboQuoteA = StoreBboQuote(dataStore, Money::ONE,
    100, Money::ONE + Money::CENT, 100, m_timeClient->GetTime(), sequence);
  sequence = Increment(sequence);
  SequencedSecurityBboQuote bboQuoteB = StoreBboQuote(dataStore, Money::ONE,
    200, Money::ONE + Money::CENT, 200, m_timeClient->GetTime(), sequence);
  sequence = Increment(sequence);
  SequencedSecurityBboQuote bboQuoteC = StoreBboQuote(dataStore, Money::ONE,
    300, Money::ONE + Money::CENT, 300, m_timeClient->GetTime(), sequence);
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

SequencedSecurityBboQuote LocalHistoricalDataStoreTester::StoreBboQuote(
    LocalHistoricalDataStore& dataStore, Money bidPrice, Quantity bidQuantity,
    Money askPrice, Quantity askQuantity, const ptime& timestamp,
    const Beam::Queries::Sequence& sequence) {
  SequencedSecurityBboQuote quote(SecurityBboQuote(BboQuote(
    Quote(bidPrice, bidQuantity, Side::BID),
    Quote(askPrice, askQuantity, Side::ASK), timestamp), TEST_SECURITY),
    sequence);
  dataStore.Store(quote);
  return quote;
}

void LocalHistoricalDataStoreTester::TestBboQuoteQuery(
    LocalHistoricalDataStore& dataStore, const Beam::Queries::Range& range,
    const SnapshotLimit& limit,
    const vector<SequencedSecurityBboQuote>& expectedResult) {
  SecurityMarketDataQuery query;
  query.SetIndex(TEST_SECURITY);
  query.SetRange(range);
  query.SetSnapshotLimit(limit);
  vector<SequencedBboQuote> queryResult = dataStore.LoadBboQuotes(query);
  vector<SequencedBboQuote> transformedExpectedResult(expectedResult.begin(),
    expectedResult.end());
  CPPUNIT_ASSERT(transformedExpectedResult == queryResult);
}
