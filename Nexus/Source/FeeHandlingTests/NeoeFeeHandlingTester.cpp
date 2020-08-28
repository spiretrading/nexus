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

  auto BuildOrderFields(Money price) {
    return OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      GetTestSecurity(), DefaultCurrencies::CAD(), Side::BID,
      DefaultDestinations::NEOE(), 100, price);
  }

  auto BuildFeeTable() {
    auto feeTable = NeoeFeeTable();
    PopulateFeeTable(Store(feeTable.m_generalFeeTable));
    PopulateFeeTable(Store(feeTable.m_interlistedFeeTable));
    return feeTable;
  }
}

TEST_SUITE("NeoeFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = BuildFeeTable();
    TestFeeTableIndex(feeTable, feeTable.m_generalFeeTable, LookupGeneralFee,
      LIQUIDITY_FLAG_COUNT, NeoeFeeTable::PRICE_CLASS_COUNT);
    TestFeeTableIndex(feeTable, feeTable.m_interlistedFeeTable,
      LookupInterlistedFee, LIQUIDITY_FLAG_COUNT,
      NeoeFeeTable::PRICE_CLASS_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = BuildFeeTable();
    auto orderFields = BuildOrderFields(Money::ONE);
    TestPerShareFeeCalculation(feeTable, orderFields.m_price, 0,
      LiquidityFlag::NONE, std::bind(&CalculateFee, std::placeholders::_1,
      false, orderFields, std::placeholders::_2), Money::ZERO);
    TestPerShareFeeCalculation(feeTable, orderFields.m_price, 0,
      LiquidityFlag::NONE, std::bind(&CalculateFee, std::placeholders::_1, true,
      orderFields, std::placeholders::_2), Money::ZERO);
  }

  TEST_CASE("active") {
    auto feeTable = BuildFeeTable();
    {
      auto orderFields = BuildOrderFields(Money::ONE);
      auto expectedFee = LookupGeneralFee(feeTable, LiquidityFlag::ACTIVE,
        NeoeFeeTable::PriceClass::DEFAULT);
      TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
        LiquidityFlag::ACTIVE,
        std::bind(&CalculateFee, std::placeholders::_1, false, orderFields,
        std::placeholders::_2), expectedFee);
    }
    {
      auto orderFields = BuildOrderFields(Money::ONE);
      auto expectedFee = LookupInterlistedFee(feeTable, LiquidityFlag::ACTIVE,
        NeoeFeeTable::PriceClass::DEFAULT);
      TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
        LiquidityFlag::ACTIVE,
        std::bind(&CalculateFee, std::placeholders::_1, true, orderFields,
        std::placeholders::_2), expectedFee);
    }
  }

  TEST_CASE("passive") {
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

  TEST_CASE("subdollar_active") {
    auto feeTable = BuildFeeTable();
    {
      auto orderFields = BuildOrderFields(Money::CENT);
      auto expectedFee = LookupGeneralFee(feeTable, LiquidityFlag::ACTIVE,
        NeoeFeeTable::PriceClass::SUBDOLLAR);
      TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
        LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
        false, orderFields, std::placeholders::_2), expectedFee);
    }
    {
      auto orderFields = BuildOrderFields(Money::CENT);
      auto expectedFee = LookupInterlistedFee(feeTable, LiquidityFlag::ACTIVE,
        NeoeFeeTable::PriceClass::SUBDOLLAR);
      TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
        LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
        true, orderFields, std::placeholders::_2), expectedFee);
    }
  }

  TEST_CASE("subdollar_passive") {
    auto feeTable = BuildFeeTable();
    {
      auto orderFields = BuildOrderFields(Money::CENT);
      auto expectedFee = LookupGeneralFee(feeTable, LiquidityFlag::PASSIVE,
        NeoeFeeTable::PriceClass::SUBDOLLAR);
      TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
        LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        false, orderFields, std::placeholders::_2), expectedFee);
    }
    {
      auto orderFields = BuildOrderFields(Money::CENT);
      auto expectedFee = LookupInterlistedFee(feeTable, LiquidityFlag::PASSIVE,
        NeoeFeeTable::PriceClass::SUBDOLLAR);
      TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
        LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        true, orderFields, std::placeholders::_2), expectedFee);
    }
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = BuildFeeTable();
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "AP";
      {
        auto orderFields = BuildOrderFields(Money::ONE);
        auto calculatedFee = CalculateFee(feeTable, false, orderFields,
          executionReport);
        auto expectedFee = executionReport.m_lastQuantity * LookupGeneralFee(
          feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
        REQUIRE(calculatedFee == expectedFee);
      }
      {
        auto orderFields = BuildOrderFields(Money::ONE);
        auto calculatedFee = CalculateFee(feeTable, true, orderFields,
          executionReport);
        auto expectedFee = executionReport.m_lastQuantity *
          LookupInterlistedFee(feeTable, LiquidityFlag::ACTIVE,
          NeoeFeeTable::PriceClass::DEFAULT);
        REQUIRE(calculatedFee == expectedFee);
      }
    }
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "PA";
      {
        auto orderFields = BuildOrderFields(Money::CENT);
        auto calculatedFee = CalculateFee(feeTable, false, orderFields,
          executionReport);
        auto expectedFee = executionReport.m_lastQuantity * LookupGeneralFee(
          feeTable, LiquidityFlag::ACTIVE,
          NeoeFeeTable::PriceClass::SUBDOLLAR);
        REQUIRE(calculatedFee == expectedFee);
      }
      {
        auto orderFields = BuildOrderFields(Money::CENT);
        auto calculatedFee = CalculateFee(feeTable, true, orderFields,
          executionReport);
        auto expectedFee = executionReport.m_lastQuantity *
          LookupInterlistedFee(feeTable, LiquidityFlag::ACTIVE,
          NeoeFeeTable::PriceClass::SUBDOLLAR);
        REQUIRE(calculatedFee == expectedFee);
      }
    }
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "?????";
      {
        auto orderFields = BuildOrderFields(Money::ONE);
        auto calculatedFee = CalculateFee(feeTable, false, orderFields,
          executionReport);
        auto expectedFee = executionReport.m_lastQuantity * LookupGeneralFee(
          feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
        REQUIRE(calculatedFee == expectedFee);
      }
      {
        auto orderFields = BuildOrderFields(Money::ONE);
        auto calculatedFee = CalculateFee(feeTable, true, orderFields,
          executionReport);
        auto expectedFee = executionReport.m_lastQuantity *
          LookupInterlistedFee(feeTable, LiquidityFlag::ACTIVE,
          NeoeFeeTable::PriceClass::DEFAULT);
        REQUIRE(calculatedFee == expectedFee);
      }
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = BuildFeeTable();
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "";
      {
        auto orderFields = BuildOrderFields(Money::ONE);
        auto calculatedFee = CalculateFee(feeTable, false, orderFields,
          executionReport);
        auto expectedFee = executionReport.m_lastQuantity * LookupGeneralFee(
          feeTable, LiquidityFlag::ACTIVE, NeoeFeeTable::PriceClass::DEFAULT);
        REQUIRE(calculatedFee == expectedFee);
      }
      {
        auto orderFields = BuildOrderFields(Money::ONE);
        auto calculatedFee = CalculateFee(feeTable, true, orderFields,
          executionReport);
        auto expectedFee = executionReport.m_lastQuantity *
          LookupInterlistedFee(feeTable, LiquidityFlag::ACTIVE,
          NeoeFeeTable::PriceClass::DEFAULT);
        REQUIRE(calculatedFee == expectedFee);
      }
    }
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "";
      {
        auto orderFields = BuildOrderFields(Money::CENT);
        auto calculatedFee = CalculateFee(feeTable, false, orderFields,
          executionReport);
        auto expectedFee = executionReport.m_lastQuantity * LookupGeneralFee(
          feeTable, LiquidityFlag::ACTIVE,
          NeoeFeeTable::PriceClass::SUBDOLLAR);
        REQUIRE(calculatedFee == expectedFee);
      }
      {
        auto orderFields = BuildOrderFields(Money::CENT);
        auto calculatedFee = CalculateFee(feeTable, true, orderFields,
          executionReport);
        auto expectedFee = executionReport.m_lastQuantity *
          LookupInterlistedFee(feeTable, LiquidityFlag::ACTIVE,
          NeoeFeeTable::PriceClass::SUBDOLLAR);
        REQUIRE(calculatedFee == expectedFee);
      }
    }
  }
}
