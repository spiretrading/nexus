#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Compliance/ComplianceParameter.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"

using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::DefaultVenues;

TEST_SUITE("ComplianceRuleSchema") {
  TEST_CASE("constructor") {
    auto parameter = ComplianceParameter();
    parameter.m_name = "security";
    parameter.m_value = ComplianceValue(Security("TST", TSX));
    auto parameters = std::vector<ComplianceParameter>();
    parameters.push_back(parameter);
    auto rule = ComplianceRuleSchema("test_rule", parameters);
    REQUIRE(rule.get_name() == "test_rule");
    REQUIRE(rule.get_parameters().size() == 1);
    REQUIRE(rule.get_parameters().front() == parameter);
  }

  TEST_CASE("stream_operator") {
    auto parameter = ComplianceParameter();
    parameter.m_name = "security";
    parameter.m_value = ComplianceValue(Security("TST", TSX));
    auto rule = ComplianceRuleSchema(
      "test_rule", std::vector<ComplianceParameter>(1, parameter));
    auto stream = std::ostringstream();
    stream << rule;
    REQUIRE(stream.str() == "(test_rule [(security TST.TSX)])");
  }

  TEST_CASE("shuttle") {
    auto parameter = ComplianceParameter();
    parameter.m_name = "security";
    parameter.m_value = ComplianceValue(Security("TST", TSX));
    auto rule = ComplianceRuleSchema(
      "test_rule", std::vector<ComplianceParameter>(1, parameter));
    Beam::Serialization::Tests::TestRoundTripShuttle(rule);
  }
}
