#include <doctest/doctest.h>
#include "Nexus/FeeHandling/MatnFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto MakeFeeTable() {
    auto feeTable = MatnFeeTable();
    PopulateFeeTable(Store(feeTable.m_generalFeeTable));
    PopulateFeeTable(Store(feeTable.m_alternativeFeeTable));
    for(auto i = 0; i < MatnFeeTable::PRICE_CLASS_COUNT; ++i) {
      feeTable.m_generalFeeTable[i][
        static_cast<int>(MatnFeeTable::GeneralIndex::MAX_CHARGE)] *= 1000;
    }
    return feeTable;
  }

  auto LookupGeneralFee(const MatnFeeTable& feeTable,
      MatnFeeTable::GeneralIndex index, MatnFeeTable::PriceClass priceClass) {
    return LookupFee(feeTable, index, priceClass);
  }

  auto LookupAlternativeFee(const MatnFeeTable& feeTable,
      LiquidityFlag liquidityFlag, MatnFeeTable::Category category) {
    return LookupFee(feeTable, liquidityFlag, category);
  }
}

TEST_SUITE("MatnFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = MakeFeeTable();
    TestFeeTableIndex(feeTable, feeTable.m_generalFeeTable, LookupGeneralFee,
      MatnFeeTable::GENERAL_INDEX_COUNT, MatnFeeTable::PRICE_CLASS_COUNT);
    TestFeeTableIndex(feeTable, feeTable.m_alternativeFeeTable,
      LookupAlternativeFee, LIQUIDITY_FLAG_COUNT,
      MatnFeeTable::CATEGORY_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
      std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      Money::ZERO);
  }

  TEST_CASE("active_default_trade") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, 5 * Money::ONE, 100,
      LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedFee);
    auto expectedMaxFee = LookupFee(feeTable,
      MatnFeeTable::GeneralIndex::MAX_CHARGE,
      MatnFeeTable::PriceClass::DEFAULT);
    TestFeeCalculation(feeTable, 5 * Money::ONE, 1000000, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedMaxFee);
  }

  TEST_CASE("passive_default_trade") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, 5 * Money::ONE, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedFee);
    auto expectedMaxFee = LookupFee(feeTable,
      MatnFeeTable::GeneralIndex::MAX_CHARGE,
      MatnFeeTable::PriceClass::DEFAULT);
    TestFeeCalculation(feeTable, 5 * Money::ONE, 1000000,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedMaxFee);
  }

  TEST_CASE("active_dollar_trade") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::SUBFIVE_DOLLAR);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedFee);
    auto expectedMaxFee = LookupFee(feeTable,
      MatnFeeTable::GeneralIndex::MAX_CHARGE,
      MatnFeeTable::PriceClass::SUBFIVE_DOLLAR);
    TestFeeCalculation(feeTable, Money::ONE, 1000000, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedMaxFee);
  }

  TEST_CASE("passive_dollar_trade") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::SUBFIVE_DOLLAR);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedFee);
    auto expectedMaxFee = LookupFee(feeTable,
      MatnFeeTable::GeneralIndex::MAX_CHARGE,
      MatnFeeTable::PriceClass::SUBFIVE_DOLLAR);
    TestFeeCalculation(feeTable, Money::ONE, 1000000, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedMaxFee);
  }

  TEST_CASE("active_subdollar_trade") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::ACTIVE, std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedFee);
    auto expectedMaxFee = LookupFee(feeTable,
      MatnFeeTable::GeneralIndex::MAX_CHARGE,
      MatnFeeTable::PriceClass::SUBDOLLAR);
    TestFeeCalculation(feeTable, Money::CENT, 1000000, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedMaxFee);
  }

  TEST_CASE("passive_subdollar_trade") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::SUBDOLLAR);
    TestPerShareFeeCalculation(feeTable, Money::CENT, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedFee);
    auto expectedMaxFee = LookupFee(feeTable,
      MatnFeeTable::GeneralIndex::MAX_CHARGE,
      MatnFeeTable::PriceClass::SUBDOLLAR);
    TestFeeCalculation(feeTable, Money::CENT, 1000000, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedMaxFee);
  }

  TEST_CASE("active_etf_trade") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      MatnFeeTable::Category::ETF);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::ETF, std::placeholders::_2), expectedFee);
  }

  TEST_CASE("passive_etf_trade") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      MatnFeeTable::Category::ETF);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100,
      LiquidityFlag::PASSIVE, std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::ETF, std::placeholders::_2), expectedFee);
  }

  TEST_CASE("active_odd_lot_trade") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      MatnFeeTable::Category::ODD_LOT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedFee);
  }

  TEST_CASE("passive_odd_lot_trade") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      MatnFeeTable::Category::ODD_LOT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedFee);
  }

  TEST_CASE("active_odd_lot_etf") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::ACTIVE,
      MatnFeeTable::Category::ODD_LOT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::ETF, std::placeholders::_2), expectedFee);
  }

  TEST_CASE("passive_odd_lot_etf") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, LiquidityFlag::PASSIVE,
      MatnFeeTable::Category::ODD_LOT);
    TestPerShareFeeCalculation(feeTable, Money::ONE, 50, LiquidityFlag::PASSIVE,
      std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::ETF, std::placeholders::_2), expectedFee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "AP";
      auto calculatedFee = CalculateFee(feeTable,
        MatnFeeTable::Classification::DEFAULT, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        MatnFeeTable::GeneralIndex::FEE, MatnFeeTable::PriceClass::SUBDOLLAR);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "PA";
      auto calculatedFee = CalculateFee(feeTable,
        MatnFeeTable::Classification::DEFAULT, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        MatnFeeTable::GeneralIndex::FEE,
        MatnFeeTable::PriceClass::SUBFIVE_DOLLAR);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::MakeInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "PA";
      auto calculatedFee = CalculateFee(feeTable,
        MatnFeeTable::Classification::ETF, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        LiquidityFlag::ACTIVE, MatnFeeTable::Category::ETF);
      REQUIRE(calculatedFee == expectedFee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = LookupFee(feeTable, MatnFeeTable::GeneralIndex::FEE,
      MatnFeeTable::PriceClass::DEFAULT);
    TestPerShareFeeCalculation(feeTable, 5 * Money::ONE, 100,
      LiquidityFlag::NONE, std::bind(&CalculateFee, std::placeholders::_1,
        MatnFeeTable::Classification::DEFAULT, std::placeholders::_2),
      expectedFee);
  }
}
