#include <doctest/doctest.h>
#include "Nexus/Accounting/PositionOrderBook.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::OrderExecutionService;

namespace {
  const auto TEST_SECURITY = Security("TST", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());

  auto BuildOrderFields(Side side, Quantity quantity, Money price) {
    return OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      TEST_SECURITY, DefaultCurrencies::USD(), side, "NYSE", quantity, price);
  }

  auto AddOrder(OrderId id, const OrderFields& fields,
      PositionOrderBook& book) {
    auto order = std::make_shared<PrimitiveOrder>(OrderInfo{fields, id,
      second_clock::universal_time()});
    book.Add(*order);
    return order;
  }

  void Fill(PrimitiveOrder& order, PositionOrderBook& book, Quantity quantity,
      Money price) {
    order.With(
      [&] (OrderStatus status,
          const std::vector<ExecutionReport>& executionReports) {
        auto update = ExecutionReport::BuildUpdatedReport(
          executionReports.back(), OrderStatus::FILLED,
          second_clock::universal_time());
        update.m_lastQuantity = quantity;
        update.m_lastPrice = price;
        book.Update(update);
      });
  }

  void Cancel(PrimitiveOrder& order, PositionOrderBook& book) {
    order.With(
      [&] (OrderStatus status,
          const std::vector<ExecutionReport>& executionReports) {
        auto update = ExecutionReport::BuildUpdatedReport(
          executionReports.back(), OrderStatus::CANCELED,
          second_clock::universal_time());
        book.Update(update);
      });
  }

  void AssertOpeningOrders(const std::vector<const Order*>& openingOrders,
      const std::vector<const Order*>& expectedOrders) {
    REQUIRE(expectedOrders.size() == openingOrders.size());
    for(auto expectedOrder : expectedOrders) {
      REQUIRE(find(openingOrders.begin(),
        openingOrders.end(), expectedOrder) != openingOrders.end());
    }
  }
}

TEST_SUITE("PositionOrderBook") {
  TEST_CASE("bid_opening_order_submission") {
    auto book = PositionOrderBook();
    auto orderFieldsA = BuildOrderFields(Side::BID, 100, Money::ONE);
    REQUIRE(book.TestOpeningOrderSubmission(orderFieldsA));
    auto orderA = AddOrder(1, orderFieldsA, book);
    Fill(*orderA, book, orderA->GetInfo().m_fields.m_quantity,
      orderA->GetInfo().m_fields.m_price);
    auto orderFieldsB = BuildOrderFields(Side::BID, 100, Money::ONE);
    REQUIRE(book.TestOpeningOrderSubmission(orderFieldsB));
    auto orderFieldsC = BuildOrderFields(Side::ASK, 100, Money::ONE);
    REQUIRE(!book.TestOpeningOrderSubmission(orderFieldsC));
    auto orderC = AddOrder(2, orderFieldsC, book);
    auto orderFieldsD = BuildOrderFields(Side::ASK, 100, Money::ONE);
    REQUIRE(book.TestOpeningOrderSubmission(orderFieldsD));
    Fill(*orderC, book, orderC->GetInfo().m_fields.m_quantity,
      orderC->GetInfo().m_fields.m_price);
    auto orderFieldsE = BuildOrderFields(Side::BID, 100, Money::ONE);
    REQUIRE(book.TestOpeningOrderSubmission(orderFieldsE));
    auto orderFieldsF = BuildOrderFields(Side::ASK, 100, Money::ONE);
    REQUIRE(book.TestOpeningOrderSubmission(orderFieldsF));
  }

  TEST_CASE("ask_opening_order_submission") {
    auto book = PositionOrderBook();
    auto orderFieldsA = BuildOrderFields(Side::ASK, 100, Money::ONE);
    REQUIRE(book.TestOpeningOrderSubmission(orderFieldsA));
    auto orderA = AddOrder(1, orderFieldsA, book);
    Fill(*orderA, book, orderA->GetInfo().m_fields.m_quantity,
      orderA->GetInfo().m_fields.m_price);
    auto orderFieldsB = BuildOrderFields(Side::ASK, 100, Money::ONE);
    REQUIRE(book.TestOpeningOrderSubmission(orderFieldsB));
    auto orderFieldsC = BuildOrderFields(Side::BID, 100, Money::ONE);
    REQUIRE(!book.TestOpeningOrderSubmission(orderFieldsC));
    auto orderC = AddOrder(2, orderFieldsC, book);
    auto orderFieldsD = BuildOrderFields(Side::BID, 100, Money::ONE);
    REQUIRE(book.TestOpeningOrderSubmission(orderFieldsD));
    Fill(*orderC, book, orderC->GetInfo().m_fields.m_quantity,
      orderC->GetInfo().m_fields.m_price);
    auto orderFieldsE = BuildOrderFields(Side::ASK, 100, Money::ONE);
    REQUIRE(book.TestOpeningOrderSubmission(orderFieldsE));
    auto orderFieldsF = BuildOrderFields(Side::BID, 100, Money::ONE);
    REQUIRE(book.TestOpeningOrderSubmission(orderFieldsF));
  }

  TEST_CASE("removing_orders") {
    auto book = PositionOrderBook();
    auto orderFieldsA = BuildOrderFields(Side::ASK, 100, Money::ONE);
    auto orderA = AddOrder(1, orderFieldsA, book);
    auto orderFieldsB = BuildOrderFields(Side::ASK, 100, Money::ONE);
    auto orderB = AddOrder(2, orderFieldsA, book);
    Fill(*orderA, book, orderA->GetInfo().m_fields.m_quantity,
      orderA->GetInfo().m_fields.m_price);
    Cancel(*orderB, book);
    auto orderFieldsC = BuildOrderFields(Side::BID, 200, Money::ONE);
    auto orderC = AddOrder(3, orderFieldsC, book);
    Fill(*orderC, book, orderC->GetInfo().m_fields.m_quantity,
      orderC->GetInfo().m_fields.m_price);
    auto orderFieldsD = BuildOrderFields(Side::ASK, 200, Money::ONE);
    REQUIRE(book.TestOpeningOrderSubmission(orderFieldsD));
  }

  TEST_CASE("get_opening_orders") {
    auto book = PositionOrderBook();
    auto orderFieldsA = BuildOrderFields(Side::BID, 100, Money::ONE);
    auto orderA = AddOrder(1, orderFieldsA, book);
    AssertOpeningOrders(book.GetOpeningOrders(), {orderA.get()});
    auto orderFieldsB = BuildOrderFields(Side::ASK, 100, Money::ONE);
    auto orderB = AddOrder(2, orderFieldsB, book);
    AssertOpeningOrders(book.GetOpeningOrders(), {orderA.get(), orderB.get()});
    Fill(*orderA, book, orderA->GetInfo().m_fields.m_quantity,
      orderA->GetInfo().m_fields.m_price);
    AssertOpeningOrders(book.GetOpeningOrders(), {});
    auto orderFieldsC = BuildOrderFields(Side::ASK, 100, Money::ONE);
    auto orderC = AddOrder(3, orderFieldsC, book);
    AssertOpeningOrders(book.GetOpeningOrders(), {orderC.get()});
    Fill(*orderC, book, orderC->GetInfo().m_fields.m_quantity,
      orderC->GetInfo().m_fields.m_price);
    AssertOpeningOrders(book.GetOpeningOrders(), {orderB.get()});
    Fill(*orderB, book, orderB->GetInfo().m_fields.m_quantity,
      orderB->GetInfo().m_fields.m_price);
    AssertOpeningOrders(book.GetOpeningOrders(), {});
    auto orderFieldsD = BuildOrderFields(Side::BID, 200, Money::ONE);
    auto orderD = AddOrder(4, orderFieldsD, book);
    AssertOpeningOrders(book.GetOpeningOrders(), {orderD.get()});
  }
}
