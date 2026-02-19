#include <doctest/doctest.h>
#include "Nexus/FeeHandling/CxdFeeTable.hpp"

using namespace Nexus;

namespace {
  ExecutionReport make_execution_report(
      Money price, Quantity quantity, std::string liquidity_flag) {
    auto report = ExecutionReport(0, {});
    report.m_last_price = price;
    report.m_last_quantity = quantity;
    report.m_liquidity_flag = std::move(liquidity_flag);
    return report;
  }

  CxdFeeTable make_fee_table() {
    auto table = CxdFeeTable();
    for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
      for(auto c = 0; c < CxdFeeTable::BBO_PRICE_PAIR_COUNT; ++c) {
        table.m_fee_table[l][c] = Money(l * 100 + c);
        table.m_max_fee_table[l][c] = Money(1000 + l * 100 + c);
        table.m_etf_fee_table[l][c] = Money(10000 + l * 100 + c);
      }
    }
    return table;
  }
}

TEST_SUITE("CxdFeeTable") {
  TEST_CASE("get_cxd_bbo_type") {
    REQUIRE(get_cxd_bbo_type("a") == CxdFeeTable::BboType::INSIDE_BBO);
    REQUIRE(get_cxd_bbo_type("r") == CxdFeeTable::BboType::INSIDE_BBO);
    REQUIRE(get_cxd_bbo_type("d") == CxdFeeTable::BboType::AT_BBO);
    REQUIRE(get_cxd_bbo_type("D") == CxdFeeTable::BboType::AT_BBO);
    REQUIRE(get_cxd_bbo_type("x") == CxdFeeTable::BboType::NONE);
    REQUIRE(get_cxd_bbo_type("") == CxdFeeTable::BboType::NONE);
  }

  TEST_CASE("get_cxd_liquidity_flag") {
    REQUIRE(get_cxd_liquidity_flag("a") == LiquidityFlag::PASSIVE);
    REQUIRE(get_cxd_liquidity_flag("d") == LiquidityFlag::PASSIVE);
    REQUIRE(get_cxd_liquidity_flag("r") == LiquidityFlag::ACTIVE);
    REQUIRE(get_cxd_liquidity_flag("D") == LiquidityFlag::ACTIVE);
    REQUIRE(get_cxd_liquidity_flag("x") == LiquidityFlag::NONE);
    REQUIRE(get_cxd_liquidity_flag("") == LiquidityFlag::NONE);
  }

  TEST_CASE("get_cxd_price_class") {
    REQUIRE(
      get_cxd_price_class(Money(0)) == CxdFeeTable::PriceClass::SUBDOLLAR);
    REQUIRE(
      get_cxd_price_class(Money::CENT) == CxdFeeTable::PriceClass::SUBDOLLAR);
    REQUIRE(get_cxd_price_class(Money::ONE - Money::CENT) ==
      CxdFeeTable::PriceClass::SUBDOLLAR);
    REQUIRE(
      get_cxd_price_class(Money::ONE) == CxdFeeTable::PriceClass::SUBFIVE);
    REQUIRE(get_cxd_price_class(Money(4) + Money::CENT) ==
      CxdFeeTable::PriceClass::SUBFIVE);
    REQUIRE(get_cxd_price_class(Money(5)) == CxdFeeTable::PriceClass::DEFAULT);
    REQUIRE(
      get_cxd_price_class(Money(100)) == CxdFeeTable::PriceClass::DEFAULT);
  }

  TEST_CASE("lookup") {
    auto table = make_fee_table();
    REQUIRE(lookup_fee(table, LiquidityFlag::ACTIVE,
      CxdFeeTable::SecurityClass::DEFAULT, CxdFeeTable::PriceClass::SUBDOLLAR,
      CxdFeeTable::BboType::AT_BBO) ==
        table.m_fee_table[static_cast<std::size_t>(LiquidityFlag::ACTIVE)][0]);
    REQUIRE(lookup_fee(table, LiquidityFlag::PASSIVE,
      CxdFeeTable::SecurityClass::DEFAULT, CxdFeeTable::PriceClass::SUBFIVE,
      CxdFeeTable::BboType::INSIDE_BBO) ==
        table.m_fee_table[static_cast<std::size_t>(LiquidityFlag::PASSIVE)][
          CxdFeeTable::PRICE_CLASS_COUNT + 1]);
    REQUIRE(lookup_fee(table, LiquidityFlag::ACTIVE,
      CxdFeeTable::SecurityClass::ETF, CxdFeeTable::PriceClass::DEFAULT,
      CxdFeeTable::BboType::INSIDE_BBO) ==
        table.m_etf_fee_table[static_cast<std::size_t>(LiquidityFlag::ACTIVE)][
          CxdFeeTable::PRICE_CLASS_COUNT + 2]);
    REQUIRE(lookup_max_fee(table, LiquidityFlag::PASSIVE,
      CxdFeeTable::PriceClass::DEFAULT, CxdFeeTable::BboType::AT_BBO) ==
        table.m_max_fee_table[
          static_cast<std::size_t>(LiquidityFlag::PASSIVE)][2]);
  }

  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto report = make_execution_report(Money::ONE, 0, "a");
    auto fee =
      calculate_fee(table, CxdFeeTable::SecurityClass::DEFAULT, report);
    REQUIRE(fee == Money::ZERO);
  }

  TEST_CASE("default_ticker_fee_below_max") {
    auto table = make_fee_table();
    auto report = make_execution_report(Money::ONE, 1, "a");
    auto liquidity_flag = get_cxd_liquidity_flag("a");
    auto bbo_type = get_cxd_bbo_type("a");
    auto price_class = get_cxd_price_class(Money::ONE);
    auto per_share_fee = lookup_fee(table, liquidity_flag,
      CxdFeeTable::SecurityClass::DEFAULT, price_class, bbo_type);
    auto max_fee = lookup_max_fee(table, liquidity_flag, price_class, bbo_type);
    auto expected_fee = std::min(per_share_fee, max_fee);
    auto fee =
      calculate_fee(table, CxdFeeTable::SecurityClass::DEFAULT, report);
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("default_ticker_fee_above_max") {
    auto table = make_fee_table();
    auto report = make_execution_report(Money::ONE, 1000, "a");
    auto liquidity_flag = get_cxd_liquidity_flag("a");
    auto bbo_type = get_cxd_bbo_type("a");
    auto price_class = get_cxd_price_class(Money::ONE);
    auto per_share_fee = lookup_fee(table, liquidity_flag,
      CxdFeeTable::SecurityClass::DEFAULT, price_class, bbo_type);
    auto max_fee = lookup_max_fee(table, liquidity_flag, price_class, bbo_type);
    auto expected_fee = std::min(1000 * per_share_fee, max_fee);
    auto fee =
      calculate_fee(table, CxdFeeTable::SecurityClass::DEFAULT, report);
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("etf_ticker_no_max_fee") {
    auto table = make_fee_table();
    auto report = make_execution_report(Money(5), 10, "D");
    auto liquidity_flag = get_cxd_liquidity_flag("D");
    auto bbo_type = get_cxd_bbo_type("D");
    auto price_class = get_cxd_price_class(Money(5));
    auto per_share_fee = lookup_fee(table, liquidity_flag,
      CxdFeeTable::SecurityClass::ETF, price_class, bbo_type);
    auto expected_fee = 10 * per_share_fee;
    auto fee = calculate_fee(table, CxdFeeTable::SecurityClass::ETF, report);
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("unknown_liquidity_flag_defaults") {
    auto table = make_fee_table();
    auto report = make_execution_report(Money::ONE, 5, "x");
    auto liquidity_flag = LiquidityFlag::ACTIVE;
    auto bbo_type = CxdFeeTable::BboType::AT_BBO;
    auto price_class = get_cxd_price_class(Money::ONE);
    auto per_share_fee = lookup_fee(table, liquidity_flag,
      CxdFeeTable::SecurityClass::DEFAULT, price_class, bbo_type);
    auto max_fee = lookup_max_fee(table, liquidity_flag, price_class, bbo_type);
    auto expected_fee = std::min(5 * per_share_fee, max_fee);
    auto fee =
      calculate_fee(table, CxdFeeTable::SecurityClass::DEFAULT, report);
    REQUIRE(fee == expected_fee);
  }
}
