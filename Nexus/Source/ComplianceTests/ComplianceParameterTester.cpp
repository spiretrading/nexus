#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceParameter.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

TEST_SUITE("ComplianceParameter") {
  TEST_CASE("compliance_value_stream") {
    auto value = ComplianceValue(CAD);
    REQUIRE(to_string(value) == "CAD");
  }

  TEST_CASE("compliance_value_vector_stream") {
    auto vector = std::vector<ComplianceValue>();
    vector.push_back(parse_ticker("TST.TSX"));
    vector.push_back(parse_ticker("ABC.ASX"));
    auto value = ComplianceValue(vector);
    REQUIRE(to_string(value) == "[TST.TSX, ABC.ASX]");
  }

  TEST_CASE("compliance_value_nested_vector_stream") {
    auto inner_vector = std::vector<ComplianceValue>();
    inner_vector.push_back(parse_ticker("TST.TSX"));
    inner_vector.push_back(parse_ticker("ABC.ASX"));
    auto outer_vector = std::vector<ComplianceValue>();
    outer_vector.push_back(inner_vector);
    outer_vector.push_back(parse_ticker("XYZ.TSXV"));
    auto value = ComplianceValue(outer_vector);
    REQUIRE(to_string(value) == "[[TST.TSX, ABC.ASX], XYZ.TSXV]");
  }

  TEST_CASE("stream") {
    auto parameter = ComplianceParameter();
    parameter.m_name = "security";
    parameter.m_value = ComplianceValue(parse_ticker("TST.TSX"));
    REQUIRE(to_string(parameter) == "(security TST.TSX)");
  }

  TEST_CASE("shuttle") {
    auto inner_vector = std::vector<ComplianceValue>();
    inner_vector.push_back(parse_ticker("TST.TSX"));
    inner_vector.push_back(parse_ticker("ABC.ASX"));
    auto outer_vector = std::vector<ComplianceValue>();
    outer_vector.push_back(inner_vector);
    outer_vector.push_back(parse_ticker("XYZ.TSXV"));
    auto value = ComplianceValue(outer_vector);
    test_round_trip_shuttle(value);
  }
}
