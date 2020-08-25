#include <doctest/doctest.h>
#include "Nexus/Accounting/BuyingPowerModel.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::OrderExecutionService;

TEST_SUITE("BuyingPowerModel") {
  TEST_CASE("two_bids") {
    auto model = BuyingPowerModel();
    auto security = Security("A", DefaultMarkets::NASDAQ(),
      DefaultCountries::US());
    auto bidFieldsA = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "", 100, Money::ONE);
    auto buyingPower = model.Submit(0, bidFieldsA, bidFieldsA.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto bidFieldsB = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "", 200, Money::ONE);
    buyingPower = model.Submit(1, bidFieldsB, 2 * Money::ONE);
    REQUIRE(buyingPower == 500 * Money::ONE);
  }

  TEST_CASE("bid_and_smaller_ask") {
    auto model = BuyingPowerModel();
    auto security = Security("A", DefaultMarkets::NASDAQ(),
      DefaultCountries::US());
    auto bidFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "", 100, Money::ONE);
    auto buyingPower = model.Submit(0, bidFields, bidFields.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto askFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::ASK, "", 50, Money::ONE);
    buyingPower = model.Submit(1, askFields, askFields.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
  }

  TEST_CASE("bid_and_greater_ask") {
    auto model = BuyingPowerModel();
    auto security = Security("A", DefaultMarkets::NASDAQ(),
      DefaultCountries::US());
    auto bidFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "", 100, Money::ONE);
    auto buyingPower = model.Submit(0, bidFields, bidFields.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto askFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::ASK, "", 200, Money::ONE);
    buyingPower = model.Submit(1, askFields, askFields.m_price);
    REQUIRE(buyingPower == 200 * Money::ONE);
  }

  TEST_CASE("bid_and_cancel") {
    auto model = BuyingPowerModel();
    auto security = Security("A", DefaultMarkets::NASDAQ(),
      DefaultCountries::US());
    auto bidFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "", 100, Money::ONE);
    auto buyingPower = model.Submit(0, bidFields, bidFields.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto cancelReport = ExecutionReport();
    cancelReport.m_id = 0;
    cancelReport.m_status = OrderStatus::CANCELED;
    model.Update(cancelReport);
    buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
    REQUIRE(buyingPower == Money::ZERO);
  }

  TEST_CASE("bid_and_fill") {
    auto model = BuyingPowerModel();
    auto security = Security("A", DefaultMarkets::NASDAQ(),
      DefaultCountries::US());
    auto bidFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "", 100, Money::ONE);
    auto buyingPower = model.Submit(0, bidFields, bidFields.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto fillReport = ExecutionReport();
    fillReport.m_id = 0;
    fillReport.m_status = OrderStatus::FILLED;
    fillReport.m_lastQuantity = 100;
    fillReport.m_lastPrice = 50 * Money::CENT;
    model.Update(fillReport);
    buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
    REQUIRE(buyingPower == 100 * 50 * Money::CENT);
  }

  TEST_CASE("bid_and_partial_fill_and_cancel") {
    auto model = BuyingPowerModel();
    auto security = Security("A", DefaultMarkets::NASDAQ(),
      DefaultCountries::US());
    auto bidFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "", 100, Money::ONE);
    auto buyingPower = model.Submit(0, bidFields, bidFields.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto partialFillReport = ExecutionReport();
    partialFillReport.m_id = 0;
    partialFillReport.m_status = OrderStatus::PARTIALLY_FILLED;
    partialFillReport.m_lastQuantity = 50;
    partialFillReport.m_lastPrice = 50 * Money::CENT;
    model.Update(partialFillReport);
    buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
    REQUIRE(buyingPower == (50 * 50 * Money::CENT) + (50 * Money::ONE));
    auto cancelReport = ExecutionReport();
    cancelReport.m_id = 0;
    cancelReport.m_status = OrderStatus::CANCELED;
    model.Update(cancelReport);
    buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
    REQUIRE(buyingPower == (50 * 50 * Money::CENT));
  }

  TEST_CASE("bid_and_offsetting_ask") {
    auto model = BuyingPowerModel();
    auto security = Security("A", DefaultMarkets::NASDAQ(),
      DefaultCountries::US());
    auto bidFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "", 100, Money::ONE);
    auto buyingPower = model.Submit(0, bidFields, bidFields.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto bidFillReport = ExecutionReport();
    bidFillReport.m_id = 0;
    bidFillReport.m_status = OrderStatus::FILLED;
    bidFillReport.m_lastQuantity = 100;
    bidFillReport.m_lastPrice = 50 * Money::CENT;
    model.Update(bidFillReport);
    buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
    REQUIRE(buyingPower == 100 * 50 * Money::CENT);
    auto askFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::ASK, "", 100, 2 * Money::ONE);
    buyingPower = model.Submit(1, askFields, askFields.m_price);
    REQUIRE(buyingPower == 100 * 50 * Money::CENT);
    auto askFillReport = ExecutionReport();
    askFillReport.m_id = 1;
    askFillReport.m_status = OrderStatus::FILLED;
    askFillReport.m_lastQuantity = 100;
    askFillReport.m_lastPrice = 2 * Money::ONE;
    model.Update(askFillReport);
    buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
    REQUIRE(buyingPower == Money::ZERO);
  }

  TEST_CASE("bid_and_partial_offsetting_ask") {
    auto model = BuyingPowerModel();
    auto security = Security("A", DefaultMarkets::NASDAQ(),
      DefaultCountries::US());
    auto bidFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "", 100, Money::ONE);
    auto buyingPower = model.Submit(0, bidFields, bidFields.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto bidFillReport = ExecutionReport();
    bidFillReport.m_id = 0;
    bidFillReport.m_status = OrderStatus::FILLED;
    bidFillReport.m_lastQuantity = 100;
    bidFillReport.m_lastPrice = 50 * Money::CENT;
    model.Update(bidFillReport);
    buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
    REQUIRE(buyingPower == 100 * 50 * Money::CENT);
    auto askFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::ASK, "", 50, 2 * Money::ONE);
    buyingPower = model.Submit(1, askFields, askFields.m_price);
    REQUIRE(buyingPower == 100 * 50 * Money::CENT);
    auto askFillReport = ExecutionReport();
    askFillReport.m_id = 1;
    askFillReport.m_status = OrderStatus::FILLED;
    askFillReport.m_lastQuantity = 50;
    askFillReport.m_lastPrice = 2 * Money::ONE;
    model.Update(askFillReport);
    buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
    REQUIRE(buyingPower == 50 * 50 * Money::CENT);
  }

  TEST_CASE("partial_bid_and_offsetting_ask") {
    auto model = BuyingPowerModel();
    auto security = Security("A", DefaultMarkets::NASDAQ(),
      DefaultCountries::US());
    auto bidFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "", 100, Money::ONE);
    auto buyingPower = model.Submit(0, bidFields, bidFields.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto bidFillReport = ExecutionReport();
    bidFillReport.m_id = 0;
    bidFillReport.m_status = OrderStatus::PARTIALLY_FILLED;
    bidFillReport.m_lastQuantity = 50;
    bidFillReport.m_lastPrice = bidFields.m_price;
    model.Update(bidFillReport);
    buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto askFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::ASK, "", 50, 2 * Money::ONE);
    buyingPower = model.Submit(1, askFields, askFields.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto askFillReport = ExecutionReport();
    askFillReport.m_id = 1;
    askFillReport.m_status = OrderStatus::FILLED;
    askFillReport.m_lastQuantity = 50;
    askFillReport.m_lastPrice = 2 * Money::ONE;
    model.Update(askFillReport);
    buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
    REQUIRE(buyingPower == 50 * Money::ONE);
  }

  TEST_CASE("partial_bid_and_partial_offsetting_ask") {
    auto model = BuyingPowerModel();
    auto security = Security("A", DefaultMarkets::NASDAQ(),
      DefaultCountries::US());
    auto bidFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "", 100, Money::ONE);
    auto buyingPower = model.Submit(0, bidFields, bidFields.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto bidFillReport = ExecutionReport();
    bidFillReport.m_id = 0;
    bidFillReport.m_status = OrderStatus::PARTIALLY_FILLED;
    bidFillReport.m_lastQuantity = 50;
    bidFillReport.m_lastPrice = bidFields.m_price;
    model.Update(bidFillReport);
    buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto askFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::ASK, "", 50, 2 * Money::ONE);
    buyingPower = model.Submit(1, askFields, askFields.m_price);
    REQUIRE(buyingPower == 100 * Money::ONE);
    auto askFillReport = ExecutionReport();
    askFillReport.m_id = 1;
    askFillReport.m_status = OrderStatus::PARTIALLY_FILLED;
    askFillReport.m_lastQuantity = 25;
    askFillReport.m_lastPrice = 2 * Money::ONE;
    model.Update(askFillReport);
    buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
    REQUIRE(buyingPower == 75 * Money::ONE);
  }

  TEST_CASE("bid_and_multiple_asks") {
    auto model = BuyingPowerModel();
    auto security = Security("A", DefaultMarkets::NASDAQ(),
      DefaultCountries::US());
    {
      auto bidFields = OrderFields::BuildLimitOrder(
        DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
        Side::BID, "", 100, Money::ONE);
      auto buyingPower = model.Submit(0, bidFields, bidFields.m_price);
      REQUIRE(buyingPower == 100 * Money::ONE);
      auto fillReport = ExecutionReport();
      fillReport.m_id = 0;
      fillReport.m_status = OrderStatus::FILLED;
      fillReport.m_lastQuantity = 100;
      fillReport.m_lastPrice = 50 * Money::CENT;
      model.Update(fillReport);
      buyingPower = model.GetBuyingPower(DefaultCurrencies::USD());
      REQUIRE(buyingPower == (50 * Money::ONE));
    }
    {
      auto askFields = OrderFields::BuildLimitOrder(
        DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
        Side::ASK, "", 100, 2 * Money::ONE);
      auto buyingPower = model.Submit(1, askFields, askFields.m_price);
      REQUIRE(buyingPower == 50 * Money::ONE);
    }
    {
      auto askFields = OrderFields::BuildLimitOrder(
        DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
        Side::ASK, "", 100, 2 * Money::ONE);
      auto buyingPower = model.Submit(1, askFields, askFields.m_price);
      REQUIRE(buyingPower == 200 * Money::ONE);
    }
  }
}
