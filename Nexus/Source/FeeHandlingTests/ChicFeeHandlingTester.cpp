#include "Nexus/FeeHandlingTests/ChicFeeHandlingTester.hpp"
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/ChicFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;
using namespace std;

namespace {
  OrderFields BuildOrderFields(Money price) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{"TST", DefaultMarkets::TSX(), DefaultCountries::CA()},
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::CHIX(), 100,
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

void ChicFeeHandlingTester::TestFeeTableCalculations() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  TestFeeTableIndex(feeTable, feeTable.m_feeTable, LookupFee,
    ChicFeeTable::TYPE_COUNT, ChicFeeTable::CATEGORY_COUNT);
}

void ChicFeeHandlingTester::TestZeroQuantity() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  fields.m_quantity = 0;
  auto expectedFee = Money::ZERO;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
    std::bind(CalculateFee, std::placeholders::_1, false, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestDefaultActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestDefaultPassive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::PASSIVE,
    ChicFeeTable::Category::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestDefaultHiddenPassive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::HIDDEN,
    ChicFeeTable::Category::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestDefaultHiddenActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestSubDollarActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(20 * Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestSubDollarPassive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(20 * Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::PASSIVE,
    ChicFeeTable::Category::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestSubDimeActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestSubDimePassive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::PASSIVE,
    ChicFeeTable::Category::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestSubDollarHiddenActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(20 * Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestSubDimeHiddenActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestInterlistedActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestInterlistedPassive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::PASSIVE,
    ChicFeeTable::Category::INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestInterlistedHiddenPassive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::HIDDEN,
    ChicFeeTable::Category::INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestInterlistedHiddenActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestSubDollarInterlistedActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(20 * Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestSubDimeInterlistedActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, false, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestEtfActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::ETF);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, true, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestEtfPassive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::PASSIVE,
    ChicFeeTable::Category::ETF);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(CalculateFee, std::placeholders::_1, true, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestEtfHiddenPassive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::HIDDEN,
    ChicFeeTable::Category::ETF);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(CalculateFee, std::placeholders::_1, true, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestEtfHiddenActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::ETF);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, true, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestSubDollarEtfActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(20 * Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, true, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestSubDimeEtfActive() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(CalculateFee, std::placeholders::_1, true, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void ChicFeeHandlingTester::TestUnknownLiquidityFlag() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "AP";
    auto calculatedFee = CalculateFee(feeTable, false, false, fields,
      executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      ChicFeeTable::Type::ACTIVE, ChicFeeTable::Category::DEFAULT);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::CENT;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "PA";
    auto calculatedFee = CalculateFee(feeTable, false, false, fields,
      executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      ChicFeeTable::Type::ACTIVE, ChicFeeTable::Category::SUB_DIME);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "?";
    auto calculatedFee = CalculateFee(feeTable, false, false, fields,
      executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      ChicFeeTable::Type::ACTIVE, ChicFeeTable::Category::DEFAULT);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
}

void ChicFeeHandlingTester::TestEmptyLiquidityFlag() {
  ChicFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Type::ACTIVE,
    ChicFeeTable::Category::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
    std::bind(CalculateFee, std::placeholders::_1, false, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}
