#include <doctest/doctest.h>
#include "Nexus/FeeHandling/OmgaFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  auto make_order_fields(Money price, Quantity quantity) {
    return make_limit_order_fields(DirectoryEntry::ROOT_ACCOUNT,
      Security("TST", TSX), CAD, Side::BID, DefaultDestinations::OMEGA,
      quantity, price);
  }

  auto make_order_fields(Money price) {
    return make_order_fields(price, 100);
  }

  auto make_hidden_order_fields(Money price, Quantity quantity) {
    auto fields = make_order_fields(price, quantity);
    fields.m_type = OrderType::PEGGED;
    fields.m_additional_fields.emplace_back(18, "M");
    return fields;
  }

  auto make_hidden_order_fields(Money price) {
    return make_hidden_order_fields(price, 100);
  }
}

TEST_SUITE("OmgaFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    test_fee_table_index(table, table.m_fee_table, lookup_fee,
      OmgaFeeTable::TYPE_COUNT, OmgaFeeTable::PRICE_CLASS_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::ONE);
    fields.m_quantity = 0;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::NONE, Money::ZERO,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("default_active") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ACTIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("default_passive") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::PASSIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("default_hidden_passive") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_hidden_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::HIDDEN_PASSIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("default_hidden_active") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_hidden_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::HIDDEN_ACTIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("etf_active") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ETF_ACTIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, true, fields, report);
      });
  }

  TEST_CASE("etf_passive") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ETF_PASSIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, true, fields, report);
      });
  }

  TEST_CASE("odd_lot_active") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::ONE, 50);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("odd_lot_passive") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::ONE, 50);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("subdollar_active") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ACTIVE,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("subdollar_passive") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::PASSIVE,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("subdollar_hidden_passive") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_hidden_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::HIDDEN_PASSIVE,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("subdollar_hidden_active") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_hidden_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::HIDDEN_ACTIVE,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("subdollar_etf_active") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ETF_ACTIVE,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, true, fields, report);
      });
  }

  TEST_CASE("subdollar_etf_passive") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ETF_PASSIVE,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, true, fields, report);
      });
  }

  TEST_CASE("subdollar_odd_lot_active") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::CENT, 50);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("subdollar_odd_lot_passive") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::CENT, 50);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }

  TEST_CASE("hidden_etf_active") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_hidden_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::HIDDEN_ACTIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, true, fields, report);
      });
  }

  TEST_CASE("hidden_etf_passive") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_hidden_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::HIDDEN_PASSIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, true, fields, report);
      });
  }

  TEST_CASE("odd_lot_etf_active") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::ONE, 50);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, true, fields, report);
      });
  }

  TEST_CASE("odd_lot_etf_passive") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::ONE, 50);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, true, fields, report);
      });
  }

  TEST_CASE("odd_lot_hidden_etf_active") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_hidden_order_fields(Money::ONE, 50);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, true, fields, report);
      });
  }

  TEST_CASE("odd_lot_hidden_etf_passive") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_hidden_order_fields(Money::ONE, 50);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, true, fields, report);
      });
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::ONE);
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "AP";
      auto calculated_fee = calculate_fee(table, false, fields, report);
      auto expected_fee = report.m_last_quantity * lookup_fee(
        table, OmgaFeeTable::Type::PASSIVE, OmgaFeeTable::PriceClass::DEFAULT);
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::CENT;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "PA";
      auto calculated_fee = calculate_fee(table, false, fields, report);
      auto expected_fee = report.m_last_quantity * lookup_fee(table,
        OmgaFeeTable::Type::PASSIVE, OmgaFeeTable::PriceClass::SUBDOLLAR);
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "?";
      auto calculated_fee = calculate_fee(table, true, fields, report);
      auto expected_fee = report.m_last_quantity * lookup_fee(table,
        OmgaFeeTable::Type::ETF_PASSIVE, OmgaFeeTable::PriceClass::DEFAULT);
      REQUIRE(calculated_fee == expected_fee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto table = OmgaFeeTable();
    populate_fee_table(out(table.m_fee_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, OmgaFeeTable::Type::PASSIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::NONE, expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, false, fields, report);
      });
  }
}
