#include "Nexus/FeeHandlingTests/OmgaFeeHandlingTester.hpp"
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/FeeHandling/OmgaFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;
using namespace std;

namespace {
  OrderFields BuildOrderFields(Money price, Quantity quantity) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{"TST", DefaultMarkets::TSX(), DefaultCountries::CA()},
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::OMEGA(),
      quantity, price);
    return fields;
  }

  OrderFields BuildOrderFields(Money price) {
    return BuildOrderFields(price, 100);
  }

  OrderFields BuildHiddenOrderFields(Money price, Quantity quantity) {
    auto fields = BuildOrderFields(price, quantity);
    fields.m_type = OrderType::PEGGED;
    fields.m_additionalFields.emplace_back(18, "M");
    return fields;
  }

  OrderFields BuildHiddenOrderFields(Money price) {
    return BuildHiddenOrderFields(price, 100);
  }
}

void OmgaFeeHandlingTester::TestFeeTableCalculations() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  TestFeeTableIndex(feeTable, feeTable.m_feeTable, LookupFee,
    OmgaFeeTable::TYPE_COUNT, OmgaFeeTable::PRICE_CLASS_COUNT);
}

void OmgaFeeHandlingTester::TestZeroQuantity() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  fields.m_quantity = 0;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
    std::bind(&CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), Money::ZERO);
}

void OmgaFeeHandlingTester::TestDefaultActive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ACTIVE,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestDefaultPassive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::PASSIVE,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestDefaultHiddenPassive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::HIDDEN_PASSIVE,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestDefaultHiddenActive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::HIDDEN_ACTIVE,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestEtfActive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ETF_ACTIVE,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestEtfPassive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ETF_PASSIVE,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestOddLotActive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE, 50);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestOddLotPassive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE, 50);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestSubDollarActive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ACTIVE,
    OmgaFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestSubDollarPassive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::PASSIVE,
    OmgaFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestSubDollarHiddenPassive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::HIDDEN_PASSIVE,
    OmgaFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestSubDollarHiddenActive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::HIDDEN_ACTIVE,
    OmgaFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestSubDollarEtfActive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ETF_ACTIVE,
    OmgaFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestSubDollarEtfPassive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ETF_PASSIVE,
    OmgaFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestSubDollarOddLotActive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::CENT, 50);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
    OmgaFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestSubDollarOddLotPassive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::CENT, 50);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
    OmgaFeeTable::PriceClass::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, false,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestHiddenEtfActive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::HIDDEN_ACTIVE,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestHiddenEtfPassive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::HIDDEN_PASSIVE,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestOddLotEtfActive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE, 50);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestOddLotEtfPassive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE, 50);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestOddLotHiddenEtfActive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::ONE, 50);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestOddLotHiddenEtfPassive() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildHiddenOrderFields(Money::ONE, 50);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
    std::bind(&CalculateFee, std::placeholders::_1, true,
    std::placeholders::_2, std::placeholders::_3), expectedFee);
}

void OmgaFeeHandlingTester::TestUnknownLiquidityFlag() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "AP";
    auto calculatedFee = CalculateFee(feeTable, false, fields, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      OmgaFeeTable::Type::PASSIVE, OmgaFeeTable::PriceClass::DEFAULT);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::CENT;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "PA";
    auto calculatedFee = CalculateFee(feeTable, false, fields, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      OmgaFeeTable::Type::PASSIVE, OmgaFeeTable::PriceClass::SUB_DOLLAR);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "?";
    auto calculatedFee = CalculateFee(feeTable, true, fields, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      OmgaFeeTable::Type::ETF_PASSIVE, OmgaFeeTable::PriceClass::DEFAULT);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
}

void OmgaFeeHandlingTester::TestEmptyLiquidityFlag() {
  OmgaFeeTable feeTable;
  PopulateFeeTable(Store(feeTable.m_feeTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::PASSIVE,
    OmgaFeeTable::PriceClass::DEFAULT);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
    std::bind(CalculateFee, std::placeholders::_1, false, std::placeholders::_2,
    std::placeholders::_3), expectedFee);
}
