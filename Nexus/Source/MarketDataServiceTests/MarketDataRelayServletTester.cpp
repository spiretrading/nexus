#include <atomic>
#include <future>
#include <Beam/IO/ConnectException.hpp>
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
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/MarketDataService/DistributedMarketDataClient.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataRelayServlet.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace Nexus::Venues;

namespace {
  struct AvailableMarketDataClient : TestMarketDataClient {
    std::atomic_bool* m_available;

    AvailableMarketDataClient(
        ScopedQueueWriter<std::shared_ptr<Operation>> operations,
        std::atomic_bool& available)
        : TestMarketDataClient(std::move(operations)),
          m_available(&available) {}

    using TestMarketDataClient::query;

    void query(const TickerQuery& query,
        ScopedQueueWriter<SequencedTickerStatus> queue) {
      if(!*m_available) {
        throw ConnectException();
      }
      TestMarketDataClient::query(query, std::move(queue));
    }
  };

  struct Fixture {
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaMarketDataRelayServlet<MarketDataClient, AdministrationClient>>;
    bool m_discover;
    QueueWriterPublisher<ServiceUpdate> m_services;
    std::atomic_bool m_available;
    FixedTimeClient m_time_client;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    optional<ServiceLocatorClient> m_servlet_service_locator_client;
    optional<AdministrationClient> m_servlet_administration_client;
    std::shared_ptr<LocalServerConnection> m_server_connection;
    optional<ServletContainer> m_container;
    std::shared_ptr<Queue<std::shared_ptr<TestMarketDataClient::Operation>>>
      m_operations;
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

    auto make_relay_client() {
      if(m_discover) {
        return std::make_unique<MarketDataClient>(std::in_place_type<
          DistributedMarketDataClient<std::shared_ptr<TriggerTimer>>>,
          [=, this] (ScopedQueueWriter<ServiceUpdate> queue) {
            m_services.monitor(std::move(queue));
          }, [] (const auto&) {
            return Scope(TSX);
          }, [=, this] (const Scope&) {
            return std::make_shared<MarketDataClient>(
              std::in_place_type<AvailableMarketDataClient>, m_operations,
              m_available);
          }, std::make_shared<TriggerTimer>());
      }
      return std::make_unique<MarketDataClient>(
        std::in_place_type<AvailableMarketDataClient>, m_operations,
        m_available);
    }

    Fixture()
      : Fixture(false) {}

    explicit Fixture(bool discover)
        : m_discover(discover),
          m_available(true),
          m_time_client(time_from_string("2024-07-04 12:00:00")),
          m_server_connection(std::make_shared<LocalServerConnection>()),
          m_administration_environment(
            make_administration_service_test_environment(
              m_service_locator_environment)),
          m_operations(std::make_shared<
            Queue<std::shared_ptr<TestMarketDataClient::Operation>>>()) {
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
      m_container.emplace(
        init(*m_servlet_service_locator_client, init(seconds(100),
          std::bind_front(&Fixture::make_relay_client, this), 1, 1,
          m_administration_environment.make_client(
            Ref(*m_servlet_service_locator_client)))),
        m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
      m_client_account = make_account("client", DirectoryEntry::STAR_DIRECTORY);
      m_administration_environment.grant_all_entitlements(m_client_account);
      std::tie(m_client_account, m_client) = make_client("client");
    }
  };
}

TEST_SUITE("MarketDataRegistryServlet") {
  TEST_CASE("query_ticker_status_waits_for_scope") {
    auto fixture = Fixture(true);
    auto ticker = parse_ticker("TST.TSX");
    auto result = Async<QueryTickerStatusService::Return>();
    auto request = RoutineHandler(spawn([&] {
      try {
        result.get_eval().set(fixture.m_client->send_request<
          QueryTickerStatusService>(make_real_time_query(ticker)));
      } catch(...) {
        result.get_eval().set_exception(std::current_exception());
      }
    }));
    flush_pending_routines();
    REQUIRE(result.get_state() == BaseAsync::State::PENDING);
    REQUIRE(!fixture.m_operations->try_pop());
    fixture.m_services.push(ServiceUpdate::add(ServiceEntry(
      "market_data_service", JsonObject(), 1, DirectoryEntry::ROOT_ACCOUNT)));
    auto info = fixture.m_operations->pop();
    auto& info_query =
      std::get<TestMarketDataClient::TickerInfoQueryOperation>(*info);
    info_query.m_result.set({TickerInfo(ticker, "Test", "Tech", 100)});
    auto initial = fixture.m_operations->pop();
    std::get<TestMarketDataClient::QuerySequencedTickerStatusOperation>(
      *initial).m_queue.close();
    auto subscription = fixture.m_operations->pop();
    auto& subscription_query = std::get<
      TestMarketDataClient::QuerySequencedTickerStatusOperation>(*subscription);
    REQUIRE(subscription_query.m_query.get_index() == ticker);
    auto snapshot = fixture.m_operations->pop();
    std::get<TestMarketDataClient::QuerySequencedTickerStatusOperation>(
      *snapshot).m_queue.close();
    REQUIRE(result.get().m_id != -1);
  }

  TEST_CASE("query_ticker_status_after_connection_failure") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto query = make_real_time_query(ticker);
    fixture.m_available = false;
    auto first_result = std::async(std::launch::async, [&] {
      return fixture.m_client->send_request<QueryTickerStatusService>(query);
    });
    auto info = fixture.m_operations->pop();
    auto& info_query =
      std::get<TestMarketDataClient::TickerInfoQueryOperation>(*info);
    info_query.m_result.set({TickerInfo(ticker, "Test", "Tech", 100)});
    REQUIRE_THROWS_AS(first_result.get(), ServiceRequestException);
    fixture.m_available = true;
    auto result = std::async(std::launch::async, [&] {
      return fixture.m_client->send_request<QueryTickerStatusService>(query);
    });
    auto initial = fixture.m_operations->pop();
    auto& initial_query = std::get<
      TestMarketDataClient::QuerySequencedTickerStatusOperation>(*initial);
    initial_query.m_queue.close();
    auto subscription = fixture.m_operations->pop();
    auto& subscription_query = std::get<
      TestMarketDataClient::QuerySequencedTickerStatusOperation>(
        *subscription);
    REQUIRE(subscription_query.m_query.get_index() == ticker);
    REQUIRE(
      subscription_query.m_query.get_range().get_end() == Beam::Sequence::LAST);
    auto snapshot = fixture.m_operations->pop();
    auto& snapshot_query = std::get<
      TestMarketDataClient::QuerySequencedTickerStatusOperation>(*snapshot);
    snapshot_query.m_queue.close();
    REQUIRE(result.get().m_id != -1);
  }

  TEST_CASE("query_ticker_info") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto query = TickerInfoQuery();
    query.set_index(ticker);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto query_thread = std::async(std::launch::async, [&] {
      return fixture.m_client->send_request<QueryTickerInfoService>(query);
    });
    auto operation = fixture.m_operations->pop();
    auto& ticker_info_operation =
      std::get<TestMarketDataClient::TickerInfoQueryOperation>(*operation);
    REQUIRE(ticker_info_operation.m_query.get_index() == ticker);
    auto ticker_info = TickerInfo();
    ticker_info.m_ticker = ticker;
    ticker_info.m_name = "Test";
    ticker_info.m_sector = "Tech";
    ticker_info.m_board_lot = 100;
    ticker_info_operation.m_result.set({ticker_info});
    auto result = query_thread.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == ticker_info);
  }

  TEST_CASE("query_ticker_status") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(Beam::Sequence::FIRST, Beam::Sequence::PRESENT);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto query_thread = std::async(std::launch::async, [&] {
      return fixture.m_client->send_request<QueryTickerStatusService>(query);
    });
    auto info_operation_ptr = fixture.m_operations->pop();
    auto& info_operation =
      std::get<TestMarketDataClient::TickerInfoQueryOperation>(
        *info_operation_ptr);
    auto ticker_info = TickerInfo(ticker, "Test", "Tech", 100);
    info_operation.m_result.set({ticker_info});
    auto query_operation_ptr = fixture.m_operations->pop();
    auto& query_operation =
      std::get<TestMarketDataClient::QuerySequencedTickerStatusOperation>(
        *query_operation_ptr);
    REQUIRE(query_operation.m_query.get_index() == ticker);
    auto status = SequencedTickerStatus(
      TickerStatus(TSX, "Authorized", TickerStatus::Flag::IS_CONTINUOUS,
        time_from_string("2024-07-04 09:30:00")), Beam::Sequence(1));
    query_operation.m_queue.push(status);
    query_operation.m_queue.close();
    auto result = query_thread.get();
    REQUIRE(result.m_snapshot.size() == 1);
    REQUIRE(result.m_snapshot.front() == status);
  }
}
