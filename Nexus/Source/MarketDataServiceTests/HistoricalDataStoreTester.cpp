#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/SqlHistoricalDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;

namespace {
  const auto SECURITY_A = Security("A", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  const auto SECURITY_B = Security("B", DefaultMarkets::TSX(),
    DefaultCountries::CA());
  const auto SECURITY_C = Security("C", DefaultMarkets::NYSE(),
    DefaultCountries::US());

  template<typename T>
  struct SpecializedMakeDataStore {
    auto operator ()() const {
      return T();
    }
  };

  template<>
  struct SpecializedMakeDataStore<
      SqlHistoricalDataStore<Viper::Sqlite3::Connection>> {
    auto operator ()() const {
      return SqlHistoricalDataStore<Viper::Sqlite3::Connection>(
        [] {
          return Viper::Sqlite3::Connection("file::memory:?cache=shared");
        });
    }
  };

  template<typename T>
  auto MakeDataStore() {
    return SpecializedMakeDataStore<T>()();
  };

  template<typename DataStore>
  auto StoreBboQuote(DataStore& dataStore, Money bidPrice, Quantity bidQuantity,
      Money askPrice, Quantity askQuantity, ptime timestamp,
      const Beam::Queries::Sequence& sequence) {
    auto quote = SequencedSecurityBboQuote(SecurityBboQuote(BboQuote(
      Quote(bidPrice, bidQuantity, Side::BID),
      Quote(askPrice, askQuantity, Side::ASK), timestamp), SECURITY_A),
      sequence);
    dataStore.Store(quote);
    return quote;
  }

  template<typename DataStore>
  void TestBboQuoteQuery(DataStore& dataStore,
      const Beam::Queries::Range& range, const SnapshotLimit& limit,
      const std::vector<SequencedSecurityBboQuote>& expectedResult) {
    auto query = SecurityMarketDataQuery();
    query.SetIndex(SECURITY_A);
    query.SetRange(range);
    query.SetSnapshotLimit(limit);
    auto queryResult = dataStore.LoadBboQuotes(query);
    auto transformedExpectedResult = std::vector<SequencedBboQuote>(
      expectedResult.begin(), expectedResult.end());
    REQUIRE(transformedExpectedResult == queryResult);
  }
}

TEST_SUITE("HistoricalDataStore") {
  TEST_CASE_TEMPLATE("store_and_load_bbo_quote", T, LocalHistoricalDataStore,
      SqlHistoricalDataStore<Viper::Sqlite3::Connection>) {
    auto dataStore = MakeDataStore<T>();
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

  TEST_CASE_TEMPLATE("query_security_info_head", T, LocalHistoricalDataStore,
      SqlHistoricalDataStore<Viper::Sqlite3::Connection>) {
    auto dataStore = MakeDataStore<T>();
    auto infoA = SecurityInfo(SECURITY_A, "Alpha", "Finance", 100);
    auto infoB = SecurityInfo(SECURITY_B, "Beta", "Technology", 100);
    auto infoC = SecurityInfo(SECURITY_C, "Gamma", "Medicine", 100);
    dataStore.Store(infoB);
    dataStore.Store(infoC);
    dataStore.Store(infoA);
    auto queryA = SecurityInfoQuery();
    queryA.SetIndex(DefaultCountries::US());
    queryA.SetSnapshotLimit(SnapshotLimit::FromHead(1));
    auto matchesA = dataStore.LoadSecurityInfo(queryA);
    REQUIRE(matchesA.size() == 1);
    REQUIRE(matchesA[0] == infoA);
    auto queryB = SecurityInfoQuery();
    queryB.SetIndex(DefaultCountries::US());
    queryB.SetSnapshotLimit(SnapshotLimit::FromHead(1));
    queryB.SetAnchor(SECURITY_A);
    auto matchesB = dataStore.LoadSecurityInfo(queryB);
    REQUIRE(matchesB.size() == 1);
    REQUIRE(matchesB[0] == infoC);
    auto queryC = SecurityInfoQuery();
    queryC.SetIndex(Region::Global());
    queryC.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto matchesC = dataStore.LoadSecurityInfo(queryC);
    REQUIRE(matchesC.size() == 3);
    REQUIRE(matchesC[0] == infoA);
    REQUIRE(matchesC[1] == infoB);
    REQUIRE(matchesC[2] == infoC);
  }

  TEST_CASE_TEMPLATE("query_security_info_tail", T, LocalHistoricalDataStore,
      SqlHistoricalDataStore<Viper::Sqlite3::Connection>) {
    auto dataStore = MakeDataStore<T>();
    auto infoA = SecurityInfo(SECURITY_A, "Alpha", "Finance", 100);
    auto infoB = SecurityInfo(SECURITY_B, "Beta", "Technology", 100);
    auto infoC = SecurityInfo(SECURITY_C, "Gamma", "Medicine", 100);
    dataStore.Store(infoB);
    dataStore.Store(infoC);
    dataStore.Store(infoA);
    auto queryA = SecurityInfoQuery();
    queryA.SetIndex(DefaultCountries::US());
    queryA.SetSnapshotLimit(SnapshotLimit::FromTail(1));
    auto matchesA = dataStore.LoadSecurityInfo(queryA);
    REQUIRE(matchesA.size() == 1);
    REQUIRE(matchesA[0] == infoC);
    auto queryB = SecurityInfoQuery();
    queryB.SetIndex(DefaultCountries::US());
    queryB.SetSnapshotLimit(SnapshotLimit::FromTail(1));
    queryB.SetAnchor(SECURITY_C);
    auto matchesB = dataStore.LoadSecurityInfo(queryB);
    REQUIRE(matchesB.size() == 1);
    REQUIRE(matchesB[0] == infoA);
    auto queryC = SecurityInfoQuery();
    queryC.SetIndex(Region::Global());
    queryC.SetSnapshotLimit(SnapshotLimit::FromTail(2));
    auto matchesC = dataStore.LoadSecurityInfo(queryC);
    REQUIRE(matchesC.size() == 2);
    REQUIRE(matchesC[0] == infoB);
    REQUIRE(matchesC[1] == infoC);
  }
}
