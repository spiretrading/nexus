#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/Venue.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  boost::local_time::tz_database load_test_tz_database() {
    auto tz_data = std::string();
    tz_data += "\"Australian_Eastern_Standard_Time\",\"AEST\",\"AEST\",\"AEST\",\"AEST\",\"+10:00:00\",\"+01:00:00\",\"1;0;10\",\"+02:00:00\",\"1;0;4\",\"+03:00:00\"\n";
    tz_data += "\"Eastern_Time\",\"EST\",\"Eastern Standard Time\",\"EDT\",\"Eastern Daylight Time\",\"-05:00:00\",\"+01:00:00\",\"2;0;3\",\"+02:00:00\",\"1;0;11\",\"+02:00:00\"\n";
    tz_data += "\"UTC\",\"UTC\",\"UTC\",\"\",\"\",\"+00:00:00\",\"+00:00:00\",\"\",\"\",\"\",\"+00:00:00\"\n";
    auto stream = std::istringstream(tz_data);
    auto tz_database = boost::local_time::tz_database();
    tz_database.load_from_stream(stream);
    return tz_database;
  }

  VenueDatabase load_test_venue_database() {
    auto database = VenueDatabase();
    auto entry = VenueDatabase::Entry();
    entry.m_venue = Venue("AEST");
    entry.m_time_zone = "Australian_Eastern_Standard_Time";
    entry.m_country_code = CountryCode::NONE;
    entry.m_currency = CurrencyId::NONE;
    entry.m_description = "";
    entry.m_display_name = "";
    database.add(entry);
    entry.m_venue = Venue("EST");
    entry.m_time_zone = "Eastern_Time";
    database.add(entry);
    entry.m_venue = Venue("UTC");
    entry.m_time_zone = "UTC";
    database.add(entry);
    return database;
  }
}

TEST_SUITE("Venue") {
  TEST_CASE("equality") {
    auto default_v1 = Venue();
    auto default_v2 = Venue();
    CHECK(default_v1 == default_v2);
    auto mic = FixedString<4>("ABCD");
    auto v1 = Venue(mic);
    auto v2 = Venue("ABCD");
    CHECK(v1 == v2);
    auto v3 = Venue("WXYZ");
    CHECK_FALSE(v1 == v3);
  }

  TEST_CASE("constructor") {
    auto database = VenueDatabase();
    CHECK(database.get_entries().empty());
    auto none = VenueDatabase::NONE;
    auto result = database.from(Venue(Venue::Code("ABCD")));
    CHECK(result == none);
  }

  TEST_CASE("find_remove") {
    auto database = VenueDatabase();
    auto venue = Venue("ABCD");
    auto entry = VenueDatabase::Entry();
    entry.m_venue = venue;
    entry.m_country_code = CountryCode();
    entry.m_time_zone = "UTC";
    entry.m_currency = CurrencyId();
    entry.m_description = "Description";
    entry.m_display_name = "DisplayName";
    database.add(entry);
    CHECK(database.get_entries().size() == 1);
    auto& found = database.from(venue);
    CHECK(found != VenueDatabase::NONE);
    CHECK(found == entry);
    auto& found_by_name = database.from_display_name("DisplayName");
    CHECK(found_by_name == entry);
    auto country_list = database.from(CountryCode());
    CHECK(country_list.size() == 1);
    CHECK(country_list.front() == entry);
    database.remove(venue);
    CHECK(database.get_entries().empty());
  }

  TEST_CASE("venue_data_utc") {
    auto tz_database = load_test_tz_database();
    auto venue_database = load_test_venue_database();
    auto date_time = ptime(date(2025, 6, 26), hours(0));
    auto result =
      venue_date_to_utc(Venue("UTC"), date_time, venue_database, tz_database);
    REQUIRE(result == date_time);
  }

  TEST_CASE("aest_start") {
    auto tz_database = load_test_tz_database();
    auto venue_database = load_test_venue_database();
    auto date_time = ptime(date(2025, 6, 26), hours(0));
    auto expected = ptime(date(2025, 6, 25), hours(14));
    auto result =
      venue_date_to_utc(Venue("AEST"), date_time, venue_database, tz_database);
    REQUIRE(result == expected);
  }

  TEST_CASE("est_in_dst_start") {
    auto tz_database = load_test_tz_database();
    auto venue_database = load_test_venue_database();
    auto date_time = ptime(date(2025, 6, 26), hours(0));
    auto expected = ptime(date(2025, 6, 25), hours(4));
    auto result =
      venue_date_to_utc(Venue("EST"), date_time, venue_database, tz_database);
    REQUIRE(result == expected);
  }

  TEST_CASE("est_in_standard_start") {
    auto tz_database = load_test_tz_database();
    auto venue_database = load_test_venue_database();
    auto date_time = ptime(date(2025, 1, 15), hours(0));
    auto expected = ptime(date(2025, 1, 14), hours(5));
    auto result =
      venue_date_to_utc(Venue("EST"), date_time, venue_database, tz_database);
    REQUIRE(result == expected);
  }

  TEST_CASE("parse_venue_database_entry") {
    const auto yaml_text = R"(
      venue: "NYC1"
      country_code: "US"
      time_zone: "America/New_York"
      currency: "USD"
      description: "New York Stock Exchange"
      display_name: "NYSE")";
    auto node = YAML::Load(yaml_text);
    auto entry =
      parse_venue_database_entry(node, DEFAULT_COUNTRIES, DEFAULT_CURRENCIES);
    CHECK(entry.m_venue == "NYC1");
    CHECK(entry.m_country_code == DefaultCountries::US);
    CHECK(entry.m_time_zone == "America/New_York");
    CHECK(entry.m_currency == DefaultCurrencies::USD);
    CHECK(entry.m_description == "New York Stock Exchange");
    CHECK(entry.m_display_name == "NYSE");
  }

  TEST_CASE("parse_venue_database_entry_invalid_country") {
    const auto yaml_text = R"(
      venue: "LON1"
      country_code: "XX"
      time_zone: "Europe/London"
      currency: "GBP"
      description: "London Stock Exchange"
      display_name: "LSE")";
    auto node = YAML::Load(yaml_text);
    CHECK_THROWS_AS(parse_venue_database_entry(
      node, DEFAULT_COUNTRIES, DEFAULT_CURRENCIES), std::runtime_error);
  }

  TEST_CASE("parse_venue_database_entry_invalid_currency") {
    const auto yaml_text = R"(
      venue: "TOK1"
      country_code: "JP"
      time_zone: "Asia/Tokyo"
      currency: "XXX"
      description: "Tokyo Stock Exchange"
      display_name: "TSE")";
    auto node = YAML::Load(yaml_text);
    CHECK_THROWS_AS(parse_venue_database_entry(
      node, DEFAULT_COUNTRIES, DEFAULT_CURRENCIES), std::runtime_error);
  }


  TEST_CASE("parse_venue_database") {
    const auto yaml_text = R"(
      - venue: "ABC"
        country_code: "US"
        time_zone: "America/New_York"
        currency: "USD"
        description: "Desc1"
        display_name: "Alpha"
      - venue: "DEF"
        country_code: "GB"
        time_zone: "Europe/London"
        currency: "GBP"
        description: "Desc2"
        display_name: "Beta")";
    auto node = YAML::Load(yaml_text);
    auto database =
      parse_venue_database(node, DEFAULT_COUNTRIES, DEFAULT_CURRENCIES);
    auto entry_alpha = parse_venue_entry("Alpha", database);
    CHECK(entry_alpha.m_venue.get_code() == "ABC");
    CHECK(entry_alpha.m_display_name == "Alpha");
    auto entry_beta = parse_venue_entry("Beta", database);
    CHECK(entry_beta.m_venue.get_code() == "DEF");
    CHECK(entry_beta.m_display_name == "Beta");
    auto entry_by_code = parse_venue_entry("DEF", database);
    CHECK(entry_by_code.m_venue.get_code() == "DEF");
  }

  TEST_CASE("parse_venue") {
    auto entry = VenueDatabase::Entry();
    entry.m_venue = "ABC";
    entry.m_country_code = DefaultCountries::US;
    entry.m_time_zone = "America/New_York";
    entry.m_currency = DefaultCurrencies::USD;
    entry.m_description = "Desc1";
    entry.m_display_name = "Alpha";
    auto database = VenueDatabase();
    database.add(entry);
    auto venue_alpha = parse_venue("Alpha", database);
    CHECK(venue_alpha.get_code() == "ABC");
    auto venue_code = parse_venue("ABC", database);
    CHECK(venue_code.get_code() == "ABC");
  }

  TEST_CASE("to_string") {
    auto entry = VenueDatabase::Entry();
    entry.m_venue = "XYZ";
    entry.m_country_code = DefaultCountries::US;
    entry.m_time_zone = "America/New_York";
    entry.m_currency = DefaultCurrencies::USD;
    entry.m_description = "Desc";
    entry.m_display_name = "XyzVenue";
    auto database = VenueDatabase();
    database.add(entry);
    auto known = parse_venue("XyzVenue", database);
    CHECK(to_string(known, database) == "XyzVenue");
    auto unknown = Venue("UNK");
    CHECK(to_string(unknown, database) == "UNK");
  }

  TEST_CASE("stream") {
    auto entry = VenueDatabase::Entry();
    entry.m_venue = "LMN";
    entry.m_country_code = DefaultCountries::CA;
    entry.m_time_zone = "America/Toronto";
    entry.m_currency = DefaultCurrencies::CAD;
    entry.m_description = "Desc3";
    entry.m_display_name = "LmnVenue";
    auto database = VenueDatabase();
    database.add(entry);
    auto venue = parse_venue("LmnVenue", database);
    auto ss = std::ostringstream();
    ss << venue;
    CHECK(ss.str() == "LMN");
    ss.str(std::string());
    auto parsed_entry = parse_venue_entry("LmnVenue", database);
    ss << parsed_entry;
    CHECK(ss.str() == "(LMN 124 America/Toronto 124 Desc3 LmnVenue)");
  }
}
