#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceParameter.hpp"
#include "Nexus/Definitions/Ticker.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Venues;

TEST_SUITE("ComplianceParameter") {
  TEST_CASE("format_value") {
    auto test = [] (const ComplianceValue& value, const std::string& expected) {
      REQUIRE(std::format("{}", value) == expected);
      REQUIRE(to_string(value) == expected);
    };
    test(true, "true");
    test(false, "false");
    test(Quantity(123), "123");
    test(Quantity(1e20), "100000000000000000000");
    test(1.23456789, "1.23456789");
    test(std::string("text"), "text");
    test(std::string(), "");
    test(time_from_string("2026-10-16 09:30:00"), "2026-Oct-16 09:30:00");
    test(ptime(pos_infin), "+infinity");
    test(duration_from_string("01:02:03"), "01:02:03");
    test(DirectoryEntry::make_account(1, "test"), "(ACCOUNT 1 test)");
    test(Currencies::CAD, "CAD");
    test(parse_money("12.50"), "12.50");
    test(Money(1e20), "100000000000000000000.00");
    test(parse_ticker("TST.TSX"), "TST.TSX");
    test(TSX, "TSX");
    test(Scope("test_scope"), "test_scope");
    test(std::vector<ComplianceValue>(), "[]");
    auto nested = std::vector{ComplianceValue(std::string()),
      ComplianceValue(std::vector{ComplianceValue(true),
        ComplianceValue(Quantity(2))}), ComplianceValue(std::string())};
    test(nested, "[, [true, 2], ]");
    auto value = ComplianceValue(nested);
    REQUIRE(std::format("{:*>{}}", value, 17) == "**[, [true, 2], ]");
    REQUIRE(std::format("{:.4}", value) == "[, [");
  }

  TEST_CASE("compliance_value_stream") {
    auto value = ComplianceValue(parse_ticker("TST.TSX"));
    REQUIRE(to_string(value) == "TST.TSX");
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
    parameter.m_name = "ticker";
    parameter.m_value = ComplianceValue(parse_ticker("TST.TSX"));
    REQUIRE(to_string(parameter) == "(ticker TST.TSX)");
    REQUIRE(std::format("{}", parameter) == "(ticker TST.TSX)");
    REQUIRE(std::format("{:*>18}", parameter) == "**(ticker TST.TSX)");
    REQUIRE(std::format("{:.7}", parameter) == "(ticker");
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
