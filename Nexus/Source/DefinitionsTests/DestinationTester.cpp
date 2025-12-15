#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Destination.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;

TEST_SUITE("Destination") {
  TEST_CASE("empty") {
    auto database = DestinationDatabase();
    auto count = 0;
    for(auto& entry : database.get_entries()) {
      ++count;
    }
    REQUIRE(count == 0);
    REQUIRE(database.from("nope") == DestinationDatabase::NONE);
    REQUIRE(
      database.get_preferred_destination(Venue()) == DestinationDatabase::NONE);
    REQUIRE_FALSE(database.get_manual_order_entry_destination().has_value());
  }

  TEST_CASE("copy") {
    auto database1 = DestinationDatabase();
    auto entry = DestinationDatabase::Entry();
    entry.m_id = "Z";
    entry.m_description = "z desc";
    entry.m_venues.push_back(Venue("vZ"));
    auto database2 = database1;
    database1.add(entry);
    REQUIRE(database1.from("Z") == entry);
    REQUIRE(database2.from("Z") == DestinationDatabase::NONE);
  }

  TEST_CASE("update") {
    auto database = DestinationDatabase();
    auto entry1 = DestinationDatabase::Entry();
    entry1.m_id = "A";
    entry1.m_description = "first";
    entry1.m_venues.push_back(Venue("v1"));
    database.add(entry1);
    auto entry2 = DestinationDatabase::Entry();
    entry2.m_id = "B";
    entry2.m_description = "second";
    entry2.m_venues.push_back(Venue("v2"));
    database.add(entry2);
    auto ids = std::vector<std::string>();
    for(auto& entry : database.get_entries()) {
      ids.push_back(entry.m_id);
    }
    REQUIRE(ids.size() == 2);
    REQUIRE(ids.at(0) == "A");
    REQUIRE(ids.at(1) == "B");
    REQUIRE(database.from("A") != DestinationDatabase::NONE);
    REQUIRE(database.from("A").m_description == "first");
    REQUIRE(database.from("C") == DestinationDatabase::NONE);
    database.remove("A");
    auto count_after = 0;
    for(auto& entry : database.get_entries()) {
      ++count_after;
    }
    REQUIRE(count_after == 1);
    REQUIRE(database.from("A") == DestinationDatabase::NONE);
  }

  TEST_CASE("preferred_destination") {
    auto database = DestinationDatabase();
    auto entry = DestinationDatabase::Entry();
    entry.m_id = "X";
    entry.m_description = "desc";
    auto venue = Venue("v");
    entry.m_venues.push_back(venue);
    database.add(entry);
    database.set_preferred_destination(venue, entry.m_id);
    REQUIRE(
      database.get_preferred_destination(venue) != DestinationDatabase::NONE);
    REQUIRE(database.get_preferred_destination(venue).m_id == "X");
    database.remove_preferred_destination(venue);
    REQUIRE(
      database.get_preferred_destination(venue) == DestinationDatabase::NONE);
  }

  TEST_CASE("manual_order_entry") {
    auto database = DestinationDatabase();
    auto entry = DestinationDatabase::Entry();
    entry.m_id = "MO";
    entry.m_description = "manual";
    entry.m_venues.push_back(Venue("m"));
    database.set_manual_order_entry_destination(entry);
    auto moe = database.get_manual_order_entry_destination();
    REQUIRE(moe.has_value());
    REQUIRE(moe->m_id == "MO");
  }

  TEST_CASE("select") {
    auto database = DestinationDatabase();
    auto entry1 = DestinationDatabase::Entry();
    entry1.m_id = "10";
    entry1.m_description = "foo";
    database.add(entry1);
    auto entry2 = DestinationDatabase::Entry();
    entry2.m_id = "20";
    entry2.m_description = "bar";
    database.add(entry2);
    auto entry3 = DestinationDatabase::Entry();
    entry3.m_id = "30";
    entry3.m_description = "baz";
    database.add(entry3);
    auto first = database.select_first([] (const auto& e) {
      return e.m_id == "20";
    });
    REQUIRE(first != DestinationDatabase::NONE);
    REQUIRE(first.m_id == "20");
    auto all = database.select_all([](const auto& e) {
      return e.m_description.find('a') != std::string::npos;
    });
    REQUIRE(all.size() == 2);
    REQUIRE(std::find(all.begin(), all.end(), entry2) != all.end());
    REQUIRE(std::find(all.begin(), all.end(), entry3) != all.end());
  }

  TEST_CASE("shuttle") {
    test_round_trip_shuttle(
      DEFAULT_DESTINATIONS, [] (const auto& destinations) {
        auto expected_entries = DEFAULT_DESTINATIONS.get_entries();
        auto entries = destinations.get_entries();
        REQUIRE(expected_entries.size() == entries.size());
        for(auto i = std::size_t(0); i != entries.size(); ++i) {
          REQUIRE(expected_entries[i] == entries[i]);
        }
        REQUIRE((destinations.get_manual_order_entry_destination() ==
          DEFAULT_DESTINATIONS.get_manual_order_entry_destination()));
        for(auto& venue : DEFAULT_VENUES.get_entries()) {
          REQUIRE(destinations.get_preferred_destination(venue.m_venue) ==
            DEFAULT_DESTINATIONS.get_preferred_destination(venue.m_venue));
        }
      });
  }

  TEST_CASE("parse_destination_database_entry") {
    auto node = YAML::Load(R"(
      id: "X1"
      venues: ['ASX','TSX']
      description: "one")");
    auto entry = parse_destination_database_entry(node, DEFAULT_VENUES);
    REQUIRE(entry.m_id == "X1");
    REQUIRE(entry.m_venues.size() == 2);
    REQUIRE(entry.m_venues[0] == Venue("XASX"));
    REQUIRE(entry.m_venues[1] == Venue("XTSE"));
    REQUIRE(entry.m_description == "one");
  }

  TEST_CASE("parse_destination_database") {
    auto yaml = R"(
      destinations:
        - id: "A1"
          venues: ['ASX']
          description: "first"
        - id: "B2"
          venues: ['TSXV','TSX']
          description: "second"
      preferred_destinations:
        - venue: 'ASX'
          destination: 'A1'
      manual_order_entry:
        id: 'M0'
        venues: ['TSX']
        description: 'manual'
      )";
    auto node = YAML::Load(yaml);
    auto database = parse_destination_database(node, DEFAULT_VENUES);
    auto ids = std::vector<std::string>();
    for(auto& entry : database.get_entries()) {
      ids.push_back(entry.m_id);
    }
    REQUIRE(ids.size() == 2);
    REQUIRE(ids[0] == "A1");
    REQUIRE(ids[1] == "B2");
    auto a1 = database.from("A1");
    REQUIRE(a1.m_description == "first");
    auto b2 = database.from("B2");
    REQUIRE(b2.m_venues.size() == 2);
    REQUIRE(b2.m_description == "second");
    REQUIRE(database.get_preferred_destination(Venue("XASX")) !=
      DestinationDatabase::NONE);
    REQUIRE(database.get_preferred_destination(Venue("XASX")).m_id == "A1");
    auto moe = database.get_manual_order_entry_destination();
    REQUIRE(moe.has_value());
    REQUIRE(moe->m_id == "M0");
    REQUIRE(moe->m_venues.size() == 1);
  }
}
