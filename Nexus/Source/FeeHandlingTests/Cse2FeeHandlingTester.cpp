#include <doctest/doctest.h>
#include "Nexus/FeeHandling/Cse2FeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  auto make_fee_table() {
    auto table = Cse2FeeTable();
    populate_fee_table(out(table.m_regular_table));
    populate_fee_table(out(table.m_dark_table));
    populate_fee_table(out(table.m_debentures_or_notes_table));
    populate_fee_table(out(table.m_cse_listed_government_bonds_table));
    populate_fee_table(out(table.m_oddlot_table));
    table.m_large_trade_size = 200000;
    table.m_large_trade_fee = 2 * Money::ONE;
    return table;
  }

  auto make_order_fields(
      std::string symbol, Money price, Quantity quantity, Venue venue) {
    return make_limit_order_fields(DirectoryEntry::ROOT_ACCOUNT,
      Security(symbol, venue), CAD, Side::BID, DefaultDestinations::CSE2,
      quantity, price);
  }

  auto make_order_fields(std::string symbol, Money price, Venue venue) {
    return make_order_fields(symbol, price, 100, venue);
  }

  auto make_order_fields(Money price) {
    return make_order_fields("TST", price, 100, TSX);
  }

  auto make_execution_report(Money price, Quantity quantity, std::string flag) {
    auto report = ExecutionReport();
    report.m_id = 123;
    report.m_last_market = TSX.get_code().get_data();
    report.m_last_price = price;
    report.m_last_quantity = quantity;
    report.m_liquidity_flag = flag;
    report.m_sequence = 0;
    return report;
  }
}

TEST_SUITE("Cse2FeeHandling") {
  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto expected_fee = Money::ZERO;
    auto fee = calculate_fee(table, make_order_fields(Money::CENT),
      make_execution_report(Money::ZERO, 0, ""));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("active_subdollar_regular") {
    auto table = make_fee_table();
    auto expected_fee = 100 * lookup_regular_fee(
      table, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::SUBDOLLAR);
    auto fee = calculate_fee(table, make_order_fields(Money::CENT),
      make_execution_report(Money::CENT, 100, "TT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("active_default_regular") {
    auto table = make_fee_table();
    auto expected_fee = 200 * lookup_regular_fee(
      table, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = calculate_fee(table, make_order_fields(Money::ONE),
      make_execution_report(Money::ONE, 200, "TT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("passive_sub_dollar_regular") {
    auto table = make_fee_table();
    auto expected_fee = 300 * lookup_regular_fee(
      table, LiquidityFlag::PASSIVE, Cse2FeeTable::PriceClass::SUBDOLLAR);
    auto fee = calculate_fee(table, make_order_fields(Money::CENT),
      make_execution_report(Money::CENT, 300, "PT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("passive_default_regular") {
    auto table = make_fee_table();
    auto expected_fee = 400 * lookup_regular_fee(
      table, LiquidityFlag::PASSIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = calculate_fee(table, make_order_fields(Money::ONE),
      make_execution_report(Money::ONE, 400, "PT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("active_subdollar_dark") {
    auto table = make_fee_table();
    auto expected_fee = 500 * lookup_dark_fee(
      table, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::SUBDOLLAR);
    auto fee = calculate_fee(table, make_order_fields(Money::CENT),
      make_execution_report(Money::CENT, 500, "TTD"));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("active_default_dark") {
    auto table = make_fee_table();
    auto expected_fee = 500 * lookup_dark_fee(
      table, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = calculate_fee(table, make_order_fields(Money::ONE),
      make_execution_report(Money::ONE, 500, "TTD"));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("passive_subdollar_dark") {
    auto table = make_fee_table();
    auto expected_fee = 600 * lookup_dark_fee(
      table, LiquidityFlag::PASSIVE, Cse2FeeTable::PriceClass::SUBDOLLAR);
    auto fee = calculate_fee(table, make_order_fields(Money::CENT),
      make_execution_report(Money::CENT, 600, "PTD"));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("passive_default_dark") {
    auto table = make_fee_table();
    auto expected_fee = 600 * lookup_dark_fee(
      table, LiquidityFlag::PASSIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = calculate_fee(table, make_order_fields(Money::ONE),
      make_execution_report(Money::ONE, 600, "PTD"));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("active_cse_debenture") {
    auto table = make_fee_table();
    auto expected_fee = 100 * lookup_debentures_or_notes_fee(table,
      LiquidityFlag::ACTIVE, Cse2FeeTable::ListingMarket::CSE);
    auto fee = calculate_fee(table, make_order_fields("TST.DB", Money::CENT,
      CSE), make_execution_report(Money::CENT, 100, "TC "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("passive_cse_debenture") {
    auto table = make_fee_table();
    auto expected_fee = 100 * lookup_debentures_or_notes_fee(table,
      LiquidityFlag::PASSIVE, Cse2FeeTable::ListingMarket::CSE);
    auto fee = calculate_fee(table, make_order_fields("TST.NO", Money::CENT,
      CSE), make_execution_report(Money::CENT, 100, "PC "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("active_tsx_debenture") {
    auto table = make_fee_table();
    auto expected_fee = 300 * lookup_debentures_or_notes_fee(table,
      LiquidityFlag::ACTIVE, Cse2FeeTable::ListingMarket::TSX_TSXV);
    auto fee = calculate_fee(table, make_order_fields("TST.NT", Money::CENT,
      TSX), make_execution_report(Money::CENT, 300, "TT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("passive_tsx_debenture") {
    auto table = make_fee_table();
    auto expected_fee = 400 * lookup_debentures_or_notes_fee(table,
      LiquidityFlag::PASSIVE, Cse2FeeTable::ListingMarket::TSX_TSXV);
    auto fee = calculate_fee(table, make_order_fields("TST.NS", Money::ONE,
      TSXV), make_execution_report(Money::ONE, 400, "PV "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("active_subdollar_oddlot") {
    auto table = make_fee_table();
    auto expected_fee = 50 * lookup_oddlot_fee(
      table, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::SUBDOLLAR);
    auto fee = calculate_fee(table, make_order_fields(Money::CENT),
      make_execution_report(Money::CENT, 50, "TT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("passive_subdollar_oddlot") {
    auto table = make_fee_table();
    auto expected_fee = 20 * lookup_oddlot_fee(
      table, LiquidityFlag::PASSIVE, Cse2FeeTable::PriceClass::SUBDOLLAR);
    auto fee = calculate_fee(table, make_order_fields(Money::CENT),
      make_execution_report(Money::CENT, 20, "PT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("active_oddlot") {
    auto table = make_fee_table();
    auto expected_fee = 50 * lookup_oddlot_fee(
      table, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = calculate_fee(table, make_order_fields(Money::ONE),
      make_execution_report(Money::ONE, 50, "TT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("passive_oddlot") {
    auto table = make_fee_table();
    auto expected_fee = 20 * lookup_oddlot_fee(
      table, LiquidityFlag::PASSIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = calculate_fee(table, make_order_fields(Money::ONE),
      make_execution_report(Money::ONE, 20, "PT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("large_trade_regular") {
    auto table = make_fee_table();
    auto expected_fee = table.m_large_trade_fee;
    auto fee = calculate_fee(table, make_order_fields(Money::ONE),
      make_execution_report(Money::ONE, 200000, "TT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("large_trade_regular_above_threshold") {
    auto table = make_fee_table();
    auto expected_fee = table.m_large_trade_fee;
    auto fee = calculate_fee(table, make_order_fields(Money::ONE),
      make_execution_report(Money::ONE, 500000, "PT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("large_trade_below_threshold") {
    auto table = make_fee_table();
    auto expected_fee = 199999 * lookup_regular_fee(
      table, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = calculate_fee(table, make_order_fields(Money::ONE),
      make_execution_report(Money::ONE, 199999, "TT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("large_trade_dark_not_applied") {
    auto table = make_fee_table();
    auto expected_fee = 200000 * lookup_dark_fee(
      table, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = calculate_fee(table, make_order_fields(Money::ONE),
      make_execution_report(Money::ONE, 200000, "TTD"));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("large_trade_oddlot_not_applied") {
    auto table = make_fee_table();
    auto expected_fee = 50 * lookup_oddlot_fee(
      table, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = calculate_fee(table, make_order_fields(Money::ONE),
      make_execution_report(Money::ONE, 50, "TT "));
    REQUIRE(fee == expected_fee);
  }

  TEST_CASE("large_trade_debentures") {
    auto table = make_fee_table();
    auto expected_fee = table.m_large_trade_fee;
    auto fee = calculate_fee(table, make_order_fields("TST.DB", Money::ONE,
      CSE), make_execution_report(Money::ONE, 200000, "TC "));
    REQUIRE(fee == expected_fee);
  }
}
