#include <future>
#include <type_traits>
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServlet.hpp"
#include "Nexus/OrderExecutionService/ServiceOrderExecutionClient.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionDataStore.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Destinations;
using namespace Nexus::Tests;
using namespace Nexus::Venues;

namespace {
  const auto TST = parse_ticker("TST.TSX");

  template<typename D = LocalOrderExecutionDataStore>
  struct Fixture {
    using DataStore = D;
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaOrderExecutionServlet<FixedTimeClient*, ServiceLocatorClient,
        UidClient, AdministrationClient, MockOrderExecutionDriver*,
        DataStore*>>;
    using ClientBuilder = AuthenticatedServiceProtocolClientBuilder<
      ServiceLocatorClient, MessageProtocol<std::unique_ptr<LocalClientChannel>,
        BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
    FixedTimeClient m_time_client;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    UidServiceTestEnvironment m_uid_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    MockOrderExecutionDriver m_driver;
    std::shared_ptr<Queue<std::shared_ptr<PrimitiveOrder>>> m_submissions;
    DataStore m_data_store;
    DirectoryEntry m_client_account;
    optional<ServiceLocatorClient> m_servlet_service_locator;
    optional<AdministrationClient> m_servlet_administration_client;
    std::shared_ptr<LocalServerConnection> m_server_connection;
    optional<ServletContainer> m_container;
    optional<ServiceLocatorClient> m_client_service_locator;
    optional<OrderExecutionClient> m_client;

    template<typename... Args>
    explicit Fixture(Args&&... args)
        : m_time_client(time_from_string("2025-04-12 13:33:12:55")),
          m_administration_environment(
            make_administration_service_test_environment(
              m_service_locator_environment)),
          m_submissions(
            std::make_shared<Queue<std::shared_ptr<PrimitiveOrder>>>()),
          m_data_store(std::forward<Args>(args)...),
          m_server_connection(std::make_shared<LocalServerConnection>()) {
      m_driver.set_order_status_new_on_submission(true);
      m_driver.get_publisher().monitor(m_submissions);
      auto servlet_account =
        m_service_locator_environment.get_root().make_account(
          "order_execution_service", "", DirectoryEntry::STAR_DIRECTORY);
      m_administration_environment.make_administrator(servlet_account);
      m_service_locator_environment.get_root().store(
        servlet_account, DirectoryEntry::STAR_DIRECTORY, Permissions(~0));
      m_client_account = m_service_locator_environment.get_root().make_account(
        "client", "1234", DirectoryEntry::STAR_DIRECTORY);
      m_servlet_service_locator.emplace(
        m_service_locator_environment.make_client(servlet_account.m_name, ""));
      m_servlet_administration_client.emplace(
        m_administration_environment.make_client(
          Ref(*m_servlet_service_locator)));
    }

    void start() {
      m_container.emplace(init(*m_servlet_service_locator, init(
        &m_time_client, *m_servlet_service_locator,
        m_uid_environment.make_client(), *m_servlet_administration_client,
        &m_driver, &m_data_store)),
        m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
      m_client_service_locator.emplace(
        m_service_locator_environment.make_client("client", "1234"));
      m_client.emplace(std::in_place_type<
        ServiceOrderExecutionClient<ClientBuilder>>, ClientBuilder(
          Ref(*m_client_service_locator), std::bind_front(factory<
            std::unique_ptr<ClientBuilder::Channel>>(), "test_client",
            std::ref(*m_server_connection)),
          factory<std::unique_ptr<ClientBuilder::Timer>>()));
    }

    auto make_client(const std::string& name, const std::string& password) {
      auto service_locator_client =
        m_service_locator_environment.make_client(name, password);
      auto authenticator = SessionAuthenticator(Ref(service_locator_client));
      auto protocol_client = std::make_unique<TestServiceProtocolClient>(
        std::make_unique<LocalClientChannel>(name, *m_server_connection),
        init());
      Nexus::register_query_types(
        out(protocol_client->get_slots().get_registry()));
      register_order_execution_services(out(protocol_client->get_slots()));
      register_order_execution_messages(out(protocol_client->get_slots()));
      authenticator(*protocol_client);
      return protocol_client;
    }
  };

  template<typename D>
  std::shared_ptr<Order> submit_and_fill(
      Fixture<D>& fixture, const OrderFields& fields) {
    auto order = fixture.m_client->submit(fields);
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto driver_order = fixture.m_submissions->pop();
    fill(*driver_order, fields.m_quantity);
    while(reports->pop().m_status != OrderStatus::FILLED) {}
    return order;
  }

  template<typename D>
  Beam::Sequence load_last_sequence(
      Fixture<D>& fixture, const DirectoryEntry& account) {
    auto query = AccountQuery();
    query.set_index(account);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto records = fixture.m_data_store.load_order_records(query);
    REQUIRE(!records.empty());
    return records.back().get_sequence();
  }

  void service(TestOrderExecutionDataStore::Operation& operation,
      LocalOrderExecutionDataStore& data_store) {
    std::visit([&] (auto& operation) {
      using Operation = std::remove_cvref_t<decltype(operation)>;
      if constexpr(std::is_same_v<
          Operation, TestOrderExecutionDataStore::LoadOrderRecordOperation>) {
        operation.m_result.set(data_store.load_order_record(operation.m_id));
      } else if constexpr(std::is_same_v<
          Operation, TestOrderExecutionDataStore::LoadOrderRecordsOperation>) {
        operation.m_result.set(
          data_store.load_order_records(operation.m_query));
      } else if constexpr(std::is_same_v<
          Operation, TestOrderExecutionDataStore::StoreOrderInfoOperation>) {
        data_store.store(operation.m_info);
        operation.m_result.set();
      } else if constexpr(std::is_same_v<Operation,
          TestOrderExecutionDataStore::StoreOrderInfoListOperation>) {
        data_store.store(operation.m_info);
        operation.m_result.set();
      } else if constexpr(std::is_same_v<Operation,
          TestOrderExecutionDataStore::LoadExecutionReportsOperation>) {
        operation.m_result.set(
          data_store.load_execution_reports(operation.m_query));
      } else if constexpr(std::is_same_v<Operation,
          TestOrderExecutionDataStore::StoreExecutionReportOperation>) {
        data_store.store(operation.m_report);
        operation.m_result.set();
      } else if constexpr(std::is_same_v<Operation,
          TestOrderExecutionDataStore::StoreExecutionReportListOperation>) {
        data_store.store(operation.m_reports);
        operation.m_result.set();
      } else if constexpr(std::is_same_v<Operation,
          TestOrderExecutionDataStore::LoadInventorySnapshotOperation>) {
        operation.m_result.set(
          data_store.load_inventory_snapshot(operation.m_account));
      } else if constexpr(std::is_same_v<Operation,
          TestOrderExecutionDataStore::StoreInventorySnapshotOperation>) {
        data_store.store(operation.m_account, operation.m_snapshot);
        operation.m_result.set();
      }
    }, operation);
  }
}

TEST_SUITE("OrderExecutionServlet") {
  TEST_CASE("store_snapshot_on_close") {
    auto fixture = Fixture();
    fixture.start();
    submit_and_fill(fixture,
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 200, Money::ONE));
    auto live_order = fixture.m_client->submit(
      make_limit_order_fields(TST, CAD, Side::ASK, "TSX", 50, 2 * Money::ONE));
    fixture.m_submissions->pop();
    fixture.m_container->close();
    auto snapshot =
      fixture.m_data_store.load_inventory_snapshot(fixture.m_client_account);
    REQUIRE(snapshot.m_excluded_orders ==
      std::vector{live_order->get_info().m_id});
    REQUIRE(snapshot.m_sequence ==
      load_last_sequence(fixture, fixture.m_client_account));
    REQUIRE(snapshot.m_inventories.size() == 1);
    REQUIRE(snapshot.m_inventories.front() ==
      Inventory(Position(TST, CAD, 200, 200 * Money::ONE), Money::ZERO,
        Money::ZERO, 200, 1));
  }

  TEST_CASE("load_snapshot_on_start") {
    auto fixture = Fixture();
    auto seed = InventorySnapshot();
    seed.m_inventories.push_back(Inventory(
      Position(TST, CAD, 100, 100 * Money::ONE), 10 * Money::ONE, Money::ONE,
      300, 3));
    seed.m_excluded_orders.push_back(42);
    fixture.m_data_store.store(fixture.m_client_account, seed);
    fixture.start();
    submit_and_fill(fixture,
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 50, 2 * Money::ONE));
    fixture.m_container->close();
    auto snapshot =
      fixture.m_data_store.load_inventory_snapshot(fixture.m_client_account);
    REQUIRE(snapshot.m_excluded_orders == std::vector<OrderId>{42});
    REQUIRE(snapshot.m_sequence ==
      load_last_sequence(fixture, fixture.m_client_account));
    REQUIRE(snapshot.m_inventories.size() == 1);
    REQUIRE(snapshot.m_inventories.front() ==
      Inventory(Position(TST, CAD, 150, 200 * Money::ONE), 10 * Money::ONE,
        Money::ONE, 350, 4));
  }

  TEST_CASE("load_non_existent_order") {
    auto fixture = Fixture();
    fixture.start();
    auto client = fixture.make_client("client", "1234");
    REQUIRE(!client->send_request<LoadOrderByIdService>(999));
  }

  TEST_CASE("load_terminal_order") {
    auto fixture = Fixture();
    auto& account = fixture.m_client_account;
    auto fields =
      make_limit_order_fields(account, TST, CAD, Side::BID, "TSX", 100,
        Money::ONE);
    auto info = OrderInfo(fields, 1, time_from_string("2025-04-12 12:00:00"));
    fixture.m_data_store.store(SequencedAccountOrderInfo(
      IndexedValue(info, account), Beam::Sequence(1)));
    auto report = ExecutionReport(1, time_from_string("2025-04-12 12:00:01"));
    report.m_status = OrderStatus::FILLED;
    report.m_sequence = 0;
    fixture.m_data_store.store(SequencedAccountExecutionReport(
      IndexedValue(report, account), Beam::Sequence(1)));
    fixture.start();
    auto client = fixture.make_client("client", "1234");
    auto order = client->send_request<LoadOrderByIdService>(1);
    REQUIRE(order);
    REQUIRE((**order)->m_info == info);
    REQUIRE((*order)->get_index() == account);
    REQUIRE(order->get_sequence() == Beam::Sequence(1));
    REQUIRE((**order)->m_execution_reports.size() == 1);
    REQUIRE((**order)->m_execution_reports[0].m_status == OrderStatus::FILLED);
  }

  TEST_CASE("load_live_order") {
    auto fixture = Fixture();
    auto& account = fixture.m_client_account;
    auto fields =
      make_limit_order_fields(account, TST, CAD, Side::BID, "TSX", 100,
        Money::ONE);
    auto info = OrderInfo(fields, 1, time_from_string("2025-04-12 12:00:00"));
    fixture.m_data_store.store(SequencedAccountOrderInfo(
      IndexedValue(info, account), Beam::Sequence(1)));
    auto report = ExecutionReport(1, time_from_string("2025-04-12 12:00:01"));
    report.m_status = OrderStatus::PENDING_NEW;
    report.m_sequence = 0;
    fixture.m_data_store.store(SequencedAccountExecutionReport(
      IndexedValue(report, account), Beam::Sequence(1)));
    fixture.start();
    auto client = fixture.make_client("client", "1234");
    auto order = client->send_request<LoadOrderByIdService>(1);
    REQUIRE(order);
    REQUIRE((**order)->m_info == info);
    REQUIRE((*order)->get_index() == account);
    REQUIRE(order->get_sequence() == Beam::Sequence(1));
    REQUIRE((**order)->m_execution_reports.size() == 1);
    REQUIRE(
      (**order)->m_execution_reports[0].m_status == OrderStatus::PENDING_NEW);
  }

  TEST_CASE("load_order_without_permission") {
    auto fixture = Fixture();
    auto& account = fixture.m_client_account;
    fixture.m_service_locator_environment.get_root().make_account(
      "intruder", "1234", DirectoryEntry::STAR_DIRECTORY);
    auto fields =
      make_limit_order_fields(account, TST, CAD, Side::BID, "TSX", 100,
        Money::ONE);
    auto info = OrderInfo(fields, 1, time_from_string("2025-04-12 12:00:00"));
    fixture.m_data_store.store(SequencedAccountOrderInfo(
      IndexedValue(info, account), Beam::Sequence(1)));
    fixture.start();
    auto intruder_client = fixture.make_client("intruder", "1234");
    REQUIRE(!intruder_client->send_request<LoadOrderByIdService>(1));
  }

  TEST_CASE("update_order") {
    auto fixture = Fixture();
    auto admin_account = fixture.m_service_locator_environment.get_root().
      make_account("admin", "1234", DirectoryEntry::STAR_DIRECTORY);
    fixture.m_administration_environment.make_administrator(admin_account);
    fixture.start();
    fixture.m_client->submit(
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE));
    auto driver_order = fixture.m_submissions->pop();
    auto admin_client = fixture.make_client("admin", "1234");
    auto report = ExecutionReport();
    report.m_id = 999;
    report.m_status = OrderStatus::FILLED;
    auto expected_timestamp = ptime();
    SUBCASE("default_timestamp") {
      expected_timestamp = time_from_string("2025-04-12 13:33:12:55");
    }
    SUBCASE("provided_timestamp") {
      report.m_timestamp = time_from_string("2025-04-12 13:40:00");
      expected_timestamp = report.m_timestamp;
    }
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    driver_order->get_publisher().monitor(reports);
    admin_client->send_request<UpdateOrderService>(
      driver_order->get_info().m_id, report);
    auto update = reports->pop();
    while(update.m_status != OrderStatus::FILLED) {
      update = reports->pop();
    }
    REQUIRE(update.m_id == driver_order->get_info().m_id);
    REQUIRE(update.m_timestamp == expected_timestamp);
  }

  TEST_CASE("update_order_without_permission") {
    auto fixture = Fixture();
    fixture.start();
    auto client = fixture.make_client("client", "1234");
    auto report = ExecutionReport();
    report.m_id = 1;
    report.m_status = OrderStatus::FILLED;
    REQUIRE_THROWS_AS(client->send_request<UpdateOrderService>(1, report),
      ServiceRequestException);
  }

  TEST_CASE("submit_ask_without_position_is_shorting") {
    auto fixture = Fixture();
    fixture.start();
    fixture.m_client->submit(
      make_limit_order_fields(TST, CAD, Side::ASK, "TSX", 100, Money::ONE));
    auto driver_order = fixture.m_submissions->pop();
    REQUIRE(driver_order->get_info().m_shorting_flag);
  }

  TEST_CASE("submit_bid_is_not_shorting") {
    auto fixture = Fixture();
    fixture.start();
    fixture.m_client->submit(
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE));
    auto driver_order = fixture.m_submissions->pop();
    REQUIRE(!driver_order->get_info().m_shorting_flag);
  }

  TEST_CASE("submit_ask_against_position_is_not_shorting") {
    auto fixture = Fixture();
    fixture.start();
    submit_and_fill(fixture,
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE));
    fixture.m_client->submit(
      make_limit_order_fields(TST, CAD, Side::ASK, "TSX", 100, Money::ONE));
    auto ask_order = fixture.m_submissions->pop();
    REQUIRE(!ask_order->get_info().m_shorting_flag);
    fixture.m_client->submit(
      make_limit_order_fields(TST, CAD, Side::ASK, "TSX", 1, Money::ONE));
    auto excess_order = fixture.m_submissions->pop();
    REQUIRE(excess_order->get_info().m_shorting_flag);
  }

  TEST_CASE("recover_seeds_shorting_position") {
    auto fixture = Fixture();
    auto seed = InventorySnapshot();
    seed.m_inventories.push_back(Inventory(
      Position(TST, CAD, 100, 100 * Money::ONE), Money::ZERO, Money::ZERO,
      100, 1));
    fixture.m_data_store.store(fixture.m_client_account, seed);
    fixture.start();
    fixture.m_client->submit(
      make_limit_order_fields(TST, CAD, Side::ASK, "TSX", 100, Money::ONE));
    auto ask_order = fixture.m_submissions->pop();
    REQUIRE(!ask_order->get_info().m_shorting_flag);
    fixture.m_client->submit(
      make_limit_order_fields(TST, CAD, Side::ASK, "TSX", 1, Money::ONE));
    auto excess_order = fixture.m_submissions->pop();
    REQUIRE(excess_order->get_info().m_shorting_flag);
  }

  TEST_CASE("store_reports_after_client_disconnect") {
    auto operations = std::make_shared<TestOrderExecutionDataStore::Queue>();
    auto data_store = LocalOrderExecutionDataStore();
    auto client = std::unique_ptr<TestServiceProtocolClient>();
    auto servicer = std::async(std::launch::async, [&] {
      try {
        while(true) {
          auto operation = operations->pop();
          if(auto store_operation = std::get_if<
              TestOrderExecutionDataStore::StoreOrderInfoOperation>(
                &*operation)) {
            data_store.store(store_operation->m_info);
            if(client) {
              client->close();
            }
            store_operation->m_result.set();
          } else {
            service(*operation, data_store);
          }
        }
      } catch(const std::exception&) {}
    });
    auto fixture = Fixture<TestOrderExecutionDataStore>(operations);
    fixture.start();
    client = fixture.make_client("client", "1234");
    auto parameters = NewOrderSingleService::Parameters();
    parameters.fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    client->send(NewOrderSingleService::Request<TestServiceProtocolClient>(
      1, parameters));
    auto driver_order = fixture.m_submissions->pop();
    fill(*driver_order, 100);
    submit_and_fill(fixture,
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 200, Money::ONE));
    auto record = data_store.load_order_record(driver_order->get_info().m_id);
    REQUIRE(record);
    auto& reports = (**record)->m_execution_reports;
    REQUIRE(reports.size() == 3);
    REQUIRE(reports[0].m_status == OrderStatus::PENDING_NEW);
    REQUIRE(reports[1].m_status == OrderStatus::NEW);
    REQUIRE(reports[2].m_status == OrderStatus::FILLED);
  }
}
