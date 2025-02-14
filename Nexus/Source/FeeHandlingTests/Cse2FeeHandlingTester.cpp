#include <doctest/doctest.h>
#include "Nexus/FeeHandling/Cse2FeeTable.hpp"
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
    auto feeTable = Cse2FeeTable();
    PopulateFeeTable(Store(feeTable.m_regularTable));
    PopulateFeeTable(Store(feeTable.m_darkTable));
    PopulateFeeTable(Store(feeTable.m_debenturesOrNotesTable));
    PopulateFeeTable(Store(feeTable.m_cseListedGovernmentBondsTable));
    PopulateFeeTable(Store(feeTable.m_oddlotTable));
    return feeTable;
  }

  auto MakeOrderFields(
      std::string symbol, Money price, Quantity quantity, MarketCode market) {
    return OrderFields::MakeLimitOrder(DirectoryEntry::GetRootAccount(),
      Security(symbol, market, DefaultCountries::CA()), DefaultCurrencies::CAD(),
      Side::BID, DefaultDestinations::CSE2(), quantity, price);
  }

  auto MakeOrderFields(std::string symbol, Money price, MarketCode market) {
    return MakeOrderFields(symbol, price, 100, market);
  }

  auto MakeOrderFields(Money price) {
    return MakeOrderFields("TST", price, 100, DefaultMarkets::TSX());
  }

  auto MakeExecutionReport(Money price, Quantity quantity, std::string flag) {
    auto report = ExecutionReport();
    report.m_id = 123;
    report.m_lastMarket = DefaultMarkets::TSX().GetData();
    report.m_lastPrice = price;
    report.m_lastQuantity = quantity;
    report.m_liquidityFlag = flag;
    report.m_sequence = 0;
    return report;
  }
}

TEST_SUITE("Cse2FeeHandling") {
  TEST_CASE("zero_quantity") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = Money::ZERO;
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::CENT),
      MakeExecutionReport(Money::ZERO, 0, ""));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_subdollar_regular") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 100 * LookupRegularFee(
      feeTable, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::SUBDOLLAR);
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::CENT),
      MakeExecutionReport(Money::CENT, 100, "TT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_default_regular") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 200 * LookupRegularFee(
      feeTable, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::ONE),
      MakeExecutionReport(Money::ONE, 200, "TT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_sub_dollar_regular") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 300 * LookupRegularFee(
      feeTable, LiquidityFlag::PASSIVE, Cse2FeeTable::PriceClass::SUBDOLLAR);
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::CENT),
      MakeExecutionReport(Money::CENT, 300, "PT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_default_regular") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 400 * LookupRegularFee(
      feeTable, LiquidityFlag::PASSIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::ONE),
      MakeExecutionReport(Money::ONE, 400, "PT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_subdollar_dark") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 500 * LookupDarkFee(
      feeTable, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::SUBDOLLAR);
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::CENT),
      MakeExecutionReport(Money::CENT, 500, "TTD"));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_default_dark") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 500 * LookupDarkFee(
      feeTable, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::ONE),
      MakeExecutionReport(Money::ONE, 500, "TTD"));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_subdollar_dark") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 600 * LookupDarkFee(
      feeTable, LiquidityFlag::PASSIVE, Cse2FeeTable::PriceClass::SUBDOLLAR);
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::CENT),
      MakeExecutionReport(Money::CENT, 600, "PTD"));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_default_dark") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 600 * LookupDarkFee(
      feeTable, LiquidityFlag::PASSIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::ONE),
      MakeExecutionReport(Money::ONE, 600, "PTD"));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_cse_debenture") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 100 * LookupDebenturesOrNotesFee(feeTable,
      LiquidityFlag::ACTIVE, Cse2FeeTable::ListingMarket::CSE);
    auto fee = CalculateFee(feeTable, MakeOrderFields("TST.DB", Money::CENT,
      DefaultMarkets::CSE()), MakeExecutionReport(Money::CENT, 100, "TC "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_cse_debenture") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 100 * LookupDebenturesOrNotesFee(feeTable,
      LiquidityFlag::PASSIVE, Cse2FeeTable::ListingMarket::CSE);
    auto fee = CalculateFee(feeTable, MakeOrderFields("TST.NO", Money::CENT,
      DefaultMarkets::CSE()), MakeExecutionReport(Money::CENT, 100, "PC "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_tsx_debenture") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 300 * LookupDebenturesOrNotesFee(feeTable,
      LiquidityFlag::ACTIVE, Cse2FeeTable::ListingMarket::TSX_TSXV);
    auto fee = CalculateFee(feeTable, MakeOrderFields("TST.NT", Money::CENT,
      DefaultMarkets::TSX()), MakeExecutionReport(Money::CENT, 300, "TT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_tsx_debenture") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 400 * LookupDebenturesOrNotesFee(feeTable,
      LiquidityFlag::PASSIVE, Cse2FeeTable::ListingMarket::TSX_TSXV);
    auto fee = CalculateFee(feeTable, MakeOrderFields("TST.NS", Money::ONE,
      DefaultMarkets::TSXV()), MakeExecutionReport(Money::ONE, 400, "PV "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_subdollar_oddlot") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 50 * LookupOddlotFee(
      feeTable, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::SUBDOLLAR);
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::CENT),
      MakeExecutionReport(Money::CENT, 50, "TT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_subdollar_oddlot") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 20 * LookupOddlotFee(
      feeTable, LiquidityFlag::PASSIVE, Cse2FeeTable::PriceClass::SUBDOLLAR);
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::CENT),
      MakeExecutionReport(Money::CENT, 20, "PT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_oddlot") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 50 * LookupOddlotFee(
      feeTable, LiquidityFlag::ACTIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::ONE),
      MakeExecutionReport(Money::ONE, 50, "TT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_oddlot") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 20 * LookupOddlotFee(
      feeTable, LiquidityFlag::PASSIVE, Cse2FeeTable::PriceClass::DEFAULT);
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::ONE),
      MakeExecutionReport(Money::ONE, 20, "PT "));
    REQUIRE(fee == expectedFee);
  }
}
