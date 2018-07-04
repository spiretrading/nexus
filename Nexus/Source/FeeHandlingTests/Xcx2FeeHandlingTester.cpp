#include "Nexus/FeeHandlingTests/Xcx2FeeHandlingTester.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/Xcx2FeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;
using namespace std;

namespace {
  Xcx2FeeTable BuildFeeTable() {
    Xcx2FeeTable feeTable;
    PopulateFeeTable(Store(feeTable.m_feeTable));
    feeTable.m_largeTradeSize = 1000;
    return feeTable;
  }

  OrderFields BuildOrderFields(Money price) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{"TST", DefaultMarkets::TSX(), DefaultCountries::CA()},
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::CX2(), 100,
      price);
    return fields;
  }

  OrderFields BuildHiddenOrderFields(Money price) {
    auto fields = BuildOrderFields(price);
    fields.m_type = OrderType::PEGGED;
    fields.m_additionalFields.emplace_back(18, "M");
    return fields;
  }
}

void Xcx2FeeHandlingTester::TestFeeTableCalculations() {
  auto feeTable = BuildFeeTable();
  TestFeeTableIndex(feeTable, feeTable.m_feeTable, LookupFee,
    Xcx2FeeTable::TYPE_COUNT, Xcx2FeeTable::PRICE_CLASS_COUNT);
}

void Xcx2FeeHandlingTester::TestZeroQuantity() {
  auto feeTable = BuildFeeTable();
  auto fields = BuildOrderFields(Money::ONE);
  fields.m_quantity = 0;
  auto expectedFee = Money::ZERO;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestActiveDefault() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::ACTIVE,
    Xcx2FeeTable::PriceClass::DEFAULT);
  auto fields = BuildOrderFields(Money::ONE);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestPassiveDefault() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::PASSIVE,
    Xcx2FeeTable::PriceClass::DEFAULT);
  auto fields = BuildOrderFields(Money::ONE);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestLargeActiveDefault() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::LARGE_ACTIVE,
    Xcx2FeeTable::PriceClass::DEFAULT);
  auto fields = BuildOrderFields(Money::ONE);
  fields.m_quantity = feeTable.m_largeTradeSize;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestLargePassiveDefault() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::LARGE_PASSIVE,
    Xcx2FeeTable::PriceClass::DEFAULT);
  auto fields = BuildOrderFields(Money::ONE);
  fields.m_quantity = feeTable.m_largeTradeSize;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestHiddenActiveOverDollar() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::HIDDEN_ACTIVE,
    Xcx2FeeTable::PriceClass::DEFAULT);
  auto fields = BuildHiddenOrderFields(Money::ONE);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestHiddenPassiveOverDollar() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::HIDDEN_PASSIVE,
    Xcx2FeeTable::PriceClass::DEFAULT);
  auto fields = BuildHiddenOrderFields(Money::ONE);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestActiveOddLotOverDollar() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::ODD_LOT,
    Xcx2FeeTable::PriceClass::DEFAULT);
  auto fields = BuildOrderFields(Money::ONE);
  fields.m_quantity = 50;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestPassiveOddLotOverDollar() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::ODD_LOT,
    Xcx2FeeTable::PriceClass::DEFAULT);
  auto fields = BuildOrderFields(Money::ONE);
  fields.m_quantity = 50;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestActiveSubDollar() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::ACTIVE,
    Xcx2FeeTable::PriceClass::SUB_DOLLAR);
  auto fields = BuildOrderFields(20 * Money::CENT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestPassiveSubDollar() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::PASSIVE,
    Xcx2FeeTable::PriceClass::SUB_DOLLAR);
  auto fields = BuildOrderFields(20 * Money::CENT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestActiveSubDime() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::ACTIVE,
    Xcx2FeeTable::PriceClass::SUB_DIME);
  auto fields = BuildOrderFields(Money::CENT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestPassiveSubDime() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::PASSIVE,
    Xcx2FeeTable::PriceClass::SUB_DIME);
  auto fields = BuildOrderFields(Money::CENT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestLargeActiveSubDollar() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::LARGE_ACTIVE,
    Xcx2FeeTable::PriceClass::SUB_DOLLAR);
  auto fields = BuildOrderFields(20 * Money::CENT);
  fields.m_quantity = feeTable.m_largeTradeSize;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestLargePassiveSubDollar() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::LARGE_PASSIVE,
    Xcx2FeeTable::PriceClass::SUB_DOLLAR);
  auto fields = BuildOrderFields(20 * Money::CENT);
  fields.m_quantity = feeTable.m_largeTradeSize;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestLargeActiveSubDime() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::LARGE_ACTIVE,
    Xcx2FeeTable::PriceClass::SUB_DIME);
  auto fields = BuildOrderFields(Money::CENT);
  fields.m_quantity = feeTable.m_largeTradeSize;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestLargePassiveSubDime() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::LARGE_PASSIVE,
    Xcx2FeeTable::PriceClass::SUB_DIME);
  auto fields = BuildOrderFields(Money::CENT);
  fields.m_quantity = feeTable.m_largeTradeSize;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestHiddenActiveSubDollar() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::HIDDEN_ACTIVE,
    Xcx2FeeTable::PriceClass::SUB_DOLLAR);
  auto fields = BuildHiddenOrderFields(20 * Money::CENT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestHiddenPassiveSubDollar() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::HIDDEN_PASSIVE,
    Xcx2FeeTable::PriceClass::SUB_DOLLAR);
  auto fields = BuildHiddenOrderFields(20 * Money::CENT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestHiddenActiveSubDime() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::HIDDEN_ACTIVE,
    Xcx2FeeTable::PriceClass::SUB_DIME);
  auto fields = BuildHiddenOrderFields(Money::CENT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestHiddenPassiveSubDime() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::HIDDEN_PASSIVE,
    Xcx2FeeTable::PriceClass::SUB_DIME);
  auto fields = BuildHiddenOrderFields(Money::CENT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestActiveOddLotSubDollar() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::ODD_LOT,
    Xcx2FeeTable::PriceClass::SUB_DOLLAR);
  auto fields = BuildOrderFields(20 * Money::CENT);
  fields.m_quantity = 50;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestPassiveOddLotSubDollar() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::ODD_LOT,
    Xcx2FeeTable::PriceClass::SUB_DOLLAR);
  auto fields = BuildOrderFields(20 * Money::CENT);
  fields.m_quantity = 50;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestActiveOddLotSubDime() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::ODD_LOT,
    Xcx2FeeTable::PriceClass::SUB_DIME);
  auto fields = BuildOrderFields(Money::CENT);
  fields.m_quantity = 50;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestPassiveOddLotSubDime() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::ODD_LOT,
    Xcx2FeeTable::PriceClass::SUB_DIME);
  auto fields = BuildOrderFields(Money::CENT);
  fields.m_quantity = 50;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestHiddenOddLot() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::ODD_LOT,
    Xcx2FeeTable::PriceClass::SUB_DOLLAR);
  auto fields = BuildHiddenOrderFields(20 * Money::CENT);
  fields.m_quantity = 50;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}

void Xcx2FeeHandlingTester::TestUnknownLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = 20 * Money::CENT;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "AP";
    auto orderFields = BuildOrderFields(20 * Money::CENT);
    auto calculatedFee = CalculateFee(feeTable, orderFields, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      Xcx2FeeTable::Type::PASSIVE, Xcx2FeeTable::PriceClass::SUB_DOLLAR);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "PA";
    auto orderFields = BuildOrderFields(Money::ONE);
    auto calculatedFee = CalculateFee(feeTable, orderFields, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      Xcx2FeeTable::Type::PASSIVE, Xcx2FeeTable::PriceClass::DEFAULT);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::CENT;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "?";
    auto orderFields = BuildHiddenOrderFields(Money::CENT);
    auto calculatedFee = CalculateFee(feeTable, orderFields, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      Xcx2FeeTable::Type::HIDDEN_PASSIVE, Xcx2FeeTable::PriceClass::SUB_DIME);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
}

void Xcx2FeeHandlingTester::TestEmptyLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  auto orderFields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, Xcx2FeeTable::Type::PASSIVE,
    Xcx2FeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, orderFields, LiquidityFlag::NONE,
    CalculateFee, expectedFee);
}
