#include <doctest/doctest.h>
#include "Nexus/Definitions/Country.hpp"

using namespace Nexus;

TEST_SUITE("Country") {
  TEST_CASE("comparison") {
    auto default_code = CountryCode();
    CHECK(default_code == CountryCode::NONE);
    auto code1 = CountryCode(42);
    auto code2 = CountryCode(42);
    auto code3 = CountryCode(7);
    CHECK(code1 == code2);
    CHECK(code3 < code1);
    CHECK(code1 > code3);
  }

  TEST_CASE("stream") {
    auto ss = std::stringstream();
    ss << CountryCode(156);
    CHECK(ss.str() == "CN");
  }

  TEST_CASE("stream_unknown") {
    auto ss = std::stringstream();
    ss << CountryCode(9999);
    CHECK(ss.str() == "9999");
  }

  TEST_CASE("context_stream") {
    auto entry1 = CountryDatabase::Entry();
    entry1.m_code = CountryCode(124);
    entry1.m_name = "Canada";
    entry1.m_two_letter_code = "CA";
    entry1.m_three_letter_code = "CAN";
    auto database = CountryDatabase();
    database.add(entry1);
    auto ss = std::stringstream();
    ss << database << entry1.m_code;
    CHECK(ss.str() == "CA");
  }

  TEST_CASE("none") {
    auto none = CountryDatabase::NONE;
    CHECK(none.m_code == CountryCode::NONE);
    CHECK(none.m_name == "");
    CHECK(none.m_two_letter_code == "??");
    CHECK(none.m_three_letter_code == "???");
  }

  TEST_CASE("code_lookup") {
    auto entry1 = CountryDatabase::Entry();
    entry1.m_code = CountryCode(2);
    entry1.m_name = "Two";
    entry1.m_two_letter_code = "TT";
    entry1.m_three_letter_code = "TTT";
    auto database = CountryDatabase();
    database.add(entry1);
    auto entry2 = CountryDatabase::Entry();
    entry2.m_code = CountryCode(1);
    entry2.m_name = "One";
    entry2.m_two_letter_code = "OO";
    entry2.m_three_letter_code = "OOO";
    database.add(entry2);
    auto& entries = database.get_entries();
    CHECK(entries.size() == 2);
    CHECK(entries[0].m_code == CountryCode(1));
    CHECK(entries[1].m_code == CountryCode(2));
    CHECK(database.from(CountryCode(1)).m_name == "One");
    CHECK(database.from(CountryCode(3)).m_code == CountryCode::NONE);
  }

  TEST_CASE("name_lookup") {
    auto database = CountryDatabase();
    auto entry = CountryDatabase::Entry();
    entry.m_code = CountryCode(5);
    entry.m_name = "FiveLand";
    entry.m_two_letter_code = "FL";
    entry.m_three_letter_code = "FIV";
    database.add(entry);
    CHECK(database.from_name("FiveLand").m_code == CountryCode(5));
    CHECK(database.from("FL").m_code == CountryCode(5));
    CHECK(database.from("FIV").m_code == CountryCode(5));
    CHECK(database.from_name("Unknown").m_code == CountryCode::NONE);
  }

  TEST_CASE("remove") {
    auto database = CountryDatabase();
    auto entry = CountryDatabase::Entry();
    entry.m_code = CountryCode(9);
    entry.m_name = "Nine";
    entry.m_two_letter_code = "NI";
    entry.m_three_letter_code = "NIN";
    database.add(entry);
    CHECK(database.from(CountryCode(9)).m_code == CountryCode(9));
    database.remove(CountryCode(9));
    CHECK(database.from(CountryCode(9)).m_code == CountryCode::NONE);
  }

  TEST_CASE("parse") {
    auto database = CountryDatabase();
    auto entry = CountryDatabase::Entry();
    entry.m_code = CountryCode(8);
    entry.m_name = "Eight";
    entry.m_two_letter_code = "EI";
    entry.m_three_letter_code = "EGT";
    database.add(entry);
    CHECK(parse_country_code("EI", database) == CountryCode(8));
    CHECK(parse_country_code("EGT", database) == CountryCode(8));
    CHECK(parse_country_code("Eight", database) == CountryCode(8));
    CHECK(parse_country_code("Unknown", database) == CountryCode::NONE);
  }

  TEST_CASE("parse_country_database_entry") {
    auto yaml = R"(
      - name: "United States"
        two_letter_code: "US"
        three_letter_code: "USA"
        code: 840)";
    auto node = YAML::Load(yaml);
    auto entry = parse_country_database_entry(node[0]);
    REQUIRE(entry.m_name == "United States");
    REQUIRE(entry.m_two_letter_code == "US");
    REQUIRE(entry.m_three_letter_code == "USA");
    REQUIRE(entry.m_code == CountryCode(840));
  }

  TEST_CASE("parse_country_database") {
    auto yaml = R"(
    - name: "Country A"
      two_letter_code: "AA"
      three_letter_code: "AAA"
      code: 10
    - name: "Country B"
      two_letter_code: "BB"
      three_letter_code: "BBB"
      code: 20)";
    auto node = YAML::Load(yaml);
    auto database = parse_country_database(node);
    REQUIRE(database.get_entries().size() == 2);
    auto country_a = database.from("AA");
    REQUIRE(country_a.m_name == "Country A");
    REQUIRE(country_a.m_two_letter_code == "AA");
    REQUIRE(country_a.m_three_letter_code == "AAA");
    REQUIRE(country_a.m_code == CountryCode(10));
    auto country_b = database.from("BB");
    REQUIRE(country_b.m_name == "Country B");
    REQUIRE(country_b.m_two_letter_code == "BB");
    REQUIRE(country_b.m_three_letter_code == "BBB");
    REQUIRE(country_b.m_code == CountryCode(20));
  }
}
