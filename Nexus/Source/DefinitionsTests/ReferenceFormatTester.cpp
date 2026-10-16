#include <sstream>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/ExchangeRate.hpp"
#include "Nexus/Definitions/Scope.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("ReferenceFormat") {
  TEST_CASE("country") {
    REQUIRE(std::format("{}", CountryCode(124)) == "CA");
    REQUIRE(std::format("{}", CountryCode(9999)) == "9999");
    REQUIRE(std::format("{}", CountryCode::NONE) == "65535");
    REQUIRE(std::format("{:>5}", CountryCode(124)) == "   CA");
    REQUIRE(std::format("{:.1}", CountryCode(124)) == "C");
  }

  TEST_CASE("currency") {
    REQUIRE(std::format("{}", Currencies::CAD) == "CAD");
    REQUIRE(std::format("{}", CurrencyId(9999)) == "9999");
    REQUIRE(std::format("{}", CurrencyId::NONE) == "65535");
    REQUIRE(std::format("{:>5}", Currencies::CAD) == "  CAD");
    REQUIRE(std::format("{:.2}", Currencies::CAD) == "CA");
  }

  TEST_CASE("venue") {
    REQUIRE(std::format("{}", Venues::TSX) == "TSX");
    REQUIRE(std::format("{}", Venue("TEST")) == "TEST");
    REQUIRE(std::format("{}", Venue()) == "");
    REQUIRE(std::format("{:>5}", Venues::TSX) == "  TSX");
    REQUIRE(std::format("{:.2}", Venues::TSX) == "TS");
  }

  TEST_CASE("currency_pair_and_exchange_rate") {
    auto pair = CurrencyPair(Currencies::CAD, Currencies::USD);
    auto rate = ExchangeRate(pair, boost::rational<int>(3, 2));
    REQUIRE(std::format("{}", pair) == "CAD/USD");
    REQUIRE(std::format("{:>9}", pair) == "  CAD/USD");
    REQUIRE(std::format("{}", rate) == "(CAD/USD 3/2)");
    REQUIRE(std::format("{:>15}", rate) == "  (CAD/USD 3/2)");
    REQUIRE(to_string(pair) == "CAD/USD");
    REQUIRE(to_string(rate) == "(CAD/USD 3/2)");
  }

  TEST_CASE("ticker") {
    auto ticker = parse_ticker("ABC.TSX");
    REQUIRE(std::format("{}", ticker) == "ABC.TSX");
    REQUIRE(std::format("{:>9}", ticker) == "  ABC.TSX");
    REQUIRE(std::format("{}", Ticker()) == "");
    REQUIRE(std::format("{}", Ticker("ABC", Venue())) == "ABC");
    REQUIRE(std::format("{}", Ticker("", Venues::TSX)) == "");
    REQUIRE(std::format("{}", Ticker("ABC", Venue("TEST"))) == "ABC.TEST");
    REQUIRE(to_string(ticker) == "ABC.TSX");
  }

  TEST_CASE("scope") {
    auto scope = Scope("Canada");
    REQUIRE(std::format("{}", scope) == "Canada");
    REQUIRE(std::format("{:>8}", scope) == "  Canada");
    REQUIRE(std::format("{:.3}", scope) == "Can");
    REQUIRE(std::format("{}", Scope()) == "");
    REQUIRE(to_string(scope) == "Canada");
  }

  TEST_CASE("custom_databases") {
    auto countries = CountryDatabase();
    auto country = CountryDatabase::Entry();
    country.m_code = CountryCode(124);
    country.m_two_letter_code = "ZZ";
    countries.add(country);
    auto currencies = CurrencyDatabase();
    auto currency = CurrencyDatabase::Entry();
    currency.m_id = Currencies::CAD;
    currency.m_code = "AAA";
    currencies.add(currency);
    auto venues = VenueDatabase();
    auto venue = VenueDatabase::Entry();
    venue.m_venue = Venues::TSX;
    venue.m_country_code = country.m_code;
    venue.m_market_center = "CENTER";
    venue.m_time_zone = "Etc/UTC";
    venue.m_currency = currency.m_id;
    venue.m_description = "description";
    venue.m_display_name = "CUSTOM";
    venues.add(venue);
    SUBCASE("locale") {
      auto locale = make_locale(countries, currencies, venues);
      REQUIRE(std::format(locale, "{} {} {}",
        country.m_code, currency.m_id, venue.m_venue) == "ZZ AAA CUSTOM");
      REQUIRE(std::format(locale, "{}",
        CurrencyPair(Currencies::CAD, Currencies::USD)) == "AAA/840");
      REQUIRE(std::format(locale, "{}",
        ExchangeRate(CurrencyPair(Currencies::CAD, Currencies::USD),
          boost::rational<int>(3, 2))) == "(AAA/840 3/2)");
      REQUIRE(std::format(locale, "{}", parse_ticker("ABC.TSX")) ==
        "ABC.CUSTOM");
      REQUIRE(std::format(locale, "{}", venue) ==
        "(CUSTOM ZZ CENTER Etc/UTC AAA description CUSTOM)");
      REQUIRE(std::format(make_locale(currencies), "{} {}",
        country.m_code, currency.m_id) == "CA AAA");
    }
    SUBCASE("stream_locale") {
      auto output = std::ostringstream();
      output.imbue(make_locale(countries, currencies, venues));
      auto pair = CurrencyPair(Currencies::CAD, Currencies::USD);
      auto rate = ExchangeRate(pair, boost::rational<int>(3, 2));
      output << country.m_code << ' ' << currency.m_id << ' ' <<
        venue.m_venue << ' ' << pair << ' ' << rate << ' ' <<
        parse_ticker("ABC.TSX") <<
        ' ' << venue;
      REQUIRE(output.str() == "ZZ AAA CUSTOM AAA/840 (AAA/840 3/2) ABC.CUSTOM "
        "(CUSTOM ZZ CENTER Etc/UTC AAA description CUSTOM)");
    }
    SUBCASE("stream_locale_overrides") {
      auto output = std::ostringstream();
      output.imbue(make_locale(countries, currencies, venues));
      output << COUNTRIES << country.m_code;
      output << ' ' << CURRENCIES << currency.m_id;
      output << ' ' << VENUES << venue.m_venue;
      output << ' ' << country.m_code << ' ' << currency.m_id <<
        ' ' << venue.m_venue;
      REQUIRE(output.str() == "CA CAD TSX ZZ AAA CUSTOM");
      output.str("");
      auto pair = CurrencyPair(Currencies::CAD, Currencies::USD);
      auto rate = ExchangeRate(pair, boost::rational<int>(3, 2));
      output << CURRENCIES << pair << ' ' << rate;
      output << ' ' << VENUES << parse_ticker("ABC.TSX");
      REQUIRE(output.str() == "CAD/USD (CAD/USD 3/2) ABC.TSX");
      output.str("");
      output << CURRENCIES << venue;
      output << ' ' << COUNTRIES << CURRENCIES << VENUES << venue;
      output << ' ' << venue;
      REQUIRE(output.str() ==
        "(CUSTOM ZZ CENTER Etc/UTC CAD description CUSTOM) "
        "(TSX CA CENTER Etc/UTC CAD description CUSTOM) "
        "(CUSTOM ZZ CENTER Etc/UTC AAA description CUSTOM)");
    }
    SUBCASE("identifiers") {
      auto output = std::ostringstream();
      output << countries << country.m_code;
      output << ' ' << country.m_code;
      output << ' ' << currencies << currency.m_id;
      output << ' ' << currency.m_id;
      output << ' ' << venues << venue.m_venue;
      output << ' ' << venue.m_venue;
      REQUIRE(output.str() == "ZZ CA AAA CAD CUSTOM TSX");
      REQUIRE(std::format("{} {} {}",
        country.m_code, currency.m_id, venue.m_venue) == "CA CAD TSX");
    }
    SUBCASE("currency_pair_and_exchange_rate") {
      auto pair = CurrencyPair(Currencies::CAD, Currencies::USD);
      auto rate = ExchangeRate(pair, boost::rational<int>(3, 2));
      auto output = std::ostringstream();
      output << currencies << pair << ' ' << rate;
      REQUIRE(output.str() == "AAA/840 (AAA/840 3/2)");
      REQUIRE(std::format("{} {}", pair, rate) == "CAD/USD (CAD/USD 3/2)");
    }
    SUBCASE("ticker") {
      auto ticker = parse_ticker("ABC.TSX");
      auto output = std::ostringstream();
      output << venues << ticker;
      REQUIRE(output.str() == "ABC.CUSTOM");
      REQUIRE(std::format("{}", ticker) == "ABC.TSX");
    }
    SUBCASE("venue_entry") {
      auto output = std::ostringstream();
      output << countries << currencies << venues << venue;
      REQUIRE(output.str() ==
        "(CUSTOM ZZ CENTER Etc/UTC AAA description CUSTOM)");
      REQUIRE(std::format("{}", venue) ==
        "(TSX CA CENTER Etc/UTC CAD description CUSTOM)");
      REQUIRE(to_string(venue) ==
        "(TSX CA CENTER Etc/UTC CAD description CUSTOM)");
      REQUIRE(std::format("{:.7}", venue) == "(TSX CA");
    }
  }
}
