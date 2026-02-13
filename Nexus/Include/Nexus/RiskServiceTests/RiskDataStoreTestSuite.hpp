#ifndef NEXUS_RISK_DATA_STORE_TEST_SUITE_HPP
#define NEXUS_RISK_DATA_STORE_TEST_SUITE_HPP
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/RiskService/RiskDataStore.hpp"

namespace Nexus::Tests {
  TEST_CASE_TEMPLATE_DEFINE("RiskDataStore", T, RiskDataStoreTestSuite) {
    using namespace Beam;
    using namespace Nexus;
    using namespace Nexus::DefaultCurrencies;
    auto data_store = T()();

    SUBCASE("load_empty_inventories") {
      auto snapshot = data_store.load_inventory_snapshot(
        DirectoryEntry::make_account(123, "test"));
      REQUIRE(snapshot.m_sequence == Sequence::FIRST);
      REQUIRE(snapshot.m_inventories.empty());
      REQUIRE(snapshot.m_excluded_orders.empty());
    }

    SUBCASE("store_load_inventory") {
      auto inventories = std::vector<Inventory>();
      inventories.emplace_back(parse_ticker("A.ASX"), AUD);
      inventories.back().m_position.m_cost_basis = 1000 * Money::ONE;
      inventories.back().m_position.m_quantity = 123;
      inventories.back().m_fees = 3 * Money::ONE;
      inventories.back().m_transaction_count = 332;
      inventories.back().m_volume = 433;
      auto excluded_orders = std::vector<OrderId>();
      excluded_orders.push_back(100);
      excluded_orders.push_back(102);
      excluded_orders.push_back(110);
      auto account = DirectoryEntry::make_account(123, "test");
      auto snapshot =
        InventorySnapshot(inventories, Sequence(200), excluded_orders);
      data_store.store(account, snapshot);
      auto stored_snapshot = data_store.load_inventory_snapshot(account);
      REQUIRE(stored_snapshot == snapshot);
    }

    SUBCASE("store_empty_inventory") {
      auto inventories = std::vector<Inventory>();
      inventories.emplace_back(parse_ticker("A.ASX"), AUD);
      inventories.back().m_position.m_cost_basis = 1000 * Money::ONE;
      inventories.back().m_position.m_quantity = 123;
      inventories.back().m_fees = 3 * Money::ONE;
      inventories.back().m_transaction_count = 332;
      inventories.back().m_volume = 433;
      inventories.emplace_back(parse_ticker("B.ASX"), AUD);
      auto account = DirectoryEntry::make_account(123, "test");
      auto snapshot = InventorySnapshot(inventories, Sequence(200), {});
      data_store.store(account, snapshot);
      auto stored_snapshot = data_store.load_inventory_snapshot(account);
      REQUIRE(stored_snapshot.m_inventories.size() == 1);
      REQUIRE(stored_snapshot.m_inventories[0] == inventories[0]);
    }
  }
}

#endif
