#include "Nexus/FeeHandlingTests/PureFeeHandlingTester.hpp"
#include "Nexus/FeeHandling/PureFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  Security GetDesignatedSecurity() {
    return Security{"DSG", DefaultMarkets::TSX(), DefaultCountries::CA()};
  }

  Security GetTsxSecurity() {
    return Security{"TSX", DefaultMarkets::TSX(), DefaultCountries::CA()};
  }

  Security GetTsxVentureSecurity() {
    return Security{"TSXV", DefaultMarkets::TSXV(), DefaultCountries::CA()};
  }

  Security GetUnlistedSecurity() {
    return Security{"???", DefaultMarkets::ASX(), DefaultCountries::CA()};
  }

  PureFeeTable BuildFeeTable() {
    PureFeeTable feeTable;
    PopulateFeeTable(Store(feeTable.m_tsxListedFeeTable));
    PopulateFeeTable(Store(feeTable.m_tsxVentureListedFeeTable));
    for(auto i = 0; i < PureFeeTable::PRICE_CLASS_COUNT; ++i) {
      feeTable.m_tsxVentureListedFeeTable[i][
        static_cast<int>(LiquidityFlag::PASSIVE)] *= -1;
    }
    feeTable.m_tsxVentureListedSubDimeCap = 1000 * Money::CENT;
    feeTable.m_oddLot = 12 * Money::ONE;
    feeTable.m_designatedSecurities.insert(GetDesignatedSecurity());
    return feeTable;
  }
}

void PureFeeHandlingTester::TestFeeTableCalculations() {
  auto feeTable = BuildFeeTable();
  TestFeeTableIndex(feeTable, feeTable.m_tsxListedFeeTable, LookupTsxListedFee,
    LIQUIDITY_FLAG_COUNT, PureFeeTable::PRICE_CLASS_COUNT);
  TestFeeTableIndex(feeTable, feeTable.m_tsxVentureListedFeeTable,
    LookupTsxVentureListedFee, LIQUIDITY_FLAG_COUNT,
    PureFeeTable::PRICE_CLASS_COUNT);
}

void PureFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = BuildFeeTable();
  TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxSecurity(),
    std::placeholders::_2), Money::ZERO);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
    std::placeholders::_2), Money::ZERO);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
    std::bind(&CalculateFee, std::placeholders::_1, GetUnlistedSecurity(),
    std::placeholders::_2), Money::ZERO);
}

void PureFeeHandlingTester::TestTsxDefaultActive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::ACTIVE,
    PureFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxSecurity(),
    std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestTsxDefaultPassive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::PASSIVE,
    PureFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxSecurity(),
    std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestTsxDesignatedActive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::ACTIVE,
    PureFeeTable::PriceClass::DESIGNATED);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    GetDesignatedSecurity(), std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestTsxDesignatedPassive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::PASSIVE,
    PureFeeTable::PriceClass::DESIGNATED);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1,
    GetDesignatedSecurity(), std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestTsxSubDollarDesignatedActive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::ACTIVE,
    PureFeeTable::PriceClass::DESIGNATED);
  TestPerShareFeeCalculation(feeTable, 99 * Money::CENT, 100,
    LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
    GetDesignatedSecurity(), std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestTsxSubDollarDesignatedPassive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::PASSIVE,
    PureFeeTable::PriceClass::DESIGNATED);
  TestPerShareFeeCalculation(feeTable, 99 * Money::CENT, 100,
    LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
    GetDesignatedSecurity(), std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestTsxSubDollarActive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::ACTIVE,
    PureFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, 10 * Money::CENT, 100,
    LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
    GetTsxSecurity(), std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestTsxSubDollarPassive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::PASSIVE,
    PureFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, 10 * Money::CENT, 100,
    LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
    GetTsxSecurity(), std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestTsxVentureDefaultActive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxVentureListedFee(feeTable, LiquidityFlag::ACTIVE,
    PureFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
    std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestTsxVentureDefaultPassive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxVentureListedFee(feeTable, LiquidityFlag::PASSIVE,
    PureFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
    std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestTsxVentureSubDollarActive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxVentureListedFee(feeTable, LiquidityFlag::ACTIVE,
    PureFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, 50 * Money::CENT, 100,
    LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
    GetTsxVentureSecurity(), std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestTsxVentureSubDollarPassive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxVentureListedFee(feeTable, LiquidityFlag::PASSIVE,
    PureFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, 50 * Money::CENT, 100,
    LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
    GetTsxVentureSecurity(), std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestTsxVentureSubDimeActive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxVentureListedFee(feeTable, LiquidityFlag::ACTIVE,
    PureFeeTable::PriceClass::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, Money::CENT, 100, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
    std::placeholders::_2), expectedFee);
  TestFeeCalculation(feeTable, Money::CENT, 10000000, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
    std::placeholders::_2), feeTable.m_tsxVentureListedSubDimeCap);
}

void PureFeeHandlingTester::TestTsxVentureSubDimePassive() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxVentureListedFee(feeTable, LiquidityFlag::PASSIVE,
    PureFeeTable::PriceClass::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, Money::CENT, 100, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
    std::placeholders::_2), expectedFee);
  TestFeeCalculation(feeTable, Money::CENT, 10000000, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
    std::placeholders::_2), -feeTable.m_tsxVentureListedSubDimeCap);
}

void PureFeeHandlingTester::TestOddLot() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = feeTable.m_oddLot;
  TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxSecurity(),
    std::placeholders::_2), expectedFee);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxSecurity(),
    std::placeholders::_2), expectedFee);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
    std::placeholders::_2), expectedFee);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
    std::placeholders::_2), expectedFee);
}

void PureFeeHandlingTester::TestUnknownLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "AP";
    auto calculatedFee = CalculateFee(feeTable, GetTsxSecurity(),
      executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupTsxListedFee(
      feeTable, LiquidityFlag::ACTIVE, PureFeeTable::PriceClass::DEFAULT);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::CENT;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "PA";
    auto calculatedFee = CalculateFee(feeTable, GetTsxVentureSecurity(),
      executionReport);
    auto expectedFee = executionReport.m_lastQuantity *
      LookupTsxVentureListedFee(feeTable, LiquidityFlag::ACTIVE,
      PureFeeTable::PriceClass::SUB_DIME);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "?";
    auto calculatedFee = CalculateFee(feeTable, GetUnlistedSecurity(),
      executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupTsxListedFee(
      feeTable, LiquidityFlag::ACTIVE, PureFeeTable::PriceClass::DEFAULT);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
}

void PureFeeHandlingTester::TestEmptyLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::ACTIVE,
    PureFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::NONE,
    std::bind(CalculateFee, std::placeholders::_1, GetTsxSecurity(),
    std::placeholders::_2), expectedFee);
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::NONE,
    std::bind(CalculateFee, std::placeholders::_1, GetUnlistedSecurity(),
    std::placeholders::_2), expectedFee);
}
