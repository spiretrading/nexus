#include "Nexus/FeeHandlingTests/LynxFeeHandlingTester.hpp"
#include "Nexus/FeeHandling/LynxFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;
using namespace std;

namespace {
  Money LookupLiquidityFee(const LynxFeeTable& feeTable,
      LiquidityFlag liquidityFlag, LynxFeeTable::PriceClass priceClass) {
    return LookupFee(feeTable, liquidityFlag, priceClass);
  }

  LynxFeeTable BuildFeeTable() {
    LynxFeeTable feeTable;
    PopulateFeeTable(Store(feeTable.m_feeTable));
    return feeTable;
  }
}

void LynxFeeHandlingTester::TestFeeTableCalculations() {
  auto feeTable = BuildFeeTable();
  TestFeeTableIndex(feeTable, feeTable.m_feeTable, LookupLiquidityFee,
    LIQUIDITY_FLAG_COUNT, LynxFeeTable::PRICE_CLASS_COUNT);
}

void LynxFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = BuildFeeTable();
  TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
    std::bind(&CalculateFee, std::placeholders::_1, std::placeholders::_2),
    Money::ZERO);
}

void LynxFeeHandlingTester::TestActive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    LynxFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, std::placeholders::_2),
    expectedFee);
}

void LynxFeeHandlingTester::TestPassive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
    LynxFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, std::placeholders::_2),
    expectedFee);
}

void LynxFeeHandlingTester::TestSubDollarActive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    LynxFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, Money::CENT, 100, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, std::placeholders::_2),
    expectedFee);
}

void LynxFeeHandlingTester::TestSubDollarPassive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
    LynxFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, Money::CENT, 100, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, std::placeholders::_2),
    expectedFee);
}

void LynxFeeHandlingTester::TestUnknownLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "AP";
    auto calculatedFee = CalculateFee(feeTable, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      LiquidityFlag::ACTIVE, LynxFeeTable::PriceClass::DEFAULT);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::CENT;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "PA";
    auto calculatedFee = CalculateFee(feeTable, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      LiquidityFlag::ACTIVE, LynxFeeTable::PriceClass::SUB_DOLLAR);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "?";
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      LynxFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, std::placeholders::_2),
      expectedFee);
  }
}

void LynxFeeHandlingTester::TestEmptyLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "";
    auto calculatedFee = CalculateFee(feeTable, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      LiquidityFlag::ACTIVE, LynxFeeTable::PriceClass::DEFAULT);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "";
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      LynxFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, std::placeholders::_2),
      expectedFee);
  }
}
