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
  auto GetDesignatedSecurity() {
    return Security("DSG", DefaultMarkets::TSX(), DefaultCountries::CA());
  }

  auto GetTsxSecurity() {
    return Security("TSX", DefaultMarkets::TSX(), DefaultCountries::CA());
  }

  auto GetTsxVentureSecurity() {
    return Security("TSXV", DefaultMarkets::TSXV(), DefaultCountries::CA());
  }

  auto GetUnlistedSecurity() {
    return Security("???", DefaultMarkets::ASX(), DefaultCountries::CA());
  }

  auto MakeFeeTable() {
    auto feeTable = PureFeeTable();
    PopulateFeeTable(Store(feeTable.m_tsxListedFeeTable));
    PopulateFeeTable(Store(feeTable.m_tsxVentureListedFeeTable));
    for(auto i = 0; i < PureFeeTable::PRICE_CLASS_COUNT; ++i) {
      feeTable.m_tsxVentureListedFeeTable[i][
        static_cast<int>(LiquidityFlag::PASSIVE)] *= -1;
    }
    feeTable.m_tsxVentureListedSubdimeCap = 1000 * Money::ONE;
    feeTable.m_oddLot = 12 * Money::ONE;
    feeTable.m_designatedSecurities.insert(GetDesignatedSecurity());
    return feeTable;
  }
}

TEST_SUITE("PureFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = MakeFeeTable();
    TestFeeTableIndex(feeTable, feeTable.m_tsxListedFeeTable,
      LookupTsxListedFee, LIQUIDITY_FLAG_COUNT,
      PureFeeTable::PRICE_CLASS_COUNT);
    TestFeeTableIndex(feeTable, feeTable.m_tsxVentureListedFeeTable,
      LookupTsxVentureListedFee, LIQUIDITY_FLAG_COUNT,
      PureFeeTable::PRICE_CLASS_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
      std::bind(&CalculateFee, std::placeholders::_1, GetTsxSecurity(),
        std::placeholders::_2), Money::ZERO);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
      std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
        std::placeholders::_2), Money::ZERO);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
      std::bind(&CalculateFee, std::placeholders::_1, GetUnlistedSecurity(),
        std::placeholders::_2), Money::ZERO);
  }

  TEST_CASE("tsx_default_active") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::ACTIVE,
      PureFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, GetTsxSecurity(),
        std::placeholders::_2), expectedFee);
  }

  TEST_CASE("tsx_default_passive") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::PASSIVE,
      PureFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        GetTsxSecurity(), std::placeholders::_2), expectedFee);
  }

  TEST_CASE("tsx_designated_active") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::ACTIVE,
      PureFeeTable::PriceClass::DESIGNATED);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1,
        GetDesignatedSecurity(), std::placeholders::_2), expectedFee);
  }

  TEST_CASE("tsx_designated_passive") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::PASSIVE,
      PureFeeTable::PriceClass::DESIGNATED);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        GetDesignatedSecurity(), std::placeholders::_2), expectedFee);
  }

  TEST_CASE("tsx_subdollar_designated_active") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::ACTIVE,
      PureFeeTable::PriceClass::DESIGNATED);
    TestPerShareFeeCalculation(feeTable, 99 * Money::CENT, 100,
      LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
        GetDesignatedSecurity(), std::placeholders::_2), expectedFee);
  }

  TEST_CASE("tsx_subdollar_designated_passive") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::PASSIVE,
      PureFeeTable::PriceClass::DESIGNATED);
    TestPerShareFeeCalculation(feeTable, 99 * Money::CENT, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        GetDesignatedSecurity(), std::placeholders::_2), expectedFee);
  }

  TEST_CASE("tsx_subdollar_active") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::ACTIVE,
      PureFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, 10 * Money::CENT, 100,
      LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
        GetTsxSecurity(), std::placeholders::_2), expectedFee);
  }

  TEST_CASE("tsx_subdollar_passive") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxListedFee(feeTable, LiquidityFlag::PASSIVE,
      PureFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, 10 * Money::CENT, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        GetTsxSecurity(), std::placeholders::_2), expectedFee);
  }

  TEST_CASE("tsx_venture_default_active") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxVentureListedFee(feeTable,
      LiquidityFlag::ACTIVE, PureFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
        std::placeholders::_2), expectedFee);
  }

  TEST_CASE("tsx_venture_default_passive") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxVentureListedFee(feeTable,
      LiquidityFlag::PASSIVE, PureFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        GetTsxVentureSecurity(), std::placeholders::_2), expectedFee);
  }

  TEST_CASE("tsx_venture_subdollar_active") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxVentureListedFee(feeTable,
      LiquidityFlag::ACTIVE, PureFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, 50 * Money::CENT, 100,
      LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
        GetTsxVentureSecurity(), std::placeholders::_2), expectedFee);
  }

  TEST_CASE("tsx_venture_subdollar_passive") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxVentureListedFee(feeTable,
      LiquidityFlag::PASSIVE, PureFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, 50 * Money::CENT, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        GetTsxVentureSecurity(), std::placeholders::_2), expectedFee);
  }

  TEST_CASE("tsx_venture_subdime_active") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxVentureListedFee(feeTable,
      LiquidityFlag::ACTIVE, PureFeeTable::PriceClass::SUBDIME);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
        GetTsxVentureSecurity(), std::placeholders::_2), expectedFee);
    TestFeeCalculation(feeTable, Money::CENT, 10000000, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
        std::placeholders::_2), feeTable.m_tsxVentureListedSubdimeCap);
  }

  TEST_CASE("tsx_venture_subdime_passive") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxVentureListedFee(feeTable,
      LiquidityFlag::PASSIVE, PureFeeTable::PriceClass::SUBDIME);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        GetTsxVentureSecurity(), std::placeholders::_2), expectedFee);
    TestFeeCalculation(feeTable, Money::CENT, 10000000, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
        std::placeholders::_2), -feeTable.m_tsxVentureListedSubdimeCap);
  }

  TEST_CASE("odd_lot") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = feeTable.m_oddLot;
    TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, GetTsxSecurity(),
        std::placeholders::_2), expectedFee);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, GetTsxSecurity(),
        std::placeholders::_2), expectedFee);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
        std::placeholders::_2), expectedFee);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
        std::placeholders::_2), expectedFee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "AP";
      auto calculatedFee = CalculateFee(feeTable, GetTsxSecurity(),
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupTsxListedFee(
        feeTable, LiquidityFlag::ACTIVE, PureFeeTable::PriceClass::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "PA";
      auto calculatedFee = CalculateFee(feeTable, GetTsxVentureSecurity(),
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity *
        LookupTsxVentureListedFee(feeTable, LiquidityFlag::ACTIVE,
        PureFeeTable::PriceClass::SUBDIME);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "?";
      auto calculatedFee = CalculateFee(feeTable, GetUnlistedSecurity(),
        executionReport);
      auto expectedFee = executionReport.m_lastQuantity *
        LookupTsxVentureListedFee(
          feeTable, LiquidityFlag::ACTIVE, PureFeeTable::PriceClass::DEFAULT);
      REQUIRE(calculatedFee == expectedFee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupTsxVentureListedFee(
      feeTable, LiquidityFlag::ACTIVE, PureFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::NONE,
      std::bind(CalculateFee, std::placeholders::_1, GetTsxVentureSecurity(),
        std::placeholders::_2), expectedFee);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::NONE,
      std::bind(CalculateFee, std::placeholders::_1, GetUnlistedSecurity(),
        std::placeholders::_2), expectedFee);
  }
}
