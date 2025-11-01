#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/RiskModification.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

TEST_SUITE("risk_modification") {
  TEST_CASE("shuttle") {
    auto parameters = RiskParameters();
    parameters.m_currency = CAD;
    parameters.m_buying_power = Money(100);
    parameters.m_allowed_state = RiskState::Type::ACTIVE;
    parameters.m_net_loss = Money(200);
    parameters.m_transition_time = seconds(100);
    auto modification = RiskModification(parameters);
    test_round_trip_shuttle(modification, [&] (const auto& received) {
      REQUIRE(received.get_parameters() == parameters);
    });
  }
}
