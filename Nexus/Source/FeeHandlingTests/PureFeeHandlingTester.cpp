#include <doctest/doctest.h>
#include "Nexus/FeeHandling/PureFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto MakeFeeTable() {
    auto feeTable = PureFeeTable();
    for(auto i = 0; i != PureFeeTable::SECTION_COUNT; ++i) {
      PopulateFeeTable(Store(feeTable.m_feeTable[i]));
    }
    return feeTable;
  }

  static const auto LIQUIDITY_INDICATOR = [] {
    auto indicators = std::map<LiquidityFlag, std::string>();
    indicators[LiquidityFlag::ACTIVE] = "T";
    indicators[LiquidityFlag::PASSIVE] = "P";
    return indicators;
  }();
}

TEST_SUITE("PureFeeHandling") {
  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    auto calculateDefaultFee =
      [] (const auto& feeTable, const auto& executionReport) {
        return CalculateFee(
          feeTable, PureFeeTable::Section::DEFAULT, executionReport);
      };
    auto calculateEtfFee =
      [] (const auto& feeTable, const auto& executionReport) {
        return CalculateFee(
          feeTable, PureFeeTable::Section::ETF, executionReport);
      };
    auto calculateInterlistedFee =
      [] (const auto& feeTable, const auto& executionReport) {
        return CalculateFee(
          feeTable, PureFeeTable::Section::INTERLISTED, executionReport);
      };
    TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
      calculateDefaultFee, Money::ZERO);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
      calculateEtfFee, Money::ZERO);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
      calculateInterlistedFee, Money::ZERO);
  }

  TEST_CASE("fills") {
    auto feeTable = MakeFeeTable();
    for(auto section : {PureFeeTable::Section::DEFAULT,
        PureFeeTable::Section::ETF, PureFeeTable::Section::INTERLISTED}) {
      auto calculateDefaultFee =
        [=] (const auto& feeTable, const auto& executionReport) {
          return CalculateFee(feeTable, section, executionReport);
        };
      SUBCASE("subdollar") {
        auto activeFee = LookupFee(feeTable, section,
          PureFeeTable::Row::SUBDOLLAR, LiquidityFlag::ACTIVE);
        TestPerShareFeeCalculation(feeTable, Money::CENT, 100, "TT",
          calculateDefaultFee, activeFee);
        auto passiveFee = LookupFee(feeTable, section,
          PureFeeTable::Row::SUBDOLLAR, LiquidityFlag::PASSIVE);
        TestPerShareFeeCalculation(feeTable, Money::CENT, 100, "PC",
          calculateDefaultFee, passiveFee);
      }
      SUBCASE("default") {
        auto activeFee = LookupFee(feeTable, section,
          PureFeeTable::Row::DEFAULT, LiquidityFlag::ACTIVE);
        TestPerShareFeeCalculation(feeTable, Money::ONE, 100, "TT",
          calculateDefaultFee, activeFee);
        auto passiveFee = LookupFee(feeTable, section,
          PureFeeTable::Row::DEFAULT, LiquidityFlag::PASSIVE);
        TestPerShareFeeCalculation(feeTable, Money::ONE, 100, "PC",
          calculateDefaultFee, passiveFee);
      }
      SUBCASE("dark_subdollar") {
        auto activeFee = LookupFee(feeTable, section,
          PureFeeTable::Row::DARK_SUBDOLLAR, LiquidityFlag::ACTIVE);
        TestPerShareFeeCalculation(feeTable, Money::CENT, 100, "TTD",
          calculateDefaultFee, activeFee);
        auto passiveFee = LookupFee(feeTable, section,
          PureFeeTable::Row::DARK_SUBDOLLAR, LiquidityFlag::PASSIVE);
        TestPerShareFeeCalculation(feeTable, Money::CENT, 100, "PCD",
          calculateDefaultFee, passiveFee);
      }
      SUBCASE("dark") {
        auto activeFee = LookupFee(feeTable, section,
          PureFeeTable::Row::DARK, LiquidityFlag::ACTIVE);
        TestPerShareFeeCalculation(feeTable, Money::ONE, 100, "TTD",
          calculateDefaultFee, activeFee);
        auto passiveFee = LookupFee(feeTable, section,
          PureFeeTable::Row::DARK, LiquidityFlag::PASSIVE);
        TestPerShareFeeCalculation(feeTable, Money::ONE, 100, "PCD",
          calculateDefaultFee, passiveFee);
      }
    }
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    auto executionReport =
      ExecutionReport::MakeInitialReport(0, second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "Q";
    auto calculatedFee =
      CalculateFee(feeTable, PureFeeTable::Section::DEFAULT, executionReport);
    auto expectedFee = executionReport.m_lastQuantity *
      LookupFee(feeTable, PureFeeTable::Section::DEFAULT,
        PureFeeTable::Row::DEFAULT, LiquidityFlag::ACTIVE);
    REQUIRE(calculatedFee == expectedFee);
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    auto executionReport =
      ExecutionReport::MakeInitialReport(0, second_clock::universal_time());
    executionReport.m_lastPrice = Money::ONE;
    executionReport.m_lastQuantity = 100;
    executionReport.m_liquidityFlag = "";
    auto calculatedFee =
      CalculateFee(feeTable, PureFeeTable::Section::DEFAULT, executionReport);
    auto expectedFee = executionReport.m_lastQuantity *
      LookupFee(feeTable, PureFeeTable::Section::DEFAULT,
        PureFeeTable::Row::DEFAULT, LiquidityFlag::ACTIVE);
    REQUIRE(calculatedFee == expectedFee);
  }
}
