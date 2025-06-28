#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/Definitions/Destination.hpp"

using namespace Nexus;

TEST_SUITE("Destination") {
  TEST_CASE("empty") {
    auto database = DestinationDatabase();
    auto count = 0;
    for(auto& entry : database.get_entries()) {
      ++count;
    }
    CHECK(count == 0);
    CHECK(database.from("nope") == DestinationDatabase::NONE);
    CHECK(database.get_preferred_destination(Venue()) ==
      DestinationDatabase::NONE);
    CHECK_FALSE(database.get_manual_order_entry_destination().has_value());
  }

  TEST_CASE("copy") {
    auto database1 = DestinationDatabase();
    auto entry = DestinationDatabase::Entry();
    entry.m_id = "Z";
    entry.m_description = "z desc";
    entry.m_venues.push_back(Venue("vZ"));
    auto database2 = database1;
    database1.add(entry);
    CHECK(database1.from("Z") == entry);
    CHECK(database2.from("Z") == entry);
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
    CHECK(ids.size() == 2);
    CHECK(ids.at(0) == "A");
    CHECK(ids.at(1) == "B");
    CHECK(database.from("A") != DestinationDatabase::NONE);
    CHECK(database.from("A").m_description == "first");
    CHECK(database.from("C") == DestinationDatabase::NONE);
    database.remove("A");
    auto count_after = 0;
    for(auto& entry : database.get_entries()) {
      ++count_after;
    }
    CHECK(count_after == 1);
    CHECK(database.from("A") == DestinationDatabase::NONE);
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
    CHECK(
      database.get_preferred_destination(venue) != DestinationDatabase::NONE);
    CHECK(database.get_preferred_destination(venue).m_id == "X");
    database.remove_preferred_destination(venue);
    CHECK(
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
    CHECK(moe.has_value());
    CHECK(moe->m_id == "MO");
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
    CHECK(first != DestinationDatabase::NONE);
    CHECK(first.m_id == "20");
    auto all = database.select_all([](const auto& e) {
      return e.m_description.find('a') != std::string::npos;
    });
    CHECK(all.size() == 2);
    CHECK(std::find(all.begin(), all.end(), entry2) != all.end());
    CHECK(std::find(all.begin(), all.end(), entry3) != all.end());
  }

  TEST_CASE("parse_destination_database_entry") {
    auto node = YAML::Load(R"(
      id: "X1"
      venues: ['NYSE','TSX']
      description: "one")");
    auto entry = parse_destination_database_entry(node, DEFAULT_VENUES);
    REQUIRE(entry.m_id == "X1");
    REQUIRE(entry.m_venues.size() == 2);
    CHECK(entry.m_venues[0] == Venue("XNYS"));
    CHECK(entry.m_venues[1] == Venue("XTSE"));
    CHECK(entry.m_description == "one");
  }

  TEST_CASE("parse_destination_database") {
    auto yaml = R"(
      destinations:
        - id: "A1"
          venues: ['NYSE']
          description: "first"
        - id: "B2"
          venues: ['NSDQ','TSX']
          description: "second"
      preferred_destinations:
        - venue: 'NYSE'
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
    CHECK(ids[0] == "A1");
    CHECK(ids[1] == "B2");
    auto a1 = database.from("A1");
    CHECK(a1.m_description == "first");
    auto b2 = database.from("B2");
    CHECK(b2.m_venues.size() == 2);
    CHECK(b2.m_description == "second");
    CHECK(database.get_preferred_destination(Venue("XNYS")) !=
      DestinationDatabase::NONE);
    CHECK(database.get_preferred_destination(Venue("XNYS")).m_id == "A1");
    auto moe = database.get_manual_order_entry_destination();
    REQUIRE(moe.has_value());
    CHECK(moe->m_id == "M0");
    CHECK(moe->m_venues.size() == 1);
  }
}
