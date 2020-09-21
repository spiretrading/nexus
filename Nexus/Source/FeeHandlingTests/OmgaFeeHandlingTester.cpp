#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <doctest/doctest.h>
#include "Nexus/FeeHandling/OmgaFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto BuildOrderFields(Money price, Quantity quantity) {
    return OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{"TST", DefaultMarkets::TSX(), DefaultCountries::CA()},
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::OMEGA(),
      quantity, price);
  }

  auto BuildOrderFields(Money price) {
    return BuildOrderFields(price, 100);
  }

  auto BuildHiddenOrderFields(Money price, Quantity quantity) {
    auto fields = BuildOrderFields(price, quantity);
    fields.m_type = OrderType::PEGGED;
    fields.m_additionalFields.emplace_back(18, "M");
    return fields;
  }

  auto BuildHiddenOrderFields(Money price) {
    return BuildHiddenOrderFields(price, 100);
  }
}

TEST_SUITE("OmgaFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    TestFeeTableIndex(feeTable, feeTable.m_feeTable, LookupFee,
      OmgaFeeTable::TYPE_COUNT, OmgaFeeTable::PRICE_CLASS_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::ONE);
    fields.m_quantity = 0;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
      std::bind(&CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), Money::ZERO);
  }

  TEST_CASE("default_active") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ACTIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("default_passive") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::PASSIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("default_hidden_passive") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildHiddenOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::HIDDEN_PASSIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("default_hidden_active") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildHiddenOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::HIDDEN_ACTIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("etf_active") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ETF_ACTIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, true,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("etf_passive") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ETF_PASSIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, true,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("odd_lot_active") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::ONE, 50);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("odd_lot_passive") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::ONE, 50);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("subdollar_active") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ACTIVE,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("subdollar_passive") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::PASSIVE,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("subdollar_hidden_passive") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildHiddenOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::HIDDEN_PASSIVE,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("subdollar_hidden_active") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildHiddenOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::HIDDEN_ACTIVE,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("subdollar_etf_active") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ETF_ACTIVE,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, true,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("subdollar_etf_passive") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ETF_PASSIVE,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, true,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("subdollar_odd_lot_active") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::CENT, 50);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("subdollar_odd_lot_passive") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::CENT, 50);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("hidden_etf_active") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildHiddenOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::HIDDEN_ACTIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, true,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("hidden_etf_passive") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildHiddenOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::HIDDEN_PASSIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, true,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("odd_lot_etf_active") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::ONE, 50);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, true,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("odd_lot_etf_passive") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::ONE, 50);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, true,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("odd_lot_hidden_etf_active") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildHiddenOrderFields(Money::ONE, 50);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, true,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("odd_lot_hidden_etf_passive") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildHiddenOrderFields(Money::ONE, 50);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::ODD_LOT,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, true,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::ONE);
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "AP";
      auto calculatedFee = CalculateFee(feeTable, false, fields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        OmgaFeeTable::Type::PASSIVE, OmgaFeeTable::PriceClass::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "PA";
      auto calculatedFee = CalculateFee(feeTable, false, fields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        OmgaFeeTable::Type::PASSIVE, OmgaFeeTable::PriceClass::SUBDOLLAR);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "?";
      auto calculatedFee = CalculateFee(feeTable, true, fields,
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        OmgaFeeTable::Type::ETF_PASSIVE, OmgaFeeTable::PriceClass::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = OmgaFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, OmgaFeeTable::Type::PASSIVE,
      OmgaFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
      std::bind(CalculateFee, std::placeholders::_1, false,
      std::placeholders::_2, std::placeholders::_3), expectedFee);
  }
}
