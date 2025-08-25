#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;

TEST_SUITE("OrderExecutionSession") {
  TEST_CASE("constructor") {
    auto session = OrderExecutionSession();
    REQUIRE(!session.is_administrator());
    REQUIRE(!session.is_globally_subscribed());
    auto account = DirectoryEntry::MakeAccount(123);
    REQUIRE(!session.has_permission(account));
  }

  TEST_CASE("grant_permission") {
    auto session = OrderExecutionSession();
    auto account_a = DirectoryEntry::MakeAccount(123);
    auto account_b = DirectoryEntry::MakeAccount(456);
    REQUIRE(!session.has_permission(account_a));
    session.grant_permission(account_a);
    REQUIRE(session.has_permission(account_a));
    REQUIRE(!session.has_permission(account_b));
  }

  TEST_CASE("set_administrator") {
    auto session = OrderExecutionSession();
    auto account = DirectoryEntry::MakeAccount(123);
    REQUIRE(!session.is_administrator());
    REQUIRE(!session.has_permission(account));
    session.set_administrator(true);
    REQUIRE(session.is_administrator());
    REQUIRE(session.has_permission(account));
    session.set_administrator(false);
    REQUIRE(!session.is_administrator());
    REQUIRE(!session.has_permission(account));
  }

  TEST_CASE("set_globally_subscribed") {
    auto session = OrderExecutionSession();
    REQUIRE(!session.is_globally_subscribed());
    session.set_globally_subscribed(true);
    REQUIRE(session.is_globally_subscribed());
    session.set_globally_subscribed(false);
    REQUIRE(!session.is_globally_subscribed());
  }
}
