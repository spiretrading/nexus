#include "Nexus/AccountingTests/BuyingPowerTrackerTester.hpp"
#include "Nexus/Accounting/BuyingPowerTracker.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::Accounting::Tests;
using namespace Nexus::OrderExecutionService;

void BuyingPowerTrackerTester::TestTwoBids() {
  BuyingPowerTracker tracker;
  Security security("A", DefaultMarkets::NASDAQ(), DefaultCountries::US());
  OrderFields bidFieldsA = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "", 100, Money::ONE);
  Money buyingPower = tracker.Submit(0, bidFieldsA, bidFieldsA.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  OrderFields bidFieldsB = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "", 200, Money::ONE);
  buyingPower = tracker.Submit(1, bidFieldsB, 2 * Money::ONE);
  CPPUNIT_ASSERT(buyingPower == 500 * Money::ONE);
}

void BuyingPowerTrackerTester::TestBidAndSmallerAsk() {
  BuyingPowerTracker tracker;
  Security security("A", DefaultMarkets::NASDAQ(), DefaultCountries::US());
  OrderFields bidFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "", 100, Money::ONE);
  Money buyingPower = tracker.Submit(0, bidFields, bidFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  OrderFields askFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::ASK, "", 50, Money::ONE);
  buyingPower = tracker.Submit(1, askFields, askFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
}

void BuyingPowerTrackerTester::TestBidAndGreaterAsk() {
  BuyingPowerTracker tracker;
  Security security("A", DefaultMarkets::NASDAQ(), DefaultCountries::US());
  OrderFields bidFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "", 100, Money::ONE);
  Money buyingPower = tracker.Submit(0, bidFields, bidFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  OrderFields askFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::ASK, "", 200, Money::ONE);
  buyingPower = tracker.Submit(1, askFields, askFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 200 * Money::ONE);
}

void BuyingPowerTrackerTester::TestBidAndCancel() {
  BuyingPowerTracker tracker;
  Security security("A", DefaultMarkets::NASDAQ(), DefaultCountries::US());
  OrderFields bidFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "", 100, Money::ONE);
  Money buyingPower = tracker.Submit(0, bidFields, bidFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  ExecutionReport cancelReport;
  cancelReport.m_id = 0;
  cancelReport.m_status = OrderStatus::CANCELED;
  tracker.Update(cancelReport);
  buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
  CPPUNIT_ASSERT(buyingPower == Money::ZERO);
}

void BuyingPowerTrackerTester::TestBidAndFill() {
  BuyingPowerTracker tracker;
  Security security("A", DefaultMarkets::NASDAQ(), DefaultCountries::US());
  OrderFields bidFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "", 100, Money::ONE);
  Money buyingPower = tracker.Submit(0, bidFields, bidFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  ExecutionReport fillReport;
  fillReport.m_id = 0;
  fillReport.m_status = OrderStatus::FILLED;
  fillReport.m_lastQuantity = 100;
  fillReport.m_lastPrice = 50 * Money::CENT;
  tracker.Update(fillReport);
  buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
  CPPUNIT_ASSERT(buyingPower == 100 * 50 * Money::CENT);
}

void BuyingPowerTrackerTester::TestBidAndPartialFillAndCancel() {
  BuyingPowerTracker tracker;
  Security security("A", DefaultMarkets::NASDAQ(), DefaultCountries::US());
  OrderFields bidFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "", 100, Money::ONE);
  Money buyingPower = tracker.Submit(0, bidFields, bidFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  ExecutionReport partialFillReport;
  partialFillReport.m_id = 0;
  partialFillReport.m_status = OrderStatus::PARTIALLY_FILLED;
  partialFillReport.m_lastQuantity = 50;
  partialFillReport.m_lastPrice = 50 * Money::CENT;
  tracker.Update(partialFillReport);
  buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
  CPPUNIT_ASSERT(buyingPower == (50 * 50 * Money::CENT) + (50 * Money::ONE));
  ExecutionReport cancelReport;
  cancelReport.m_id = 0;
  cancelReport.m_status = OrderStatus::CANCELED;
  tracker.Update(cancelReport);
  buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
  CPPUNIT_ASSERT(buyingPower == (50 * 50 * Money::CENT));
}

void BuyingPowerTrackerTester::TestBidAndOffsettingAsk() {
  BuyingPowerTracker tracker;
  Security security("A", DefaultMarkets::NASDAQ(), DefaultCountries::US());
  OrderFields bidFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "", 100, Money::ONE);
  Money buyingPower = tracker.Submit(0, bidFields, bidFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  ExecutionReport bidFillReport;
  bidFillReport.m_id = 0;
  bidFillReport.m_status = OrderStatus::FILLED;
  bidFillReport.m_lastQuantity = 100;
  bidFillReport.m_lastPrice = 50 * Money::CENT;
  tracker.Update(bidFillReport);
  buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
  CPPUNIT_ASSERT(buyingPower == 100 * 50 * Money::CENT);
  OrderFields askFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::ASK, "", 100, 2 * Money::ONE);
  buyingPower = tracker.Submit(1, askFields, askFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * 50 * Money::CENT);
  ExecutionReport askFillReport;
  askFillReport.m_id = 1;
  askFillReport.m_status = OrderStatus::FILLED;
  askFillReport.m_lastQuantity = 100;
  askFillReport.m_lastPrice = 2 * Money::ONE;
  tracker.Update(askFillReport);
  buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
  CPPUNIT_ASSERT(buyingPower == Money::ZERO);
}

void BuyingPowerTrackerTester::TestBidAndPartialOffsettingAsk() {
  BuyingPowerTracker tracker;
  Security security("A", DefaultMarkets::NASDAQ(), DefaultCountries::US());
  OrderFields bidFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "", 100, Money::ONE);
  Money buyingPower = tracker.Submit(0, bidFields, bidFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  ExecutionReport bidFillReport;
  bidFillReport.m_id = 0;
  bidFillReport.m_status = OrderStatus::FILLED;
  bidFillReport.m_lastQuantity = 100;
  bidFillReport.m_lastPrice = 50 * Money::CENT;
  tracker.Update(bidFillReport);
  buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
  CPPUNIT_ASSERT(buyingPower == 100 * 50 * Money::CENT);
  OrderFields askFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::ASK, "", 50, 2 * Money::ONE);
  buyingPower = tracker.Submit(1, askFields, askFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * 50 * Money::CENT);
  ExecutionReport askFillReport;
  askFillReport.m_id = 1;
  askFillReport.m_status = OrderStatus::FILLED;
  askFillReport.m_lastQuantity = 50;
  askFillReport.m_lastPrice = 2 * Money::ONE;
  tracker.Update(askFillReport);
  buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
  CPPUNIT_ASSERT(buyingPower == 50 * 50 * Money::CENT);
}

void BuyingPowerTrackerTester::TestPartialBidAndOffsettingAsk() {
  BuyingPowerTracker tracker;
  Security security("A", DefaultMarkets::NASDAQ(), DefaultCountries::US());
  OrderFields bidFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "", 100, Money::ONE);
  Money buyingPower = tracker.Submit(0, bidFields, bidFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  ExecutionReport bidFillReport;
  bidFillReport.m_id = 0;
  bidFillReport.m_status = OrderStatus::PARTIALLY_FILLED;
  bidFillReport.m_lastQuantity = 50;
  bidFillReport.m_lastPrice = bidFields.m_price;
  tracker.Update(bidFillReport);
  buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  OrderFields askFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::ASK, "", 50, 2 * Money::ONE);
  buyingPower = tracker.Submit(1, askFields, askFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  ExecutionReport askFillReport;
  askFillReport.m_id = 1;
  askFillReport.m_status = OrderStatus::FILLED;
  askFillReport.m_lastQuantity = 50;
  askFillReport.m_lastPrice = 2 * Money::ONE;
  tracker.Update(askFillReport);
  buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
  CPPUNIT_ASSERT(buyingPower == 50 * Money::ONE);
}

void BuyingPowerTrackerTester::TestPartialBidAndPartialOffsettingAsk() {
  BuyingPowerTracker tracker;
  Security security("A", DefaultMarkets::NASDAQ(), DefaultCountries::US());
  OrderFields bidFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "", 100, Money::ONE);
  Money buyingPower = tracker.Submit(0, bidFields, bidFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  ExecutionReport bidFillReport;
  bidFillReport.m_id = 0;
  bidFillReport.m_status = OrderStatus::PARTIALLY_FILLED;
  bidFillReport.m_lastQuantity = 50;
  bidFillReport.m_lastPrice = bidFields.m_price;
  tracker.Update(bidFillReport);
  buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  OrderFields askFields = OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::ASK, "", 50, 2 * Money::ONE);
  buyingPower = tracker.Submit(1, askFields, askFields.m_price);
  CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
  ExecutionReport askFillReport;
  askFillReport.m_id = 1;
  askFillReport.m_status = OrderStatus::PARTIALLY_FILLED;
  askFillReport.m_lastQuantity = 25;
  askFillReport.m_lastPrice = 2 * Money::ONE;
  tracker.Update(askFillReport);
  buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
  CPPUNIT_ASSERT(buyingPower == 75 * Money::ONE);
}

void BuyingPowerTrackerTester::TestBidAndMultipleAsks() {
  BuyingPowerTracker tracker;
  Security security("A", DefaultMarkets::NASDAQ(), DefaultCountries::US());
  {
    auto bidFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "", 100, Money::ONE);
    auto buyingPower = tracker.Submit(0, bidFields, bidFields.m_price);
    CPPUNIT_ASSERT(buyingPower == 100 * Money::ONE);
    ExecutionReport fillReport;
    fillReport.m_id = 0;
    fillReport.m_status = OrderStatus::FILLED;
    fillReport.m_lastQuantity = 100;
    fillReport.m_lastPrice = 50 * Money::CENT;
    tracker.Update(fillReport);
    buyingPower = tracker.GetBuyingPower(DefaultCurrencies::USD());
    CPPUNIT_ASSERT(buyingPower == (50 * Money::ONE));
  }
  {
    auto askFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::ASK, "", 100, 2 * Money::ONE);
    auto buyingPower = tracker.Submit(1, askFields, askFields.m_price);
    CPPUNIT_ASSERT(buyingPower == 50 * Money::ONE);
  }
  {
    auto askFields = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::ASK, "", 100, 2 * Money::ONE);
    auto buyingPower = tracker.Submit(1, askFields, askFields.m_price);
    CPPUNIT_ASSERT(buyingPower == 200 * Money::ONE);
  }
}
