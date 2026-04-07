#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/RiskService/InventorySnapshot.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  struct Fixture {
    ServiceLocatorTestEnvironment m_service_locator_environment;
    UidServiceTestEnvironment m_uid_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    OrderExecutionServiceTestEnvironment m_order_execution_environment;
    DirectoryEntry m_client_account;
    optional<ServiceLocatorClient> m_client_service_locator;
    optional<OrderExecutionClient> m_order_execution_client;

    Fixture()
        : m_administration_environment(
            make_administration_service_test_environment(
              m_service_locator_environment)),
          m_order_execution_environment(
            make_order_execution_service_test_environment(
              m_service_locator_environment, m_uid_environment,
                m_administration_environment)) {
      m_client_account = m_service_locator_environment.get_root().make_account(
        "client", "1234", DirectoryEntry::STAR_DIRECTORY);
      m_client_service_locator.emplace(
        m_service_locator_environment.make_client("client", "1234"));
      m_order_execution_client.emplace(
        m_order_execution_environment.make_client(
          Ref(*m_client_service_locator)));
    }
  };
}

TEST_SUITE("InventorySnapshot") {
  TEST_CASE("strip") {
    auto abc = Security("ABC", TSX);
    auto xyz = Security("XYZ", TSX);
    auto abc_inventory = Inventory(
      Position(abc, CAD, 1, Money::ONE), Money::ZERO, Money::ZERO, 1, 1);
    auto xyz_inventory = Inventory(
      Position(xyz, CAD, 0, Money::ZERO), Money::ZERO, Money::ZERO, 0, 0);
    auto snapshot = InventorySnapshot();
    snapshot.m_inventories.push_back(abc_inventory);
    snapshot.m_inventories.push_back(xyz_inventory);
    auto result = strip(snapshot);
    REQUIRE(result.m_inventories.size() == 1);
    REQUIRE(result.m_inventories.front() == abc_inventory);
  }

  TEST_CASE("make_portfolio") {
    auto fixture = Fixture();
    auto abc = Security("ABC", TSX);
    auto xyz = Security("XYZ", TSX);
    auto abc_inventory = Inventory(
      Position(abc, CAD, 10, Money::ONE), Money::ZERO, Money::ZERO, 2, 2);
    auto xyz_inventory = Inventory(
      Position(xyz, CAD, 5, Money::ONE), Money::ZERO, Money::ZERO, 1, 1);
    auto snapshot_sequence = Beam::Sequence(100);
    auto excluded_order_ids = std::vector<OrderId>{1, 2};
    auto snapshot = InventorySnapshot();
    snapshot.m_inventories = {abc_inventory, xyz_inventory};
    snapshot.m_sequence = snapshot_sequence;
    snapshot.m_excluded_orders = excluded_order_ids;
    auto now = time_from_string("2025-08-14 13:00:05:23");
    for(auto id : excluded_order_ids) {
      auto fields = make_limit_order_fields(
        fixture.m_client_account, abc, CAD, Side::BID, "TSX", 100, Money::ONE);
      auto info = OrderInfo(fields, fixture.m_client_account, id, false, now);
      fixture.m_order_execution_environment.get_data_store().store(
        SequencedValue(IndexedValue(info, fixture.m_client_account),
          Beam::Sequence(id)));
    }
    auto trailing_order_ids = std::vector<OrderId>{101, 102};
    for(auto id : trailing_order_ids) {
      auto fields = make_limit_order_fields(
        fixture.m_client_account, xyz, CAD, Side::ASK, "TSX", 50, Money::ONE);
      auto info = OrderInfo(fields, fixture.m_client_account, id, false, now);
      fixture.m_order_execution_environment.get_data_store().store(
        SequencedValue(IndexedValue(info, fixture.m_client_account),
          Beam::Sequence(id)));
    }
    auto [portfolio, sequence, excluded_orders] =
      make_portfolio(snapshot, fixture.m_client_account, DEFAULT_VENUES,
        *fixture.m_order_execution_client);
    REQUIRE(sequence == Beam::Sequence(102));
    auto expected_excluded_ids = excluded_order_ids;
    expected_excluded_ids.insert(expected_excluded_ids.end(),
      trailing_order_ids.begin(), trailing_order_ids.end());
    auto actual_excluded_ids = std::vector<OrderId>();
    for(auto& order : excluded_orders) {
      actual_excluded_ids.push_back(order->get_info().m_id);
    }
    std::sort(expected_excluded_ids.begin(), expected_excluded_ids.end());
    std::sort(actual_excluded_ids.begin(), actual_excluded_ids.end());
    REQUIRE(expected_excluded_ids == actual_excluded_ids);
    auto found_abc = false;
    auto found_xyz = false;
    for(auto& inventory : portfolio.get_bookkeeper().get_inventory_range()) {
      if(inventory == abc_inventory) {
        found_abc = true;
      }
      if(inventory == xyz_inventory) {
        found_xyz = true;
      }
    }
    REQUIRE(found_abc);
    REQUIRE(found_xyz);
  }

  TEST_CASE("shuttle") {
    auto abc = Security("ABC", TSX);
    auto xyz = Security("XYZ", TSX);
    auto abc_inventory = Inventory(Position(abc, CAD, 5, 12 * Money::CENT),
      10 * Money::ONE, 6 * Money::ONE, 5, 3);
    auto xyz_inventory = Inventory(
      Position(xyz, CAD, 0, Money::ZERO), Money::ZERO, Money::ZERO, 0, 0);
    auto snapshot = InventorySnapshot();
    snapshot.m_inventories.push_back(abc_inventory);
    snapshot.m_inventories.push_back(xyz_inventory);
    test_round_trip_shuttle(snapshot);
  }
}
