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

  TEST_CASE("active_sub_dollar_regular") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 100 * feeTable.m_regularTable[
      static_cast<int>(Cse2FeeTable::PriceClass::SUBDOLLAR)][
      static_cast<int>(LiquidityFlag::ACTIVE)];
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::CENT),
      MakeExecutionReport(Money::CENT, 100, "TT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_default_regular") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 200 * feeTable.m_regularTable[
      static_cast<int>(Cse2FeeTable::PriceClass::DEFAULT)][
      static_cast<int>(LiquidityFlag::ACTIVE)];
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::ONE),
      MakeExecutionReport(Money::ONE, 200, "TT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_sub_dollar_regular") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 300 * feeTable.m_regularTable[
      static_cast<int>(Cse2FeeTable::PriceClass::SUBDOLLAR)][
      static_cast<int>(LiquidityFlag::PASSIVE)];
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::CENT),
      MakeExecutionReport(Money::CENT, 300, "PT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_default_regular") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 400 * feeTable.m_regularTable[
      static_cast<int>(Cse2FeeTable::PriceClass::DEFAULT)][
      static_cast<int>(LiquidityFlag::PASSIVE)];
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::ONE),
      MakeExecutionReport(Money::ONE, 400, "PT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_dark") {
    auto feeTable = MakeFeeTable();
    auto expectedFee =
      500 * feeTable.m_darkTable[static_cast<int>(LiquidityFlag::ACTIVE)];
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::ONE),
      MakeExecutionReport(Money::ONE, 500, "TTD"));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_dark") {
    auto feeTable = MakeFeeTable();
    auto expectedFee =
      600 * feeTable.m_darkTable[static_cast<int>(LiquidityFlag::PASSIVE)];
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::CENT),
      MakeExecutionReport(Money::CENT, 600, "PTD"));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_cse_debenture") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 100 * feeTable.m_debenturesOrNotesTable[
      static_cast<int>(Cse2FeeTable::ListingMarket::CSE)][
      static_cast<int>(LiquidityFlag::ACTIVE)];
    auto fee = CalculateFee(feeTable, MakeOrderFields("TST.DB", Money::CENT,
      DefaultMarkets::CSE()), MakeExecutionReport(Money::CENT, 100, "TC "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_cse_debenture") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 100 * feeTable.m_debenturesOrNotesTable[
      static_cast<int>(Cse2FeeTable::ListingMarket::CSE)][
      static_cast<int>(LiquidityFlag::PASSIVE)];
    auto fee = CalculateFee(feeTable, MakeOrderFields("TST.NO", Money::CENT,
      DefaultMarkets::CSE()), MakeExecutionReport(Money::CENT, 100, "PC "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_tsx_debenture") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 300 * feeTable.m_debenturesOrNotesTable[
      static_cast<int>(Cse2FeeTable::ListingMarket::TSX_TSXV)][
      static_cast<int>(LiquidityFlag::ACTIVE)];
    auto fee = CalculateFee(feeTable, MakeOrderFields("TST.NT", Money::CENT,
      DefaultMarkets::TSX()), MakeExecutionReport(Money::CENT, 300, "TT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_tsx_debenture") {
    auto feeTable = MakeFeeTable();
    auto expectedFee = 400 * feeTable.m_debenturesOrNotesTable[
      static_cast<int>(Cse2FeeTable::ListingMarket::TSX_TSXV)][
      static_cast<int>(LiquidityFlag::PASSIVE)];
    auto fee = CalculateFee(feeTable, MakeOrderFields("TST.NS", Money::ONE,
      DefaultMarkets::TSXV()), MakeExecutionReport(Money::ONE, 400, "PV "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("active_oddlot") {
    auto feeTable = MakeFeeTable();
    auto expectedFee =
      50 * feeTable.m_oddlotTable[static_cast<int>(LiquidityFlag::ACTIVE)];
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::ONE),
      MakeExecutionReport(Money::ONE, 50, "TT "));
    REQUIRE(fee == expectedFee);
  }

  TEST_CASE("passive_oddlot") {
    auto feeTable = MakeFeeTable();
    auto expectedFee =
      20 * feeTable.m_oddlotTable[static_cast<int>(LiquidityFlag::PASSIVE)];
    auto fee = CalculateFee(feeTable, MakeOrderFields(Money::CENT),
      MakeExecutionReport(Money::CENT, 20, "PT "));
    REQUIRE(fee == expectedFee);
  }
}
