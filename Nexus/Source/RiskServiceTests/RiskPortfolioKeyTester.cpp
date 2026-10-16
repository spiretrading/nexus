#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::Venues;

TEST_SUITE("RiskPortfolioKey") {
  TEST_CASE("format_databases") {
    auto venues = VenueDatabase();
    auto venue = VenueDatabase::Entry();
    venue.m_venue = TSX;
    venue.m_display_name = "VENUE";
    venues.add(venue);
    auto currencies = CurrencyDatabase();
    auto currency = CurrencyDatabase::Entry();
    currency.m_id = Currencies::CAD;
    currency.m_code = "CUS";
    currencies.add(currency);
    auto key = RiskPortfolioKey(DirectoryEntry::make_account(123, "test"),
      parse_ticker("ABC.TSX"));
    auto expected = std::string("((ACCOUNT 123 test) ABC.VENUE)");
    auto locale = make_locale(currencies, venues);
    REQUIRE(std::format(locale, "{}", key) == expected);
    auto output = std::ostringstream();
    output.imbue(locale);
    output << key;
    REQUIRE(output.str() == expected);
    output.str("");
    output << VENUES << key;
    REQUIRE(output.str() == to_string(key));
    output.str("");
    output.imbue(std::locale::classic());
    output << venues << key;
    REQUIRE(output.str() == expected);
    auto entry = RiskInventoryEntry(key,
      Inventory(Position(key.m_ticker, Currencies::CAD, 100, Money(1000)),
        Money(50), Money::CENT, 200, 2));
    auto entry_text = std::string("(((ACCOUNT 123 test) ABC.VENUE) "
      "((ABC.VENUE CUS 100 1000.00) 50.00 0.01 200 2))");
    REQUIRE(std::format(locale, "{}", entry) == entry_text);
    output.str("");
    output.imbue(locale);
    output << entry;
    REQUIRE(output.str() == entry_text);
  }

  TEST_CASE("stream") {
    auto account = DirectoryEntry::make_account(123, "test_account");
    auto ticker = parse_ticker("ABC.TSX");
    auto key = RiskPortfolioKey(account, ticker);
    auto expected = std::string("((ACCOUNT 123 test_account) ABC.TSX)");
    REQUIRE(to_string(key) == expected);
    REQUIRE(std::format("{}", key) == expected);
    REQUIRE(std::format("{:*>{}}", key, expected.size() + 2) ==
      "**" + expected);
    REQUIRE(std::format("{:.9}", key) == "((ACCOUNT");
    test_round_trip_shuttle(key);
  }
}
