#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;

TEST_SUITE("LocalRiskDataStore") {
  TEST_CASE("load_empty_position") {
    auto dataStore = LocalRiskDataStore();
    dataStore.Open();
    auto snapshot = dataStore.LoadPositionSnapshot(DirectoryEntry::MakeAccount(
      123, "test"));
    REQUIRE(snapshot.m_sequence == Sequence::First());
    REQUIRE(snapshot.m_positions.empty());
    REQUIRE(snapshot.m_excludedOrders.empty());
  }

  TEST_CASE("store_load_position") {
    auto dataStore = LocalRiskDataStore();
    dataStore.Open();
    auto positions = std::vector<RiskPortfolioPosition>();
    positions.emplace_back(RiskPortfolioPosition::Key(
      Security("A", DefaultMarkets::NYSE(), DefaultCountries::US()),
      DefaultCurrencies::USD()));
    positions.back().m_costBasis = 1000 * Money::ONE;
    positions.back().m_quantity = 123;
    auto excludedOrders = std::vector<OrderId>();
    excludedOrders.push_back(100);
    excludedOrders.push_back(102);
    excludedOrders.push_back(110);
    auto account = DirectoryEntry::MakeAccount(123, "test");
    auto snapshot = PositionSnapshot{positions, Sequence(200), excludedOrders};
    dataStore.Store(account, snapshot);
    auto storedSnapshot = dataStore.LoadPositionSnapshot(account);
    REQUIRE(storedSnapshot == snapshot);
  }
}
