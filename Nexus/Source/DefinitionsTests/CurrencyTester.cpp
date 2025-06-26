#include <doctest/doctest.h>
#include "Nexus/Definitions/Currency.hpp"

using namespace Nexus;

TEST_SUITE("Currency") {
  TEST_CASE("comparison") {
    auto default_id = CurrencyId();
    CHECK(default_id == CurrencyId::NONE);
    auto id1 = CurrencyId(100);
    auto id2 = CurrencyId(100);
    auto id3 = CurrencyId(50);
    CHECK(id1 == id2);
    CHECK(id3 < id1);
    CHECK(id1 > id3);
  }

  TEST_CASE("stream") {
    auto ss = std::stringstream();
    ss << CurrencyId(256);
    auto read_id = CurrencyId();
    ss >> read_id;
    CHECK(read_id == CurrencyId(256));
  }

  TEST_CASE("none") {
    auto none = Nexus::CurrencyDatabase::NONE;
    CHECK(none.m_id == CurrencyId::NONE);
    CHECK(none.m_code == "???");
    CHECK(none.m_sign == "?");
  }

  TEST_CASE("id_lookup") {
    auto database = Nexus::CurrencyDatabase();
    auto entry1 = Nexus::CurrencyDatabase::Entry();
    entry1.m_id = CurrencyId(10);
    entry1.m_code = "AAA";
    entry1.m_sign = "A$";
    database.add(entry1);
    auto entry2 = Nexus::CurrencyDatabase::Entry();
    entry2.m_id = CurrencyId(5);
    entry2.m_code = "BBB";
    entry2.m_sign = "B$";
    database.add(entry2);
    CHECK(database.from(CurrencyId(10)).m_sign == "A$");
    CHECK(database.from(CurrencyId(5)).m_sign == "B$");
    CHECK(database.from(CurrencyId(7)).m_id == CurrencyId::NONE);
  }

  TEST_CASE("code_lookup") {
    auto database = Nexus::CurrencyDatabase();
    auto entry = Nexus::CurrencyDatabase::Entry();
    entry.m_id = CurrencyId(20);
    entry.m_code = "CCC";
    entry.m_sign = "C$";
    database.add(entry);
    CHECK(database.from("CCC").m_id == CurrencyId(20));
    CHECK(database.from("XXX").m_id == CurrencyId::NONE);
  }

  TEST_CASE("remove") {
    auto database = Nexus::CurrencyDatabase();
    auto entry = Nexus::CurrencyDatabase::Entry();
    entry.m_id = CurrencyId(30);
    entry.m_code = "DDD";
    entry.m_sign = "D$";
    database.add(entry);
    CHECK(database.from(CurrencyId(30)).m_id == CurrencyId(30));
    database.remove(CurrencyId(30));
    CHECK(database.from(CurrencyId(30)).m_id == CurrencyId::NONE);
  }

  TEST_CASE("sort_order") {
    auto database = Nexus::CurrencyDatabase();
    auto low = Nexus::CurrencyDatabase::Entry();
    low.m_id = CurrencyId(1);
    low.m_code = "ZZZ";
    low.m_sign = "Z$";
    database.add(low);
    auto high = Nexus::CurrencyDatabase::Entry();
    high.m_id = CurrencyId(2);
    high.m_code = "AAA";
    high.m_sign = "A$";
    database.add(high);
    auto& entries = database.get_entries();
    REQUIRE(entries.size() == 2);
    CHECK(entries[0].m_id == CurrencyId(1));
    CHECK(entries[1].m_id == CurrencyId(2));
  }
}
