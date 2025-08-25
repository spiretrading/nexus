#include <doctest/doctest.h>
#include "Nexus/RiskService/RiskSession.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus;

TEST_SUITE("risk_session") {
  TEST_CASE("constructor") {
    auto session = RiskSession();
    auto group = DirectoryEntry::MakeDirectory(1, "group1");
    REQUIRE(!session.has_subscription(group));
  }

  TEST_CASE("copy_constructor") {
    auto session = RiskSession();
    auto group1 = DirectoryEntry::MakeDirectory(1, "group1");
    auto group2 = DirectoryEntry::MakeDirectory(2, "group2");
    session.add(group1);
    session.add_all();
    auto copied = RiskSession(session);
    REQUIRE(copied.has_subscription(group1));
    REQUIRE(copied.has_subscription(group2));
  }

  TEST_CASE("add") {
    auto session = RiskSession();
    auto group1 = DirectoryEntry::MakeDirectory(1, "group1");
    auto group2 = DirectoryEntry::MakeDirectory(2, "group2");
    session.add(group1);
    REQUIRE(session.has_subscription(group1));
    REQUIRE(!session.has_subscription(group2));
  }

  TEST_CASE("add_all") {
    auto session = RiskSession();
    auto group1 = DirectoryEntry::MakeDirectory(1, "group1");
    auto group2 = DirectoryEntry::MakeDirectory(2, "group2");
    session.add_all();
    REQUIRE(session.has_subscription(group1));
    REQUIRE(session.has_subscription(group2));
  }
}
