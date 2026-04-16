#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Spire/BookView/LocalBookViewModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_book_quote(std::string mpid, Money price, Quantity size,
      Side side) {
    return BookQuote(std::move(mpid), true, DefaultVenues::TSX,
      Quote(price, size, side), time_from_string("2025-08-14 09:00:00"));
  }

  auto make_bbo(Money bid, Money ask) {
    return BboQuote(make_bid(bid, 100), make_ask(ask, 100),
      time_from_string("2025-08-14 09:00:00"));
  }

  auto make_order(const OrderFields& fields) {
    auto info = OrderInfo(fields, 1, time_from_string("2025-08-14 09:00:00"));
    return std::make_shared<PrimitiveOrder>(info);
  }

  auto make_order(const OrderFields& fields, OrderId id) {
    auto info = OrderInfo(fields, id, time_from_string("2025-08-14 09:00:00"));
    return std::make_shared<PrimitiveOrder>(info);
  }

  auto make_entry(std::shared_ptr<Order> order) {
    return OrderLogModel::OrderEntry(std::move(order));
  }
}

TEST_SUITE("LocalBookViewModel") {
  TEST_CASE("update_bbo") {
    auto model = LocalBookViewModel();
    model.update(make_bbo(parse_money("10.00"), parse_money("10.01")));
    auto& bbo = model.get_bbo_quote()->get();
    REQUIRE(bbo.m_bid.m_price == parse_money("10.00"));
    REQUIRE(bbo.m_ask.m_price == parse_money("10.01"));
  }

  TEST_CASE("update_book_quote") {
    auto model = LocalBookViewModel();
    model.update(make_book_quote("TSX", parse_money("10.00"), 100, Side::BID));
    REQUIRE(model.get_bids()->get_size() == 1);
    REQUIRE(model.get_bids()->get(0).m_quote.m_price == parse_money("10.00"));
    model.update(make_book_quote("TSX", parse_money("10.01"), 200, Side::ASK));
    REQUIRE(model.get_asks()->get_size() == 1);
    REQUIRE(model.get_asks()->get(0).m_quote.m_price == parse_money("10.01"));
  }

  TEST_CASE("remove_book_quote") {
    auto model = LocalBookViewModel();
    model.update(make_book_quote("TSX", parse_money("10.00"), 100, Side::BID));
    REQUIRE(model.get_bids()->get_size() == 1);
    model.update(make_book_quote("TSX", parse_money("10.00"), 0, Side::BID));
    REQUIRE(model.get_bids()->get_size() == 0);
  }

  TEST_CASE("update_time_and_sale") {
    auto model = LocalBookViewModel();
    auto ts = TimeAndSale();
    ts.m_price = parse_money("10.00");
    ts.m_size = 100;
    model.update(ts);
    auto& candlestick = model.get_session_candlestick()->get();
    REQUIRE(candlestick.get_open() == parse_money("10.00"));
    REQUIRE(candlestick.get_close() == parse_money("10.00"));
    REQUIRE(candlestick.get_volume() == 100);
  }

  TEST_CASE("add_limit_order") {
    auto model = LocalBookViewModel();
    auto order = make_order(make_limit_order_fields(
      parse_ticker("ABX.TSX"), Side::BID, 100, parse_money("10.00")));
    model.add(make_entry(order));
    REQUIRE(model.get_bid_orders()->get_size() == 1);
    auto& user_order = model.get_bid_orders()->get(0);
    REQUIRE(user_order.m_price == parse_money("10.00"));
    REQUIRE(user_order.m_size == 100);
  }

  TEST_CASE("remove_order") {
    auto model = LocalBookViewModel();
    auto order = make_order(make_limit_order_fields(
      parse_ticker("ABX.TSX"), Side::ASK, 200, parse_money("10.05")));
    auto entry = make_entry(order);
    model.add(entry);
    REQUIRE(model.get_ask_orders()->get_size() == 1);
    model.remove(entry);
    REQUIRE(model.get_ask_orders()->get_size() == 0);
  }

  TEST_CASE("execution_report") {
    auto model = LocalBookViewModel();
    auto order = make_order(make_limit_order_fields(
      parse_ticker("ABX.TSX"), Side::BID, 100, parse_money("10.00")));
    model.add(make_entry(order));
    auto report = ExecutionReport();
    report.m_id = order->get_info().m_id;
    report.m_status = OrderStatus::FILLED;
    report.m_last_quantity = 100;
    report.m_last_price = parse_money("10.00");
    model.update(report);
    auto& user_order = model.get_bid_orders()->get(0);
    REQUIRE(user_order.m_status == OrderStatus::FILLED);
    REQUIRE(user_order.m_size == 0);
  }

  TEST_CASE("pegged_order") {
    auto model = LocalBookViewModel();
    model.update(make_bbo(parse_money("10.00"), parse_money("10.01")));
    auto order = make_order(make_pegged_order_fields(
      parse_ticker("ABX.TSX"), Side::BID, 100, Money::ZERO, Money::ZERO));
    model.add(make_entry(order));
    REQUIRE(model.get_bid_orders()->get_size() == 1);
    REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("10.00"));
  }

  TEST_CASE("pegged_order_follows_bbo") {
    auto model = LocalBookViewModel();
    model.update(make_bbo(parse_money("10.00"), parse_money("10.01")));
    auto order = make_order(make_pegged_order_fields(
      parse_ticker("ABX.TSX"), Side::BID, 100, Money::ZERO, Money::ZERO));
    model.add(make_entry(order));
    model.update(make_bbo(parse_money("10.50"), parse_money("10.60")));
    REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("10.50"));
  }

  TEST_CASE("pegged_order_ratchet") {
    auto model = LocalBookViewModel();
    model.update(make_bbo(parse_money("10.00"), parse_money("10.01")));
    auto order = make_order(make_pegged_order_fields(
      parse_ticker("ABX.TSX"), Side::BID, 100, Money::ZERO, Money::ZERO));
    model.add(make_entry(order));
    model.update(make_bbo(parse_money("10.50"), parse_money("10.60")));
    REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("10.50"));
    model.update(make_bbo(parse_money("9.80"), parse_money("9.90")));
    REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("10.50"));
  }

  TEST_CASE("pegged_order_with_limit") {
    auto model = LocalBookViewModel();
    model.update(make_bbo(parse_money("9.99"), parse_money("10.01")));
    auto order = make_order(make_pegged_order_fields(parse_ticker("ABX.TSX"),
      Side::ASK, 100, parse_money("9.95"), Money::ZERO));
    model.add(make_entry(order));
    REQUIRE(model.get_ask_orders()->get(0).m_price == parse_money("10.01"));
    model.update(make_bbo(parse_money("9.80"), parse_money("9.90")));
    REQUIRE(model.get_ask_orders()->get(0).m_price == parse_money("9.95"));
  }

  TEST_CASE("pegged_order_with_peg_difference") {
    auto model = LocalBookViewModel();
    model.update(make_bbo(parse_money("10.00"), parse_money("10.01")));
    auto order = make_order(make_pegged_order_fields(parse_ticker("ABX.TSX"),
      Side::BID, 100, Money::ZERO, parse_money("0.03")));
    model.add(make_entry(order));
    REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("9.97"));
  }

  TEST_CASE("market_pegged_order") {
    auto model = LocalBookViewModel();
    model.update(make_bbo(parse_money("9.99"), parse_money("10.00")));
    auto order = make_order(make_pegged_order_fields(parse_ticker("ABX.TSX"),
      Side::ASK, 100, Money::ZERO, Money::ZERO, PegType::MARKET));
    model.add(make_entry(order));
    REQUIRE(model.get_ask_orders()->get(0).m_price == parse_money("9.99"));
  }

  TEST_CASE("midpoint_pegged_order") {
    auto model = LocalBookViewModel();
    model.update(make_bbo(parse_money("9.90"), parse_money("10.10")));
    auto order = make_order(make_pegged_order_fields(parse_ticker("ABX.TSX"),
      Side::BID, 100, Money::ZERO, Money::ZERO, PegType::MID_POINT));
    model.add(make_entry(order));
    REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("10.00"));
  }

  TEST_CASE("pegged_entry_removed_on_terminal") {
    auto model = LocalBookViewModel();
    model.update(make_bbo(parse_money("10.00"), parse_money("10.01")));
    auto order = make_order(make_pegged_order_fields(
      parse_ticker("ABX.TSX"), Side::BID, 100, Money::ZERO, Money::ZERO));
    model.add(make_entry(order));
    auto report = ExecutionReport();
    report.m_id = order->get_info().m_id;
    report.m_status = OrderStatus::FILLED;
    report.m_last_quantity = 100;
    model.update(report);
    model.update(make_bbo(parse_money("10.50"), parse_money("10.60")));
    REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("10.00"));
  }

  TEST_CASE("clear_orders") {
    auto model = LocalBookViewModel();
    model.update(make_bbo(parse_money("10.00"), parse_money("10.01")));
    model.add(make_entry(make_order(make_limit_order_fields(
      parse_ticker("ABX.TSX"), Side::BID, 100, parse_money("10.00")))));
    model.add(make_entry(make_order(make_pegged_order_fields(
      parse_ticker("ABX.TSX"), Side::ASK, 200, Money::ZERO, Money::ZERO))));
    REQUIRE(model.get_bid_orders()->get_size() == 1);
    REQUIRE(model.get_ask_orders()->get_size() == 1);
    model.clear_orders();
    REQUIRE(model.get_bid_orders()->get_size() == 0);
    REQUIRE(model.get_ask_orders()->get_size() == 0);
  }

  TEST_CASE("clear_book_quotes") {
    auto model = LocalBookViewModel();
    model.update(make_book_quote("TSX", parse_money("10.00"), 100, Side::BID));
    model.update(make_book_quote("ARCA", parse_money("9.99"), 200, Side::BID));
    model.update(make_book_quote("TSX", parse_money("10.01"), 150, Side::ASK));
    REQUIRE(model.get_bids()->get_size() == 2);
    REQUIRE(model.get_asks()->get_size() == 1);
    model.clear_book_quotes();
    REQUIRE(model.get_bids()->get_size() == 0);
    REQUIRE(model.get_asks()->get_size() == 0);
  }

  TEST_CASE("transact") {
    auto model = LocalBookViewModel();
    model.transact([&] {
      model.update(
        make_book_quote("TSX", parse_money("10.00"), 100, Side::BID));
      model.update(
        make_book_quote("TSX", parse_money("10.01"), 200, Side::ASK));
    });
    REQUIRE(model.get_bids()->get_size() == 1);
    REQUIRE(model.get_asks()->get_size() == 1);
  }
}
