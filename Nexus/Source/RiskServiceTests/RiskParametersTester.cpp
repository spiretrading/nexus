#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/RiskService/RiskParameters.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("RiskParameters") {
  TEST_CASE("stream") {
    auto currency = USD;
    auto buying_power = 20000 * Money::ONE;
    auto allowed_state = RiskState(
      RiskState::Type::ACTIVE, time_from_string("2025-08-15 09:30:00"));
    auto net_loss = 1000 * Money::ONE;
    auto transition_time = hours(1) + minutes(15);
    auto parameters = RiskParameters(
      currency, buying_power, allowed_state, net_loss, transition_time);
    REQUIRE(to_string(parameters) ==
      "(USD 20000.00 (ACTIVE 2025-Aug-15 09:30:00) 1000.00 01:15:00)");
    test_round_trip_shuttle(parameters);
  }
}
