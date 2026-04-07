#include <doctest/doctest.h>
#include "Nexus/FeeHandling/Xcx2FeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  const auto TST = Security("TST", TSX);
  const auto TST2 = Security("TST2", TSXV);
  const auto ETF = Security("TST3", TSX);

  auto make_fee_table() {
    auto table = Xcx2FeeTable();
    populate_fee_table(out(table.m_default_table));
    populate_fee_table(out(table.m_tsx_table));
    table.m_large_trade_size = 1000;
    table.m_etfs.insert(ETF);
    return table;
  }

  auto make_order_fields(Security security, Money price) {
    auto fields = make_limit_order_fields(DirectoryEntry::ROOT_ACCOUNT,
      std::move(security), CAD, Side::BID, DefaultDestinations::CX2, 100,
      price);
    return fields;
  }

  auto make_order_fields(Money price) {
    return make_order_fields(TST2, price);
  }

  auto make_tsx_order_fields(Money price) {
    return make_order_fields(TST, price);
  }

  auto make_etf_order_fields(Money price) {
    return make_order_fields(ETF, price);
  }
}

TEST_SUITE("Xcx2FeeHandling") {
  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    fields.m_quantity = 0;
    auto expected_fee = Money::ZERO;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::NONE, expected_fee, calculate_fee);
  }

  TEST_CASE("active_default") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::ONE);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ACTIVE,
      Xcx2FeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_default") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::ONE);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::PASSIVE,
      Xcx2FeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("large_active_default") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::ONE);
    fields.m_quantity = table.m_large_trade_size;
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::LARGE_ACTIVE, Xcx2FeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("large_passive_default") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::ONE);
    fields.m_quantity = table.m_large_trade_size;
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::LARGE_PASSIVE, Xcx2FeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("hidden_active_default") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::ONE);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::HIDDEN_ACTIVE, Xcx2FeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, "r", expected_fee, calculate_fee);
  }

  TEST_CASE("hidden_passive_default") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::ONE);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::HIDDEN_PASSIVE, Xcx2FeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, "a", expected_fee, calculate_fee);
  }

  TEST_CASE("active_odd_lot_default") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::ONE);
    fields.m_quantity = 50;
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_odd_lot_default") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::ONE);
    fields.m_quantity = 50;
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("active_etf") {
    auto table = make_fee_table();
    auto fields = make_etf_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(
      table, fields, Xcx2FeeTable::Type::ACTIVE, Xcx2FeeTable::PriceClass::ETF);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_etf") {
    auto table = make_fee_table();
    auto fields = make_etf_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::PASSIVE,
      Xcx2FeeTable::PriceClass::ETF);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("active_sub_five_dollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ACTIVE,
      Xcx2FeeTable::PriceClass::SUB_FIVE_DOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_sub_five_dollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::PASSIVE,
      Xcx2FeeTable::PriceClass::SUB_FIVE_DOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("active_subdollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ACTIVE,
      Xcx2FeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_subdollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::PASSIVE,
      Xcx2FeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("active_subdime") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ACTIVE,
      Xcx2FeeTable::PriceClass::SUBDIME);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_subdime") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::PASSIVE,
      Xcx2FeeTable::PriceClass::SUBDIME);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("large_active_etf") {
    auto table = make_fee_table();
    auto fields = make_etf_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::LARGE_ACTIVE, Xcx2FeeTable::PriceClass::ETF);
    fields.m_quantity = table.m_large_trade_size;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("large_passive_etf") {
    auto table = make_fee_table();
    auto fields = make_etf_order_fields(5 * Money::ONE);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::LARGE_PASSIVE, Xcx2FeeTable::PriceClass::ETF);
    fields.m_quantity = table.m_large_trade_size;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("large_active_sub_five_dollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::LARGE_ACTIVE,
      Xcx2FeeTable::PriceClass::SUB_FIVE_DOLLAR);
    fields.m_quantity = table.m_large_trade_size;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("large_passive_sub_five_dollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::LARGE_PASSIVE,
      Xcx2FeeTable::PriceClass::SUB_FIVE_DOLLAR);
    fields.m_quantity = table.m_large_trade_size;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("large_active_subdollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::LARGE_ACTIVE, Xcx2FeeTable::PriceClass::SUBDOLLAR);
    fields.m_quantity = table.m_large_trade_size;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("large_passive_subdollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::LARGE_PASSIVE, Xcx2FeeTable::PriceClass::SUBDOLLAR);
    fields.m_quantity = table.m_large_trade_size;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("large_active_subdime") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::CENT);
    fields.m_quantity = table.m_large_trade_size;
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::LARGE_ACTIVE, Xcx2FeeTable::PriceClass::SUBDIME);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("large_passive_subdime") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::CENT);
    fields.m_quantity = table.m_large_trade_size;
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::LARGE_PASSIVE, Xcx2FeeTable::PriceClass::SUBDIME);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("hidden_active_etf") {
    auto table = make_fee_table();
    auto fields = make_etf_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::HIDDEN_ACTIVE, Xcx2FeeTable::PriceClass::ETF);
    test_per_share_fee_calculation(
      table, fields, "r", expected_fee, calculate_fee);
  }

  TEST_CASE("hidden_passive_etf") {
    auto table = make_fee_table();
    auto fields = make_etf_order_fields(5 * Money::ONE);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::HIDDEN_PASSIVE, Xcx2FeeTable::PriceClass::ETF);
    test_per_share_fee_calculation(
      table, fields, "a", expected_fee, calculate_fee);
  }

  TEST_CASE("hidden_active_sub_five_dollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(
      table, fields, Xcx2FeeTable::Type::HIDDEN_ACTIVE,
      Xcx2FeeTable::PriceClass::SUB_FIVE_DOLLAR);
    test_per_share_fee_calculation(
      table, fields, "r", expected_fee, calculate_fee);
  }

  TEST_CASE("hidden_passive_sub_five_dollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(
      table, fields, Xcx2FeeTable::Type::HIDDEN_PASSIVE,
      Xcx2FeeTable::PriceClass::SUB_FIVE_DOLLAR);
    test_per_share_fee_calculation(
      table, fields, "a", expected_fee, calculate_fee);
  }

  TEST_CASE("hidden_active_subdollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::HIDDEN_ACTIVE, Xcx2FeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, "r", expected_fee, calculate_fee);
  }

  TEST_CASE("hidden_passive_subdollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::HIDDEN_PASSIVE, Xcx2FeeTable::PriceClass::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, "a", expected_fee, calculate_fee);
  }

  TEST_CASE("hidden_active_subdime") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::HIDDEN_ACTIVE, Xcx2FeeTable::PriceClass::SUBDIME);
    test_per_share_fee_calculation(
      table, fields, "r", expected_fee, calculate_fee);
  }

  TEST_CASE("hidden_passive_subdime") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, fields,
      Xcx2FeeTable::Type::HIDDEN_PASSIVE, Xcx2FeeTable::PriceClass::SUBDIME);
    test_per_share_fee_calculation(
      table, fields, "a", expected_fee, calculate_fee);
  }

  TEST_CASE("active_odd_lot_etf") {
    auto table = make_fee_table();
    auto fields = make_etf_order_fields(5 * Money::ONE);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::ETF);
    fields.m_quantity = 50;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_odd_lot_etf") {
    auto table = make_fee_table();
    auto fields = make_etf_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::ETF);
    fields.m_quantity = 50;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("active_odd_lot_sub_five_dollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::SUB_FIVE_DOLLAR);
    fields.m_quantity = 50;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_odd_lot_sub_five_dollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::SUB_FIVE_DOLLAR);
    fields.m_quantity = 50;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("active_odd_lot_subdollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::SUBDOLLAR);
    fields.m_quantity = 50;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_odd_lot_subdollar") {
    auto table = make_fee_table();
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::SUBDOLLAR);
    fields.m_quantity = 50;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("active_odd_lot_subdime") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::SUBDIME);
    fields.m_quantity = 50;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("passive_odd_lot_subdime") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::SUBDIME);
    fields.m_quantity = 50;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("hidden_odd_lot") {
    auto table = make_fee_table();
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::SUBDOLLAR);
    fields.m_quantity = 50;
    test_per_share_fee_calculation(
      table, fields, "r", expected_fee, calculate_fee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto table = make_fee_table();
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = 20 * Money::CENT;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "AP";
      auto fields = make_order_fields(20 * Money::CENT);
      auto calculated_fee = calculate_fee(table, fields, report);
      auto expected_fee = report.m_last_quantity *
        lookup_fee(table, fields, Xcx2FeeTable::Type::PASSIVE,
          Xcx2FeeTable::PriceClass::SUBDOLLAR);
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = 5 * Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "PA";
      auto fields = make_order_fields(5 * Money::ONE);
      auto calculated_fee = calculate_fee(table, fields, report);
      auto expected_fee = report.m_last_quantity * lookup_fee(table,
        fields, Xcx2FeeTable::Type::PASSIVE, Xcx2FeeTable::PriceClass::DEFAULT);
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::CENT;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "?";
      auto fields = make_order_fields(Money::CENT);
      auto calculated_fee = calculate_fee(table, fields, report);
      auto expected_fee = report.m_last_quantity * lookup_fee(table,
        fields, Xcx2FeeTable::Type::PASSIVE, Xcx2FeeTable::PriceClass::SUBDIME);
      REQUIRE(calculated_fee == expected_fee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto table = make_fee_table();
    auto fields = make_order_fields(5 * Money::ONE);
    auto expected_fee = lookup_fee(table, fields, Xcx2FeeTable::Type::PASSIVE,
      Xcx2FeeTable::PriceClass::DEFAULT);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::NONE, expected_fee, calculate_fee);
  }
}
