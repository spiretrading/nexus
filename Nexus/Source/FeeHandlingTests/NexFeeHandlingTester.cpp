#include <doctest/doctest.h>
#include "Nexus/FeeHandling/NexFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

namespace {
  auto TST = parse_ticker("TST.TSXV");

  auto make_order_fields(Money price) {
    return make_limit_order_fields(DirectoryEntry::ROOT_ACCOUNT, TST, CAD,
      Side::BID, DefaultDestinations::TSX, 100, price);
  }

  auto make_fee_table() {
    auto table = NexFeeTable();
    table.m_fee = Money::ONE;
    return table;
  }
}

TEST_SUITE("NexFeeHandling") {
  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    test_per_share_fee_calculation(table, fields.m_price, 0,
      LiquidityFlag::NONE, Money::ZERO, calculate_fee);
  }

  TEST_CASE("execution") {
    auto table = make_fee_table();
    auto fields = make_order_fields(Money::ONE);
    auto expected_fee = Money::ONE;
    test_per_share_fee_calculation(table, Money::ONE, 100,
      LiquidityFlag::ACTIVE, expected_fee, calculate_fee);
  }
}
