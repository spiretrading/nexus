#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Compliance/ComplianceParameter.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace Nexus;
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

  TEST_CASE("wrap_and_unwrap") {
    auto parameter = ComplianceParameter("security", Security("ABC", TSX));
    auto parameters = std::vector{parameter};
    auto base_schema = ComplianceRuleSchema("base_rule", parameters);
    auto wrapped_schema = wrap("wrapper_rule",
      {{"region", ComplianceValue(Region::GLOBAL)}}, base_schema);
    REQUIRE(wrapped_schema.get_name() == "wrapper_rule");
    auto found_name = false;
    auto found_arguments = false;
    auto found_region = false;
    for(auto& parameter : wrapped_schema.get_parameters()) {
      if(parameter.m_name == "name") {
        REQUIRE(get<std::string>(parameter.m_value) == "base_rule");
        found_name = true;
      } else if(parameter.m_name == "arguments") {
        auto arguments = get<std::vector<ComplianceValue>>(parameter.m_value);
        REQUIRE(arguments.size() == 1);
        auto argument = get<std::vector<ComplianceValue>>(arguments[0]);
        REQUIRE(argument.size() == 2);
        REQUIRE(get<std::string>(argument[0]) == "security");
        REQUIRE(get<Security>(argument[1]) == Security("ABC", TSX));
        found_arguments = true;
      } else if(parameter.m_name == "region") {
        REQUIRE(get<Region>(parameter.m_value) == Region::GLOBAL);
        found_region = true;
      }
    }
    REQUIRE(found_name);
    REQUIRE(found_arguments);
    REQUIRE(found_region);
    auto unwrapped_schema = unwrap(wrapped_schema);
    REQUIRE(unwrapped_schema.get_name() == "base_rule");
    REQUIRE(unwrapped_schema.get_parameters().size() == 1);
    REQUIRE(unwrapped_schema.get_parameters()[0] == parameter);
  }

  TEST_CASE("wrap_and_unwrap_no_extra_parameters") {
    auto parameter = ComplianceParameter("count", Quantity(5));
    auto base_rule = ComplianceRuleSchema("limit_rule", {parameter});
    auto wrapped_schema = wrap("per_side", base_rule);
    REQUIRE(wrapped_schema.get_name() == "per_side");
    auto unwrapped_schema = unwrap(wrapped_schema);
    REQUIRE(unwrapped_schema.get_name() == "limit_rule");
    REQUIRE(unwrapped_schema.get_parameters().size() == 1);
    REQUIRE(unwrapped_schema.get_parameters()[0] == parameter);
  }

  TEST_CASE("unwrap_invalid_arguments_throws") {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("name", std::string("bad_rule"));
    auto bad_argument = std::vector<ComplianceValue>{std::string("only_one")};
    auto arguments = std::vector<ComplianceValue>();
    arguments.push_back(bad_argument);
    parameters.emplace_back("arguments", arguments);
    auto bad_schema = ComplianceRuleSchema("bad_wrapper", parameters);
    REQUIRE_THROWS_AS(unwrap(bad_schema), std::runtime_error);
  }

  TEST_CASE("stream_operator") {
    auto parameter = ComplianceParameter();
    parameter.m_name = "security";
    parameter.m_value = ComplianceValue(Security("TST", TSX));
    auto rule = ComplianceRuleSchema(
      "test_rule", std::vector<ComplianceParameter>(1, parameter));
    REQUIRE(to_string(rule) == "(test_rule [(security TST.TSX)])");
    test_round_trip_shuttle(rule);
  }
}
