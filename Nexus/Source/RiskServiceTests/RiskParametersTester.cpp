#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/RiskService/RiskParameters.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("RiskParameters") {
  TEST_CASE("format_currency") {
    auto currencies = CurrencyDatabase();
    auto currency = CurrencyDatabase::Entry();
    currency.m_id = USD;
    currency.m_code = "CUS";
    currencies.add(currency);
    auto parameters =
      RiskParameters(USD, Money(20000), RiskState(), Money(1000), minutes(15));
    auto expected =
      std::string("(CUS 20000.00 (ACTIVE +infinity) 1000.00 00:15:00)");
    auto locale = make_locale(currencies);
    REQUIRE(std::format(locale, "{}", parameters) == expected);
    auto output = std::ostringstream();
    output.imbue(locale);
    output << parameters;
    REQUIRE(output.str() == expected);
    output.str("");
    output << CURRENCIES << parameters;
    REQUIRE(output.str() == to_string(parameters));
    output.str("");
    output.imbue(std::locale::classic());
    output << currencies << parameters;
    REQUIRE(output.str() == expected);
  }

  TEST_CASE("stream") {
    auto currency = USD;
    auto buying_power = 20000 * Money::ONE;
    auto allowed_state = RiskState(
      RiskState::Type::ACTIVE, time_from_string("2025-08-15 09:30:00"));
    auto net_loss = 1000 * Money::ONE;
    auto transition_time = duration_from_string("01:15:00");
    auto parameters = RiskParameters(
      currency, buying_power, allowed_state, net_loss, transition_time);
    auto expected = std::string(
      "(USD 20000.00 (ACTIVE 2025-Aug-15 09:30:00) 1000.00 01:15:00)");
    REQUIRE(to_string(parameters) == expected);
    REQUIRE(std::format("{}", parameters) == expected);
    REQUIRE(std::format("{:*>{}}", parameters, expected.size() + 2) ==
      "**" + expected);
    REQUIRE(std::format("{:.4}", parameters) == "(USD");
    test_round_trip_shuttle(parameters);
  }
}
