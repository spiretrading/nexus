#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/RiskModification.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::AdministrationService;
using namespace Nexus::RiskService;

TEST_SUITE("risk_modification") {
  TEST_CASE("shuttle") {
    auto parameters = RiskParameters();
    parameters.m_currency = CAD;
    parameters.m_buyingPower = Money(100);
    parameters.m_allowedState = RiskState::Type::ACTIVE;
    parameters.m_netLoss = Money(200);
    parameters.m_lossFromTop = 23;
    parameters.m_transitionTime = seconds(100);
    auto modification = RiskModification(parameters);
    Beam::Serialization::Tests::TestRoundTripShuttle(modification,
      [&](const auto& received) {
        REQUIRE(received.get_parameters() == parameters);
      });
  }
}
