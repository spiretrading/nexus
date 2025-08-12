#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Backtester/BacktesterMarketDataService.hpp"
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

  MarketDataClient make_market_data_client(TestEnvironment& environment) {
    environment.get_service_locator_environment().GetRoot().
      MakeAccount("backtester", "", DirectoryEntry::GetStarDirectory());
    auto service_locator =
      environment.get_service_locator_environment().MakeClient(
        "backtester", "");
    grant_all_entitlements(environment.get_administration_environment(),
      service_locator.GetAccount());
    return environment.get_market_data_environment().make_registry_client(
      service_locator);
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
      m_source_market_data_client =
        make_market_data_client(m_source_environment);
      m_backtesting_market_data_client =
        make_market_data_client(m_event_handler_environment);
    }
  };
}

TEST_SUITE("BacktesterMarketDataService") {
  TEST_CASE("MarketDataEvent") {
    auto fixture = Fixture();
    auto timestamp =
      fixture.m_event_handler_environment.get_time_environment().GetTime();
    auto event_handler = BacktesterEventHandler(
      fixture.m_event_handler_environment.get_time_environment().GetTime());
    auto service = BacktesterMarketDataService(Ref(event_handler),
      Ref(fixture.m_event_handler_environment.get_market_data_environment()),
      *fixture.m_source_market_data_client);
    auto bbo = BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(Money::ONE, 100, Side::ASK), timestamp);
    auto event = MarketDataEvent(TD, bbo, timestamp, Ref(service));
    REQUIRE(event.get_timestamp() == timestamp);
    auto bbo_queue = std::make_shared<Queue<BboQuote>>();
    auto query = MakeRealTimeQuery(TD);
    fixture.m_backtesting_market_data_client->query(query, bbo_queue);
    event.execute();
    auto received_bbo = bbo_queue->Pop();
    REQUIRE(received_bbo == bbo);
  }

  TEST_CASE("query_bbo_quotes") {
    auto fixture = Fixture();
    auto start_time =
      fixture.m_event_handler_environment.get_time_environment().GetTime();
    auto& data_store = fixture.
      m_source_environment.get_market_data_environment().get_data_store();
    auto bbo_before = BboQuote(Quote(Money(99), 100, Side::BID),
      Quote(Money(101), 100, Side::ASK), start_time - minutes(10));
    data_store.store(SequencedValue(
      IndexedValue(bbo_before, TD), Beam::Queries::Sequence(10)));
    auto bbo_at_start = BboQuote(Quote(Money(100), 100, Side::BID),
      Quote(Money(102), 100, Side::ASK), start_time);
    data_store.store(SequencedValue(
      IndexedValue(bbo_at_start, TD), Beam::Queries::Sequence(11)));
    auto bbo_after = BboQuote(Quote(Money(101), 100, Side::BID),
      Quote(Money(103), 100, Side::ASK), start_time + minutes(10));
    data_store.store(
      SequencedValue(IndexedValue(bbo_after, TD), Beam::Queries::Sequence(12)));
    auto event_handler = BacktesterEventHandler(start_time);
    auto service = BacktesterMarketDataService(Ref(event_handler),
      Ref(fixture.m_event_handler_environment.get_market_data_environment()),
      *fixture.m_source_market_data_client);
    auto bbo_queue = std::make_shared<Queue<BboQuote>>();
    auto query = MakeRealTimeQuery(TD);
    fixture.m_backtesting_market_data_client->query(query, bbo_queue);
    service.query_bbo_quotes(query);
    auto received_bbo1 = bbo_queue->Pop();
    REQUIRE(received_bbo1 == bbo_at_start);
    auto received_bbo2 = bbo_queue->Pop();
    REQUIRE(received_bbo2 == bbo_after);
  }
}
