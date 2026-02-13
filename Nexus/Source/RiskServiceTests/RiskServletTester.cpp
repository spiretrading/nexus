#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/RiskServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

namespace {
  auto S32 = parse_ticker("S32.ASX");
  auto SHOP = parse_ticker("SHOP.TSX");

  struct Fixture {
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaRiskServlet<AdministrationClient, MarketDataClient,
        OrderExecutionClient, TriggerTimer, FixedTimeClient*,
        LocalRiskDataStore*>>;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    UidServiceTestEnvironment m_uid_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    MarketDataServiceTestEnvironment m_market_data_environment;
    OrderExecutionServiceTestEnvironment m_order_execution_environment;
    FixedTimeClient m_time_client;
    LocalRiskDataStore m_data_store;
    ExchangeRateTable m_exchange_rates;
    optional<ServiceLocatorClient> m_service_locator;
    optional<AdministrationClient> m_administration_client;
    optional<MarketDataClient> m_market_data_client;
    optional<OrderExecutionClient> m_service_order_execution_client;
    std::shared_ptr<Queue<DirectoryEntry>> m_accounts_queue;
    std::shared_ptr<LocalServerConnection> m_server_connection;
    optional<ServletContainer> m_container;
    std::shared_ptr<Queue<std::shared_ptr<PrimitiveOrder>>> m_order_submissions;

    Fixture()
        : m_administration_environment(
            make_administration_service_test_environment(
              m_service_locator_environment)),
          m_market_data_environment(make_market_data_service_test_environment(
            m_service_locator_environment, m_administration_environment)),
          m_order_execution_environment(
            make_order_execution_service_test_environment(
              m_service_locator_environment, m_uid_environment,
              m_administration_environment)),
          m_time_client(time_from_string("2025-07-14 6:23:00:00")),
          m_accounts_queue(std::make_shared<Queue<DirectoryEntry>>()),
          m_server_connection(std::make_shared<LocalServerConnection>()),
          m_order_submissions(
            std::make_shared<Queue<std::shared_ptr<PrimitiveOrder>>>()) {
      auto servlet_account =
        m_service_locator_environment.get_root().make_account(
          "risk_service", "", DirectoryEntry::STAR_DIRECTORY);
      m_administration_environment.make_administrator(servlet_account);
      m_service_locator.emplace(
        m_service_locator_environment.make_client("risk_service", ""));
      grant_all_entitlements(
        m_administration_environment, m_service_locator->get_account());
      m_administration_client.emplace(
        m_administration_environment.make_client(Ref(*m_service_locator)));
      m_market_data_client.emplace(
        m_market_data_environment.make_registry_client(
          Ref(*m_service_locator)));
      m_service_order_execution_client.emplace(
        m_order_execution_environment.make_client(Ref(*m_service_locator)));
      m_container.emplace(init(*m_service_locator,
        init(m_accounts_queue, *m_administration_client, *m_market_data_client,
          *m_service_order_execution_client,
          factory<std::unique_ptr<TriggerTimer>>(), &m_time_client,
          &m_data_store, m_exchange_rates, DEFAULT_DESTINATIONS)),
        m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
      m_order_execution_environment.get_driver().as<
        MockOrderExecutionDriver>().get_publisher().monitor(
          m_order_submissions);
    }

    auto make_client(const std::string& name) {
      if(!m_service_locator_environment.get_root().find_account(name)) {
        m_service_locator_environment.get_root().make_account(
          name, "", DirectoryEntry::STAR_DIRECTORY);
      }
      auto service_locator_client =
        m_service_locator_environment.make_client(name, "");
      auto authenticator = SessionAuthenticator(Ref(service_locator_client));
      auto protocol_client = std::make_unique<TestServiceProtocolClient>(
        std::make_unique<LocalClientChannel>(name, *m_server_connection),
        init());
      register_risk_services(out(protocol_client->get_slots()));
      register_risk_messages(out(protocol_client->get_slots()));
      authenticator(*protocol_client);
      return std::tuple(
        service_locator_client.get_account(), std::move(protocol_client));
    }
  };

  void submit_and_fill(Fixture& fixture, const DirectoryEntry& account,
      const Ticker& ticker, Side side, Quantity quantity, Money price) {
    auto order_fields = make_market_order_fields(ticker, side, quantity);
    order_fields.m_account = account;
    fixture.m_service_order_execution_client->submit(order_fields);
    auto order = fixture.m_order_submissions->pop();
    accept(*order);
    fill(*order, price, quantity);
  }

  auto setup_account(Fixture& fixture, const std::string& name,
      const std::vector<Inventory>& inventories) {
    auto account =
      fixture.m_service_locator_environment.get_root().make_account(
        name, "", DirectoryEntry::STAR_DIRECTORY);
    auto snapshot = InventorySnapshot();
    snapshot.m_inventories = inventories;
    snapshot.m_sequence = Beam::Sequence(1);
    fixture.m_data_store.store(account, snapshot);
    fixture.m_accounts_queue->push(account);
    flush_pending_routines();
    auto [account_, client] = fixture.make_client(name);
    return std::tuple(account, std::move(client), inventories);
  }

  void require_inventory_message(TestServiceProtocolClient& client,
      const DirectoryEntry& expected_account, const Ticker& expected_ticker,
      Quantity expected_quantity, Money expected_cost_basis) {
    auto message = client.read_message();
    auto update_message = std::dynamic_pointer_cast<
      RecordMessage<InventoryMessage, TestServiceProtocolClient>>(message);
    REQUIRE(update_message);
    REQUIRE(update_message->get_record().inventories.size() == 1);
    auto& inventory = update_message->get_record().inventories[0].inventory;
    REQUIRE(inventory.m_position.m_ticker == expected_ticker);
    REQUIRE(inventory.m_position.m_quantity == expected_quantity);
    REQUIRE(inventory.m_position.m_cost_basis == expected_cost_basis);
  }
}

TEST_SUITE("RiskServlet") {
  TEST_CASE("load_inventory") {
    auto fixture = Fixture();
    auto account1 =
      fixture.m_service_locator_environment.get_root().make_account(
        "account1", "", DirectoryEntry::STAR_DIRECTORY);
    auto snapshot1 = InventorySnapshot();
    snapshot1.m_sequence = Beam::Sequence(123);
    fixture.m_data_store.store(account1, snapshot1);
    auto account2 =
      fixture.m_service_locator_environment.get_root().make_account(
        "account2", "", DirectoryEntry::STAR_DIRECTORY);
    auto snapshot2 = InventorySnapshot();
    snapshot2.m_sequence = Beam::Sequence(456);
    fixture.m_data_store.store(account2, snapshot2);
    auto [account1_, client] = fixture.make_client("account1");
    auto loaded_snapshot =
      client->send_request<LoadInventorySnapshotService>(account1);
    REQUIRE(loaded_snapshot.m_sequence == Beam::Sequence(123));
    REQUIRE_THROWS_AS(client->send_request<LoadInventorySnapshotService>(
      account2), ServiceRequestException);
    auto admin_account =
      fixture.m_service_locator_environment.get_root().make_account(
        "admin", "", DirectoryEntry::STAR_DIRECTORY);
    fixture.m_administration_environment.make_administrator(admin_account);
    auto [admin_account_, admin_client] = fixture.make_client("admin");
    auto admin_loaded_snapshot1 =
      admin_client->send_request<LoadInventorySnapshotService>(account1);
    REQUIRE(admin_loaded_snapshot1.m_sequence == Beam::Sequence(123));
    auto admin_loaded_snapshot2 =
      admin_client->send_request<LoadInventorySnapshotService>(account2);
    REQUIRE(admin_loaded_snapshot2.m_sequence == Beam::Sequence(456));
  }

  TEST_CASE("reset") {
    auto fixture = Fixture();
    auto admin_account =
      fixture.m_service_locator_environment.get_root().make_account(
        "admin", "", DirectoryEntry::STAR_DIRECTORY);
    fixture.m_administration_environment.make_administrator(admin_account);
    auto [admin_entry, admin_client] = fixture.make_client("admin");
    auto [account1, client1, inventories1] = setup_account(fixture, "account1",
      {
        Inventory(Position(S32, AUD, 200, 200 * Money::ONE),
          Money::ZERO, 7 * Money::ONE, 200, 2),
        Inventory(Position(SHOP, CAD, 100, 100 * Money::ONE),
          Money::ZERO, 5 * Money::ONE, 100, 1)
      });
    auto [account2, client2, inventories2] = setup_account(fixture, "account2",
      {
        Inventory(Position(S32, AUD, 400, 300 * Money::ONE),
          Money::ZERO, 14 * Money::ONE, 300, 6),
        Inventory(Position(SHOP, CAD, 300, 300 * Money::ONE),
          Money::ZERO, 10 * Money::ONE, 300, 1)
      });
    auto scope = Scope(SHOP);
    admin_client->send_request<ResetScopeService>(scope);
    auto reset_inventories1 = admin_client->send_request<
      LoadInventorySnapshotService>(account1).m_inventories;
    std::sort(reset_inventories1.begin(), reset_inventories1.end(),
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_position.m_ticker < rhs.m_position.m_ticker;
      });
    REQUIRE(reset_inventories1.size() == 2);
    REQUIRE(reset_inventories1[0] == inventories1[0]);
    REQUIRE(reset_inventories1[1].m_position == inventories1[1].m_position);
    REQUIRE(reset_inventories1[1].m_gross_profit_and_loss == Money::ZERO);
    REQUIRE(reset_inventories1[1].m_fees == Money::ZERO);
    REQUIRE(reset_inventories1[1].m_volume == 0);
    REQUIRE(reset_inventories1[1].m_transaction_count == 0);
    auto reset_inventories2 = admin_client->send_request<
      LoadInventorySnapshotService>(account2).m_inventories;
    std::sort(reset_inventories2.begin(), reset_inventories2.end(),
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_position.m_ticker < rhs.m_position.m_ticker;
      });
    REQUIRE(reset_inventories2.size() == 2);
    REQUIRE(reset_inventories2[0] == inventories2[0]);
    REQUIRE(reset_inventories2[1].m_position == inventories2[1].m_position);
    REQUIRE(reset_inventories2[1].m_gross_profit_and_loss == Money::ZERO);
    REQUIRE(reset_inventories2[1].m_fees == Money::ZERO);
    REQUIRE(reset_inventories2[1].m_volume == 0);
    REQUIRE(reset_inventories2[1].m_transaction_count == 0);
    REQUIRE_THROWS_AS(client1->send_request<ResetScopeService>(scope),
      ServiceRequestException);
  }

  TEST_CASE("subscribe") {
    auto fixture = Fixture();
    auto admin_account =
      fixture.m_service_locator_environment.get_root().make_account(
        "admin", "", DirectoryEntry::STAR_DIRECTORY);
    fixture.m_administration_environment.make_administrator(admin_account);
    auto [admin_account_, admin_client] = fixture.make_client("admin");
    auto [account1, client1, inventories1] = setup_account(fixture, "account1",
      {
        Inventory(Position(S32, AUD, 200, 200 * Money::ONE),
          Money::ZERO, 7 * Money::ONE, 200, 2),
        Inventory(Position(SHOP, CAD, 100, 100 * Money::ONE),
          Money::ZERO, 5 * Money::ONE, 100, 1)
      });
    auto [account2, client2, inventories2] = setup_account(fixture, "account2",
      {
        Inventory(Position(S32, AUD, 400, 300 * Money::ONE),
          Money::ZERO, 14 * Money::ONE, 300, 6),
        Inventory(Position(SHOP, CAD, 300, 300 * Money::ONE),
          Money::ZERO, 10 * Money::ONE, 300, 1)
      });
    auto entries1 =
      client1->send_request<SubscribeRiskPortfolioUpdatesService>();
    std::sort(entries1.begin(), entries1.end(),
      [] (const auto& lhs, const auto& rhs) {
        return std::tie(lhs.m_key.m_account.m_id, lhs.m_key.m_ticker) <
          std::tie(rhs.m_key.m_account.m_id, rhs.m_key.m_ticker);
      });
    REQUIRE(entries1.size() == 2);
    REQUIRE(entries1[0].m_key.m_account == account1);
    REQUIRE(entries1[0].m_key.m_ticker == S32);
    REQUIRE(entries1[0].m_value == inventories1[0]);
    REQUIRE(entries1[1].m_key.m_account == account1);
    REQUIRE(entries1[1].m_key.m_ticker == SHOP);
    REQUIRE(entries1[1].m_value == inventories1[1]);
    auto admin_entries =
      admin_client->send_request<SubscribeRiskPortfolioUpdatesService>();
    std::sort(admin_entries.begin(), admin_entries.end(),
      [] (const auto& lhs, const auto& rhs) {
        return std::tie(lhs.m_key.m_account.m_id, lhs.m_key.m_ticker) <
          std::tie(rhs.m_key.m_account.m_id, rhs.m_key.m_ticker);
      });
    REQUIRE(admin_entries.size() == 4);
    REQUIRE(admin_entries[0].m_key.m_account == account1);
    REQUIRE(admin_entries[0].m_key.m_ticker == S32);
    REQUIRE(admin_entries[0].m_value == inventories1[0]);
    REQUIRE(admin_entries[1].m_key.m_account == account1);
    REQUIRE(admin_entries[1].m_key.m_ticker == SHOP);
    REQUIRE(admin_entries[1].m_value == inventories1[1]);
    REQUIRE(admin_entries[2].m_key.m_account == account2);
    REQUIRE(admin_entries[2].m_key.m_ticker == S32);
    REQUIRE(admin_entries[2].m_value == inventories2[0]);
    REQUIRE(admin_entries[3].m_key.m_account == account2);
    REQUIRE(admin_entries[3].m_key.m_ticker == SHOP);
    REQUIRE(admin_entries[3].m_value == inventories2[1]);
    submit_and_fill(fixture, account2, S32, Side::BID, 300, Money::ONE);
    require_inventory_message(
      *admin_client, account2, S32, 700, 600 * Money::ONE);
    submit_and_fill(fixture, account1, SHOP, Side::BID, 100, Money::ONE);
    require_inventory_message(
      *admin_client, account1, SHOP, 200, 200 * Money::ONE);
    require_inventory_message(*client1, account1, SHOP, 200, 200 * Money::ONE);
  }
}
