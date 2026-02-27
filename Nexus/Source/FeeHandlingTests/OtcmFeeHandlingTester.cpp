#include <doctest/doctest.h>
#include "Nexus/FeeHandling/OtcmFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  const auto TIMESTAMP = time_from_string("2025-02-11 12:00:00");

  auto make_fee_table() {
    auto table = OtcmFeeTable();
    populate_fee_table(out(table.m_fee_table));
    populate_fee_table(out(table.m_subdollar_fee_table));
    for(auto i = std::size_t(0); i < OtcmFeeTable::TYPE_COUNT; ++i) {
      table.m_subpenny_rate_table[i] =
        rational<int>(std::rand() % 100 + 1, 1000);
    }
    populate_fee_table(out(table.m_closing_cross_fee_table));
    for(auto i = std::size_t(0); i < LIQUIDITY_FLAG_COUNT; ++i) {
      table.m_closing_cross_subpenny_rate_table[i] =
        rational<int>(std::rand() % 100 + 1, 1000);
    }
    return table;
  }

  auto make_order_fields(Money price, Quantity quantity) {
    return make_limit_order_fields(DirectoryEntry::ROOT_ACCOUNT,
      Security("TST", OTCM), USD, Side::BID, DefaultDestinations::OTCM,
      quantity, price);
  }

  auto make_order_fields(Money price) {
    return make_order_fields(price, 100);
  }
}

TEST_SUITE("OtcmFeeHandling") {
  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    fields.m_quantity = 0;
    test_per_share_fee_calculation(
      table, fields, "A", Money::ZERO, calculate_fee);
  }

  TEST_CASE("default_passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OtcmFeeTable::Type::PASSIVE);
    test_per_share_fee_calculation(
      table, fields, "A", expected_fee, calculate_fee);
  }

  TEST_CASE("default_hidden_passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OtcmFeeTable::Type::HIDDEN_PASSIVE);
    test_per_share_fee_calculation(
      table, fields, "B", expected_fee, calculate_fee);
  }

  TEST_CASE("default_active") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OtcmFeeTable::Type::ACTIVE);
    test_per_share_fee_calculation(
      table, fields, "R", expected_fee, calculate_fee);
  }

  TEST_CASE("default_routed_to_otc_link") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee =
      lookup_fee(table, OtcmFeeTable::Type::ROUTED_TO_OTC_LINK);
    test_per_share_fee_calculation(
      table, fields, "L", expected_fee, calculate_fee);
  }

  TEST_CASE("default_routed_to_external_ecn") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee =
      lookup_fee(table, OtcmFeeTable::Type::ROUTED_TO_EXTERNAL_ECN);
    test_per_share_fee_calculation(
      table, fields, "X", expected_fee, calculate_fee);
  }

  TEST_CASE("default_routed_to_external_liquidity") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee =
      lookup_fee(table, OtcmFeeTable::Type::ROUTED_TO_EXTERNAL_LIQUIDITY);
    test_per_share_fee_calculation(
      table, fields, "P", expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(50 * Money::CENT);
    auto expected_fee =
      lookup_subdollar_fee(table, OtcmFeeTable::Type::PASSIVE);
    test_per_share_fee_calculation(
      table, fields, "D", expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_hidden_passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(50 * Money::CENT);
    auto expected_fee =
      lookup_subdollar_fee(table, OtcmFeeTable::Type::HIDDEN_PASSIVE);
    test_per_share_fee_calculation(
      table, fields, "E", expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_active") {
    auto table = make_fee_table();
    auto fields = make_order_fields(50 * Money::CENT);
    auto expected_fee =
      lookup_subdollar_fee(table, OtcmFeeTable::Type::ACTIVE);
    test_per_share_fee_calculation(
      table, fields, "H", expected_fee, calculate_fee);
  }

  TEST_CASE("subpenny_passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::CENT / 100);
    auto rate = lookup_subpenny_rate(table, OtcmFeeTable::Type::PASSIVE);
    auto report = ExecutionReport(0, TIMESTAMP);
    report.m_last_price = fields.m_price;
    report.m_last_quantity = fields.m_quantity;
    report.m_liquidity_flag = "D";
    auto calculated_fee = calculate_fee(table, fields, report);
    auto expected_fee = rate * (report.m_last_quantity * report.m_last_price);
    REQUIRE(calculated_fee == expected_fee);
  }

  TEST_CASE("subpenny_hidden_passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::CENT / 100);
    auto rate = lookup_subpenny_rate(table, OtcmFeeTable::Type::HIDDEN_PASSIVE);
    auto report = ExecutionReport(0, TIMESTAMP);
    report.m_last_price = fields.m_price;
    report.m_last_quantity = fields.m_quantity;
    report.m_liquidity_flag = "E";
    auto calculated_fee = calculate_fee(table, fields, report);
    auto expected_fee = rate * (report.m_last_quantity * report.m_last_price);
    REQUIRE(calculated_fee == expected_fee);
  }

  TEST_CASE("subpenny_active") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::CENT / 100);
    auto rate = lookup_subpenny_rate(table, OtcmFeeTable::Type::ACTIVE);
    auto report = ExecutionReport(0, TIMESTAMP);
    report.m_last_price = fields.m_price;
    report.m_last_quantity = fields.m_quantity;
    report.m_liquidity_flag = "H";
    auto calculated_fee = calculate_fee(table, fields, report);
    auto expected_fee = rate * (report.m_last_quantity * report.m_last_price);
    REQUIRE(calculated_fee == expected_fee);
  }

  TEST_CASE("subpenny_routed_to_otc_link") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::CENT / 100);
    auto rate =
      lookup_subpenny_rate(table, OtcmFeeTable::Type::ROUTED_TO_OTC_LINK);
    auto report = ExecutionReport(0, TIMESTAMP);
    report.m_last_price = fields.m_price;
    report.m_last_quantity = fields.m_quantity;
    report.m_liquidity_flag = "L";
    auto calculated_fee = calculate_fee(table, fields, report);
    auto expected_fee = rate * (report.m_last_quantity * report.m_last_price);
    REQUIRE(calculated_fee == expected_fee);
  }

  TEST_CASE("closing_cross_default_passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_closing_cross_fee(
      table, LiquidityFlag::PASSIVE, OtcmFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, "a", expected_fee, calculate_fee);
  }

  TEST_CASE("closing_cross_default_active") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_closing_cross_fee(
      table, LiquidityFlag::ACTIVE, OtcmFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, "r", expected_fee, calculate_fee);
  }

  TEST_CASE("closing_cross_subdollar_passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(50 * Money::CENT);
    auto expected_fee = lookup_closing_cross_fee(
      table, LiquidityFlag::PASSIVE, OtcmFeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, "d", expected_fee, calculate_fee);
  }

  TEST_CASE("closing_cross_subdollar_active") {
    auto table = make_fee_table();
    auto fields = make_order_fields(50 * Money::CENT);
    auto expected_fee = lookup_closing_cross_fee(
      table, LiquidityFlag::ACTIVE, OtcmFeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, "h", expected_fee, calculate_fee);
  }

  TEST_CASE("closing_cross_subpenny_passive") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::CENT / 100);
    auto rate =
      lookup_closing_cross_subpenny_rate(table, LiquidityFlag::PASSIVE);
    auto report = ExecutionReport(0, TIMESTAMP);
    report.m_last_price = fields.m_price;
    report.m_last_quantity = fields.m_quantity;
    report.m_liquidity_flag = "d";
    auto calculated_fee = calculate_fee(table, fields, report);
    auto expected_fee = rate * (report.m_last_quantity * report.m_last_price);
    REQUIRE(calculated_fee == expected_fee);
  }

  TEST_CASE("closing_cross_subpenny_active") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::CENT / 100);
    auto rate =
      lookup_closing_cross_subpenny_rate(table, LiquidityFlag::ACTIVE);
    auto report = ExecutionReport(0, TIMESTAMP);
    report.m_last_price = fields.m_price;
    report.m_last_quantity = fields.m_quantity;
    report.m_liquidity_flag = "h";
    auto calculated_fee = calculate_fee(table, fields, report);
    auto expected_fee = rate * (report.m_last_quantity * report.m_last_price);
    REQUIRE(calculated_fee == expected_fee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    {
      auto report = ExecutionReport(0, TIMESTAMP);
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "?";
      auto calculated_fee = calculate_fee(table, fields, report);
      auto expected_fee =
        report.m_last_quantity * lookup_fee(table, OtcmFeeTable::Type::ACTIVE);
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, TIMESTAMP);
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "AB";
      auto calculated_fee = calculate_fee(table, fields, report);
      auto expected_fee =
        report.m_last_quantity * lookup_fee(table, OtcmFeeTable::Type::ACTIVE);
      REQUIRE(calculated_fee == expected_fee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OtcmFeeTable::Type::ACTIVE);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::NONE, expected_fee, calculate_fee);
  }
}
