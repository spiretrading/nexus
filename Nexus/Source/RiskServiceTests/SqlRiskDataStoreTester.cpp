#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/RiskService/SqlRiskDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace Viper;
using namespace Viper::Sqlite3;

namespace {
  using TestSqlRiskDataStore = SqlRiskDataStore<Connection>;
}

TEST_SUITE("SqlRiskDataStore") {
  TEST_CASE("load_empty_inventories") {
    auto dataStore = TestSqlRiskDataStore(
      std::make_unique<Connection>(":memory:"));
    dataStore.Open();
    auto snapshot = dataStore.LoadInventorySnapshot(DirectoryEntry::MakeAccount(
      123, "test"));
    REQUIRE(snapshot.m_sequence == Sequence::First());
    REQUIRE(snapshot.m_inventories.empty());
    REQUIRE(snapshot.m_excludedOrders.empty());
  }

  TEST_CASE("store_load_inventory") {
    auto dataStore = TestSqlRiskDataStore(
      std::make_unique<Connection>(":memory:"));
    dataStore.Open();
    auto inventories = std::vector<RiskInventory>();
    inventories.emplace_back(RiskInventory::Position::Key(
      Security("A", DefaultMarkets::NYSE(), DefaultCountries::US()),
      DefaultCurrencies::USD()));
    inventories.back().m_position.m_costBasis = 1000 * Money::ONE;
    inventories.back().m_position.m_quantity = 123;
    inventories.back().m_fees = 3 * Money::ONE;
    inventories.back().m_transactionCount = 332;
    inventories.back().m_volume = 433;
    auto excludedOrders = std::vector<OrderId>();
    excludedOrders.push_back(100);
    excludedOrders.push_back(102);
    excludedOrders.push_back(110);
    auto account = DirectoryEntry::MakeAccount(123, "test");
    auto snapshot = InventorySnapshot{inventories, Sequence(200),
      excludedOrders};
    dataStore.Store(account, snapshot);
    auto storedSnapshot = dataStore.LoadInventorySnapshot(account);
    REQUIRE(storedSnapshot == snapshot);
  }

  TEST_CASE("store_empty_inventory") {
    auto dataStore = TestSqlRiskDataStore(
      std::make_unique<Connection>(":memory:"));
    dataStore.Open();
    auto inventories = std::vector<RiskInventory>();
    inventories.emplace_back(RiskInventory::Position::Key(
      Security("A", DefaultMarkets::NYSE(), DefaultCountries::US()),
      DefaultCurrencies::USD()));
    inventories.back().m_position.m_costBasis = 1000 * Money::ONE;
    inventories.back().m_position.m_quantity = 123;
    inventories.back().m_fees = 3 * Money::ONE;
    inventories.back().m_transactionCount = 332;
    inventories.back().m_volume = 433;
    inventories.emplace_back(RiskInventory::Position::Key(
      Security("B", DefaultMarkets::NYSE(), DefaultCountries::US()),
      DefaultCurrencies::USD()));
    auto account = DirectoryEntry::MakeAccount(123, "test");
    auto snapshot = InventorySnapshot{inventories, Sequence(200), {}};
    dataStore.Store(account, snapshot);
    auto storedSnapshot = dataStore.LoadInventorySnapshot(account);
    REQUIRE(storedSnapshot.m_inventories.size() == 1);
    REQUIRE(storedSnapshot.m_inventories[0] == inventories[0]);
  }
}
