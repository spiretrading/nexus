#include <doctest/doctest.h>
#include "Nexus/FeeHandling/CseFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto make_fee_table() {
    auto table = CseFeeTable();
    populate_fee_table(Store(table.m_fee_table));
    return table;
  }
}

TEST_SUITE("CseFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto table = make_fee_table();
    test_fee_table_index(table, table.m_fee_table, lookup_fee,
      LIQUIDITY_FLAG_COUNT, CseFeeTable::SECTION_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    test_per_share_fee_calculation(
      table, Money::ONE, 0, LiquidityFlag::NONE, Money::ZERO, calculate_fee);
  }

  TEST_CASE("default_active") {
    auto table = make_fee_table();
    auto expected_fee =
      lookup_fee(table, LiquidityFlag::ACTIVE, CseFeeTable::Section::DEFAULT);
    test_per_share_fee_calculation(
      table, Money::ONE, 100, "TC", expected_fee, calculate_fee);
  }

  TEST_CASE("default_passive") {
    auto table = make_fee_table();
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::PASSIVE, CseFeeTable::Section::DEFAULT);
    test_per_share_fee_calculation(
      table, Money::ONE, 100, "PC", expected_fee, calculate_fee);
  }

  TEST_CASE("dark_active") {
    auto table = make_fee_table();
    auto expected_fee =
      lookup_fee(table, LiquidityFlag::ACTIVE, CseFeeTable::Section::DARK);
    test_per_share_fee_calculation(
      table, Money::CENT, 100, "TCD", expected_fee, calculate_fee);
  }

  TEST_CASE("dark_passive") {
    auto table = make_fee_table();
    auto expected_fee =
      lookup_fee(table, LiquidityFlag::PASSIVE, CseFeeTable::Section::DARK);
    test_per_share_fee_calculation(
      table, Money::CENT, 100, "PCD", expected_fee, calculate_fee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto table = make_fee_table();
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "TC";
      auto calculated_fee = calculate_fee(table, report);
      auto expected_fee = report.m_last_quantity *
        lookup_fee(table, LiquidityFlag::ACTIVE, CseFeeTable::Section::DEFAULT);
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::CENT;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "TCD";
      auto calculated_fee = calculate_fee(table, report);
      auto expected_fee = report.m_last_quantity *
        lookup_fee(table, LiquidityFlag::ACTIVE, CseFeeTable::Section::DARK);
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "?";
      auto calculated_fee = calculate_fee(table, report);
      auto expected_fee = report.m_last_quantity *
        lookup_fee(table, LiquidityFlag::ACTIVE, CseFeeTable::Section::DEFAULT);
      REQUIRE(calculated_fee == expected_fee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto table = make_fee_table();
    auto expected_fee =
      lookup_fee(table, LiquidityFlag::ACTIVE, CseFeeTable::Section::DEFAULT);
    test_per_share_fee_calculation(
      table, Money::ONE, 100, LiquidityFlag::NONE, expected_fee, calculate_fee);
  }
}
