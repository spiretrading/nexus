#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Backtester/BacktesterMarketDataClient.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;

namespace {
  const auto TD = Security("TD", TSX);

  struct Fixture {
    TestEnvironment m_source_environment;
    TestEnvironment m_event_handler_environment;
    BacktesterEventHandler m_event_handler;
    BacktesterMarketDataService m_market_data_service;

    Fixture()
      : m_source_environment(time_from_string("2025-08-12 09:00:00.000")),
        m_event_handler_environment(
          time_from_string("2025-08-12 09:00:00.000")),
        m_event_handler(time_from_string("2025-08-12 09:00:00.000")),
        m_market_data_service(Ref(m_event_handler),
          Ref(m_event_handler_environment.get_market_data_environment()),
          make_market_data_client(m_source_environment, "back_tester")) {
      auto start_time =
        m_event_handler_environment.get_time_environment().GetTime();
      auto& data_store =
        m_source_environment.get_market_data_environment().get_data_store();
      data_store.store(SequencedValue(IndexedValue(BboQuote(
        Quote(99 * Money::ONE, 100, Side::BID),
        Quote(100 * Money::ONE, 100, Side::ASK), start_time), TD),
        Beam::Queries::Sequence(10)));
      data_store.store(SequencedValue(IndexedValue(BboQuote(
        Quote(100 * Money::ONE, 100, Side::BID),
        Quote(101 * Money::ONE, 100, Side::ASK), start_time + seconds(1)), TD),
        Beam::Queries::Sequence(11)));
      data_store.store(SequencedValue(IndexedValue(BboQuote(
        Quote(101 * Money::ONE, 100, Side::BID),
        Quote(102 * Money::ONE, 100, Side::ASK), start_time + seconds(2)), TD),
        Beam::Queries::Sequence(12)));
    }
  };
}

TEST_SUITE("BacktesterMarketDataClient") {
  TEST_CASE("query_bbo_quote") {
    auto fixture = Fixture();
    auto client = BacktesterMarketDataClient(Ref(fixture.m_market_data_service),
      make_market_data_client(fixture.m_event_handler_environment, "client1"));
    auto query = MakeRealTimeQuery(TD);
    auto queue = std::make_shared<Queue<BboQuote>>();
    client.query(query, queue);
    auto bbo = queue->Pop();
    REQUIRE(bbo.m_bid.m_price == 99 * Money::ONE);
    bbo = queue->Pop();
    REQUIRE(bbo.m_bid.m_price == 100 * Money::ONE);
    bbo = queue->Pop();
    REQUIRE(bbo.m_bid.m_price == 101 * Money::ONE);
  }

  TEST_CASE("historical_query") {
    auto fixture = Fixture();
    auto start_time = time_from_string("2016-05-06 00:00:00.000");
    auto& data_store = fixture.m_event_handler_environment.
      get_market_data_environment().get_data_store();
    auto count = 6;
    for(auto i = 0; i < count; ++i) {
      auto timestamp = start_time - seconds(count - i - 1);
      auto bbo_quote = SequencedValue(IndexedValue(BboQuote(
        Quote(Money::ONE, 100, Side::BID), Quote(Money::ONE, 100, Side::ASK),
        timestamp), TD), Beam::Queries::Sequence(i + 2));
      data_store.store(bbo_quote);
    }
    auto client = BacktesterMarketDataClient(Ref(fixture.m_market_data_service),
      make_market_data_client(fixture.m_event_handler_environment, "handler1"));
    auto query = SecurityMarketDataQuery();
    query.SetIndex(TD);
    query.SetRange(Range::Historical());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queue = std::make_shared<Queue<SequencedBboQuote>>();
    client.query(query, queue);
    auto received = std::vector<SequencedBboQuote>();
    Flush(queue, std::back_inserter(received));
    REQUIRE(received.size() == 6);
    REQUIRE(received[0]->m_timestamp == start_time - seconds(5));
    REQUIRE(received[1]->m_timestamp == start_time - seconds(4));
    REQUIRE(received[2]->m_timestamp == start_time - seconds(3));
    REQUIRE(received[3]->m_timestamp == start_time - seconds(2));
    REQUIRE(received[4]->m_timestamp == start_time - seconds(1));
    REQUIRE(received[5]->m_timestamp == start_time);
  }
}
