#include <doctest/doctest.h>
#include "Nexus/FeeHandling/LynxFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto LookupLiquidityFee(const LynxFeeTable& feeTable,
      LiquidityFlag liquidityFlag, LynxFeeTable::PriceClass priceClass) {
    return LookupFee(feeTable, liquidityFlag, priceClass);
  }

  auto MakeFeeTable() {
    auto feeTable = LynxFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    return feeTable;
  }
}

TEST_SUITE("LynxFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = MakeFeeTable();
    TestFeeTableIndex(feeTable, feeTable.m_feeTable, LookupLiquidityFee,
      LIQUIDITY_FLAG_COUNT, LynxFeeTable::PRICE_CLASS_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
      &CalculateFee, Money::ZERO);
  }

  TEST_CASE("active") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      LynxFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      &CalculateFee, expectedFee);
  }

  TEST_CASE("passive") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      LynxFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
      LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdollar_active") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      LynxFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdollar_passive") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      LynxFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "AP";
      auto calculatedFee = CalculateFee(feeTable, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        LiquidityFlag::ACTIVE, LynxFeeTable::PriceClass::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "PA";
      auto calculatedFee = CalculateFee(feeTable, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        LiquidityFlag::ACTIVE, LynxFeeTable::PriceClass::SUBDOLLAR);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "?";
      auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
        LynxFeeTable::PriceClass::DEFAULT);
      TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
        LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "";
      auto calculatedFee = CalculateFee(feeTable, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        LiquidityFlag::ACTIVE, LynxFeeTable::PriceClass::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "";
      auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
        LynxFeeTable::PriceClass::DEFAULT);
      TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
        LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
    }
  }
}
