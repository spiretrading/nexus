#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;

TEST_SUITE("EntitlementDatabase") {
  TEST_CASE("none") {
    auto none = EntitlementDatabase::NONE;
    REQUIRE(none.m_name.empty());
    REQUIRE(none.m_price == Money::ZERO);
    REQUIRE(!none.m_currency);
    REQUIRE(none.m_group_entry == DirectoryEntry());
    REQUIRE(none.m_applicability.empty());
  }

  TEST_CASE("add") {
    auto database = EntitlementDatabase();
    auto entry = EntitlementDatabase::Entry();
    entry.m_name = "Test";
    entry.m_price = Money::ONE;
    entry.m_currency = CurrencyId(840);
    entry.m_group_entry = DirectoryEntry::make_account(123);
    entry.m_applicability[EntitlementKey(Venue("XNYS"))] =
      {MarketDataType::BBO_QUOTE};
    database.add(entry);
    auto entries = database.get_entries();
    REQUIRE(entries.size() == 1);
    REQUIRE(entries[0] == entry);
  }

  TEST_CASE("remove") {
    auto database = EntitlementDatabase();
    auto entry = EntitlementDatabase::Entry();
    entry.m_name = "Test";
    entry.m_price = Money::ONE;
    entry.m_currency = CurrencyId(840);
    entry.m_group_entry = DirectoryEntry::make_account(123);
    entry.m_applicability[EntitlementKey(Venue("XNYS"))] =
      {MarketDataType::BBO_QUOTE};
    database.add(entry);
    REQUIRE(database.get_entries().size() == 1);
    database.remove(DirectoryEntry::make_account(123));
    auto entries = database.get_entries();
    REQUIRE(database.get_entries().size() == 0);
  }

  TEST_CASE("shuttle") {
    auto database = EntitlementDatabase();
    auto entry = EntitlementDatabase::Entry();
    entry.m_name = "Test";
    entry.m_price = Money::ONE;
    entry.m_currency = CurrencyId(840);
    entry.m_group_entry = DirectoryEntry::make_account(123);
    entry.m_applicability[EntitlementKey(Venue("XNYS"))] =
      {MarketDataType::BBO_QUOTE};
    database.add(entry);
    test_round_trip_shuttle(database, [&] (const auto& destination) {
      auto source_entries = database.get_entries();
      auto destination_entries = destination.get_entries();
      REQUIRE(source_entries.size() == destination_entries.size());
      REQUIRE(source_entries[0] == destination_entries[0]);
    });
  }
}
