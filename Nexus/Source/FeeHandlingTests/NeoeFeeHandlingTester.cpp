#include <doctest/doctest.h>
#include "Nexus/FeeHandling/NeoeFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto GetTestSecurity() {
    return Security("TST", DefaultMarkets::TSX(), DefaultCountries::CA());
  }

  auto MakeOrderFields(Money price) {
    return OrderFields::MakeLimitOrder(DirectoryEntry::GetRootAccount(),
      GetTestSecurity(), DefaultCurrencies::CAD(), Side::BID,
      DefaultDestinations::NEOE(), 100, price);
  }

  auto MakeFeeTable() {
    auto feeTable = NeoeFeeTable();
    PopulateFeeTable(Store(feeTable.m_generalFeeTable));
    PopulateFeeTable(Store(feeTable.m_interlistedFeeTable));
    PopulateFeeTable(Store(feeTable.m_etfFeeTable));
    return feeTable;
  }

  auto GetFeeLookup(NeoeFeeTable::Classification classification) {
    if(classification == NeoeFeeTable::Classification::GENERAL) {
      return &LookupGeneralFee;
    } else if(classification == NeoeFeeTable::Classification::INTERLISTED) {
      return &LookupInterlistedFee;
    }
    return &LookupEtfFee;
  }
}

TEST_SUITE("NeoeFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = MakeFeeTable();
    TestFeeTableIndex(feeTable, feeTable.m_generalFeeTable, LookupGeneralFee,
      LIQUIDITY_FLAG_COUNT, NeoeFeeTable::PRICE_CLASS_COUNT);
    TestFeeTableIndex(feeTable, feeTable.m_interlistedFeeTable,
      LookupInterlistedFee, LIQUIDITY_FLAG_COUNT,
      NeoeFeeTable::PRICE_CLASS_COUNT);
    TestFeeTableIndex(feeTable, feeTable.m_etfFeeTable, LookupEtfFee,
      LIQUIDITY_FLAG_COUNT, NeoeFeeTable::PRICE_CLASS_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    auto orderFields = MakeOrderFields(Money::ONE);
    TestPerShareFeeCalculation(feeTable, orderFields.m_price, 0,
      LiquidityFlag::NONE, std::bind(&CalculateFee, std::placeholders::_1,
        NeoeFeeTable::Classification::GENERAL, orderFields,
        std::placeholders::_2), Money::ZERO);
    TestPerShareFeeCalculation(feeTable, orderFields.m_price, 0,
      LiquidityFlag::NONE, std::bind(&CalculateFee, std::placeholders::_1,
        NeoeFeeTable::Classification::INTERLISTED, orderFields,
        std::placeholders::_2), Money::ZERO);
    TestPerShareFeeCalculation(feeTable, orderFields.m_price, 0,
      LiquidityFlag::NONE, std::bind(&CalculateFee, std::placeholders::_1,
        NeoeFeeTable::Classification::ETF, orderFields, std::placeholders::_2),
      Money::ZERO);
  }

  TEST_CASE("active") {
    auto feeTable = MakeFeeTable();
    auto orderFields = MakeOrderFields(Money::ONE);
    for(auto classification : {NeoeFeeTable::Classification::GENERAL,
        NeoeFeeTable::Classification::INTERLISTED,
        NeoeFeeTable::Classification::ETF}) {
      auto expectedFee = GetFeeLookup(classification)(
        feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
      TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
        LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
          classification, orderFields, std::placeholders::_2), expectedFee);
    }
  }

  TEST_CASE("passive") {
    auto feeTable = MakeFeeTable();
    auto orderFields = MakeOrderFields(Money::ONE);
    for(auto classification : {NeoeFeeTable::Classification::GENERAL,
        NeoeFeeTable::Classification::INTERLISTED,
        NeoeFeeTable::Classification::ETF}) {
      auto expectedFee = GetFeeLookup(classification)(
        feeTable, LiquidityFlag::PASSIVE, NeoeFeeTable::PriceClass::DEFAULT);
      TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
        LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
          classification, orderFields, std::placeholders::_2), expectedFee);
    }
  }

  TEST_CASE("subdollar_active") {
    auto feeTable = MakeFeeTable();
    auto orderFields = MakeOrderFields(Money::CENT);
    for(auto classification : {NeoeFeeTable::Classification::GENERAL,
        NeoeFeeTable::Classification::INTERLISTED,
        NeoeFeeTable::Classification::ETF}) {
      auto expectedFee = GetFeeLookup(classification)(
        feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::SUBDOLLAR);
      TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
        LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
          classification, orderFields, std::placeholders::_2), expectedFee);
    }
  }

  TEST_CASE("subdollar_passive") {
    auto feeTable = MakeFeeTable();
    auto orderFields = MakeOrderFields(Money::CENT);
    for(auto classification : {NeoeFeeTable::Classification::GENERAL,
        NeoeFeeTable::Classification::INTERLISTED,
        NeoeFeeTable::Classification::ETF}) {
      auto expectedFee = GetFeeLookup(classification)(
        feeTable, LiquidityFlag::PASSIVE, NeoeFeeTable::PriceClass::SUBDOLLAR);
      TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
        LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
          classification, orderFields, std::placeholders::_2), expectedFee);
    }
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    {
      auto executionReport =
        ExecutionReport::MakeInitialReport(0, second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "AP";
      auto orderFields = MakeOrderFields(Money::ONE);
      for(auto classification : {NeoeFeeTable::Classification::GENERAL,
          NeoeFeeTable::Classification::INTERLISTED,
          NeoeFeeTable::Classification::ETF}) {
        auto calculatedFee =
          CalculateFee(feeTable, classification, orderFields, executionReport);
        auto expectedFee = executionReport.m_lastQuantity *
          GetFeeLookup(classification)(feeTable, LiquidityFlag::ACTIVE,
            NeoeFeeTable::PriceClass::DEFAULT);
        REQUIRE(calculatedFee == expectedFee);
      }
    }
    {
      auto executionReport =
        ExecutionReport::MakeInitialReport(0, second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "PA";
      auto orderFields = MakeOrderFields(Money::CENT);
      for(auto classification : {NeoeFeeTable::Classification::GENERAL,
          NeoeFeeTable::Classification::INTERLISTED,
          NeoeFeeTable::Classification::ETF}) {
        auto calculatedFee =
          CalculateFee(feeTable, classification, orderFields, executionReport);
        auto expectedFee = executionReport.m_lastQuantity *
          GetFeeLookup(classification)(feeTable, LiquidityFlag::ACTIVE,
            NeoeFeeTable::PriceClass::SUBDOLLAR);
        REQUIRE(calculatedFee == expectedFee);
      }
    }
    {
      auto executionReport =
        ExecutionReport::MakeInitialReport(0, second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "?????";
      auto orderFields = MakeOrderFields(Money::ONE);
      for(auto classification : {NeoeFeeTable::Classification::GENERAL,
          NeoeFeeTable::Classification::INTERLISTED,
          NeoeFeeTable::Classification::ETF}) {
        auto calculatedFee =
          CalculateFee(feeTable, classification, orderFields, executionReport);
        auto expectedFee = executionReport.m_lastQuantity *
          GetFeeLookup(classification)(feeTable, LiquidityFlag::ACTIVE,
            NeoeFeeTable::PriceClass::DEFAULT);
        REQUIRE(calculatedFee == expectedFee);
      }
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    {
      auto executionReport =
        ExecutionReport::MakeInitialReport(0, second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "";
      auto orderFields = MakeOrderFields(Money::ONE);
      for(auto classification : {NeoeFeeTable::Classification::GENERAL,
          NeoeFeeTable::Classification::INTERLISTED,
          NeoeFeeTable::Classification::ETF}) {
        auto calculatedFee =
          CalculateFee(feeTable, classification, orderFields, executionReport);
        auto expectedFee = executionReport.m_lastQuantity *
          GetFeeLookup(classification)(feeTable, LiquidityFlag::ACTIVE,
            NeoeFeeTable::PriceClass::DEFAULT);
        REQUIRE(calculatedFee == expectedFee);
      }
    }
    {
      auto executionReport =
        ExecutionReport::MakeInitialReport(0, second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "";
      auto orderFields = MakeOrderFields(Money::CENT);
      for(auto classification : {NeoeFeeTable::Classification::GENERAL,
          NeoeFeeTable::Classification::INTERLISTED,
          NeoeFeeTable::Classification::ETF}) {
        auto calculatedFee =
          CalculateFee(feeTable, classification, orderFields, executionReport);
        auto expectedFee = executionReport.m_lastQuantity *
          GetFeeLookup(classification)(feeTable, LiquidityFlag::ACTIVE,
            NeoeFeeTable::PriceClass::SUBDOLLAR);
        REQUIRE(calculatedFee == expectedFee);
      }
    }
  }
}
