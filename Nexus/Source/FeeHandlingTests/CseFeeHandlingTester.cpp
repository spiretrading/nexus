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
  auto MakeFeeTable() {
    auto feeTable = CseFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    return feeTable;
  }
}

TEST_SUITE("CseFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = MakeFeeTable();
    TestFeeTableIndex(feeTable, feeTable.m_feeTable, LookupFee,
      LIQUIDITY_FLAG_COUNT, CseFeeTable::PRICE_CLASS_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    TestPerShareFeeCalculation(
      feeTable, Money::ONE, 0, LiquidityFlag::NONE, &CalculateFee, Money::ZERO);
  }

  TEST_CASE("default_active") {
    auto feeTable = MakeFeeTable();
    auto expectedFee =
      LookupFee(feeTable, LiquidityFlag::ACTIVE, CseFeeTable::Section::DEFAULT);
    TestPerShareFeeCalculation(
      feeTable, Money::ONE, 100, "TC", &CalculateFee, expectedFee);
  }

  TEST_CASE("default_passive") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(
      feeTable, LiquidityFlag::PASSIVE, CseFeeTable::Section::DEFAULT);
    TestPerShareFeeCalculation(
      feeTable, Money::ONE, 100, "PC", &CalculateFee, expectedFee);
  }

  TEST_CASE("dark_active") {
    auto feeTable = MakeFeeTable();
    auto expectedFee =
      LookupFee(feeTable, LiquidityFlag::ACTIVE, CseFeeTable::Section::DARK);
    TestPerShareFeeCalculation(
      feeTable, Money::CENT, 100, "TCD", &CalculateFee, expectedFee);
  }

  TEST_CASE("dark_passive") {
    auto feeTable = MakeFeeTable();
    auto expectedFee =
      LookupFee(feeTable, LiquidityFlag::PASSIVE, CseFeeTable::Section::DARK);
    TestPerShareFeeCalculation(
      feeTable, Money::CENT, 100, "PCD", &CalculateFee, expectedFee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "TC";
      auto calculatedFee = CalculateFee(feeTable, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(
        feeTable, LiquidityFlag::ACTIVE, CseFeeTable::Section::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "TCD";
      auto calculatedFee = CalculateFee(feeTable, executionReport);
      auto expectedFee = executionReport.m_lastQuantity *
        LookupFee(feeTable, LiquidityFlag::ACTIVE,
          CseFeeTable::Section::DARK);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "?";
      auto calculatedFee = CalculateFee(feeTable, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(
        feeTable, LiquidityFlag::ACTIVE, CseFeeTable::Section::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      CseFeeTable::Section::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::NONE,
      &CalculateFee, expectedFee);
  }
}
