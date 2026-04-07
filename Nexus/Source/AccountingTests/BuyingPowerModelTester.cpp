#include <doctest/doctest.h>
#include "Nexus/Accounting/BuyingPowerModel.hpp"
#include "Nexus/Definitions/Security.hpp"

using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  auto TST = Security("TST", TSX);
  auto S32 = Security("S32", ASX);

  auto make_order_fields(Security security, CurrencyId currency, Side side,
      Quantity quantity, Money price) {
    auto fields = OrderFields();
    fields.m_security = security;
    fields.m_currency = currency;
    fields.m_side = side;
    fields.m_quantity = quantity;
    fields.m_price = price;
    return fields;
  }

  auto make_execution_report(OrderId id, OrderStatus status,
      Quantity last_quantity, Money last_price) {
    auto report = ExecutionReport();
    report.m_id = id;
    report.m_status = status;
    report.m_last_quantity = last_quantity;
    report.m_last_price = last_price;
    return report;
  }
}

TEST_SUITE("BuyingPowerModel") {
  TEST_CASE("empty_model") {
    auto model = BuyingPowerModel();
    REQUIRE(model.get_buying_power(CAD) == Money::ZERO);
    REQUIRE(model.get_buying_power(AUD) == Money::ZERO);
    REQUIRE(!model.has_order(1));
  }

  TEST_CASE("single_buy_order") {
    auto model = BuyingPowerModel();
    auto fields = make_order_fields(TST, CAD, Side::BID, 100, 10 * Money::ONE);
    model.submit(1, fields, 10 * Money::ONE);
    REQUIRE(model.has_order(1));
    REQUIRE(model.get_buying_power(CAD) == 1000 * Money::ONE);
  }

  TEST_CASE("single_sell_order") {
    auto model = BuyingPowerModel();
    auto fields = make_order_fields(TST, CAD, Side::ASK, 100, 10 * Money::ONE);
    model.submit(1, fields, 10 * Money::ONE);
    REQUIRE(model.has_order(1));
    REQUIRE(model.get_buying_power(CAD) == 1000 * Money::ONE);
  }

  TEST_CASE("buy_order_and_full_fill") {
    auto model = BuyingPowerModel();
    auto fields = make_order_fields(TST, CAD, Side::BID, 100, 10 * Money::ONE);
    model.submit(1, fields, 10 * Money::ONE);
    auto report =
      make_execution_report(1, OrderStatus::FILLED, 100, 10 * Money::ONE);
    model.update(report);
    REQUIRE(model.get_buying_power(CAD) == 1000 * Money::ONE);
  }

  TEST_CASE("sell_order_and_full_fill") {
    auto model = BuyingPowerModel();
    auto fields = make_order_fields(TST, CAD, Side::ASK, 100, 10 * Money::ONE);
    model.submit(1, fields, 10 * Money::ONE);
    auto report =
      make_execution_report(1, OrderStatus::FILLED, 100, 10 * Money::ONE);
    model.update(report);
    REQUIRE(model.get_buying_power(CAD) == 1000 * Money::ONE);
  }

  TEST_CASE("buy_order_and_partial_fill") {
    auto model = BuyingPowerModel();
    auto fields = make_order_fields(TST, CAD, Side::BID, 100, 10 * Money::ONE);
    model.submit(1, fields, 10 * Money::ONE);
    auto report = make_execution_report(
      1, OrderStatus::PARTIALLY_FILLED, 50, 10 * Money::ONE);
    model.update(report);
    REQUIRE(model.get_buying_power(CAD) == 1000 * Money::ONE);
  }

  TEST_CASE("sell_order_and_partial_fill") {
    auto model = BuyingPowerModel();
    auto fields = make_order_fields(TST, CAD, Side::ASK, 100, 10 * Money::ONE);
    model.submit(1, fields, 10 * Money::ONE);
    auto report = make_execution_report(
      1, OrderStatus::PARTIALLY_FILLED, 50, 10 * Money::ONE);
    model.update(report);
    REQUIRE(model.get_buying_power(CAD) == 1000 * Money::ONE);
  }

  TEST_CASE("buy_order_and_cancellation") {
    auto model = BuyingPowerModel();
    auto fields = make_order_fields(TST, CAD, Side::BID, 100, 10 * Money::ONE);
    model.submit(1, fields, 10 * Money::ONE);
    auto report =
      make_execution_report(1, OrderStatus::CANCELED, 0, Money::ZERO);
    model.update(report);
    REQUIRE(model.get_buying_power(CAD) == Money::ZERO);
  }

  TEST_CASE("long_position_and_sell_order") {
    auto model = BuyingPowerModel();
    auto buy_fields =
      make_order_fields(TST, CAD, Side::BID, 100, 10 * Money::ONE);
    model.submit(1, buy_fields, 10 * Money::ONE);
    auto buy_fill_report =
      make_execution_report(1, OrderStatus::FILLED, 100, 10 * Money::ONE);
    model.update(buy_fill_report);
    auto sell_fields =
      make_order_fields(TST, CAD, Side::ASK, 50, 11 * Money::ONE);
    model.submit(2, sell_fields, 11 * Money::ONE);
    REQUIRE(model.get_buying_power(CAD) == 1000 * Money::ONE);
    auto sell_fill_report =
      make_execution_report(2, OrderStatus::FILLED, 50, 11 * Money::ONE);
    model.update(sell_fill_report);
    REQUIRE(model.get_buying_power(CAD) == 500 * Money::ONE);
  }

  TEST_CASE("short_position_and_buy_order") {
    auto model = BuyingPowerModel();
    auto sell_fields =
      make_order_fields(TST, CAD, Side::ASK, 100, 10 * Money::ONE);
    model.submit(1, sell_fields, 10 * Money::ONE);
    auto sell_fill_report =
      make_execution_report(1, OrderStatus::FILLED, 100, 10 * Money::ONE);
    model.update(sell_fill_report);
    auto buy_fields =
      make_order_fields(TST, CAD, Side::BID, 50, 9 * Money::ONE);
    model.submit(2, buy_fields, 9 * Money::ONE);
    REQUIRE(model.get_buying_power(CAD) == 1000 * Money::ONE);
    auto buy_fill_report =
      make_execution_report(2, OrderStatus::FILLED, 50, 9 * Money::ONE);
    model.update(buy_fill_report);
    REQUIRE(model.get_buying_power(CAD) == 500 * Money::ONE);
  }

  TEST_CASE("multiple_buy_orders") {
    auto model = BuyingPowerModel();
    auto fields1 = make_order_fields(TST, CAD, Side::BID, 100, 10 * Money::ONE);
    model.submit(1, fields1, 10 * Money::ONE);
    auto fields2 = make_order_fields(TST, CAD, Side::BID, 50, 11 * Money::ONE);
    model.submit(2, fields2, 11 * Money::ONE);
    REQUIRE(model.get_buying_power(CAD) == 1550 * Money::ONE);
  }

  TEST_CASE("multiple_sell_orders") {
    auto model = BuyingPowerModel();
    auto fields1 = make_order_fields(TST, CAD, Side::ASK, 100, 10 * Money::ONE);
    model.submit(1, fields1, 10 * Money::ONE);
    auto fields2 = make_order_fields(TST, CAD, Side::ASK, 50, 11 * Money::ONE);
    model.submit(2, fields2, 11 * Money::ONE);
    REQUIRE(model.get_buying_power(CAD) == 1550 * Money::ONE);
  }

  TEST_CASE("multiple_orders_opposite_sides") {
    auto model = BuyingPowerModel();
    auto buy_fields =
      make_order_fields(TST, CAD, Side::BID, 100, 10 * Money::ONE);
    model.submit(1, buy_fields, 10 * Money::ONE);
    auto sell_fields =
      make_order_fields(TST, CAD, Side::ASK, 50, 11 * Money::ONE);
    model.submit(2, sell_fields, 11 * Money::ONE);
    REQUIRE(model.get_buying_power(CAD) == 1000 * Money::ONE);
  }

  TEST_CASE("multiple_securities") {
    auto model = BuyingPowerModel();
    auto tst_fields =
      make_order_fields(TST, CAD, Side::BID, 100, 10 * Money::ONE);
    model.submit(1, tst_fields, 10 * Money::ONE);
    auto S32_fields =
      make_order_fields(S32, AUD, Side::BID, 50, 150 * Money::ONE);
    model.submit(2, S32_fields, 150 * Money::ONE);
    REQUIRE(model.get_buying_power(CAD) == 1000 * Money::ONE);
    REQUIRE(model.get_buying_power(AUD) == 7500 * Money::ONE);
  }

  TEST_CASE("flipping_from_long_to_short") {
    auto model = BuyingPowerModel();
    auto buy_fields =
      make_order_fields(TST, CAD, Side::BID, 100, 10 * Money::ONE);
    model.submit(1, buy_fields, 10 * Money::ONE);
    auto fill_report1 =
      make_execution_report(1, OrderStatus::FILLED, 100, 10 * Money::ONE);
    model.update(fill_report1);
    auto sell_fields =
      make_order_fields(TST, CAD, Side::ASK, 150, 11 * Money::ONE);
    model.submit(2, sell_fields, 11 * Money::ONE);
    auto fill_report2 =
      make_execution_report(2, OrderStatus::FILLED, 150, 11 * Money::ONE);
    model.update(fill_report2);
    REQUIRE(model.get_buying_power(CAD) == 550 * Money::ONE);
  }

  TEST_CASE("flipping_from_short_to_long") {
    auto model = BuyingPowerModel();
    auto sell_fields =
      make_order_fields(TST, CAD, Side::ASK, 100, 10 * Money::ONE);
    model.submit(1, sell_fields, 10 * Money::ONE);
    auto fill_report1 =
      make_execution_report(1, OrderStatus::FILLED, 100, 10 * Money::ONE);
    model.update(fill_report1);
    auto buy_fields =
      make_order_fields(TST, CAD, Side::BID, 150, 9 * Money::ONE);
    model.submit(2, buy_fields, 9 * Money::ONE);
    auto fill_report2 =
      make_execution_report(2, OrderStatus::FILLED, 150, 9 * Money::ONE);
    model.update(fill_report2);
    REQUIRE(model.get_buying_power(CAD) == 450 * Money::ONE);
  }
}
