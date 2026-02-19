#include <doctest/doctest.h>
#include "Nexus/FeeHandling/LynxFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

namespace {
  const auto TST = parse_ticker("TST.TSX");
  const auto ETF = parse_ticker("ETF.TSX");
  const auto USA = parse_ticker("USA.TSX");

  auto lookup_liquidity_fee(const LynxFeeTable& table,
      LiquidityFlag flag, LynxFeeTable::Classification classification) {
    return lookup_fee(table, flag, classification);
  }

  auto make_fee_table() {
    auto table = LynxFeeTable();
    populate_fee_table(out(table.m_fee_table));
    table.m_etfs.insert(ETF);
    table.m_interlisted.insert(USA);
    return table;
  }

  auto make_order_fields(Ticker ticker, Money price, Quantity quantity) {
    return make_limit_order_fields(DirectoryEntry::ROOT_ACCOUNT,
      std::move(ticker), CAD, Side::BID, DefaultDestinations::LYNX, quantity,
      price);
  }

  auto make_order_fields(Money price, Quantity quantity) {
    return make_order_fields(TST, price, quantity);
  }

  auto make_order_fields(Money price) {
    return make_order_fields(price, 100);
  }

  auto make_etf_order_fields(Money price, Quantity quantity) {
    return make_order_fields(ETF, price, quantity);
  }

  auto make_etf_order_fields(Money price) {
    return make_order_fields(ETF, price, 100);
  }

  auto make_interlisted_order_fields(Money price, Quantity quantity) {
    return make_order_fields(USA, price, quantity);
  }

  auto make_interlisted_order_fields(Money price) {
    return make_order_fields(USA, price, 100);
  }

  auto make_midpoint_order_fields(Money price, Quantity quantity) {
    auto fields = make_order_fields(TST, price, quantity);
    fields.m_type = OrderType::PEGGED;
    fields.m_additional_fields.emplace_back(18, "M");
    return fields;
  }

  auto make_midpoint_order_fields(Money price) {
    return make_midpoint_order_fields(price, 100);
  }
}

TEST_SUITE("LynxFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto table = make_fee_table();
    test_fee_table_index(table, table.m_fee_table, lookup_liquidity_fee,
      LIQUIDITY_FLAG_COUNT, LynxFeeTable::CLASSIFICATION_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::NONE, 0, Money::ZERO, calculate_fee);
  }

  TEST_CASE("active") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("active_etf") {
    auto table = make_fee_table();
    auto fields = make_etf_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::ETF);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("active_interlisted") {
    auto table = make_fee_table();
    auto fields = make_interlisted_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, LiquidityFlag::ACTIVE,
      LynxFeeTable::Classification::INTERLISTED);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("active_midpoint") {
    auto table = make_fee_table();
    auto fields = make_midpoint_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, LiquidityFlag::ACTIVE,
      LynxFeeTable::Classification::MIDPOINT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::PASSIVE, LynxFeeTable::Classification::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_etf") {
    auto table = make_fee_table();
    auto fields = make_etf_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::PASSIVE, LynxFeeTable::Classification::ETF);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_interlisted") {
    auto table = make_fee_table();
    auto fields = make_interlisted_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::PASSIVE, LynxFeeTable::Classification::INTERLISTED);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_midpoint") {
    auto table = make_fee_table();
    auto fields = make_midpoint_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::PASSIVE, LynxFeeTable::Classification::MIDPOINT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_active") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_etf") {
    auto table = make_fee_table();
    auto fields = make_etf_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_interlisted") {
    auto table = make_fee_table();
    auto fields = make_interlisted_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_midpoint") {
    auto table = make_fee_table();
    auto fields = make_midpoint_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::MIDPOINT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::PASSIVE, LynxFeeTable::Classification::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_passive_etf") {
    auto table = make_fee_table();
    auto fields = make_etf_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::PASSIVE, LynxFeeTable::Classification::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_passive_interlisted") {
    auto table = make_fee_table();
    auto fields = make_interlisted_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::PASSIVE, LynxFeeTable::Classification::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_passive_midpoint") {
    auto table = make_fee_table();
    auto fields = make_midpoint_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(
      table, LiquidityFlag::PASSIVE, LynxFeeTable::Classification::MIDPOINT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "AP";
      auto calculated_fee = calculate_fee(table, fields, report);
      auto expected_fee = report.m_last_quantity * lookup_fee(
        table, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::DEFAULT);
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::CENT;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "PA";
      auto calculated_fee = calculate_fee(table, fields, report);
      auto expected_fee = report.m_last_quantity * lookup_fee(
        table, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::SUBDOLLAR);
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "?";
      auto expected_fee = lookup_fee(
        table, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::DEFAULT);
      test_per_share_fee_calculation(
        table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "";
      auto calculated_fee = calculate_fee(table, fields, report);
      auto expected_fee = report.m_last_quantity * lookup_fee(
        table, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::DEFAULT);
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "";
      auto expected_fee = lookup_fee(
        table, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::DEFAULT);
      test_per_share_fee_calculation(
        table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
    }
  }
}
