#include "Nexus/FeeHandlingTests/XatsFeeHandlingTester.hpp"
#include "Nexus/FeeHandling/XatsFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  XatsFeeTable BuildFeeTable() {
    XatsFeeTable feeTable;
    PopulateFeeTable(Store(feeTable.m_generalFeeTable));
    PopulateFeeTable(Store(feeTable.m_etfFeeTable));
    feeTable.m_intraspreadDarkToDarkMaxFee = 1000 * Money::CENT;
    feeTable.m_intraspreadDarkToDarkSubDollarMaxFee = 2000 * Money::CENT;
    return feeTable;
  }
}

void XatsFeeHandlingTester::TestFeeTableCalculations() {
  auto feeTable = BuildFeeTable();
  TestFeeTableIndex(feeTable, feeTable.m_generalFeeTable, LookupGeneralFee,
    XatsFeeTable::TYPE_COUNT, XatsFeeTable::PRICE_CLASS_COUNT);
  TestFeeTableIndex(feeTable, feeTable.m_etfFeeTable, LookupEtfFee,
    XatsFeeTable::TYPE_COUNT, XatsFeeTable::PRICE_CLASS_COUNT);
}

void XatsFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = Money::ZERO;
  TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
    std::bind(CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2), expectedFee);
}
