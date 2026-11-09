#include <Beam/Queues/TablePublisher.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <doctest/doctest.h>
#include "WebPortal/RiskPortfolioSubscription.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("RiskPortfolioSubscription") {
  TEST_CASE("snapshot_and_updates") {
    auto publisher = TablePublisher<RiskPortfolioKey, Inventory>();
    auto account = DirectoryEntry::make_account(1, "trader");
    auto ticker = parse_ticker("TST.TSX");
    auto key = RiskPortfolioKey(account, ticker);
    auto inventory = Inventory(Position(ticker, Currencies::CAD, 10,
      Money(100)), Money::ZERO, Money::ZERO, 10, 1);
    publisher.push(key, inventory);
    auto subscription = RiskPortfolioSubscription(publisher);
    REQUIRE(subscription.get_snapshot() ==
      std::vector{RiskInventoryEntry(key, inventory)});
    inventory.m_position.m_quantity = 20;
    publisher.push(key, inventory);
    inventory.m_position.m_quantity = 30;
    publisher.push(key, inventory);
    REQUIRE(subscription.pop().m_value.m_position.m_quantity == 20);
    REQUIRE(subscription.pop() == RiskInventoryEntry(key, inventory));
    auto empty = Inventory(ticker, Currencies::CAD);
    publisher.erase(key, empty);
    REQUIRE(subscription.pop() == RiskInventoryEntry(key, empty));
    auto next = RiskPortfolioSubscription(publisher);
    REQUIRE(next.get_snapshot().empty());
  }

  TEST_CASE("independent_subscriptions") {
    auto first = TablePublisher<RiskPortfolioKey, Inventory>();
    auto second = TablePublisher<RiskPortfolioKey, Inventory>();
    auto ticker = parse_ticker("TST.TSX");
    auto key = RiskPortfolioKey(
      DirectoryEntry::make_account(1, "first"), ticker);
    auto inventory = Inventory(ticker, Currencies::CAD);
    first.push(key, inventory);
    auto first_subscription = RiskPortfolioSubscription(first);
    auto second_subscription = RiskPortfolioSubscription(second);
    REQUIRE(first_subscription.get_snapshot().size() == 1);
    REQUIRE(second_subscription.get_snapshot().empty());
    first_subscription.close();
    key.m_account = DirectoryEntry::make_account(2, "second");
    second.push(key, inventory);
    REQUIRE(second_subscription.pop() == RiskInventoryEntry(key, inventory));
    REQUIRE_THROWS_AS(first_subscription.pop(), PipeBrokenException);
  }

  TEST_CASE("close_pending_reader") {
    auto publisher = TablePublisher<RiskPortfolioKey, Inventory>();
    auto subscription = RiskPortfolioSubscription(publisher);
    auto result = Queue<RiskInventoryEntry>();
    auto reader = RoutineHandler(spawn([&] {
      try {
        result.push(subscription.pop());
      } catch(const std::exception&) {
        result.close(std::current_exception());
      }
    }));
    subscription.close();
    REQUIRE_THROWS_AS(result.pop(), PipeBrokenException);
    reader.wait();
    subscription.close();
  }

  TEST_CASE("publisher_failure") {
    auto publisher = TablePublisher<RiskPortfolioKey, Inventory>();
    auto subscription = RiskPortfolioSubscription(publisher);
    publisher.close(std::runtime_error("Risk service disconnected."));
    REQUIRE_THROWS_AS(subscription.pop(), std::runtime_error);
    REQUIRE_THROWS_AS(
      static_cast<void>(RiskPortfolioSubscription(publisher)),
      std::runtime_error);
  }
}
