#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/ChicFeeTable.hpp"
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
      DefaultDestinations::CHIX(), 100, price);
  }
}

TEST_SUITE("ChicFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    TestFeeTableIndex(feeTable, feeTable.m_securityTable, LookupFee,
      ChicFeeTable::INDEX_COUNT, ChicFeeTable::CLASSIFICATION_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    fields.m_quantity = 0;
    auto expectedFee = Money::ZERO;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("default_active") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::NON_INTERLISTED);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("default_passive") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::PASSIVE,
      ChicFeeTable::Classification::NON_INTERLISTED);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("default_hidden_passive") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_PASSIVE,
      ChicFeeTable::Classification::NON_INTERLISTED);
    TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee,
      expectedFee);
  }

  TEST_CASE("default_hidden_active") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_ACTIVE,
      ChicFeeTable::Classification::NON_INTERLISTED);
    TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee,
      expectedFee);
  }

  TEST_CASE("sub_dollar_active") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::SUB_DOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("sub_dollar_passive") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::PASSIVE,
      ChicFeeTable::Classification::SUB_DOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("sub_dime_active") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::SUB_DIME);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("sub_dime_passive") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::PASSIVE,
      ChicFeeTable::Classification::SUB_DIME);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("sub_dollar_hidden_active") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_ACTIVE,
      ChicFeeTable::Classification::SUB_DOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee,
      expectedFee);
  }

  TEST_CASE("sub_dollar_hidden_passive") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_PASSIVE,
      ChicFeeTable::Classification::SUB_DOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee,
      expectedFee);
  }

  TEST_CASE("sub_dime_hidden_active") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_ACTIVE,
      ChicFeeTable::Classification::SUB_DIME);
    TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee,
      expectedFee);
  }

  TEST_CASE("sub_dime_hidden_passive") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_PASSIVE,
      ChicFeeTable::Classification::SUB_DIME);
    TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee,
      expectedFee);
  }

  TEST_CASE("interlisted_active") {
    auto feeTable = ChicFeeTable();
    feeTable.m_interlisted.insert(GetTestSecurity());
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::INTERLISTED);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("interlisted_passive") {
    auto feeTable = ChicFeeTable();
    feeTable.m_interlisted.insert(GetTestSecurity());
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::PASSIVE,
      ChicFeeTable::Classification::INTERLISTED);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("interlisted_hidden_passive") {
    auto feeTable = ChicFeeTable();
    feeTable.m_interlisted.insert(GetTestSecurity());
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_PASSIVE,
      ChicFeeTable::Classification::INTERLISTED);
    TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee,
      expectedFee);
  }

  TEST_CASE("interlisted_hidden_active") {
    auto feeTable = ChicFeeTable();
    feeTable.m_interlisted.insert(GetTestSecurity());
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_ACTIVE,
      ChicFeeTable::Classification::INTERLISTED);
    TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee,
      expectedFee);
  }

  TEST_CASE("sub_dollar_interlisted_active") {
    auto feeTable = ChicFeeTable();
    feeTable.m_interlisted.insert(GetTestSecurity());
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::SUB_DOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("sub_dime_interlisted_active") {
    auto feeTable = ChicFeeTable();
    feeTable.m_interlisted.insert(GetTestSecurity());
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::SUB_DIME);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("etf_active") {
    auto feeTable = ChicFeeTable();
    feeTable.m_etfs.insert(GetTestSecurity());
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::ETF);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("etf_passive") {
    auto feeTable = ChicFeeTable();
    feeTable.m_etfs.insert(GetTestSecurity());
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::PASSIVE,
      ChicFeeTable::Classification::ETF);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("etf_hidden_passive") {
    auto feeTable = ChicFeeTable();
    feeTable.m_etfs.insert(GetTestSecurity());
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_PASSIVE,
      ChicFeeTable::Classification::ETF);
    TestPerShareFeeCalculation(feeTable, fields, "a", CalculateFee,
      expectedFee);
  }

  TEST_CASE("etf_hidden_active") {
    auto feeTable = ChicFeeTable();
    feeTable.m_etfs.insert(GetTestSecurity());
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::HIDDEN_ACTIVE,
      ChicFeeTable::Classification::ETF);
    TestPerShareFeeCalculation(feeTable, fields, "r", CalculateFee,
      expectedFee);
  }

  TEST_CASE("sub_dollar_etf_active") {
    auto feeTable = ChicFeeTable();
    feeTable.m_etfs.insert(GetTestSecurity());
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(20 * Money::CENT);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::SUB_DOLLAR);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("sub_dime_etf_active") {
    auto feeTable = ChicFeeTable();
    feeTable.m_etfs.insert(GetTestSecurity());
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::CENT);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::SUB_DIME);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      CalculateFee, expectedFee);
  }

  TEST_CASE("unknown_liquidity_flag") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "AP";
      auto calculatedFee = CalculateFee(feeTable, fields, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        ChicFeeTable::Index::ACTIVE,
        ChicFeeTable::Classification::NON_INTERLISTED);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::CENT;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "PA";
      auto calculatedFee = CalculateFee(feeTable, fields, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        ChicFeeTable::Index::ACTIVE, ChicFeeTable::Classification::SUB_DIME);
      REQUIRE(calculatedFee == expectedFee);
    }
    {
      auto executionReport = ExecutionReport::BuildInitialReport(0,
        second_clock::universal_time());
      executionReport.m_lastPrice = Money::ONE;
      executionReport.m_lastQuantity = 100;
      executionReport.m_liquidityFlag = "?";
      auto calculatedFee = CalculateFee(feeTable, fields, executionReport);
      auto expectedFee = executionReport.m_lastQuantity * LookupFee(feeTable,
        ChicFeeTable::Index::ACTIVE,
        ChicFeeTable::Classification::NON_INTERLISTED);
      REQUIRE(calculatedFee == expectedFee);
    }
  }

  TEST_CASE("empty_liquidity_flag") {
    auto feeTable = ChicFeeTable();
    PopulateFeeTable(Store(feeTable.m_securityTable));
    auto fields = BuildOrderFields(Money::ONE);
    auto expectedFee = LookupFee(feeTable, ChicFeeTable::Index::ACTIVE,
      ChicFeeTable::Classification::NON_INTERLISTED);
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
      CalculateFee, expectedFee);
  }
}
