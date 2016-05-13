#include "Nexus/FeeHandlingTests/NeoeFeeHandlingTester.hpp"
#include "Nexus/FeeHandling/NeoeFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;
using namespace std;

namespace {
  Security GetTestSecurity() {
    return Security{"TST", DefaultMarkets::TSX(), DefaultCountries::CA()};
  }

  OrderFields BuildOrderFields(Money price) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      GetTestSecurity(), DefaultCurrencies::CAD(), Side::BID,
      DefaultDestinations::NEOE(), 100, price);
    return fields;
  }

  NeoeFeeTable BuildFeeTable() {
    NeoeFeeTable feeTable;
    PopulateFeeTable(Store(feeTable.m_generalFeeTable));
    PopulateFeeTable(Store(feeTable.m_interlistedFeeTable));
    return feeTable;
  }
}

void NeoeFeeHandlingTester::TestFeeTableCalculations() {
  auto feeTable = BuildFeeTable();
  TestFeeTableIndex(feeTable, feeTable.m_generalFeeTable, LookupGeneralFee,
    LIQUIDITY_FLAG_COUNT, NeoeFeeTable::PRICE_CLASS_COUNT);
  TestFeeTableIndex(feeTable, feeTable.m_interlistedFeeTable,
    LookupInterlistedFee, LIQUIDITY_FLAG_COUNT,
    NeoeFeeTable::PRICE_CLASS_COUNT);
}

void NeoeFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = BuildFeeTable();
  auto orderFields = BuildOrderFields(Money::ONE);
  TestPerShareFeeCalculation(feeTable, orderFields.m_price, 0,
    LiquidityFlag::NONE, std::bind(&CalculateFee, std::placeholders::_1, false,
    orderFields, std::placeholders::_2), Money::ZERO);
  TestPerShareFeeCalculation(feeTable, orderFields.m_price, 0,
    LiquidityFlag::NONE, std::bind(&CalculateFee, std::placeholders::_1, true,
    orderFields, std::placeholders::_2), Money::ZERO);
}

void NeoeFeeHandlingTester::TestActive() {
  auto feeTable = BuildFeeTable();
  {
    auto orderFields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupGeneralFee(feeTable, LiquidityFlag::ACTIVE,
      NeoeFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, false, orderFields,
      std::placeholders::_2), expectedFee);
  }
  {
    auto orderFields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupInterlistedFee(feeTable, LiquidityFlag::ACTIVE,
      NeoeFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, true, orderFields,
      std::placeholders::_2), expectedFee);
  }
}

void NeoeFeeHandlingTester::TestPassive() {
  auto feeTable = BuildFeeTable();
  {
    auto orderFields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupGeneralFee(feeTable, LiquidityFlag::PASSIVE,
      NeoeFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
      false, orderFields, std::placeholders::_2), expectedFee);
  }
  {
    auto orderFields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupInterlistedFee(feeTable, LiquidityFlag::PASSIVE,
      NeoeFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
      true, orderFields, std::placeholders::_2), expectedFee);
  }
}

void NeoeFeeHandlingTester::TestSubDollarActive() {
  auto feeTable = BuildFeeTable();
  {
    auto orderFields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupGeneralFee(feeTable, LiquidityFlag::ACTIVE,
      NeoeFeeTable::PriceClass::SUB_DOLLAR);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
      false, orderFields, std::placeholders::_2), expectedFee);
  }
  {
    auto orderFields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupInterlistedFee(feeTable, LiquidityFlag::ACTIVE,
      NeoeFeeTable::PriceClass::SUB_DOLLAR);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
      true, orderFields, std::placeholders::_2), expectedFee);
  }
}

void NeoeFeeHandlingTester::TestSubDollarPassive() {
  auto feeTable = BuildFeeTable();
  {
    auto orderFields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupGeneralFee(feeTable, LiquidityFlag::PASSIVE,
      NeoeFeeTable::PriceClass::SUB_DOLLAR);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
      false, orderFields, std::placeholders::_2), expectedFee);
  }
  {
    auto orderFields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupInterlistedFee(feeTable, LiquidityFlag::PASSIVE,
      NeoeFeeTable::PriceClass::SUB_DOLLAR);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
      true, orderFields, std::placeholders::_2), expectedFee);
  }
}

void NeoeFeeHandlingTester::TestUnknownLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "AP";
    {
      auto orderFields = BuildOrderFields(Money::ONE);
      auto calculatedFee = CalculateFee(feeTable, false, orderFields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupGeneralFee(
        feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
    {
      auto orderFields = BuildOrderFields(Money::ONE);
      auto calculatedFee = CalculateFee(feeTable, true, orderFields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupInterlistedFee(
        feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::CENT;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "PA";
    {
      auto orderFields = BuildOrderFields(Money::CENT);
      auto calculatedFee = CalculateFee(feeTable, false, orderFields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupGeneralFee(
        feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::SUB_DOLLAR);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
    {
      auto orderFields = BuildOrderFields(Money::CENT);
      auto calculatedFee = CalculateFee(feeTable, true, orderFields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupInterlistedFee(
        feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::SUB_DOLLAR);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "?????";
    {
      auto orderFields = BuildOrderFields(Money::ONE);
      auto calculatedFee = CalculateFee(feeTable, false, orderFields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupGeneralFee(
        feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
    {
      auto orderFields = BuildOrderFields(Money::ONE);
      auto calculatedFee = CalculateFee(feeTable, true, orderFields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupInterlistedFee(
        feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
  }
}

void NeoeFeeHandlingTester::TestEmptyLiquidityFlag() {
  auto feeTable = BuildFeeTable();
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "";
    {
      auto orderFields = BuildOrderFields(Money::ONE);
      auto calculatedFee = CalculateFee(feeTable, false, orderFields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupGeneralFee(
        feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
    {
      auto orderFields = BuildOrderFields(Money::ONE);
      auto calculatedFee = CalculateFee(feeTable, true, orderFields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupInterlistedFee(
        feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
  }
  {
    auto executionReport = ExecutionReport::BuildInitialReport(0,
      boost::posix_time::second_clock::universal_time());
    executionReport.m_lastPrice = Money::CENT;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "";
    {
      auto orderFields = BuildOrderFields(Money::CENT);
      auto calculatedFee = CalculateFee(feeTable, false, orderFields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupGeneralFee(
        feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::SUB_DOLLAR);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
    {
      auto orderFields = BuildOrderFields(Money::CENT);
      auto calculatedFee = CalculateFee(feeTable, true, orderFields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupInterlistedFee(
        feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::SUB_DOLLAR);
      CPPUNIT_ASSERT(calculatedFee == expectedFee);
    }
  }
}
