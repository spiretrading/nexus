#include "Nexus/FeeHandlingTests/MatnFeeHandlingTester.hpp"
#include "Nexus/FeeHandling/MatnFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;
using namespace std;

namespace {
  MatnFeeTable BuildFeeTable() {
    MatnFeeTable feeTable;
    PopulateFeeTable(Store(feeTable.m_generalFeeTable));
    PopulateFeeTable(Store(feeTable.m_alternativeFeeTable));
    for(auto i = 0; i < MatnFeeTable::PRICE_CLASS_COUNT; ++i) {
      feeTable.m_generalFeeTable[i][
        static_cast<int>(MatnFeeTable::GeneralIndex::MAX_CHARGE)] *= 1000;
    }
    return feeTable;
  }

  Money LookupGeneralFee(const MatnFeeTable& feeTable,
      MatnFeeTable::GeneralIndex index, MatnFeeTable::PriceClass priceClass) {
    return LookupFee(feeTable, index, priceClass);
  }

  Money LookupAlternativeFee(const MatnFeeTable& feeTable,
      LiquidityFlag liquidityFlag, MatnFeeTable::Category category) {
    return LookupFee(feeTable, liquidityFlag, category);
  }
}

void MatnFeeHandlingTester::TestFeeTableCalculations() {
  auto feeTable = BuildFeeTable();
  TestFeeTableIndex(feeTable, feeTable.m_generalFeeTable, LookupGeneralFee,
    MatnFeeTable::GENERAL_INDEX_COUNT, MatnFeeTable::PRICE_CLASS_COUNT);
  TestFeeTableIndex(feeTable, feeTable.m_alternativeFeeTable,
    LookupAlternativeFee, LIQUIDITY_FLAG_COUNT,
    MatnFeeTable::CATEGORY_COUNT);
}

void MatnFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = BuildFeeTable();
  TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2), Money::ZERO);
}

void MatnFeeHandlingTester::TestActiveDefaultTrade() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
    MatnFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, 5 * Money::ONE, 100,
    LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2), expectedFee);
  auto expectedMaxFee = LookupFee(feeTable,
    MatnFeeTable::GeneralIndex::MAX_CHARGE, MatnFeeTable::PriceClass::DEFAULT);
  TestFeeCalculation(feeTable, 5 * Money::ONE, 10000, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
    expectedMaxFee);
}

void MatnFeeHandlingTester::TestPassiveDefaultTrade() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
    MatnFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, 5 * Money::ONE, 100,
    LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2), expectedFee);
  auto expectedMaxFee = LookupFee(feeTable,
    MatnFeeTable::GeneralIndex::MAX_CHARGE, MatnFeeTable::PriceClass::DEFAULT);
  TestFeeCalculation(feeTable, 5 * Money::ONE, 10000, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
    expectedMaxFee);
}

void MatnFeeHandlingTester::TestActiveDollarTrade() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
    MatnFeeTable::PriceClass::SUB_FIVE_DOLLAR);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2), expectedFee);
  auto expectedMaxFee = LookupFee(feeTable,
    MatnFeeTable::GeneralIndex::MAX_CHARGE,
    MatnFeeTable::PriceClass::SUB_FIVE_DOLLAR);
  TestFeeCalculation(feeTable, Money::ONE, 10000, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
    expectedMaxFee);
}

void MatnFeeHandlingTester::TestPassiveDollarTrade() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
    MatnFeeTable::PriceClass::SUB_FIVE_DOLLAR);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2), expectedFee);
  auto expectedMaxFee = LookupFee(feeTable,
    MatnFeeTable::GeneralIndex::MAX_CHARGE,
    MatnFeeTable::PriceClass::SUB_FIVE_DOLLAR);
  TestFeeCalculation(feeTable, Money::ONE, 10000, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
    expectedMaxFee);
}

void MatnFeeHandlingTester::TestActiveSubDollarTrade() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
    MatnFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, Money::CENT, 100, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2), expectedFee);
  auto expectedMaxFee = LookupFee(feeTable,
    MatnFeeTable::GeneralIndex::MAX_CHARGE,
    MatnFeeTable::PriceClass::SUB_DOLLAR);
  TestFeeCalculation(feeTable, Money::CENT, 10000, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
    expectedMaxFee);
}

void MatnFeeHandlingTester::TestPassiveSubDollarTrade() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
    MatnFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, Money::CENT, 100, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2), expectedFee);
  auto expectedMaxFee = LookupFee(feeTable,
    MatnFeeTable::GeneralIndex::MAX_CHARGE,
    MatnFeeTable::PriceClass::SUB_DOLLAR);
  TestFeeCalculation(feeTable, Money::CENT, 10000, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
    expectedMaxFee);
}

void MatnFeeHandlingTester::TestActiveEtfTrade() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    MatnFeeTable::Category::ETF);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::ETF, std::placeholders::_2), expectedFee);
}

void MatnFeeHandlingTester::TestPassiveEtfTrade() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
    MatnFeeTable::Category::ETF);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::ETF, std::placeholders::_2), expectedFee);
}

void MatnFeeHandlingTester::TestActiveOddLotTrade() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    MatnFeeTable::Category::ODD_LOT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2), expectedFee);
}

void MatnFeeHandlingTester::TestPassiveOddLotTrade() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
    MatnFeeTable::Category::ODD_LOT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2), expectedFee);
}

void MatnFeeHandlingTester::TestActiveOddLotEtf() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    MatnFeeTable::Category::ODD_LOT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::ETF, std::placeholders::_2), expectedFee);
}

void MatnFeeHandlingTester::TestPassiveOddLotEtf() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
    MatnFeeTable::Category::ODD_LOT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::ETF, std::placeholders::_2), expectedFee);
}

void MatnFeeHandlingTester::TestUnknownLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::CENT;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "AP";
    auto calculatedFee = CalculateFee(feeTable,
      MatnFeeTable::Classification::DEFAULT, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      MatnFeeTable::GeneralIndex::FEE, MatnFeeTable::PriceClass::SUB_DOLLAR);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "PA";
    auto calculatedFee = CalculateFee(feeTable,
      MatnFeeTable::Classification::DEFAULT, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::SUB_FIVE_DOLLAR);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "PA";
    auto calculatedFee = CalculateFee(feeTable,
      MatnFeeTable::Classification::ETF, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      LiquidityFlag::ACTIVE, MatnFeeTable::Category::ETF);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
}

void MatnFeeHandlingTester::TestEmptyLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
    MatnFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, 5 * Money::ONE, 100, LiquidityFlag::NONE,
    std::bind(&CalculateFee, std::placeholders::_1,
    MatnFeeTable::Classification::DEFAULT, std::placeholders::_2), expectedFee);
}
