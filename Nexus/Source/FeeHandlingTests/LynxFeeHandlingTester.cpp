#include "Nexus/FeeHandlingTests/LynxFeeHandlingTester.hpp"
#include "Nexus/FeeHandling/LynxFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;
using namespace std;

namespace {
  Security GetSecurityLevelA() {
    return Security{"LVLA", DefaultMarkets::TSX(), DefaultCountries::CA()};
  }

  Security GetSecurityLevelB() {
    return Security{"LVLB", DefaultMarkets::TSX(), DefaultCountries::CA()};
  }

  Security GetSecurityLevelC() {
    return Security{"LVLC", DefaultMarkets::TSX(), DefaultCountries::CA()};
  }

  Security GetSecurityLevelD() {
    return Security{"LVLD", DefaultMarkets::TSX(), DefaultCountries::CA()};
  }

  Security GetSecurityLevelX() {
    return Security{"LVLX", DefaultMarkets::TSX(), DefaultCountries::CA()};
  }

  Money LookupLiquidityFee(const LynxFeeTable& feeTable,
      LiquidityFlag liquidityFlag, LynxFeeTable::LiquidityLevel level) {
    return LookupFee(feeTable, liquidityFlag, level);
  }

  Money LookupAlternativeFee(const LynxFeeTable& feeTable,
      LiquidityFlag liquidityFlag, LynxFeeTable::AlternativeTrade type) {
    return LookupFee(feeTable, liquidityFlag, type);
  }

  LynxFeeTable BuildFeeTable() {
    LynxFeeTable feeTable;
    PopulateFeeTable(Store(feeTable.m_liquidityFeeTable));
    PopulateFeeTable(Store(feeTable.m_alternativeTradeFeeTable));
    feeTable.m_liquidityLevels[GetSecurityLevelA()] =
      LynxFeeTable::LiquidityLevel::LEVEL_A;
    feeTable.m_liquidityLevels[GetSecurityLevelB()] =
      LynxFeeTable::LiquidityLevel::LEVEL_B;
    feeTable.m_liquidityLevels[GetSecurityLevelC()] =
      LynxFeeTable::LiquidityLevel::LEVEL_C;
    feeTable.m_liquidityLevels[GetSecurityLevelD()] =
      LynxFeeTable::LiquidityLevel::LEVEL_D;
    return feeTable;
  }
}

void LynxFeeHandlingTester::TestFeeTableCalculations() {
  auto feeTable = BuildFeeTable();
  TestFeeTableIndex(feeTable, feeTable.m_liquidityFeeTable, LookupLiquidityFee,
    LIQUIDITY_FLAG_COUNT, LynxFeeTable::LIQUIDITY_LEVEL_COUNT);
  TestFeeTableIndex(feeTable, feeTable.m_alternativeTradeFeeTable,
    LookupAlternativeFee, LIQUIDITY_FLAG_COUNT,
    LynxFeeTable::ALTERNATIVE_TRADE_COUNT);
}

void LynxFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = BuildFeeTable();
  TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
    std::bind(&CalculateFee, std::placeholders::_1, GetSecurityLevelA(),
    std::placeholders::_2), Money::ZERO);
}

void LynxFeeHandlingTester::TestActive() {
  auto feeTable = BuildFeeTable();
  for(auto& entry : feeTable.m_liquidityLevels) {
    auto& security = entry.first;
    auto& level = entry.second;
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE, level);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, security,
      std::placeholders::_2), expectedFee);
  }
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    LynxFeeTable::LiquidityLevel::LEVEL_D);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetSecurityLevelX(),
    std::placeholders::_2), expectedFee);
}

void LynxFeeHandlingTester::TestPassive() {
  auto feeTable = BuildFeeTable();
  for(auto& entry : feeTable.m_liquidityLevels) {
    auto& security = entry.first;
    auto& level = entry.second;
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE, level);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
      security, std::placeholders::_2), expectedFee);
  }
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
    LynxFeeTable::LiquidityLevel::LEVEL_A);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetSecurityLevelX(),
    std::placeholders::_2), expectedFee);
}

void LynxFeeHandlingTester::TestSubDollarActive() {
  auto feeTable = BuildFeeTable();
  for(auto& entry : feeTable.m_liquidityLevels) {
    auto& security = entry.first;
    auto& level = entry.second;
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      LynxFeeTable::AlternativeTrade::SUB_DOLLAR);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
      security, std::placeholders::_2), expectedFee);
  }
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    LynxFeeTable::AlternativeTrade::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, Money::CENT, 100, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetSecurityLevelX(),
    std::placeholders::_2), expectedFee);
}

void LynxFeeHandlingTester::TestSubDollarPassive() {
  auto feeTable = BuildFeeTable();
  for(auto& entry : feeTable.m_liquidityLevels) {
    auto& security = entry.first;
    auto& level = entry.second;
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      LynxFeeTable::AlternativeTrade::SUB_DOLLAR);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
      security, std::placeholders::_2), expectedFee);
  }
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
    LynxFeeTable::AlternativeTrade::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, Money::CENT, 100, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetSecurityLevelX(),
    std::placeholders::_2), expectedFee);
}

void LynxFeeHandlingTester::TestOddLotActive() {
  auto feeTable = BuildFeeTable();
  for(auto& entry : feeTable.m_liquidityLevels) {
    auto& security = entry.first;
    auto& level = entry.second;
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      LynxFeeTable::AlternativeTrade::ODD_LOT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, security,
      std::placeholders::_2), expectedFee);
  }
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    LynxFeeTable::AlternativeTrade::ODD_LOT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetSecurityLevelX(),
    std::placeholders::_2), expectedFee);
}

void LynxFeeHandlingTester::TestOddLotPassive() {
  auto feeTable = BuildFeeTable();
  for(auto& entry : feeTable.m_liquidityLevels) {
    auto& security = entry.first;
    auto& level = entry.second;
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      LynxFeeTable::AlternativeTrade::ODD_LOT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, security,
      std::placeholders::_2), expectedFee);
  }
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
    LynxFeeTable::AlternativeTrade::ODD_LOT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetSecurityLevelX(),
    std::placeholders::_2), expectedFee);
}

void LynxFeeHandlingTester::TestSubDollarOddLotActive() {
  auto feeTable = BuildFeeTable();
  for(auto& entry : feeTable.m_liquidityLevels) {
    auto& security = entry.first;
    auto& level = entry.second;
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      LynxFeeTable::AlternativeTrade::ODD_LOT);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 50, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, security,
      std::placeholders::_2), expectedFee);
  }
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
    LynxFeeTable::AlternativeTrade::ODD_LOT);
  TestPerShareFeeCalculation(feeTable, Money::CENT, 50, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetSecurityLevelX(),
    std::placeholders::_2), expectedFee);
}

void LynxFeeHandlingTester::TestSubDollarOddLotPassive() {
  auto feeTable = BuildFeeTable();
  for(auto& entry : feeTable.m_liquidityLevels) {
    auto& security = entry.first;
    auto& level = entry.second;
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      LynxFeeTable::AlternativeTrade::ODD_LOT);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 50,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
      security, std::placeholders::_2), expectedFee);
  }
  auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
    LynxFeeTable::AlternativeTrade::ODD_LOT);
  TestPerShareFeeCalculation(feeTable, Money::CENT, 50, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetSecurityLevelX(),
    std::placeholders::_2), expectedFee);
}

void LynxFeeHandlingTester::TestUnknownLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "AP";
    for(auto& entry : feeTable.m_liquidityLevels) {
      auto& security = entry.first;
      auto& level = entry.second;
      auto calculatedFee = CalculateFee(feeTable, security, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        LiquidityFlag::ACTIVE, level);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::CENT;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "PA";
    for(auto& entry : feeTable.m_liquidityLevels) {
      auto& security = entry.first;
      auto& level = entry.second;
      auto calculatedFee = CalculateFee(feeTable, security, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        LiquidityFlag::ACTIVE, LynxFeeTable::AlternativeTrade::SUB_DOLLAR);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::CENT;
    executionReport.m_lastQuantity = 50;
    executionReport.m_liquidityFlag = "?";
    for(auto& entry : feeTable.m_liquidityLevels) {
      auto& security = entry.first;
      auto& level = entry.second;
      auto calculatedFee = CalculateFee(feeTable, security, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        LiquidityFlag::ACTIVE, LynxFeeTable::AlternativeTrade::ODD_LOT);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "?";
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      LynxFeeTable::LiquidityLevel::LEVEL_D);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, GetSecurityLevelX(),
      std::placeholders::_2), expectedFee);
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
    for(auto& entry : feeTable.m_liquidityLevels) {
      auto& security = entry.first;
      auto& level = entry.second;
      auto calculatedFee = CalculateFee(feeTable, security, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        LiquidityFlag::ACTIVE, level);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "";
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      LynxFeeTable::LiquidityLevel::LEVEL_D);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, GetSecurityLevelX(),
      std::placeholders::_2), expectedFee);
  }
}
