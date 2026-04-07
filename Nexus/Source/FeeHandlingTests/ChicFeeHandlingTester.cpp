#include <doctest/doctest.h>
#include "Nexus/FeeHandling/ChicFeeTable.hpp"
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

  auto make_order_fields(Money price) {
    return make_limit_order_fields(DirectoryEntry::ROOT_ACCOUNT, TST, CAD,
      Side::BID, DefaultDestinations::CHIX, 100, price);
  }
}

TEST_SUITE("ChicFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    test_fee_table_index(table, table.m_security_table, lookup_fee,
      ChicFeeTable::INDEX_COUNT, ChicFeeTable::CLASSIFICATION_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    fields.m_quantity = 0;
    auto expected_fee = Money::ZERO;
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::NONE, expected_fee, calculate_fee);
  }

  TEST_CASE("default_active") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::NON_INTERLISTED);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("default_passive") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::PASSIVE,
      ChicFeeTable::Classification::NON_INTERLISTED);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("default_hidden_passive") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::HIDDEN_PASSIVE,
      ChicFeeTable::Classification::NON_INTERLISTED);
    test_per_share_fee_calculation(
      table, fields, "a", expected_fee, calculate_fee);
  }

  TEST_CASE("default_hidden_active") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::HIDDEN_ACTIVE,
      ChicFeeTable::Classification::NON_INTERLISTED);
    test_per_share_fee_calculation(
      table, fields, "r", expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_active") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_passive") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::PASSIVE,
      ChicFeeTable::Classification::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdime_active") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::SUBDIME);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdime_passive") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::PASSIVE,
      ChicFeeTable::Classification::SUBDIME);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_hidden_active") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::HIDDEN_ACTIVE,
      ChicFeeTable::Classification::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, "r", expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_hidden_passive") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::HIDDEN_PASSIVE,
      ChicFeeTable::Classification::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, "a", expected_fee, calculate_fee);
  }

  TEST_CASE("subdime_hidden_active") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::HIDDEN_ACTIVE,
      ChicFeeTable::Classification::SUBDIME);
    test_per_share_fee_calculation(
      table, fields, "r", expected_fee, calculate_fee);
  }

  TEST_CASE("subdime_hidden_passive") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::HIDDEN_PASSIVE,
      ChicFeeTable::Classification::SUBDIME);
    test_per_share_fee_calculation(
      table, fields, "a", expected_fee, calculate_fee);
  }

  TEST_CASE("interlisted_active") {
    auto table = ChicFeeTable();
    table.m_interlisted.insert(TST);
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::INTERLISTED);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("interlisted_passive") {
    auto table = ChicFeeTable();
    table.m_interlisted.insert(TST);
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::PASSIVE,
      ChicFeeTable::Classification::INTERLISTED);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("interlisted_hidden_passive") {
    auto table = ChicFeeTable();
    table.m_interlisted.insert(TST);
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::HIDDEN_PASSIVE,
      ChicFeeTable::Classification::INTERLISTED);
    test_per_share_fee_calculation(
      table, fields, "a", expected_fee, calculate_fee);
  }

  TEST_CASE("interlisted_hidden_active") {
    auto table = ChicFeeTable();
    table.m_interlisted.insert(TST);
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::HIDDEN_ACTIVE,
      ChicFeeTable::Classification::INTERLISTED);
    test_per_share_fee_calculation(
      table, fields, "r", expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_interlisted_active") {
    auto table = ChicFeeTable();
    table.m_interlisted.insert(TST);
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdime_interlisted_active") {
    auto table = ChicFeeTable();
    table.m_interlisted.insert(TST);
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::SUBDIME);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("etf_active") {
    auto table = ChicFeeTable();
    table.m_etfs.insert(TST);
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(
      table, ChicFeeTable::Index::ACTIVE, ChicFeeTable::Classification::ETF);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("etf_passive") {
    auto table = ChicFeeTable();
    table.m_etfs.insert(TST);
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(
      table, ChicFeeTable::Index::PASSIVE, ChicFeeTable::Classification::ETF);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::PASSIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("etf_hidden_passive") {
    auto table = ChicFeeTable();
    table.m_etfs.insert(TST);
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::HIDDEN_PASSIVE,
      ChicFeeTable::Classification::ETF);
    test_per_share_fee_calculation(
      table, fields, "a", expected_fee, calculate_fee);
  }

  TEST_CASE("etf_hidden_active") {
    auto table = ChicFeeTable();
    table.m_etfs.insert(TST);
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::HIDDEN_ACTIVE,
      ChicFeeTable::Classification::ETF);
    test_per_share_fee_calculation(
      table, fields, "r", expected_fee, calculate_fee);
  }

  TEST_CASE("subdollar_etf_active") {
    auto table = ChicFeeTable();
    table.m_etfs.insert(TST);
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(20 * Money::CENT);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::SUBDOLLAR);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("subdime_etf_active") {
    auto table = ChicFeeTable();
    table.m_etfs.insert(TST);
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::CENT);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::SUBDIME);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "AP";
      auto calculated_fee = calculate_fee(table, fields, report);
      auto expected_fee =
        report.m_last_quantity * lookup_fee(table, ChicFeeTable::Index::ACTIVE,
          ChicFeeTable::Classification::NON_INTERLISTED);
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::CENT;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "PA";
      auto calculated_fee = calculate_fee(table, fields, report);
      auto expected_fee =
        report.m_last_quantity * lookup_fee(table, ChicFeeTable::Index::ACTIVE,
          ChicFeeTable::Classification::SUBDIME);
      REQUIRE(calculated_fee == expected_fee);
    }
    {
      auto report = ExecutionReport(0, second_clock::universal_time());
      report.m_last_price = Money::ONE;
      report.m_last_quantity = 100;
      report.m_liquidity_flag = "?";
      auto calculated_fee = calculate_fee(table, fields, report);
      auto expected_fee =
        report.m_last_quantity * lookup_fee(table, ChicFeeTable::Index::ACTIVE,
          ChicFeeTable::Classification::NON_INTERLISTED);
      REQUIRE(calculated_fee == expected_fee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto table = ChicFeeTable();
    populate_fee_table(out(table.m_security_table));
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = lookup_fee(table, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::NON_INTERLISTED);
    test_per_share_fee_calculation(
      table, fields, LiquidityFlag::NONE, expected_fee, calculate_fee);
  }
}
