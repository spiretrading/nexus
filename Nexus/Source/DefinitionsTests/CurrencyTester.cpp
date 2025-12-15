#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Currency.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

TEST_SUITE("Currency") {
  TEST_CASE("comparison") {
    auto default_id = CurrencyId();
    REQUIRE(!default_id);
    auto id1 = CurrencyId(100);
    auto id2 = CurrencyId(100);
    auto id3 = CurrencyId(50);
    REQUIRE(id1 == id2);
    REQUIRE(id3 < id1);
    REQUIRE(id1 > id3);
  }

  TEST_CASE("empty") {
    REQUIRE_FALSE(static_cast<bool>(CurrencyId()));
    REQUIRE(static_cast<bool>(CurrencyId(840)));
  }

  TEST_CASE("stream") {
    auto ss = std::stringstream();
    ss << CurrencyId(256);
    auto read_id = CurrencyId();
    ss >> read_id;
    REQUIRE(read_id == CurrencyId(256));
    test_round_trip_shuttle(CurrencyId(42));
  }

  TEST_CASE("none") {
    auto none = CurrencyDatabase::NONE;
    REQUIRE(!none.m_id);
    REQUIRE(none.m_code == "???");
    REQUIRE(none.m_sign == "?");
  }

  TEST_CASE("id_lookup") {
    auto database = CurrencyDatabase();
    auto entry1 = CurrencyDatabase::Entry();
    entry1.m_id = CurrencyId(10);
    entry1.m_code = "AAA";
    entry1.m_sign = "A$";
    database.add(entry1);
    auto entry2 = CurrencyDatabase::Entry();
    entry2.m_id = CurrencyId(5);
    entry2.m_code = "BBB";
    entry2.m_sign = "B$";
    database.add(entry2);
    REQUIRE(database.from(CurrencyId(10)).m_sign == "A$");
    REQUIRE(database.from(CurrencyId(5)).m_sign == "B$");
    REQUIRE(!database.from(CurrencyId(7)).m_id);
  }

  TEST_CASE("code_lookup") {
    auto database = CurrencyDatabase();
    auto entry = CurrencyDatabase::Entry();
    entry.m_id = CurrencyId(20);
    entry.m_code = "CCC";
    entry.m_sign = "C$";
    database.add(entry);
    REQUIRE(database.from("CCC").m_id == CurrencyId(20));
    REQUIRE(!database.from("XXX").m_id);
  }

  TEST_CASE("remove") {
    auto database = CurrencyDatabase();
    auto entry = CurrencyDatabase::Entry();
    entry.m_id = CurrencyId(30);
    entry.m_code = "DDD";
    entry.m_sign = "D$";
    database.add(entry);
    REQUIRE(database.from(CurrencyId(30)).m_id == CurrencyId(30));
    database.remove(CurrencyId(30));
    REQUIRE(!database.from(CurrencyId(30)).m_id);
  }

  TEST_CASE("sort_order") {
    auto database = CurrencyDatabase();
    auto low = CurrencyDatabase::Entry();
    low.m_id = CurrencyId(1);
    low.m_code = "ZZZ";
    low.m_sign = "Z$";
    database.add(low);
    auto high = CurrencyDatabase::Entry();
    high.m_id = CurrencyId(2);
    high.m_code = "AAA";
    high.m_sign = "A$";
    database.add(high);
    auto entries = database.get_entries();
    REQUIRE(entries.size() == 2);
    REQUIRE(entries[0].m_id == CurrencyId(1));
    REQUIRE(entries[1].m_id == CurrencyId(2));
  }

  TEST_CASE("shuttle") {
    test_round_trip_shuttle(DEFAULT_CURRENCIES,
      [] (const auto& currencies) {
        auto expected_entries = DEFAULT_CURRENCIES.get_entries();
        auto entries = currencies.get_entries();
        REQUIRE(expected_entries.size() == entries.size());
        for(auto i = std::size_t(0); i != entries.size(); ++i) {
          REQUIRE(expected_entries[i] == entries[i]);
        }
      });
  }

  TEST_CASE("parse_currency") {
    auto database = CurrencyDatabase();
    database.add({CurrencyId(840), "USD", "$"});
    auto id = parse_currency("USD", database);
    REQUIRE(id == CurrencyId(840));
  }

  TEST_CASE("parse_currency_unknown") {
    auto database = CurrencyDatabase();
    auto id = parse_currency("EUR", database);
    REQUIRE(!id);
  }

  TEST_CASE("parse_currency_database_entry") {
    auto yaml = R"(
      - id: 840
        code: "USD"
        sign: "$")";
    auto node = YAML::Load(yaml);
    auto entry = parse_currency_database_entry(node[0]);
    REQUIRE(entry.m_id == CurrencyId(840));
    REQUIRE(entry.m_code == "USD");
    REQUIRE(entry.m_sign == "$");
  }

  TEST_CASE("parse_currency_database") {
    auto yaml = R"(
      - id: 978
        code: "EUR"
        sign: "E"
      - id: 826
        code: "GBP"
        sign: "L")";
    auto node = YAML::Load(yaml);
    auto database = parse_currency_database(node);
    REQUIRE(database.get_entries().size() == 2);
    auto gbp = database.from("GBP");
    REQUIRE(gbp.m_id == CurrencyId(826));
    REQUIRE(gbp.m_code == "GBP");
    REQUIRE(gbp.m_sign == "L");
    auto eur = database.from("EUR");
    REQUIRE(eur.m_id == CurrencyId(978));
    REQUIRE(eur.m_code == "EUR");
    REQUIRE(eur.m_sign == "E");
  }
}
