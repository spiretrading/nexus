#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/TradingGroup.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::AdministrationService;

TEST_SUITE("TradingGroup") {
  TEST_CASE("shuttle") {
    auto entry = DirectoryEntry::MakeAccount(1, "group_entry");
    auto managers_directory =
      DirectoryEntry::MakeAccount(2, "managers_directory");
    auto managers = std::vector{DirectoryEntry::MakeAccount(3, "manager1"),
      DirectoryEntry::MakeAccount(4, "manager2")};
    auto traders_directory =
      DirectoryEntry::MakeAccount(5, "traders_directory");
    auto traders = std::vector{DirectoryEntry::MakeAccount(6, "trader1"),
      DirectoryEntry::MakeAccount(7, "trader2")
    };
    auto trading_group = TradingGroup(
      entry, managers_directory, managers, traders_directory, traders);
    Beam::Serialization::Tests::TestRoundTripShuttle(trading_group,
      [&] (const auto& received) {
        REQUIRE(received.get_entry() == trading_group.get_entry());
        REQUIRE(received.get_managers_directory() ==
          trading_group.get_managers_directory());
        REQUIRE(received.get_managers() == trading_group.get_managers());
        REQUIRE(received.get_traders_directory() ==
          trading_group.get_traders_directory());
        REQUIRE(received.get_traders() == trading_group.get_traders());
      });
  }
}
