#include <doctest/doctest.h>
#include "Nexus/FeeHandling/NeoeFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

namespace {
  auto TST = parse_ticker("TST.TSX");

  auto make_order_fields(Money price) {
    return make_limit_order_fields(DirectoryEntry::ROOT_ACCOUNT, TST, CAD,
      Side::BID, DefaultDestinations::NEOE, 100, price);
  }

  auto make_fee_table() {
    auto table = NeoeFeeTable();
    populate_fee_table(out(table.m_general_fee_table));
    populate_fee_table(out(table.m_interlisted_fee_table));
    populate_fee_table(out(table.m_etf_table_fee));
    return table;
  }

  auto get_fee_lookup(NeoeFeeTable::Classification classification) {
    if(classification == NeoeFeeTable::Classification::GENERAL) {
      return &lookup_general_fee;
    } else if(classification == NeoeFeeTable::Classification::INTERLISTED) {
      return &lookup_interlisted_fee;
    }
    return &lookup_etf_fee;
  }
}

TEST_SUITE("NeoeFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto table = make_fee_table();
    test_fee_table_index(table, table.m_general_fee_table, lookup_general_fee,
      LIQUIDITY_FLAG_COUNT, NeoeFeeTable::PRICE_CLASS_COUNT);
    test_fee_table_index(table, table.m_interlisted_fee_table,
      lookup_interlisted_fee, LIQUIDITY_FLAG_COUNT,
      NeoeFeeTable::PRICE_CLASS_COUNT);
    test_fee_table_index(table, table.m_etf_table_fee, lookup_etf_fee,
      LIQUIDITY_FLAG_COUNT, NeoeFeeTable::PRICE_CLASS_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    test_per_share_fee_calculation(table, fields.m_price, 0,
      LiquidityFlag::NONE, Money::ZERO,
      [&] (const auto& table, const auto& report) {
        return calculate_fee(
          table, NeoeFeeTable::Classification::GENERAL, fields, report);
      });
    test_per_share_fee_calculation(
      table, fields.m_price, 0, LiquidityFlag::NONE, Money::ZERO,
      [&] (const auto& table, const auto& report) {
        return calculate_fee(
          table, NeoeFeeTable::Classification::INTERLISTED, fields, report);
      });
    test_per_share_fee_calculation(
      table, fields.m_price, 0, LiquidityFlag::NONE, Money::ZERO,
      [&] (const auto& table, const auto& report) {
        return calculate_fee(
          table, NeoeFeeTable::Classification::ETF, fields, report);
      });
  }

  TEST_CASE("active") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    for(auto classification : {NeoeFeeTable::Classification::GENERAL,
        NeoeFeeTable::Classification::INTERLISTED,
        NeoeFeeTable::Classification::ETF}) {
      auto expected_fee = get_fee_lookup(classification)(
        table, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
      test_per_share_fee_calculation(
        table, Money::ONE, 100, LiquidityFlag::ACTIVE, expected_fee,
        [&] (const auto& table, const auto& report) {
          return calculate_fee(table, classification, fields, report);
        });
    }
  }

  TEST_CASE("passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    for(auto classification : {NeoeFeeTable::Classification::GENERAL,
        NeoeFeeTable::Classification::INTERLISTED,
        NeoeFeeTable::Classification::ETF}) {
      auto expected_fee = get_fee_lookup(classification)(
        table, LiquidityFlag::PASSIVE, NeoeFeeTable::PriceClass::DEFAULT);
      test_per_share_fee_calculation(
        table, Money::ONE, 100, LiquidityFlag::PASSIVE, expected_fee,
        [&] (const auto& table, const auto& report) {
          return calculate_fee(table, classification, fields, report);
        });
    }
  }

  TEST_CASE("subdollar_active") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::CENT);
    for(auto classification : {NeoeFeeTable::Classification::GENERAL,
        NeoeFeeTable::Classification::INTERLISTED,
        NeoeFeeTable::Classification::ETF}) {
      auto expected_fee = get_fee_lookup(classification)(
        table, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::SUBDOLLAR);
      test_per_share_fee_calculation(
        table, Money::CENT, 100, LiquidityFlag::ACTIVE, expected_fee,
        [&] (const auto& table, const auto& report) {
          return calculate_fee(table, classification, fields, report);
        });
    }
  }

  TEST_CASE("subdollar_passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::CENT);
    for(auto classification : {NeoeFeeTable::Classification::GENERAL,
        NeoeFeeTable::Classification::INTERLISTED,
        NeoeFeeTable::Classification::ETF}) {
      auto expected_fee = get_fee_lookup(classification)(
        table, LiquidityFlag::PASSIVE, NeoeFeeTable::PriceClass::SUBDOLLAR);
      test_per_share_fee_calculation(
        table, Money::CENT, 100, LiquidityFlag::PASSIVE, expected_fee,
        [&] (const auto& table, const auto& report) {
          return calculate_fee(table, classification, fields, report);
        });
    }
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto table = make_fee_table();
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "AP";
      auto fields = make_order_fields(Money::ONE);
      for(auto classification : {NeoeFeeTable::Classification::GENERAL,
          NeoeFeeTable::Classification::INTERLISTED,
          NeoeFeeTable::Classification::ETF}) {
        auto calculated_fee =
          calculate_fee(table, classification, fields, report);
        auto expected_fee = report.m_last_quantity *
          get_fee_lookup(classification)(
            table, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
        REQUIRE(calculated_fee == expected_fee);
      }
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::CENT;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "PA";
      auto fields = make_order_fields(Money::CENT);
      for(auto classification : {NeoeFeeTable::Classification::GENERAL,
          NeoeFeeTable::Classification::INTERLISTED,
          NeoeFeeTable::Classification::ETF}) {
        auto calculated_fee =
          calculate_fee(table, classification, fields, report);
        auto expected_fee = report.m_last_quantity *
          get_fee_lookup(classification)(
            table, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::SUBDOLLAR);
        REQUIRE(calculated_fee == expected_fee);
      }
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "?????";
      auto fields = make_order_fields(Money::ONE);
      for(auto classification : {NeoeFeeTable::Classification::GENERAL,
          NeoeFeeTable::Classification::INTERLISTED,
          NeoeFeeTable::Classification::ETF}) {
        auto calculated_fee =
          calculate_fee(table, classification, fields, report);
        auto expected_fee = report.m_last_quantity *
          get_fee_lookup(classification)(
            table, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
        REQUIRE(calculated_fee == expected_fee);
      }
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto table = make_fee_table();
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "";
      auto fields = make_order_fields(Money::ONE);
      for(auto classification : {NeoeFeeTable::Classification::GENERAL,
          NeoeFeeTable::Classification::INTERLISTED,
          NeoeFeeTable::Classification::ETF}) {
        auto calculated_fee =
          calculate_fee(table, classification, fields, report);
        auto expected_fee = report.m_last_quantity *
          get_fee_lookup(classification)(
            table, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
        REQUIRE(calculated_fee == expected_fee);
      }
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::CENT;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "";
      auto fields = make_order_fields(Money::CENT);
      for(auto classification : {NeoeFeeTable::Classification::GENERAL,
          NeoeFeeTable::Classification::INTERLISTED,
          NeoeFeeTable::Classification::ETF}) {
        auto calculated_fee =
          calculate_fee(table, classification, fields, report);
        auto expected_fee = report.m_last_quantity *
          get_fee_lookup(classification)(
            table, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::SUBDOLLAR);
        REQUIRE(calculated_fee == expected_fee);
      }
    }
  }
}
