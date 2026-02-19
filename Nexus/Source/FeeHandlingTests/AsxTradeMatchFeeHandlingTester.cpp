#include <doctest/doctest.h>
#include "Nexus/FeeHandling/AsxTradeMatchFeeTable.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

namespace {
  auto make_order_fields(Money price) {
    return make_limit_order_fields(DirectoryEntry::ROOT_ACCOUNT,
      parse_ticker("TST.ASX"), AUD, Side::BID, DefaultDestinations::ASXT, 100,
      price);
  }

  auto make_pegged_order_fields(Money price) {
    auto fields = make_order_fields(price);
    fields.m_type = OrderType::PEGGED;
    return fields;
  }

  auto make_fee_table() {
    auto table = AsxTradeMatchFeeTable();
    table.m_gst_rate = 1;
    table.m_trade_rate = 1;
    table.m_clearing_rate_table[0][0] = rational<int>{1, 1000};
    table.m_clearing_rate_table[0][1] = rational<int>{1, 100};
    table.m_clearing_rate_table[0][2] = rational<int>{1, 10};
    table.m_clearing_rate_table[1][0] = rational<int>{5, 1000};
    table.m_clearing_rate_table[1][1] = rational<int>{5, 100};
    table.m_clearing_rate_table[1][2] = rational<int>{5, 10};
    return table;
  }
}

TEST_SUITE("AsxtFeeHandling") {
  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    fields.m_quantity = 0;
    test_notional_value_fee_calculation(table, fields, 0,
      [&] (const auto& table, const auto& report) {
        return calculate_clearing_fee(table, fields, report);
      });
    test_notional_value_fee_calculation(
      table, fields, 0, calculate_execution_fee);
  }

  TEST_CASE("regular_tier_one") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::CENT);
    fields.m_quantity = 100;
    test_notional_value_fee_calculation(table, fields,
      lookup_clearing_fee(table, AsxTradeMatchFeeTable::PriceClass::TIER_ONE,
        AsxTradeMatchFeeTable::OrderTypeClass::REGULAR),
      [&] (const auto& table, const auto& report) {
        return calculate_clearing_fee(table, fields, report);
      });
    test_fee_calculation(table, fields, "A",
      (1 + table.m_gst_rate) * table.m_trade_rate * fields.m_quantity *
        fields.m_price, calculate_execution_fee);
  }

  TEST_CASE("regular_tier_two") {
    auto table = make_fee_table();
    auto fields = make_order_fields(50 * Money::CENT);
    fields.m_quantity = 500;
    test_notional_value_fee_calculation(table, fields,
      lookup_clearing_fee(table, AsxTradeMatchFeeTable::PriceClass::TIER_TWO,
        AsxTradeMatchFeeTable::OrderTypeClass::REGULAR),
      [&] (const auto& table, const auto& report) {
        return calculate_clearing_fee(table, fields, report);
      });
    test_fee_calculation(table, fields, "R",
      (1 + table.m_gst_rate) * table.m_trade_rate * fields.m_quantity *
        fields.m_price, calculate_execution_fee);
  }

  TEST_CASE("regular_tier_three") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    fields.m_quantity = 12000;
    test_notional_value_fee_calculation(table, fields,
      lookup_clearing_fee(table, AsxTradeMatchFeeTable::PriceClass::TIER_THREE,
        AsxTradeMatchFeeTable::OrderTypeClass::REGULAR),
      [&] (const auto& table, const auto& report) {
        return calculate_clearing_fee(table, fields, report);
      });
    test_fee_calculation(table, fields, "R",
      (1 + table.m_gst_rate) * table.m_trade_rate * fields.m_quantity *
        fields.m_price, calculate_execution_fee);
  }

  TEST_CASE("pegged_tier_one") {
    auto table = make_fee_table();
    auto fields = make_pegged_order_fields(5 * Money::CENT);
    fields.m_quantity = 6000;
    test_notional_value_fee_calculation(table, fields,
      lookup_clearing_fee(table, AsxTradeMatchFeeTable::PriceClass::TIER_ONE,
        AsxTradeMatchFeeTable::OrderTypeClass::PEGGED),
      [&] (const auto& table, const auto& report) {
        return calculate_clearing_fee(table, fields, report);
      });
    test_fee_calculation(table, fields, "R",
      (1 + table.m_gst_rate) * table.m_trade_rate * fields.m_quantity *
        fields.m_price, calculate_execution_fee);
  }

  TEST_CASE("pegged_tier_two") {
    auto table = make_fee_table();
    auto fields = make_pegged_order_fields(50 * Money::CENT);
    fields.m_quantity = 300;
    test_notional_value_fee_calculation(table, fields,
      lookup_clearing_fee(table, AsxTradeMatchFeeTable::PriceClass::TIER_TWO,
        AsxTradeMatchFeeTable::OrderTypeClass::PEGGED),
      [&] (const auto& table, const auto& report) {
        return calculate_clearing_fee(table, fields, report);
      });
    test_fee_calculation(table, fields, "A",
      (1 + table.m_gst_rate) * table.m_trade_rate * fields.m_quantity *
        fields.m_price, calculate_execution_fee);
  }

  TEST_CASE("pegged_tier_three") {
    auto table = make_fee_table();
    auto fields = make_pegged_order_fields(Money::ONE);
    fields.m_quantity = 4400;
    test_notional_value_fee_calculation(table, fields,
      lookup_clearing_fee(table, AsxTradeMatchFeeTable::PriceClass::TIER_THREE,
        AsxTradeMatchFeeTable::OrderTypeClass::PEGGED),
      [&] (const auto& table, const auto& report) {
        return calculate_clearing_fee(table, fields, report);
      });
    test_fee_calculation(table, fields, "R",
      (1 + table.m_gst_rate) * table.m_trade_rate * fields.m_quantity *
        fields.m_price, calculate_execution_fee);
  }
}
