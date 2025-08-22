#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceParameter.hpp"

using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::DefaultVenues;

TEST_SUITE("ComplianceParameter") {
  TEST_CASE("compliance_value_stream") {
    auto value = ComplianceValue(Security("TST", TSX));
    auto stream = std::ostringstream();
    stream << value;
    REQUIRE(stream.str() == "TST.TSX");
  }

  TEST_CASE("compliance_value_vector_stream") {
    auto vector = std::vector<ComplianceValue>();
    vector.push_back(Security("TST", TSX));
    vector.push_back(Security("ABC", NYSE));
    auto value = ComplianceValue(vector);
    auto stream = std::ostringstream();
    stream << value;
    REQUIRE(stream.str() == "[TST.TSX, ABC.NYSE]");
  }

  TEST_CASE("compliance_value_nested_vector_stream") {
    auto inner_vector = std::vector<ComplianceValue>();
    inner_vector.push_back(Security("TST", TSX));
    inner_vector.push_back(Security("ABC", NYSE));
    auto outer_vector = std::vector<ComplianceValue>();
    outer_vector.push_back(inner_vector);
    outer_vector.push_back(Security("XYZ", NASDAQ));
    auto value = ComplianceValue(outer_vector);
    auto stream = std::ostringstream();
    stream << value;
    REQUIRE(stream.str() == "[[TST.TSX, ABC.NYSE], XYZ.NSDQ]");
  }

  TEST_CASE("stream") {
    auto parameter = ComplianceParameter();
    parameter.m_name = "security";
    parameter.m_value = ComplianceValue(Security("TST", TSX));
    auto stream = std::ostringstream();
    stream << parameter;
    REQUIRE(stream.str() == "(security TST.TSX)");
  }

  TEST_CASE("shuttle") {
    auto inner_vector = std::vector<ComplianceValue>();
    inner_vector.push_back(Security("TST", TSX));
    inner_vector.push_back(Security("ABC", NYSE));
    auto outer_vector = std::vector<ComplianceValue>();
    outer_vector.push_back(inner_vector);
    outer_vector.push_back(Security("XYZ", NASDAQ));
    auto value = ComplianceValue(outer_vector);
    Beam::Serialization::Tests::TestRoundTripShuttle(value);
  }
}
