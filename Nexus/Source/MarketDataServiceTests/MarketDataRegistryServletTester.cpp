#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  struct Fixture {
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaMarketDataRegistryServlet<MarketDataRegistry*,
        LocalHistoricalDataStore*, AdministrationClient>, NativePointerPolicy>;
    FixedTimeClient m_time_client;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    optional<ServiceLocatorClient> m_servlet_service_locator_client;
    optional<AdministrationClient> m_servlet_administration_client;
    MarketDataRegistry m_registry;
    LocalHistoricalDataStore m_data_store;
    optional<ServletContainer::Servlet::Servlet> m_servlet;
    std::shared_ptr<LocalServerConnection> m_server_connection;
    optional<ServletContainer> m_container;
    DirectoryEntry m_client_account;
    std::unique_ptr<TestServiceProtocolClient> m_client;

    auto make_account(const std::string& name, const DirectoryEntry& parent) {
      return m_service_locator_environment.get_root().make_account(
        name, "", parent);
    }

    auto make_client(const std::string& name) {
      auto service_locator_client =
        m_service_locator_environment.make_client(name, "");
      auto authenticator = SessionAuthenticator(Ref(service_locator_client));
      auto protocol_client = std::make_unique<TestServiceProtocolClient>(
        std::make_unique<LocalClientChannel>(name, *m_server_connection),
        init());
      Nexus::register_query_types(
        Beam::out(protocol_client->get_slots().get_registry()));
      register_market_data_registry_services(out(protocol_client->get_slots()));
      register_market_data_registry_messages(out(protocol_client->get_slots()));
      authenticator(*protocol_client);
      return std::tuple(
        service_locator_client.get_account(), std::move(protocol_client));
    }

    Fixture()
        : m_time_client(time_from_string("2024-07-04 12:00:00")),
          m_server_connection(std::make_shared<LocalServerConnection>()),
          m_administration_environment(
            make_administration_service_test_environment(
              m_service_locator_environment)),
          m_registry(DEFAULT_VENUES, get_default_time_zone_database()) {
      auto servlet_account =
        make_account("market_data_service", DirectoryEntry::STAR_DIRECTORY);
      m_administration_environment.make_administrator(servlet_account);
      m_service_locator_environment.get_root().store(
        servlet_account, DirectoryEntry::STAR_DIRECTORY, Permissions(~0));
      m_servlet_service_locator_client.emplace(
        m_service_locator_environment.make_client(servlet_account.m_name, ""));
      m_servlet_administration_client.emplace(
        m_administration_environment.make_client(
          Ref(*m_servlet_service_locator_client)));
      m_servlet.emplace(
        *m_servlet_administration_client, &m_registry, &m_data_store);
      m_container.emplace(init(
        *m_servlet_service_locator_client, &*m_servlet),
        m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
      m_client_account = make_account("client", DirectoryEntry::STAR_DIRECTORY);
      auto global_entitlement = m_servlet_administration_client->
        load_entitlements().get_entries().front().m_group_entry;
      m_servlet_administration_client->store_entitlements(
        m_client_account, {global_entitlement});
      std::tie(m_client_account, m_client) = make_client("client");
    }
  };

  template<typename QueryService, typename MessageType,
    typename MakeData, typename LoadFromStore, typename GetRecord>
  void test_query_publish(Fixture& fixture, const Ticker& ticker,
      MakeData&& make_data, LoadFromStore&& load_from_store,
      GetRecord&& get_record) {
    auto query = TickerMarketDataQuery();
    query.set_index(ticker);
    query.set_range(Range::REAL_TIME);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto result = fixture.m_client->send_request<QueryService>(query);
    REQUIRE(result.m_id != -1);
    REQUIRE(result.m_snapshot.empty());
    auto data = make_data();
    fixture.m_servlet->publish(data, 1);
    auto message = fixture.m_client->read_message();
    auto received_message = std::dynamic_pointer_cast<
      RecordMessage<MessageType, TestServiceProtocolClient>>(message);
    auto received_data = get_record(received_message->get_record());
    REQUIRE(*received_data == *data);
    auto data_store_query = TickerMarketDataQuery();
    data_store_query.set_index(ticker);
    data_store_query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    data_store_query.set_range(
      received_data.get_sequence(), increment(received_data.get_sequence()));
    auto stored_data = load_from_store(data_store_query);
    REQUIRE(stored_data.size() == 1);
    REQUIRE(stored_data.front() == to_sequenced_value(received_data));
    SUBCASE("query_no_entitlement") {
      auto client_account =
        fixture.make_account("client2", DirectoryEntry::STAR_DIRECTORY);
      auto client = std::unique_ptr<TestServiceProtocolClient>();
      std::tie(client_account, client) = fixture.make_client("client2");
      auto query = TickerMarketDataQuery();
      query.set_index(ticker);
      query.set_range(Range::TOTAL);
      query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto result = client->send_request<QueryService>(query);
      REQUIRE(result.m_id == -1);
      REQUIRE(result.m_snapshot.empty());
    }
  }
}

TEST_SUITE("MarketDataRegistryServlet") {
  TEST_CASE("query_ticker_info") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("A.TSX");
    auto info = TickerInfo();
    info.m_ticker = ticker;
    info.m_name = "TICKER A";
    info.m_board_lot = 100;
    fixture.m_data_store.store(info);
    fixture.m_registry.add(info);
    auto query = make_ticker_info_query(ticker);
    auto result =
      fixture.m_client->send_request<QueryTickerInfoService>(query);
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == info);
  }

  TEST_CASE("load_ticker_info_from_prefix") {
    auto fixture = Fixture();
    auto ticker_a = parse_ticker("A.TSX");
    auto info_a = TickerInfo();
    info_a.m_ticker = ticker_a;
    info_a.m_name = "TICKER A";
    info_a.m_board_lot = 100;
    fixture.m_registry.add(info_a);
    auto ticker_b = parse_ticker("B.TSX");
    auto info_b = TickerInfo();
    info_b.m_ticker = ticker_b;
    info_b.m_name = "TICKER B";
    info_b.m_board_lot = 100;
    fixture.m_registry.add(info_b);
    auto ticker_c = parse_ticker("C.TSX");
    auto info_c = TickerInfo();
    info_c.m_ticker = ticker_c;
    info_c.m_name = "TICKER C";
    info_c.m_board_lot = 100;
    fixture.m_registry.add(info_c);
    auto result =
      fixture.m_client->send_request<LoadTickerInfoFromPrefixService>("A");
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == info_a);
  }

  TEST_CASE("query_publish_order_imbalance") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("A.TSX");
    auto info = TickerInfo();
    info.m_ticker = ticker;
    info.m_name = "TICKER A";
    info.m_board_lot = 100;
    fixture.m_registry.add(info);
    auto query = VenueMarketDataQuery();
    query.set_index(TSX);
    query.set_range(Range::REAL_TIME);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto result =
      fixture.m_client->send_request<QueryOrderImbalancesService>(query);
    REQUIRE(result.m_id != -1);
    REQUIRE(result.m_snapshot.empty());
    auto imbalance = VenueOrderImbalance(
      OrderImbalance(ticker, Side::ASK, 100, Money::ONE,
        fixture.m_time_client.get_time()), TSX);
    fixture.m_servlet->publish(imbalance, 1);
    auto message = fixture.m_client->read_message();
    auto received_imbalance_message = std::dynamic_pointer_cast<
      RecordMessage<OrderImbalanceMessage, TestServiceProtocolClient>>(message);
    auto received_imbalance =
      received_imbalance_message->get_record().order_imbalance;
    REQUIRE(*received_imbalance == *imbalance);
    auto data_store_query = VenueMarketDataQuery();
    data_store_query.set_index(TSX);
    data_store_query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    data_store_query.set_range(received_imbalance.get_sequence(),
      increment(received_imbalance.get_sequence()));
    auto stored_imbalances =
      fixture.m_data_store.load_order_imbalances(data_store_query);
    REQUIRE(stored_imbalances.size() == 1);
    REQUIRE(
      stored_imbalances.front() == to_sequenced_value(received_imbalance));
    SUBCASE("query_no_entitlement") {
      auto client_account =
        fixture.make_account("client2", DirectoryEntry::STAR_DIRECTORY);
      auto client = std::unique_ptr<TestServiceProtocolClient>();
      std::tie(client_account, client) = fixture.make_client("client2");
      auto query = VenueMarketDataQuery();
      query.set_index(TSX);
      query.set_range(Range::TOTAL);
      query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto result = client->send_request<QueryOrderImbalancesService>(query);
      REQUIRE(result.m_id == -1);
      REQUIRE(result.m_snapshot.empty());
    }
  }

  TEST_CASE("query_publish_bbo_quote") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("A.TSX");
    auto info = TickerInfo();
    info.m_ticker = ticker;
    info.m_name = "TICKER A";
    info.m_board_lot = 100;
    fixture.m_registry.add(info);
    test_query_publish<QueryBboQuotesService, BboQuoteMessage>(
      fixture, ticker, [&] {
        return TickerBboQuote(
          BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
            fixture.m_time_client.get_time()), ticker);
      }, [&] (const auto& query) {
        return fixture.m_data_store.load_bbo_quotes(query);
      }, [] (const auto& record) {
        return record.bbo_quote;
      });
  }

  TEST_CASE("query_publish_book_quote") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("A.TSX");
    auto info = TickerInfo();
    info.m_ticker = ticker;
    info.m_name = "TICKER A";
    info.m_board_lot = 100;
    fixture.m_registry.add(info);
    test_query_publish<QueryBookQuotesService, BookQuoteMessage>(
      fixture, ticker, [&] {
        return TickerBookQuote(
          BookQuote("MP1", false, TSX, make_bid(Money::CENT, 100),
            fixture.m_time_client.get_time()), ticker);
      }, [&] (const auto& query) {
        return fixture.m_data_store.load_book_quotes(query);
      }, [] (const auto& record) {
        return record.book_quote;
      });
  }

  TEST_CASE("query_publish_time_and_sale") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("A.TSX");
    auto info = TickerInfo();
    info.m_ticker = ticker;
    info.m_name = "TICKER A";
    info.m_board_lot = 100;
    fixture.m_registry.add(info);
    test_query_publish<QueryTimeAndSalesService, TimeAndSaleMessage>(
      fixture, ticker, [&] {
        return TickerTimeAndSale(TimeAndSale(fixture.m_time_client.get_time(),
          Money::ONE, 100, TimeAndSale::Condition(), "TSX", "", ""), ticker);
      }, [&] (const auto& query) {
        return fixture.m_data_store.load_time_and_sales(query);
      }, [] (const auto& record) {
        return record.time_and_sale;
      });
  }
}
