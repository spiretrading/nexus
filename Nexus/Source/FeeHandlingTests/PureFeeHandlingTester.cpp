#include <doctest/doctest.h>
#include "Nexus/FeeHandling/PureFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  auto make_fee_table() {
    auto table = PureFeeTable();
    for(auto i = 0; i != PureFeeTable::SECTION_COUNT; ++i) {
      populate_fee_table(out(table.m_fee_table[i]));
    }
    return table;
  }

  static const auto LIQUIDITY_INDICATOR = [] {
    auto indicators = std::map<LiquidityFlag, std::string>();
    indicators[LiquidityFlag::ACTIVE] = "T";
    indicators[LiquidityFlag::PASSIVE] = "P";
    return indicators;
  }();
}

TEST_SUITE("PureFeeHandling") {
  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto calculate_default_fee = [] (const auto& table, const auto& report) {
      return calculate_fee(table, PureFeeTable::Section::DEFAULT, report);
    };
    auto calculate_etf_fee = [] (const auto& table, const auto& report) {
      return calculate_fee(table, PureFeeTable::Section::ETF, report);
    };
    auto calculate_interlisted_fee =
      [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, PureFeeTable::Section::INTERLISTED, report);
      };
    test_per_share_fee_calculation(table, Money::ONE, 0, LiquidityFlag::NONE,
      Money::ZERO, calculate_default_fee);
    test_per_share_fee_calculation(table, Money::ONE, 0, LiquidityFlag::NONE,
      Money::ZERO, calculate_etf_fee);
    test_per_share_fee_calculation(table, Money::ONE, 0, LiquidityFlag::NONE,
      Money::ZERO, calculate_interlisted_fee);
  }

  TEST_CASE("fills") {
    auto table = make_fee_table();
    for(auto section : {PureFeeTable::Section::DEFAULT,
        PureFeeTable::Section::ETF, PureFeeTable::Section::INTERLISTED}) {
      auto calculate_default_fee = [=] (const auto& table, const auto& report) {
        return calculate_fee(table, section, report);
      };
      SUBCASE("subdollar") {
        auto active_fee = lookup_fee(
          table, section, PureFeeTable::Row::SUBDOLLAR, LiquidityFlag::ACTIVE);
        test_per_share_fee_calculation(
          table, Money::CENT, 100, "TT", active_fee, calculate_default_fee);
        auto passive_fee = lookup_fee(
          table, section, PureFeeTable::Row::SUBDOLLAR, LiquidityFlag::PASSIVE);
        test_per_share_fee_calculation(
          table, Money::CENT, 100, "PC", passive_fee, calculate_default_fee);
      }
      SUBCASE("default") {
        auto active_fee = lookup_fee(
          table, section, PureFeeTable::Row::DEFAULT, LiquidityFlag::ACTIVE);
        test_per_share_fee_calculation(
          table, Money::ONE, 100, "TT", active_fee, calculate_default_fee);
        auto passive_fee = lookup_fee(
          table, section, PureFeeTable::Row::DEFAULT, LiquidityFlag::PASSIVE);
        test_per_share_fee_calculation(
          table, Money::ONE, 100, "PC", passive_fee, calculate_default_fee);
      }
      SUBCASE("dark_subdollar") {
        auto active_fee = lookup_fee(table, section,
          PureFeeTable::Row::DARK_SUBDOLLAR, LiquidityFlag::ACTIVE);
        test_per_share_fee_calculation(
          table, Money::CENT, 100, "TTD", active_fee, calculate_default_fee);
        auto passive_fee = lookup_fee(table, section,
          PureFeeTable::Row::DARK_SUBDOLLAR, LiquidityFlag::PASSIVE);
        test_per_share_fee_calculation(
          table, Money::CENT, 100, "PCD", passive_fee, calculate_default_fee);
      }
      SUBCASE("dark") {
        auto active_fee = lookup_fee(
          table, section, PureFeeTable::Row::DARK, LiquidityFlag::ACTIVE);
        test_per_share_fee_calculation(
          table, Money::ONE, 100, "TTD", active_fee, calculate_default_fee);
        auto passive_fee = lookup_fee(
          table, section, PureFeeTable::Row::DARK, LiquidityFlag::PASSIVE);
        test_per_share_fee_calculation(
          table, Money::ONE, 100, "PCD", passive_fee, calculate_default_fee);
      }
    }
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto table = make_fee_table();
    auto report = ExecutionReport(0, second_clock::universal_time());
    report.m_last_price = Money::ONE;
    report.m_last_quantity = 100;
    report.m_liquidity_flag = "Q";
    auto calculated_fee =
      calculate_fee(table, PureFeeTable::Section::DEFAULT, report);
    auto expected_fee = report.m_last_quantity *
      lookup_fee(table, PureFeeTable::Section::DEFAULT,
        PureFeeTable::Row::DEFAULT, LiquidityFlag::ACTIVE);
    REQUIRE(calculated_fee == expected_fee);
  }

  TEST_CASE("empty_liquidity_flag") {
    auto table = make_fee_table();
    auto report = ExecutionReport(0, second_clock::universal_time());
    report.m_last_price = Money::ONE;
    report.m_last_quantity = 100;
    report.m_liquidity_flag = "";
    auto calculated_fee =
      calculate_fee(table, PureFeeTable::Section::DEFAULT, report);
    auto expected_fee = report.m_last_quantity *
      lookup_fee(table, PureFeeTable::Section::DEFAULT,
        PureFeeTable::Row::DEFAULT, LiquidityFlag::ACTIVE);
    REQUIRE(calculated_fee == expected_fee);
  }
}
