#include <doctest/doctest.h>
#include "Nexus/Accounting/InventorySnapshotModel.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Tests;
using namespace Nexus::Venues;

namespace {
  const auto TST = parse_ticker("TST.TSX");

  auto make_order(OrderId id, Side side, Quantity quantity, Money price) {
    return std::make_shared<PrimitiveOrder>(OrderInfo(
      make_limit_order_fields(TST, CAD, side, "TSX", quantity, price), id,
      time_from_string("2026-06-12 10:00:00")));
  }
}

TEST_SUITE("InventorySnapshotModel") {
  TEST_CASE("fold_at_terminal") {
    auto model = InventorySnapshotModel();
    auto order = make_order(1, Side::BID, 200, Money::ONE);
    model.add(Beam::Sequence(1), order);
    auto snapshot = model.make_snapshot();
    REQUIRE(snapshot.m_inventories.empty());
    REQUIRE(snapshot.m_sequence == Beam::Sequence(1));
    REQUIRE(snapshot.m_excluded_orders == std::vector<OrderId>{1});
    model.update(accept(*order));
    model.update(fill(*order, 100));
    snapshot = model.make_snapshot();
    REQUIRE(snapshot.m_inventories.empty());
    REQUIRE(snapshot.m_excluded_orders == std::vector<OrderId>{1});
    model.update(fill(*order, 100));
    snapshot = model.make_snapshot();
    REQUIRE(snapshot.m_excluded_orders.empty());
    REQUIRE(snapshot.m_sequence == Beam::Sequence(1));
    REQUIRE(snapshot.m_inventories.size() == 1);
    REQUIRE(snapshot.m_inventories.front() ==
      Inventory(Position(TST, CAD, 200, 200 * Money::ONE), Money::ZERO,
        Money::ZERO, 200, 2));
  }

  TEST_CASE("cancel_after_partial_fill") {
    auto model = InventorySnapshotModel();
    auto order = make_order(1, Side::BID, 100, Money::ONE);
    model.add(Beam::Sequence(1), order);
    model.update(accept(*order));
    model.update(fill(*order, 40));
    model.update(set_order_status(*order, OrderStatus::CANCELED));
    auto snapshot = model.make_snapshot();
    REQUIRE(snapshot.m_excluded_orders.empty());
    REQUIRE(snapshot.m_inventories.size() == 1);
    REQUIRE(snapshot.m_inventories.front() ==
      Inventory(Position(TST, CAD, 40, 40 * Money::ONE), Money::ZERO,
        Money::ZERO, 40, 1));
  }

  TEST_CASE("reject_without_fill") {
    auto model = InventorySnapshotModel();
    auto order = make_order(1, Side::BID, 100, Money::ONE);
    model.add(Beam::Sequence(1), order);
    model.update(reject(*order));
    auto snapshot = model.make_snapshot();
    REQUIRE(snapshot.m_inventories.empty());
    REQUIRE(snapshot.m_excluded_orders.empty());
    REQUIRE(snapshot.m_sequence == Beam::Sequence(1));
  }

  TEST_CASE("add_terminal_order") {
    auto order = make_order(1, Side::ASK, 100, Money::ONE);
    accept(*order);
    fill(*order, 100);
    auto model = InventorySnapshotModel();
    model.add(Beam::Sequence(1), order);
    auto snapshot = model.make_snapshot();
    REQUIRE(snapshot.m_excluded_orders.empty());
    REQUIRE(snapshot.m_sequence == Beam::Sequence(1));
    REQUIRE(snapshot.m_inventories.size() == 1);
    REQUIRE(snapshot.m_inventories.front() ==
      Inventory(Position(TST, CAD, -100, -100 * Money::ONE), Money::ZERO,
        Money::ZERO, 100, 1));
  }

  TEST_CASE("seeding") {
    auto seed = InventorySnapshot();
    seed.m_inventories.push_back(Inventory(
      Position(TST, CAD, 100, 100 * Money::ONE), 10 * Money::ONE, Money::ONE,
      300, 3));
    seed.m_sequence = Beam::Sequence(5);
    seed.m_excluded_orders.push_back(2);
    auto model = InventorySnapshotModel(seed);
    REQUIRE(model.make_snapshot() == seed);
    auto folded_order = make_order(3, Side::BID, 100, Money::ONE);
    model.add(Beam::Sequence(3), folded_order);
    model.update(accept(*folded_order));
    model.update(fill(*folded_order, 100));
    REQUIRE(model.make_snapshot() == seed);
    auto excluded_order = make_order(2, Side::ASK, 50, 2 * Money::ONE);
    model.add(Beam::Sequence(2), excluded_order);
    model.update(accept(*excluded_order));
    auto snapshot = model.make_snapshot();
    REQUIRE(snapshot.m_inventories == seed.m_inventories);
    REQUIRE(snapshot.m_sequence == Beam::Sequence(5));
    REQUIRE(snapshot.m_excluded_orders == std::vector<OrderId>{2});
    model.update(fill(*excluded_order, 50));
    snapshot = model.make_snapshot();
    REQUIRE(snapshot.m_excluded_orders.empty());
    REQUIRE(snapshot.m_sequence == Beam::Sequence(5));
    REQUIRE(snapshot.m_inventories.size() == 1);
    REQUIRE(snapshot.m_inventories.front() ==
      Inventory(Position(TST, CAD, 50, 50 * Money::ONE), 60 * Money::ONE,
        Money::ONE, 350, 4));
    auto live_order = make_order(7, Side::BID, 10, Money::ONE);
    model.add(Beam::Sequence(9), live_order);
    snapshot = model.make_snapshot();
    REQUIRE(snapshot.m_sequence == Beam::Sequence(9));
    REQUIRE(snapshot.m_excluded_orders == std::vector<OrderId>{7});
  }
}
