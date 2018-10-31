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
  auto GetTestSecurity() {
    return Security("TST", DefaultMarkets::TSX(), DefaultCountries::CA());
  }

  auto BuildOrderFields(Money price) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      GetTestSecurity(), DefaultCurrencies::CAD(), Side::BID,
      DefaultDestinations::CHIX(), 100, price);
    return fields;
  }
}

void ChicFeeHandlingTester::TestFeeTableCalculations() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  TestFeeTableIndex(feeTable, feeTable.m_securityTable, LookupFee,
    ChicFeeTable::INDEX_COUNT, ChicFeeTable::CLASSIFICATION_COUNT);
}

void ChicFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  fields.m_quantity = 0;
  auto expectedFee = Money::ZERO;
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
    CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestDefaultActive() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
    ChicFeeTable::Classification::NON_INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, "R", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestDefaultPassive() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::PASSIVE,
    ChicFeeTable::Classification::NON_INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, "A", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestDefaultHiddenPassive() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_PASSIVE,
    ChicFeeTable::Classification::NON_INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestDefaultHiddenActive() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_ACTIVE,
    ChicFeeTable::Classification::NON_INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestSubDollarActive() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(20 * Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
    ChicFeeTable::Classification::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, "R", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestSubDollarPassive() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(20 * Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::PASSIVE,
    ChicFeeTable::Classification::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, "A", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestSubDimeActive() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
    ChicFeeTable::Classification::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, fields, "R", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestSubDimePassive() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::PASSIVE,
    ChicFeeTable::Classification::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, fields, "A", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestSubDollarHiddenActive() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(20 * Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_ACTIVE,
    ChicFeeTable::Classification::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestSubDollarHiddenPassive() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(20 * Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_PASSIVE,
    ChicFeeTable::Classification::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestSubDimeHiddenActive() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_ACTIVE,
    ChicFeeTable::Classification::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestSubDimeHiddenPassive() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_PASSIVE,
    ChicFeeTable::Classification::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestInterlistedActive() {
  auto feeTable = ChicFeeTable();
  feeTable.m_interlisted.insert(GetTestSecurity());
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
    ChicFeeTable::Classification::INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, "R", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestInterlistedPassive() {
  auto feeTable = ChicFeeTable();
  feeTable.m_interlisted.insert(GetTestSecurity());
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::PASSIVE,
    ChicFeeTable::Classification::INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, "A", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestInterlistedHiddenPassive() {
  auto feeTable = ChicFeeTable();
  feeTable.m_interlisted.insert(GetTestSecurity());
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_PASSIVE,
    ChicFeeTable::Classification::INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestInterlistedHiddenActive() {
  auto feeTable = ChicFeeTable();
  feeTable.m_interlisted.insert(GetTestSecurity());
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_ACTIVE,
    ChicFeeTable::Classification::INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestSubDollarInterlistedActive() {
  auto feeTable = ChicFeeTable();
  feeTable.m_interlisted.insert(GetTestSecurity());
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(20 * Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
    ChicFeeTable::Classification::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, "R", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestSubDimeInterlistedActive() {
  auto feeTable = ChicFeeTable();
  feeTable.m_interlisted.insert(GetTestSecurity());
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
    ChicFeeTable::Classification::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, fields, "R", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestEtfActive() {
  auto feeTable = ChicFeeTable();
  feeTable.m_etfs.insert(GetTestSecurity());
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
    ChicFeeTable::Classification::ETF);
  TestPerShareFeeCalculation(feeTable, fields, "R", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestEtfPassive() {
  auto feeTable = ChicFeeTable();
  feeTable.m_etfs.insert(GetTestSecurity());
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::PASSIVE,
    ChicFeeTable::Classification::ETF);
  TestPerShareFeeCalculation(feeTable, fields, "A", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestEtfHiddenPassive() {
  auto feeTable = ChicFeeTable();
  feeTable.m_etfs.insert(GetTestSecurity());
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_PASSIVE,
    ChicFeeTable::Classification::ETF);
  TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestEtfHiddenActive() {
  auto feeTable = ChicFeeTable();
  feeTable.m_etfs.insert(GetTestSecurity());
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_ACTIVE,
    ChicFeeTable::Classification::ETF);
  TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestSubDollarEtfActive() {
  auto feeTable = ChicFeeTable();
  feeTable.m_etfs.insert(GetTestSecurity());
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(20 * Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
    ChicFeeTable::Classification::SUB_DOLLAR);
  TestPerShareFeeCalculation(feeTable, fields, "R", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestSubDimeEtfActive() {
  auto feeTable = ChicFeeTable();
  feeTable.m_etfs.insert(GetTestSecurity());
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::CENT);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
    ChicFeeTable::Classification::SUB_DIME);
  TestPerShareFeeCalculation(feeTable, fields, "R", CalculateFee, expectedFee);
}

void ChicFeeHandlingTester::TestUnknownLiquidityFlag() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "AP";
    auto calculatedFee = CalculateFee(feeTable, fields, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::NON_INTERLISTED);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::CENT;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "PA";
    auto calculatedFee = CalculateFee(feeTable, fields, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      ChicFeeTable::Index::ACTIVE, ChicFeeTable::Classification::SUB_DIME);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "?";
    auto calculatedFee = CalculateFee(feeTable, fields, executionReport);
    auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
      ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::NON_INTERLISTED);
    CPPUNIT_ASSERT(calculatedFee == expectedFee);
  }
}

void ChicFeeHandlingTester::TestEmptyLiquidityFlag() {
  auto feeTable = ChicFeeTable();
  PopulateFeeTable(Store(feeTable.m_securityTable));
  auto fields = BuildOrderFields(Money::ONE);
  auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
    ChicFeeTable::Classification::NON_INTERLISTED);
  TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
    CalculateFee, expectedFee);
}
