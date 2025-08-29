#include <doctest/doctest.h>
#include "Nexus/FeeHandling/CxdFeeTable.hpp"

using namespace Nexus;
using namespace Nexus::OrderExecutionService;

namespace {
  ExecutionReport MakeExecutionReport(
      Money price, Quantity quantity, std::string liquidityFlag) {
    auto report = ExecutionReport::MakeInitialReport(0, {});
    report.m_lastPrice = price;
    report.m_lastQuantity = quantity;
    report.m_liquidityFlag = std::move(liquidityFlag);
    return report;
  }

  CxdFeeTable MakeFeeTable() {
    auto table = CxdFeeTable();
    for(auto l = 0; l < LIQUIDITY_FLAG_COUNT; ++l) {
      for(auto c = 0; c < CxdFeeTable::BBO_PRICE_PAIR_COUNT; ++c) {
        table.m_feeTable[l][c] = Money(l * 100 + c);
        table.m_maxFeeTable[l][c] = Money(1000 + l * 100 + c);
        table.m_etfFeeTable[l][c] = Money(10000 + l * 100 + c);
      }
    }
    return table;
  }
}

TEST_SUITE("CxdFeeTable") {
  TEST_CASE("GetCxdBboType") {
    REQUIRE(GetCxdBboType("a") == CxdFeeTable::BboType::INSIDE_BBO);
    REQUIRE(GetCxdBboType("r") == CxdFeeTable::BboType::INSIDE_BBO);
    REQUIRE(GetCxdBboType("d") == CxdFeeTable::BboType::AT_BBO);
    REQUIRE(GetCxdBboType("D") == CxdFeeTable::BboType::AT_BBO);
    REQUIRE(GetCxdBboType("x") == CxdFeeTable::BboType::NONE);
    REQUIRE(GetCxdBboType("") == CxdFeeTable::BboType::NONE);
  }

  TEST_CASE("GetCxdLiquidityFlag") {
    REQUIRE(GetCxdLiquidityFlag("a") == LiquidityFlag::PASSIVE);
    REQUIRE(GetCxdLiquidityFlag("d") == LiquidityFlag::PASSIVE);
    REQUIRE(GetCxdLiquidityFlag("r") == LiquidityFlag::ACTIVE);
    REQUIRE(GetCxdLiquidityFlag("D") == LiquidityFlag::ACTIVE);
    REQUIRE(GetCxdLiquidityFlag("x") == LiquidityFlag::NONE);
    REQUIRE(GetCxdLiquidityFlag("") == LiquidityFlag::NONE);
  }

  TEST_CASE("GetCxdPriceClass") {
    REQUIRE(GetCxdPriceClass(Money(0)) == CxdFeeTable::PriceClass::SUBDOLLAR);
    REQUIRE(
      GetCxdPriceClass(Money::CENT) == CxdFeeTable::PriceClass::SUBDOLLAR);
    REQUIRE(GetCxdPriceClass(Money::ONE - Money::CENT) ==
      CxdFeeTable::PriceClass::SUBDOLLAR);
    REQUIRE(GetCxdPriceClass(Money::ONE) == CxdFeeTable::PriceClass::SUBFIVE);
    REQUIRE(GetCxdPriceClass(Money(4) + Money::CENT) ==
      CxdFeeTable::PriceClass::SUBFIVE);
    REQUIRE(GetCxdPriceClass(Money(5)) == CxdFeeTable::PriceClass::DEFAULT);
    REQUIRE(GetCxdPriceClass(Money(100)) == CxdFeeTable::PriceClass::DEFAULT);
  }

  TEST_CASE("Lookup") {
    auto table = MakeFeeTable();
    REQUIRE(LookupFee(table, LiquidityFlag::ACTIVE,
      CxdFeeTable::SecurityClass::DEFAULT, CxdFeeTable::PriceClass::SUBDOLLAR,
      CxdFeeTable::BboType::AT_BBO) ==
        table.m_feeTable[static_cast<std::size_t>(LiquidityFlag::ACTIVE)][0]);
    REQUIRE(LookupFee(table, LiquidityFlag::PASSIVE,
      CxdFeeTable::SecurityClass::DEFAULT, CxdFeeTable::PriceClass::SUBFIVE,
      CxdFeeTable::BboType::INSIDE_BBO) ==
        table.m_feeTable[static_cast<std::size_t>(LiquidityFlag::PASSIVE)][
          CxdFeeTable::PRICE_CLASS_COUNT + 1]);
    REQUIRE(LookupFee(table, LiquidityFlag::ACTIVE,
      CxdFeeTable::SecurityClass::ETF, CxdFeeTable::PriceClass::DEFAULT,
      CxdFeeTable::BboType::INSIDE_BBO) ==
        table.m_etfFeeTable[static_cast<std::size_t>(LiquidityFlag::ACTIVE)][
          CxdFeeTable::PRICE_CLASS_COUNT + 2]);
    REQUIRE(LookupMaxFee(table, LiquidityFlag::PASSIVE,
      CxdFeeTable::PriceClass::DEFAULT, CxdFeeTable::BboType::AT_BBO) ==
        table.m_maxFeeTable[
          static_cast<std::size_t>(LiquidityFlag::PASSIVE)][2]);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    auto report = MakeExecutionReport(Money::ONE, 0, "a");
    auto fee =
      CalculateFee(feeTable, CxdFeeTable::SecurityClass::DEFAULT, report);
    REQUIRE(fee == Money::ZERO);
  }

  TEST_CASE("default_security_fee_below_max") {
    auto feeTable = MakeFeeTable();
    auto report = MakeExecutionReport(Money::ONE, 1, "a");
    auto liquidityFlag = GetCxdLiquidityFlag("a");
    auto bboType = GetCxdBboType("a");
    auto priceClass = GetCxdPriceClass(Money::ONE);
    auto perShareFee = LookupFee(feeTable, liquidityFlag,
      CxdFeeTable::SecurityClass::DEFAULT, priceClass, bboType);
    auto maxFee = LookupMaxFee(feeTable, liquidityFlag, priceClass, bboType);
    auto expectedFee = std::min(perShareFee, maxFee);
    auto fee =
      CalculateFee(feeTable, CxdFeeTable::SecurityClass::DEFAULT, report);
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("default_security_fee_above_max") {
    auto feeTable = MakeFeeTable();
    auto report = MakeExecutionReport(Money::ONE, 1000, "a");
    auto liquidityFlag = GetCxdLiquidityFlag("a");
    auto bboType = GetCxdBboType("a");
    auto priceClass = GetCxdPriceClass(Money::ONE);
    auto perShareFee = LookupFee(feeTable, liquidityFlag,
      CxdFeeTable::SecurityClass::DEFAULT, priceClass, bboType);
    auto maxFee = LookupMaxFee(feeTable, liquidityFlag, priceClass, bboType);
    auto expectedFee = std::min(1000 * perShareFee, maxFee);
    auto fee =
      CalculateFee(feeTable, CxdFeeTable::SecurityClass::DEFAULT, report);
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("etf_security_no_max_fee") {
    auto feeTable = MakeFeeTable();
    auto report = MakeExecutionReport(Money(5), 10, "D");
    auto liquidityFlag = GetCxdLiquidityFlag("D");
    auto bboType = GetCxdBboType("D");
    auto priceClass = GetCxdPriceClass(Money(5));
    auto perShareFee = LookupFee(feeTable, liquidityFlag,
      CxdFeeTable::SecurityClass::ETF, priceClass, bboType);
    auto expectedFee = 10 * perShareFee;
    auto fee = CalculateFee(feeTable, CxdFeeTable::SecurityClass::ETF, report);
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("unknown_liquidity_flag_defaults") {
    auto feeTable = MakeFeeTable();
    auto report = MakeExecutionReport(Money::ONE, 5, "x");
    auto liquidityFlag = LiquidityFlag::ACTIVE;
    auto bboType = CxdFeeTable::BboType::AT_BBO;
    auto priceClass = GetCxdPriceClass(Money::ONE);
    auto perShareFee = LookupFee(feeTable, liquidityFlag,
      CxdFeeTable::SecurityClass::DEFAULT, priceClass, bboType);
    auto maxFee = LookupMaxFee(feeTable, liquidityFlag, priceClass, bboType);
    auto expectedFee = std::min(5 * perShareFee, maxFee);
    auto fee =
      CalculateFee(feeTable, CxdFeeTable::SecurityClass::DEFAULT, report);
    REQUIRE(fee == expectedFee);
  }
}
