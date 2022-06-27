#include <doctest/doctest.h>
#include "Nexus/FeeHandling/LynxFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  const auto DEFAULT_SECURITY =
    Security("TST", DefaultMarkets::TSX(), DefaultCountries::CA());

  const auto ETF_SECURITY =
    Security("ETF", DefaultMarkets::TSX(), DefaultCountries::CA());

  const auto INTERLISTED_SECURITY =
    Security("USA", DefaultMarkets::TSX(), DefaultCountries::CA());

  auto LookupLiquidityFee(const LynxFeeTable& feeTable,
      LiquidityFlag liquidityFlag,
      LynxFeeTable::Classification classification) {
    return LookupFee(feeTable, liquidityFlag, classification);
  }

  auto MakeFeeTable() {
    auto feeTable = LynxFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    feeTable.m_etfs.insert(ETF_SECURITY);
    feeTable.m_interlisted.insert(INTERLISTED_SECURITY);
    return feeTable;
  }

  auto MakeOrderFields(Security security, Money price, Quantity quantity) {
    return OrderFields::MakeLimitOrder(DirectoryEntry::GetRootAccount(),
      std::move(security), DefaultCurrencies::CAD(), Side::BID,
      DefaultDestinations::LYNX(), quantity, price);
  }

  auto MakeOrderFields(Money price, Quantity quantity) {
    return MakeOrderFields(DEFAULT_SECURITY, price, quantity);
  }

  auto MakeOrderFields(Money price) {
    return MakeOrderFields(price, 100);
  }

  auto MakeEtfOrderFields(Money price, Quantity quantity) {
    return MakeOrderFields(ETF_SECURITY, price, quantity);
  }

  auto MakeEtfOrderFields(Money price) {
    return MakeOrderFields(ETF_SECURITY, price, 100);
  }

  auto MakeInterlistedOrderFields(Money price, Quantity quantity) {
    return MakeOrderFields(INTERLISTED_SECURITY, price, quantity);
  }

  auto MakeInterlistedOrderFields(Money price) {
    return MakeOrderFields(INTERLISTED_SECURITY, price, 100);
  }

  auto MakeMidpointOrderFields(Money price, Quantity quantity) {
    auto fields = MakeOrderFields(DEFAULT_SECURITY, price, quantity);
    fields.m_type = OrderType::PEGGED;
    fields.m_additionalFields.emplace_back(18, "M");
    return fields;
  }

  auto MakeMidpointOrderFields(Money price) {
    return MakeMidpointOrderFields(price, 100);
  }
}

TEST_SUITE("LynxFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = MakeFeeTable();
    TestFeeTableIndex(feeTable, feeTable.m_feeTable, LookupLiquidityFee,
      LIQUIDITY_FLAG_COUNT, LynxFeeTable::CLASSIFICATION_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::NONE, 0, &CalculateFee, Money::ZERO);
  }

  TEST_CASE("active") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    auto expectedFee = LookupFee(
      feeTable, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::DEFAULT);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("active_etf") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeEtfOrderFields(Money::ONE);
    auto expectedFee = LookupFee(
      feeTable, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::ETF);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("active_interlisted") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeInterlistedOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      LynxFeeTable::Classification::INTERLISTED);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("active_midpoint") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeMidpointOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      LynxFeeTable::Classification::MIDPOINT);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("passive") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    auto expectedFee = LookupFee(
      feeTable, LiquidityFlag::PASSIVE, LynxFeeTable::Classification::DEFAULT);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("passive_etf") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeEtfOrderFields(Money::ONE);
    auto expectedFee = LookupFee(
      feeTable, LiquidityFlag::PASSIVE, LynxFeeTable::Classification::ETF);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("passive_interlisted") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeInterlistedOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      LynxFeeTable::Classification::INTERLISTED);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("passive_midpoint") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeMidpointOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      LynxFeeTable::Classification::MIDPOINT);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdollar_active") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::CENT);
    auto expectedFee = LookupFee(
      feeTable, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::SUBDOLLAR);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdollar_etf") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeEtfOrderFields(Money::CENT);
    auto expectedFee = LookupFee(
      feeTable, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::SUBDOLLAR);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdollar_interlisted") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeInterlistedOrderFields(Money::CENT);
    auto expectedFee = LookupFee(
      feeTable, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::SUBDOLLAR);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdollar_midpoint") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeMidpointOrderFields(Money::CENT);
    auto expectedFee = LookupFee(
      feeTable, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::MIDPOINT);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdollar_passive") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable,
      LiquidityFlag::PASSIVE, LynxFeeTable::Classification::SUBDOLLAR);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdollar_passive_etf") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeEtfOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable,
      LiquidityFlag::PASSIVE, LynxFeeTable::Classification::SUBDOLLAR);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdollar_passive_interlisted") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeInterlistedOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable,
      LiquidityFlag::PASSIVE, LynxFeeTable::Classification::SUBDOLLAR);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("subdollar_passive_midpoint") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeMidpointOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable,
      LiquidityFlag::PASSIVE, LynxFeeTable::Classification::MIDPOINT);
    TestPerShareFeeCalculation(
      feeTable, fields, LiquidityFlag::PASSIVE, &CalculateFee, expectedFee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "AP";
      auto calculatedFee = CalculateFee(feeTable, fields, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(
        feeTable, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "PA";
      auto calculatedFee = CalculateFee(feeTable, fields, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        LiquidityFlag::ACTIVE, LynxFeeTable::Classification::SUBDOLLAR);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "?";
      auto expectedFee = LookupFee(
        feeTable, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::DEFAULT);
      TestPerShareFeeCalculation(
        feeTable, fields, LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "";
      auto calculatedFee = CalculateFee(feeTable, fields, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(
        feeTable, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "";
      auto expectedFee = LookupFee(
        feeTable, LiquidityFlag::ACTIVE, LynxFeeTable::Classification::DEFAULT);
      TestPerShareFeeCalculation(
        feeTable, fields, LiquidityFlag::ACTIVE, &CalculateFee, expectedFee);
    }
  }
}
