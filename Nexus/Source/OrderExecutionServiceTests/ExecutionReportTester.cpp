#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("ExecutionReport") {
  TEST_CASE("default_constructor") {
    auto report = ExecutionReport();
    REQUIRE(report.m_id == 0);
    REQUIRE(report.m_sequence == 0);
    REQUIRE(report.m_last_quantity == 0);
  }

  TEST_CASE("initial_report") {
    auto timestamp = time_from_string("2024-05-21 01:02:03");
    auto report = ExecutionReport(123, timestamp);
    REQUIRE(report.m_id == 123);
    REQUIRE(report.m_timestamp == timestamp);
    REQUIRE(report.m_sequence == 0);
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    REQUIRE(report.m_last_quantity == 0);
    REQUIRE(report.m_last_price == Money::ZERO);
    REQUIRE(report.m_execution_fee == Money::ZERO);
    REQUIRE(report.m_processing_fee == Money::ZERO);
    REQUIRE(report.m_commission == Money::ZERO);
    REQUIRE(report.m_text.empty());
    REQUIRE(report.m_additional_tags.empty());
  }

  TEST_CASE("updated_report") {
    auto timestamp1 = time_from_string("2024-05-21 01:02:03");
    auto initial_report = ExecutionReport(123, timestamp1);
    initial_report.m_last_quantity = 100;
    initial_report.m_last_price = Money::ONE;
    initial_report.m_liquidity_flag = "A";
    initial_report.m_last_market = "XNYS";
    initial_report.m_execution_fee = Money(1);
    initial_report.m_processing_fee = Money(2);
    initial_report.m_commission = Money(3);
    initial_report.m_text = "text";
    initial_report.m_additional_tags.emplace_back(100, "value");
    auto timestamp2 = time_from_string("2024-05-21 04:05:06");
    auto updated_report =
      make_update(initial_report, OrderStatus::FILLED, timestamp2);
    REQUIRE(updated_report.m_id == 123);
    REQUIRE(updated_report.m_timestamp == timestamp2);
    REQUIRE(updated_report.m_sequence == 1);
    REQUIRE(updated_report.m_status == OrderStatus::FILLED);
    REQUIRE(updated_report.m_last_quantity == 0);
    REQUIRE(updated_report.m_last_price == Money::ZERO);
    REQUIRE(updated_report.m_liquidity_flag.empty());
    REQUIRE(updated_report.m_last_market.empty());
    REQUIRE(updated_report.m_execution_fee == Money::ZERO);
    REQUIRE(updated_report.m_processing_fee == Money::ZERO);
    REQUIRE(updated_report.m_commission == Money::ZERO);
    REQUIRE(updated_report.m_text.empty());
    REQUIRE(updated_report.m_additional_tags.empty());
  }

  TEST_CASE("get_fee_total") {
    auto report = ExecutionReport();
    report.m_execution_fee = parse_money("1.23");
    report.m_processing_fee = parse_money("4.56");
    report.m_commission = parse_money("7.89");
    REQUIRE(get_fee_total(report) == parse_money("13.68"));
  }

  TEST_CASE("stream") {
    auto timestamp = time_from_string("2024-05-21 01:02:03");
    auto report = ExecutionReport(123, timestamp);
    report.m_status = OrderStatus::FILLED;
    report.m_last_quantity = 100;
    report.m_last_price = Money::ONE;
    report.m_liquidity_flag = "A";
    report.m_last_market = "XNYS";
    report.m_execution_fee = Money(1);
    report.m_processing_fee = Money(2);
    report.m_commission = Money(3);
    report.m_text = "text";
    report.m_additional_tags.emplace_back(100, "value1");
    report.m_additional_tags.emplace_back(200, "value2");
    REQUIRE(to_string(report) ==
      "(123 2024-May-21 01:02:03 0 FILLED 100 1.00 A XNYS"
      " 1.00 2.00 3.00 text [(100 value1) (200 value2)])");
    test_round_trip_shuttle(report);
  }
}
