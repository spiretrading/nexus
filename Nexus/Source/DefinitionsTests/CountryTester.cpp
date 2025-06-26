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
    ss << CountryCode(100);
    auto read_code = CountryCode();
    ss >> read_code;
    CHECK(read_code == CountryCode(100));
  }

  TEST_CASE("none") {
    auto none = CountryDatabase::NONE;
    CHECK(none.m_code == CountryCode::NONE);
    CHECK(none.m_name == "");
    CHECK(none.m_two_letter_code == "??");
    CHECK(none.m_three_letter_code == "???");
  }

  TEST_CASE("code_lookup") {
    auto database = CountryDatabase();
    auto entry1 = CountryDatabase::Entry();
    entry1.m_code = CountryCode(2);
    entry1.m_name = "Two";
    entry1.m_two_letter_code = "TT";
    entry1.m_three_letter_code = "TTT";
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
}
