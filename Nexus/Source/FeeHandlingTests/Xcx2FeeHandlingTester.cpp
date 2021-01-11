#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/Xcx2FeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto MakeFeeTable() {
    auto feeTable = Xcx2FeeTable();
    PopulateFeeTable(Store(feeTable.m_defaultTable));
    PopulateFeeTable(Store(feeTable.m_tsxTable));
    feeTable.m_largeTradeSize = 1000;
    return feeTable;
  }

  auto GetTsxSecurity() {
    return Security("TST", DefaultMarkets::TSX(), DefaultCountries::CA());
  }

  auto GetDefaultSecurity() {
    return Security("TST2", DefaultMarkets::TSXV(), DefaultCountries::CA());
  }

  auto MakeOrderFields(Money price) {
    auto fields = OrderFields::MakeLimitOrder(DirectoryEntry::GetRootAccount(),
      GetDefaultSecurity(), DefaultCurrencies::CAD(), Side::BID,
      DefaultDestinations::CX2(), 100, price);
    return fields;
  }

  auto MakeTsxOrderFields(Money price) {
    auto fields = OrderFields::MakeLimitOrder(DirectoryEntry::GetRootAccount(),
      GetTsxSecurity(), DefaultCurrencies::CAD(), Side::BID,
      DefaultDestinations::CX2(), 100, price);
    return fields;
  }
}

TEST_SUITE("Xcx2FeeHandling") {
  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    fields.m_quantity = 0;
    auto expectedFee = Money::ZERO;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("active_default") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::ACTIVE,
      Xcx2FeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("passive_default") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::PASSIVE,
      Xcx2FeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("large_active_default") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    fields.m_quantity = feeTable.m_largeTradeSize;
    auto expectedFee = LookupFee(feeTable, fields,
      Xcx2FeeTable::Type::LARGE_ACTIVE, Xcx2FeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("large_passive_default") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    fields.m_quantity = feeTable.m_largeTradeSize;
    auto expectedFee = LookupFee(feeTable, fields,
      Xcx2FeeTable::Type::LARGE_PASSIVE, Xcx2FeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("hidden_active_over_dollar") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, fields,
      Xcx2FeeTable::Type::HIDDEN_ACTIVE, Xcx2FeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee,
      expectedFee);
  }

  TEST_CASE("hidden_passive_over_dollar") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, fields,
      Xcx2FeeTable::Type::HIDDEN_PASSIVE, Xcx2FeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee,
      expectedFee);
  }

  TEST_CASE("active_odd_lot_over_dollar") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    fields.m_quantity = 50;
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("passive_odd_lot_over_dollar") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    fields.m_quantity = 50;
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("active_subdollar") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::ACTIVE,
      Xcx2FeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("passive_subdollar") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::PASSIVE,
      Xcx2FeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("active_subdime") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::ACTIVE,
      Xcx2FeeTable::PriceClass::SUBDIME);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("passive_subdime") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::PASSIVE,
      Xcx2FeeTable::PriceClass::SUBDIME);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("large_active_subdollar") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields,
      Xcx2FeeTable::Type::LARGE_ACTIVE, Xcx2FeeTable::PriceClass::SUBDOLLAR);
    fields.m_quantity = feeTable.m_largeTradeSize;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("large_passive_subdollar") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields,
      Xcx2FeeTable::Type::LARGE_PASSIVE, Xcx2FeeTable::PriceClass::SUBDOLLAR);
    fields.m_quantity = feeTable.m_largeTradeSize;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("large_active_subdime") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::CENT);
    fields.m_quantity = feeTable.m_largeTradeSize;
    auto expectedFee = LookupFee(feeTable, fields,
      Xcx2FeeTable::Type::LARGE_ACTIVE, Xcx2FeeTable::PriceClass::SUBDIME);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("large_passive_subdime") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::CENT);
    fields.m_quantity = feeTable.m_largeTradeSize;
    auto expectedFee = LookupFee(feeTable, fields,
      Xcx2FeeTable::Type::LARGE_PASSIVE, Xcx2FeeTable::PriceClass::SUBDIME);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("hidden_active_subdollar") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields,
      Xcx2FeeTable::Type::HIDDEN_ACTIVE, Xcx2FeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, "r",
      CalculateFee, expectedFee);
  }

  TEST_CASE("hidden_passive_subdollar") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields,
      Xcx2FeeTable::Type::HIDDEN_PASSIVE, Xcx2FeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee,
      expectedFee);
  }

  TEST_CASE("hidden_active_subdime") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields,
      Xcx2FeeTable::Type::HIDDEN_ACTIVE, Xcx2FeeTable::PriceClass::SUBDIME);
    TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee,
      expectedFee);
  }

  TEST_CASE("hidden_passive_subdime") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields,
      Xcx2FeeTable::Type::HIDDEN_PASSIVE, Xcx2FeeTable::PriceClass::SUBDIME);
    TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee,
      expectedFee);
  }

  TEST_CASE("active_odd_lot_subdollar") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::SUBDOLLAR);
    fields.m_quantity = 50;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("passive_odd_lot_subdollar") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::SUBDOLLAR);
    fields.m_quantity = 50;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("active_odd_lot_subdime") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::SUBDIME);
    fields.m_quantity = 50;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("passive_odd_lot_subdime") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::SUBDIME);
    fields.m_quantity = 50;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("hidden_odd_lot") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::ODD_LOT,
      Xcx2FeeTable::PriceClass::SUBDOLLAR);
    fields.m_quantity = 50;
    TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee,
      expectedFee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = 20 * Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "AP";
      auto fields = MakeOrderFields(20 * Money::CENT);
      auto calculatedFee = CalculateFee(feeTable, fields, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        fields, Xcx2FeeTable::Type::PASSIVE,
        Xcx2FeeTable::PriceClass::SUBDOLLAR);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "PA";
      auto fields = MakeOrderFields(Money::ONE);
      auto calculatedFee = CalculateFee(feeTable, fields, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        fields, Xcx2FeeTable::Type::PASSIVE, Xcx2FeeTable::PriceClass::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "?";
      auto fields = MakeOrderFields(Money::CENT);
      auto calculatedFee = CalculateFee(feeTable, fields, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        fields, Xcx2FeeTable::Type::PASSIVE,
        Xcx2FeeTable::PriceClass::SUBDIME);
      REQUIRE(calculatedFee == expectedFee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    auto fields = MakeOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, fields, Xcx2FeeTable::Type::PASSIVE,
      Xcx2FeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
      CalculateFee, expectedFee);
  }
}
