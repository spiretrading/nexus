#include <doctest/doctest.h>
#include "Nexus/FeeHandling/CseFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto BuildFeeTable() {
    auto feeTable = CseFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    return feeTable;
  }
}

TEST_SUITE("CseFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = BuildFeeTable();
    TestFeeTableIndex(feeTable, feeTable.m_feeTable, LookupFee,
      LIQUIDITY_FLAG_COUNT, CseFeeTable::PRICE_CLASS_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = BuildFeeTable();
    TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
      &CalculateFee, Money::ZERO);
  }

  TEST_CASE("default_active") {
    auto feeTable = BuildFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      CseFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      &CalculateFee, expectedFee);
  }

  TEST_CASE("default_passive") {
    auto feeTable = BuildFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      CseFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
      LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdollar_active") {
    auto feeTable = BuildFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      CseFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, 10 * Money::CENT, 100,
      LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdollar_passive") {
    auto feeTable = BuildFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      CseFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, 10 * Money::CENT, 100,
      LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdime_active") {
    auto feeTable = BuildFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      CseFeeTable::PriceClass::SUBDIME);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdime_passive") {
    auto feeTable = BuildFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      CseFeeTable::PriceClass::SUBDIME);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = BuildFeeTable();
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "AP";
      auto calculatedFee = CalculateFee(feeTable, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(
        feeTable, LiquidityFlag::ACTIVE, CseFeeTable::PriceClass::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "PA";
      auto calculatedFee = CalculateFee(feeTable, executionReport);
      auto expectedFee = executionReport.m_lastQuantity *
        LookupFee(feeTable, LiquidityFlag::ACTIVE,
        CseFeeTable::PriceClass::SUBDIME);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "?";
      auto calculatedFee = CalculateFee(feeTable, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(
        feeTable, LiquidityFlag::ACTIVE, CseFeeTable::PriceClass::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = BuildFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      CseFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::NONE,
      &CalculateFee, expectedFee);
  }
}
