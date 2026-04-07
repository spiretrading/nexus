#include <doctest/doctest.h>
#include "Nexus/FeeHandling/XatsFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  auto make_fee_table() {
    auto table = XatsFeeTable();
    populate_fee_table(out(table.m_general_fee_table));
    populate_fee_table(out(table.m_etf_fee_table));
    table.m_intraspread_dark_to_dark_max_fee = 1000 * Money::CENT;
    table.m_intraspread_dark_to_dark_subdollar_max_fee = 2000 * Money::CENT;
    return table;
  }
}

TEST_SUITE("XatsFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto table = make_fee_table();
    test_fee_table_index(table, table.m_general_fee_table, lookup_general_fee,
      XatsFeeTable::TYPE_COUNT, XatsFeeTable::PRICE_CLASS_COUNT);
    test_fee_table_index(table, table.m_etf_fee_table, lookup_etf_fee,
      XatsFeeTable::TYPE_COUNT, XatsFeeTable::PRICE_CLASS_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto expected_fee = Money::ZERO;
    test_per_share_fee_calculation(
      table, Money::ONE, 0, LiquidityFlag::NONE, expected_fee,
      [] (const auto& table, const auto& report) {
        return calculate_fee(table, false, report);
      });
  }
}
