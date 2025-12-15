#include <doctest/doctest.h>
#include "Nexus/FeeHandling/MatnFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  auto make_fee_table() {
    auto table = MatnFeeTable();
    populate_fee_table(out(table.m_general_fee_table));
    populate_fee_table(out(table.m_alternative_fee_table));
    for(auto i = 0; i < MatnFeeTable::PRICE_CLASS_COUNT; ++i) {
      table.m_general_fee_table[i][
        static_cast<int>(MatnFeeTable::GeneralIndex::MAX_CHARGE)] *= 10000;
    }
    return table;
  }

  auto lookup_general_fee(const MatnFeeTable& table,
      MatnFeeTable::GeneralIndex index, MatnFeeTable::PriceClass price_class) {
    return lookup_fee(table, index, price_class);
  }

  auto lookup_alternative_fee(const MatnFeeTable& table, LiquidityFlag flag,
      MatnFeeTable::Category category) {
    return lookup_fee(table, flag, category);
  }
}

TEST_SUITE("MatnFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto table = make_fee_table();
    test_fee_table_index(table, table.m_general_fee_table, lookup_general_fee,
      MatnFeeTable::GENERAL_INDEX_COUNT, MatnFeeTable::PRICE_CLASS_COUNT);
    test_fee_table_index(table, table.m_alternative_fee_table,
      lookup_alternative_fee, LIQUIDITY_FLAG_COUNT,
        MatnFeeTable::CATEGORY_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    test_per_share_fee_calculation(table, Money::ONE, 0, LiquidityFlag::NONE,
      Money::ZERO, [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
  }

  TEST_CASE("active_default_trade") {
    auto table = make_fee_table();
    auto expected_fee = lookup_fee(table, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, 5 * Money::ONE, 100, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
    auto expected_max_fee =
      lookup_fee(table, MatnFeeTable::GeneralIndex::MAX_CHARGE,
        MatnFeeTable::PriceClass::DEFAULT);
    test_fee_calculation(table, 5 * Money::ONE, 1000000, LiquidityFlag::ACTIVE,
      expected_max_fee, [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
  }

  TEST_CASE("passive_default_trade") {
    auto table = make_fee_table();
    auto expected_fee = lookup_fee(table, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, 5 * Money::ONE, 100, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
    auto expected_max_fee =
      lookup_fee(table, MatnFeeTable::GeneralIndex::MAX_CHARGE,
        MatnFeeTable::PriceClass::DEFAULT);
    test_fee_calculation(table, 5 * Money::ONE, 1000000, LiquidityFlag::PASSIVE,
      expected_max_fee, [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
  }

  TEST_CASE("active_dollar_trade") {
    auto table = make_fee_table();
    auto expected_fee = lookup_fee(table, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::SUBFIVE_DOLLAR);
    test_per_share_fee_calculation(
      table, Money::ONE, 100, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
    auto expected_max_fee =
      lookup_fee(table, MatnFeeTable::GeneralIndex::MAX_CHARGE,
        MatnFeeTable::PriceClass::SUBFIVE_DOLLAR);
    test_fee_calculation(table, Money::ONE, 1000000, LiquidityFlag::ACTIVE,
      expected_max_fee, [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
  }

  TEST_CASE("passive_dollar_trade") {
    auto table = make_fee_table();
    auto expected_fee = lookup_fee(table, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::SUBFIVE_DOLLAR);
    test_per_share_fee_calculation(
      table, Money::ONE, 100, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
    auto expected_max_fee =
      lookup_fee(table, MatnFeeTable::GeneralIndex::MAX_CHARGE,
        MatnFeeTable::PriceClass::SUBFIVE_DOLLAR);
    test_fee_calculation(table, Money::ONE, 1000000, LiquidityFlag::PASSIVE,
      expected_max_fee, [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
  }

  TEST_CASE("active_subdollar_trade") {
    auto table = make_fee_table();
    auto expected_fee = lookup_fee(table, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, Money::CENT, 100, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
    auto expected_max_fee =
      lookup_fee(table, MatnFeeTable::GeneralIndex::MAX_CHARGE,
        MatnFeeTable::PriceClass::SUBDOLLAR);
    test_fee_calculation(table, Money::CENT, 1000000, LiquidityFlag::ACTIVE,
      expected_max_fee, [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
  }

  TEST_CASE("passive_subdollar_trade") {
    auto table = make_fee_table();
    auto expected_fee = lookup_fee(table, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, Money::CENT, 100, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
    auto expected_max_fee =
      lookup_fee(table, MatnFeeTable::GeneralIndex::MAX_CHARGE,
        MatnFeeTable::PriceClass::SUBDOLLAR);
    test_fee_calculation(table, Money::CENT, 1000000, LiquidityFlag::PASSIVE,
      expected_max_fee, [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
  }

  TEST_CASE("active_etf_trade") {
    auto table = make_fee_table();
    auto expected_fee =
      lookup_fee(table, LiquidityFlag::ACTIVE, MatnFeeTable::Category::ETF);
    test_per_share_fee_calculation(
      table, Money::ONE, 100, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(table, MatnFeeTable::Classification::ETF, report);
      });
  }

  TEST_CASE("passive_etf_trade") {
    auto table = make_fee_table();
    auto expected_fee =
      lookup_fee(table, LiquidityFlag::PASSIVE, MatnFeeTable::Category::ETF);
    test_per_share_fee_calculation(
      table, Money::ONE, 100, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(table, MatnFeeTable::Classification::ETF, report);
      });
  }

  TEST_CASE("active_odd_lot_trade") {
    auto table = make_fee_table();
    auto expected_fee =
      lookup_fee(table, LiquidityFlag::ACTIVE, MatnFeeTable::Category::ODD_LOT);
    test_per_share_fee_calculation(
      table, Money::ONE, 50, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
  }

  TEST_CASE("passive_odd_lot_trade") {
    auto table = make_fee_table();
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::PASSIVE, MatnFeeTable::Category::ODD_LOT);
    test_per_share_fee_calculation(
      table, Money::ONE, 50, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
  }

  TEST_CASE("active_odd_lot_etf") {
    auto table = make_fee_table();
    auto expected_fee =
      lookup_fee(table, LiquidityFlag::ACTIVE, MatnFeeTable::Category::ODD_LOT);
    test_per_share_fee_calculation(
      table, Money::ONE, 50, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(table, MatnFeeTable::Classification::ETF, report);
      });
  }

  TEST_CASE("passive_odd_lot_etf") {
    auto table = make_fee_table();
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::PASSIVE, MatnFeeTable::Category::ODD_LOT);
    test_per_share_fee_calculation(
      table, Money::ONE, 50, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(table, MatnFeeTable::Classification::ETF, report);
      });
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto table = make_fee_table();
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::CENT;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "AP";
      auto calculated_fee =
        calculate_fee(table, MatnFeeTable::Classification::DEFAULT, report);
      auto max_charge =
        lookup_fee(table, MatnFeeTable::GeneralIndex::MAX_CHARGE,
          MatnFeeTable::PriceClass::SUBDOLLAR);
      auto expected_fee = std::min(max_charge, report.m_last_quantity *
        lookup_fee(table, MatnFeeTable::GeneralIndex::FEE,
          MatnFeeTable::PriceClass::SUBDOLLAR));
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "PA";
      auto calculated_fee =
        calculate_fee(table, MatnFeeTable::Classification::DEFAULT, report);
      auto max_charge =
        lookup_fee(table, MatnFeeTable::GeneralIndex::MAX_CHARGE,
          MatnFeeTable::PriceClass::SUBFIVE_DOLLAR);
      auto expected_fee = std::min(max_charge, report.m_last_quantity *
        lookup_fee(table, MatnFeeTable::GeneralIndex::FEE,
          MatnFeeTable::PriceClass::SUBFIVE_DOLLAR));
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "PA";
      auto calculated_fee =
        calculate_fee(table, MatnFeeTable::Classification::ETF, report);
      auto expected_fee = report.m_last_quantity *
        lookup_fee(table, LiquidityFlag::ACTIVE, MatnFeeTable::Category::ETF);
      REQUIRE(calculated_fee == expected_fee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto table = make_fee_table();
    auto expected_fee = lookup_fee(table, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, 5 * Money::ONE, 100, LiquidityFlag::NONE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(
          table, MatnFeeTable::Classification::DEFAULT, report);
      });
  }
}
