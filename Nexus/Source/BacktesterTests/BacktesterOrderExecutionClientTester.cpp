#include <doctest/doctest.h>
#include "Nexus/Backtester/BacktesterServiceClients.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;

TEST_SUITE("BacktesterOrderExecutionClient") {
  TEST_CASE("fill_limit_order") {
    auto startTime = ptime(date(2020, 12, 21), seconds(10));
    auto dataStore = std::make_shared<LocalHistoricalDataStore>();
    auto security = Security("TST", DefaultMarkets::NYSE(),
      DefaultCountries::US());
    auto timestamp = startTime - seconds(1);
    auto bboQuote = SequencedValue(IndexedValue(
      BboQuote(Quote(99 * Money::CENT, 100, Side::BID),
        Quote(Money::ONE, 100, Side::ASK), timestamp), security),
      EncodeTimestamp(timestamp, Beam::Queries::Sequence(
        static_cast<Beam::Queries::Sequence::Ordinal>(1))));
    dataStore->Store(bboQuote);
    timestamp = startTime + seconds(1);
    bboQuote = SequencedValue(IndexedValue(
      BboQuote(Quote(98 * Money::CENT, 100, Side::BID),
        Quote(99 * Money::CENT, 100, Side::ASK), timestamp), security),
      EncodeTimestamp(timestamp, Beam::Queries::Sequence(
        static_cast<Beam::Queries::Sequence::Ordinal>(2))));
    dataStore->Store(bboQuote);
    auto testEnvironment = TestEnvironment(HistoricalDataStoreBox(dataStore));
    auto backtesterEnvironment = BacktesterEnvironment(startTime,
      ServiceClientsBox(std::in_place_type<TestServiceClients>,
        Ref(testEnvironment)));
    auto serviceClients = BacktesterServiceClients(Ref(backtesterEnvironment));
    auto& orderExecutionClient = serviceClients.GetOrderExecutionClient();
    auto& order = orderExecutionClient.Submit(OrderFields::MakeLimitOrder(
      security, Side::BID, 100, 99 * Money::CENT));
    auto executionReports = std::make_shared<Queue<ExecutionReport>>();
    order.GetPublisher().Monitor(executionReports);
    REQUIRE(executionReports->Pop().m_status == OrderStatus::PENDING_NEW);
    REQUIRE(executionReports->Pop().m_status == OrderStatus::NEW);
    auto fill = executionReports->Pop();
    REQUIRE(fill.m_status == OrderStatus::FILLED);
    REQUIRE(fill.m_lastPrice == 99 * Money::CENT);
    REQUIRE(fill.m_lastQuantity == 100);
  }
}
