#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;

TEST_SUITE("ComplianceRuleEntry") {
  TEST_CASE("format_databases") {
    auto currencies = CurrencyDatabase();
    auto currency = CurrencyDatabase::Entry();
    currency.m_id = Currencies::CAD;
    currency.m_code = "CUS";
    currencies.add(currency);
    auto venues = VenueDatabase();
    auto venue = VenueDatabase::Entry();
    venue.m_venue = Venues::TSX;
    venue.m_display_name = "VENUE";
    venues.add(venue);
    auto locale = make_locale(currencies, venues);
    auto test = [&] (const auto& value, const std::string& expected) {
      REQUIRE(std::format(locale, "{}", value) == expected);
      auto output = std::ostringstream();
      output.imbue(locale);
      output << value;
      REQUIRE(output.str() == expected);
      output.str("");
      output << CURRENCIES << VENUES << value;
      REQUIRE(output.str() == to_string(value));
      output.str("");
      output.imbue(std::locale::classic());
      output << currencies << venues << value;
      REQUIRE(output.str() == expected);
    };
    auto value = ComplianceValue(std::vector{ComplianceValue(Currencies::CAD),
      ComplianceValue(std::vector{ComplianceValue(Venues::TSX),
        ComplianceValue(parse_ticker("TST.TSX"))})});
    test(value, "[CUS, [VENUE, TST.VENUE]]");
    auto parameter = ComplianceParameter("list", value);
    test(parameter, "(list [CUS, [VENUE, TST.VENUE]])");
    auto schema = ComplianceRuleSchema("rule", {parameter});
    test(schema, "(rule [(list [CUS, [VENUE, TST.VENUE]])])");
    auto entry = ComplianceRuleEntry(1,
      DirectoryEntry::make_account(123, "test"),
      ComplianceRuleEntry::State::ACTIVE, schema);
    test(entry, "(1 (ACCOUNT 123 test) ACTIVE "
      "(rule [(list [CUS, [VENUE, TST.VENUE]])]))");
  }

  TEST_CASE("constructor") {
    auto directory_entry = DirectoryEntry::make_account(321, "alpha");
    auto parameter = ComplianceParameter("amount", ComplianceValue(100.0));
    auto schema = ComplianceRuleSchema("limit_rule", std::vector{parameter});
    auto entry = ComplianceRuleEntry(
      99, directory_entry, ComplianceRuleEntry::State::PASSIVE, schema);
    REQUIRE(entry.get_id() == 99);
    REQUIRE(entry.get_directory_entry() == directory_entry);
    REQUIRE(entry.get_state() == ComplianceRuleEntry::State::PASSIVE);
    REQUIRE(entry.get_schema() == schema);
    entry.set_state(ComplianceRuleEntry::State::DISABLED);
    REQUIRE(entry.get_state() == ComplianceRuleEntry::State::DISABLED);
  }

  TEST_CASE("stream") {
    auto directory_entry = DirectoryEntry::make_account(123, "test");
    auto parameter = ComplianceParameter("quantity", true);
    auto schema = ComplianceRuleSchema("rule_name", std::vector{parameter});
    auto entry = ComplianceRuleEntry(
      42, directory_entry, ComplianceRuleEntry::State::ACTIVE, schema);
    auto expected = std::string(
      "(42 (ACCOUNT 123 test) ACTIVE (rule_name [(quantity true)]))");
    REQUIRE(to_string(entry) == expected);
    REQUIRE(std::format("{}", entry) == expected);
    REQUIRE(std::format("{:*>{}}", entry, expected.size() + 2) ==
      "**" + expected);
    REQUIRE(std::format("{:.3}", entry) == "(42");
    test_round_trip_shuttle(entry);
  }
}
