#include "Nexus/FeeHandlingTests/CseFeeHandlingTester.hpp"
#include "Nexus/FeeHandling/CseFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  CseFeeTable BuildFeeTable() {
    CseFeeTable feeTable;
    PopulateFeeTable(Store(feeTable.m_feeTable));
    return feeTable;
  }
}

void CseFeeHandlingTester::TestFeeTableCalculations() {
  auto feeTable = BuildFeeTable();
  TestFeeTableIndex(feeTable, feeTable.m_feeTable, LookupFee,
    LIQUIDITY_FLAG_COUNT, CseFeeTable::PRICE_CLASS_COUNT);
}

void CseFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = BuildFeeTable();
  TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
    &CalculateFee, Money::ZERO);
}

void CseFeeHandlingTester::TestDefaultActive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    CseFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
    &CalculateFee, expectedFee);
}

void CseFeeHandlingTester::TestDefaultPassive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
    CseFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::PASSIVE,
    &CalculateFee, expectedFee);
}

void CseFeeHandlingTester::TestSubDollarActive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    CseFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, 10 * Money::CENT, 100,
    LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
}

void CseFeeHandlingTester::TestSubDollarPassive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
    CseFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, 10 * Money::CENT, 100,
    LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
}

void CseFeeHandlingTester::TestSubDimeActive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    CseFeeTable::PriceClass::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, Money::CENT, 100, LiquidityFlag::ACTIVE,
    &CalculateFee, expectedFee);
}

void CseFeeHandlingTester::TestSubDimePassive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
    CseFeeTable::PriceClass::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, Money::CENT, 100, LiquidityFlag::PASSIVE,
    &CalculateFee, expectedFee);
}

void CseFeeHandlingTester::TestUnknownLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "AP";
    auto calculatedFee = CalculateFee(feeTable, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(
      feeTable, LiquidityFlag::ACTIVE, CseFeeTable::PriceClass::DEFAULT);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::CENT;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "PA";
    auto calculatedFee = CalculateFee(feeTable, executionReport);
    auto expectedFee = executionReport.m_lastQuantity *
      LookupFee(feeTable, LiquidityFlag::ACTIVE,
      CseFeeTable::PriceClass::SUB_DIME);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "?";
    auto calculatedFee = CalculateFee(feeTable, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(
      feeTable, LiquidityFlag::ACTIVE, CseFeeTable::PriceClass::DEFAULT);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
}

void CseFeeHandlingTester::TestEmptyLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    CseFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::NONE,
    &CalculateFee, expectedFee);
}
