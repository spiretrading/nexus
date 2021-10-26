#include <doctest/doctest.h>
#include "Nexus/FeeHandling/XatsFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  auto MakeFeeTable() {
    auto feeTable = XatsFeeTable();
    PopulateFeeTable(Store(feeTable.m_generalFeeTable));
    PopulateFeeTable(Store(feeTable.m_etfFeeTable));
    feeTable.m_intraspreadDarkToDarkMaxFee = 1000 * Money::CENT;
    feeTable.m_intraspreadDarkToDarkSubdollarMaxFee = 2000 * Money::CENT;
    return feeTable;
  }
}

TEST_SUITE("XatsFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = MakeFeeTable();
    TestFeeTableIndex(feeTable, feeTable.m_generalFeeTable, LookupGeneralFee,
      XatsFeeTable::TYPE_COUNT, XatsFeeTable::PRICE_CLASS_COUNT);
    TestFeeTableIndex(feeTable, feeTable.m_etfFeeTable, LookupEtfFee,
      XatsFeeTable::TYPE_COUNT, XatsFeeTable::PRICE_CLASS_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = Money::ZERO;
    TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
      std::bind(CalculateFee, std::placeholders::_1, false,
        std::placeholders::_2), expectedFee);
  }
}
