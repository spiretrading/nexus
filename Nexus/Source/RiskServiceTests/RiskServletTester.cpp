#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/RiskServlet.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::UidService;
using namespace Beam::UidService::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  auto TSLA = Security("TSLA", NASDAQ);
  auto BAC = Security("BAC", NYSE);

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
    optional<ServiceLocatorClientBox> m_service_locator;
    optional<AdministrationClient> m_administration_client;
    optional<MarketDataClient> m_market_data_client;
    optional<OrderExecutionClient> m_service_order_execution_client;
    std::shared_ptr<Queue<DirectoryEntry>> m_accounts_queue;
    std::shared_ptr<TestServerConnection> m_server_connection;
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
          m_server_connection(std::make_shared<TestServerConnection>()),
          m_order_submissions(
            std::make_shared<Queue<std::shared_ptr<PrimitiveOrder>>>()) {
      auto servlet_account =
        m_service_locator_environment.GetRoot().MakeAccount("risk_service", "",
          DirectoryEntry::GetStarDirectory());
      m_administration_environment.make_administrator(servlet_account);
      m_service_locator =
        m_service_locator_environment.MakeClient("risk_service", "");
      grant_all_entitlements(
        m_administration_environment, m_service_locator->GetAccount());
      m_administration_client =
        m_administration_environment.make_client(*m_service_locator);
      m_market_data_client =
        m_market_data_environment.make_registry_client(*m_service_locator);
      m_service_order_execution_client =
        m_order_execution_environment.make_client(*m_service_locator);
      m_container.emplace(Initialize(*m_service_locator,
        Initialize(m_accounts_queue, *m_administration_client,
          *m_market_data_client, *m_service_order_execution_client,
          factory<std::unique_ptr<TriggerTimer>>(), &m_time_client,
          &m_data_store, m_exchange_rates, DEFAULT_VENUES,
          DEFAULT_DESTINATIONS)),
        m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
      m_order_execution_environment.get_driver().as<
        MockOrderExecutionDriver>().get_publisher().Monitor(
          m_order_submissions);
    }

    auto make_client(const std::string& name) {
      if(!m_service_locator_environment.GetRoot().FindAccount(name)) {
        m_service_locator_environment.GetRoot().MakeAccount(
          name, "", DirectoryEntry::GetStarDirectory());
      }
      auto service_locator_client =
        m_service_locator_environment.MakeClient(name, "");
      auto authenticator = SessionAuthenticator(service_locator_client);
      auto protocol_client = std::make_unique<TestServiceProtocolClient>(
        Initialize(name, *m_server_connection), Initialize());
      RegisterRiskServices(Store(protocol_client->GetSlots()));
      RegisterRiskMessages(Store(protocol_client->GetSlots()));
      authenticator(*protocol_client);
      return std::tuple(
        service_locator_client.GetAccount(), std::move(protocol_client));
    }
  };

  void submit_and_fill(Fixture& fixture, const DirectoryEntry& account,
      const Security& security, Side side, Quantity quantity, Money price) {
    auto order_fields = make_market_order_fields(security, side, quantity);
    order_fields.m_account = account;
    fixture.m_service_order_execution_client->submit(order_fields);
    auto order = fixture.m_order_submissions->Pop();
    accept(*order);
    fill(*order, price, quantity);
  }

  auto setup_account(Fixture& fixture, const std::string& name,
      const std::vector<Inventory>& inventories) {
    auto account = fixture.m_service_locator_environment.GetRoot().MakeAccount(
      name, "", DirectoryEntry::GetStarDirectory());
    auto snapshot = InventorySnapshot();
    snapshot.m_inventories = inventories;
    snapshot.m_sequence = Beam::Queries::Sequence(1);
    fixture.m_data_store.store(account, snapshot);
    fixture.m_accounts_queue->Push(account);
    FlushPendingRoutines();
    auto [account_, client] = fixture.make_client(name);
    return std::tuple(account, std::move(client), inventories);
  }

  void require_inventory_message(TestServiceProtocolClient& client,
      const DirectoryEntry& expected_account, const Security& expected_security,
      Quantity expected_quantity, Money expected_cost_basis) {
    auto message = client.ReadMessage();
    auto update_message = std::dynamic_pointer_cast<
      RecordMessage<InventoryMessage, TestServiceProtocolClient>>(message);
    REQUIRE(update_message != nullptr);
    REQUIRE(update_message->GetRecord().inventories.size() == 1);
    auto& inventory = update_message->GetRecord().inventories[0].inventory;
    REQUIRE(inventory.m_position.m_security == expected_security);
    REQUIRE(inventory.m_position.m_quantity == expected_quantity);
    REQUIRE(inventory.m_position.m_cost_basis == expected_cost_basis);
  }
}

TEST_SUITE("RiskServlet") {
  TEST_CASE("load_inventory") {
    auto fixture = Fixture();
    auto account1 = fixture.m_service_locator_environment.GetRoot().MakeAccount(
      "account1", "", DirectoryEntry::GetStarDirectory());
    auto snapshot1 = InventorySnapshot();
    snapshot1.m_sequence = Beam::Queries::Sequence(123);
    fixture.m_data_store.store(account1, snapshot1);
    auto account2 = fixture.m_service_locator_environment.GetRoot().MakeAccount(
      "account2", "", DirectoryEntry::GetStarDirectory());
    auto snapshot2 = InventorySnapshot();
    snapshot2.m_sequence = Beam::Queries::Sequence(456);
    fixture.m_data_store.store(account2, snapshot2);
    auto [account1_, client] = fixture.make_client("account1");
    auto loaded_snapshot =
      client->SendRequest<LoadInventorySnapshotService>(account1);
    REQUIRE(loaded_snapshot.m_sequence == Beam::Queries::Sequence(123));
    REQUIRE_THROWS_AS(client->SendRequest<LoadInventorySnapshotService>(
      account2), ServiceRequestException);
    auto admin_account =
      fixture.m_service_locator_environment.GetRoot().MakeAccount(
        "admin", "", DirectoryEntry::GetStarDirectory());
    fixture.m_administration_environment.make_administrator(admin_account);
    auto [admin_account_, admin_client] = fixture.make_client("admin");
    auto admin_loaded_snapshot1 =
      admin_client->SendRequest<LoadInventorySnapshotService>(account1);
    REQUIRE(admin_loaded_snapshot1.m_sequence == Beam::Queries::Sequence(123));
    auto admin_loaded_snapshot2 =
      admin_client->SendRequest<LoadInventorySnapshotService>(account2);
    REQUIRE(admin_loaded_snapshot2.m_sequence == Beam::Queries::Sequence(456));
  }

  TEST_CASE("reset") {
    auto fixture = Fixture();
    auto admin_account =
      fixture.m_service_locator_environment.GetRoot().MakeAccount(
        "admin", "", DirectoryEntry::GetStarDirectory());
    fixture.m_administration_environment.make_administrator(admin_account);
    auto [admin_entry, admin_client] = fixture.make_client("admin");
    auto [account1, client1, inventories1] = setup_account(fixture, "account1",
      {
        Inventory(Position(BAC, USD, 100, 100 * Money::ONE),
          Money::ZERO, 5 * Money::ONE, 100, 1),
        Inventory(Position(TSLA, USD, 200, 200 * Money::ONE),
          Money::ZERO, 7 * Money::ONE, 200, 2)
      });
    auto [account2, client2, inventories2] = setup_account(fixture, "account2",
      {
        Inventory(Position(BAC, USD, 300, 300 * Money::ONE),
          Money::ZERO, 10 * Money::ONE, 300, 1),
        Inventory(Position(TSLA, USD, 400, 300 * Money::ONE),
          Money::ZERO, 14 * Money::ONE, 300, 6)
      });
    auto region = Region(BAC);
    admin_client->SendRequest<ResetRegionService>(region);
    auto reset_inventories1 = admin_client->SendRequest<
      LoadInventorySnapshotService>(account1).m_inventories;
    std::sort(reset_inventories1.begin(), reset_inventories1.end(),
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_position.m_security < rhs.m_position.m_security;
      });
    REQUIRE(reset_inventories1.size() == 2);
    REQUIRE(reset_inventories1[0].m_position == inventories1[0].m_position);
    REQUIRE(reset_inventories1[0].m_gross_profit_and_loss == Money::ZERO);
    REQUIRE(reset_inventories1[0].m_fees == Money::ZERO);
    REQUIRE(reset_inventories1[0].m_volume == 0);
    REQUIRE(reset_inventories1[0].m_transaction_count == 0);
    REQUIRE(reset_inventories1[1] == inventories1[1]);
    auto reset_inventories2 = admin_client->SendRequest<
      LoadInventorySnapshotService>(account2).m_inventories;
    std::sort(reset_inventories2.begin(), reset_inventories2.end(),
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_position.m_security < rhs.m_position.m_security;
      });
    REQUIRE(reset_inventories2.size() == 2);
    REQUIRE(reset_inventories2[0].m_position == inventories2[0].m_position);
    REQUIRE(reset_inventories2[0].m_gross_profit_and_loss == Money::ZERO);
    REQUIRE(reset_inventories2[0].m_fees == Money::ZERO);
    REQUIRE(reset_inventories2[0].m_volume == 0);
    REQUIRE(reset_inventories2[0].m_transaction_count == 0);
    REQUIRE(reset_inventories2[1] == inventories2[1]);
    REQUIRE_THROWS_AS(client1->SendRequest<ResetRegionService>(region),
      ServiceRequestException);
  }

  TEST_CASE("subscribe") {
    auto fixture = Fixture();
    auto admin_account =
      fixture.m_service_locator_environment.GetRoot().MakeAccount(
        "admin", "", DirectoryEntry::GetStarDirectory());
    fixture.m_administration_environment.make_administrator(admin_account);
    auto [admin_account_, admin_client] = fixture.make_client("admin");
    auto [account1, client1, inventories1] = setup_account(fixture, "account1",
      {
        Inventory(Position(BAC, USD, 100, 100 * Money::ONE),
          Money::ZERO, 5 * Money::ONE, 100, 1),
        Inventory(Position(TSLA, USD, 200, 200 * Money::ONE),
          Money::ZERO, 7 * Money::ONE, 200, 2)
      });
    auto [account2, client2, inventories2] = setup_account(fixture, "account2",
      {
        Inventory(Position(BAC, USD, 300, 300 * Money::ONE),
          Money::ZERO, 10 * Money::ONE, 300, 1),
        Inventory(Position(TSLA, USD, 400, 300 * Money::ONE),
          Money::ZERO, 14 * Money::ONE, 300, 6)
      });
    auto entries1 =
      client1->SendRequest<SubscribeRiskPortfolioUpdatesService>();
    std::sort(entries1.begin(), entries1.end(),
      [] (const auto& lhs, const auto& rhs) {
        return std::tie(lhs.m_key.m_account.m_id, lhs.m_key.m_security) <
          std::tie(rhs.m_key.m_account.m_id, rhs.m_key.m_security);
      });
    REQUIRE(entries1.size() == 2);
    REQUIRE(entries1[0].m_key.m_account == account1);
    REQUIRE(entries1[0].m_key.m_security == BAC);
    REQUIRE(entries1[0].m_value == inventories1[0]);
    REQUIRE(entries1[1].m_key.m_account == account1);
    REQUIRE(entries1[1].m_key.m_security == TSLA);
    REQUIRE(entries1[1].m_value == inventories1[1]);
    auto admin_entries =
      admin_client->SendRequest<SubscribeRiskPortfolioUpdatesService>();
    std::sort(admin_entries.begin(), admin_entries.end(),
      [] (const auto& lhs, const auto& rhs) {
        return std::tie(lhs.m_key.m_account.m_id, lhs.m_key.m_security) <
          std::tie(rhs.m_key.m_account.m_id, rhs.m_key.m_security);
      });
    for(auto entry : admin_entries) {
      std::cout << entry << std::endl;
    }
    REQUIRE(admin_entries.size() == 4);
    REQUIRE(admin_entries[0].m_key.m_account == account1);
    REQUIRE(admin_entries[0].m_key.m_security == BAC);
    REQUIRE(admin_entries[0].m_value == inventories1[0]);
    REQUIRE(admin_entries[1].m_key.m_account == account1);
    REQUIRE(admin_entries[1].m_key.m_security == TSLA);
    REQUIRE(admin_entries[1].m_value == inventories1[1]);
    REQUIRE(admin_entries[2].m_key.m_account == account2);
    REQUIRE(admin_entries[2].m_key.m_security == BAC);
    REQUIRE(admin_entries[2].m_value == inventories2[0]);
    REQUIRE(admin_entries[3].m_key.m_account == account2);
    REQUIRE(admin_entries[3].m_key.m_security == TSLA);
    REQUIRE(admin_entries[3].m_value == inventories2[1]);
    submit_and_fill(fixture, account2, TSLA, Side::BID, 300, Money::ONE);
    require_inventory_message(
      *admin_client, account2, TSLA, 700, 600 * Money::ONE);
    submit_and_fill(fixture, account1, BAC, Side::BID, 100, Money::ONE);
    require_inventory_message(
      *admin_client, account1, BAC, 200, 200 * Money::ONE);
    require_inventory_message(*client1, account1, BAC, 200, 200 * Money::ONE);
  }
}
