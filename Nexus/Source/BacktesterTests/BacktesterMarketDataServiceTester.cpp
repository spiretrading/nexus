#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Backtester/BacktesterMarketDataService.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  const auto TD = Security("TD", TSX);

  MarketDataClient make_market_data_client(TestEnvironment& environment) {
    environment.get_service_locator_environment().get_root().
      make_account("backtester", "", DirectoryEntry::STAR_DIRECTORY);
    auto service_locator =
      environment.get_service_locator_environment().make_client(
        "backtester", "");
    grant_all_entitlements(environment.get_administration_environment(),
      service_locator.get_account());
    return environment.get_market_data_environment().make_registry_client(
      Ref(service_locator));
  }

  struct Fixture {
    TestEnvironment m_source_environment;
    TestEnvironment m_event_handler_environment;
    optional<MarketDataClient> m_source_market_data_client;
    optional<MarketDataClient> m_backtesting_market_data_client;

    Fixture()
        : m_source_environment(time_from_string("2025-08-12 09:00:00.000")),
          m_event_handler_environment(
            time_from_string("2025-08-12 09:00:00.000")) {
      m_source_market_data_client.emplace(
        make_market_data_client(m_source_environment));
      m_backtesting_market_data_client.emplace(
        make_market_data_client(m_event_handler_environment));
    }
  };
}

TEST_SUITE("BacktesterMarketDataService") {
  TEST_CASE("MarketDataEvent") {
    auto fixture = Fixture();
    auto timestamp =
      fixture.m_event_handler_environment.get_time_environment().get_time();
    auto event_handler = BacktesterEventHandler(
      fixture.m_event_handler_environment.get_time_environment().get_time());
    auto service = BacktesterMarketDataService(Ref(event_handler),
      Ref(fixture.m_event_handler_environment.get_market_data_environment()),
      *fixture.m_source_market_data_client);
    auto bbo =
      BboQuote(make_bid(Money::ONE, 100), make_ask(Money::ONE, 100), timestamp);
    auto event = MarketDataEvent(TD, bbo, timestamp, Ref(service));
    REQUIRE(event.get_timestamp() == timestamp);
    auto bbo_queue = std::make_shared<Queue<BboQuote>>();
    auto query = make_real_time_query(TD);
    fixture.m_backtesting_market_data_client->query(query, bbo_queue);
    event.execute();
    auto received_bbo = bbo_queue->pop();
    REQUIRE(received_bbo == bbo);
  }

  TEST_CASE("query_bbo_quotes") {
    auto fixture = Fixture();
    auto start_time =
      fixture.m_event_handler_environment.get_time_environment().get_time();
    auto& data_store = fixture.
      m_source_environment.get_market_data_environment().get_data_store();
    auto bbo_before = BboQuote(make_bid(Money(99), 100), make_ask(Money(101),
      100), start_time - minutes(10));
    data_store.store(SequencedValue(
      IndexedValue(bbo_before, TD), Beam::Sequence(10)));
    auto bbo_at_start = BboQuote(
      make_bid(Money(100), 100), make_ask(Money(102), 100), start_time);
    data_store.store(SequencedValue(
      IndexedValue(bbo_at_start, TD), Beam::Sequence(11)));
    auto bbo_after = BboQuote(make_bid(Money(101), 100), make_ask(Money(103),
      100), start_time + minutes(10));
    data_store.store(
      SequencedValue(IndexedValue(bbo_after, TD), Beam::Sequence(12)));
    auto event_handler = BacktesterEventHandler(start_time);
    auto service = BacktesterMarketDataService(Ref(event_handler),
      Ref(fixture.m_event_handler_environment.get_market_data_environment()),
      *fixture.m_source_market_data_client);
    auto bbo_queue = std::make_shared<Queue<BboQuote>>();
    auto query = make_real_time_query(TD);
    fixture.m_backtesting_market_data_client->query(query, bbo_queue);
    service.query_bbo_quotes(query);
    auto received_bbo1 = bbo_queue->pop();
    REQUIRE(received_bbo1 == bbo_at_start);
    auto received_bbo2 = bbo_queue->pop();
    REQUIRE(received_bbo2 == bbo_after);
  }
}
