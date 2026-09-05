#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Spire/BookView/LocalBookViewModel.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto TICKER = parse_ticker("ABX.TSX");

  auto make_book_quote(std::string mpid, Money price, Quantity size,
      Side side) {
    return BookQuote(std::move(mpid), true, Venues::TSX,
      Quote(price, size, side), time_from_string("2025-08-14 09:00:00"));
  }

  auto make_bbo(Money bid, Money ask) {
    return BboQuote(make_bid(bid, 100), make_ask(ask, 100),
      time_from_string("2025-08-14 09:00:00"));
  }

  auto make_default_bbo() {
    return make_bbo(parse_money("10.00"), parse_money("10.01"));
  }

  auto make_order(const OrderFields& fields) {
    auto info = OrderInfo(fields, 1, time_from_string("2025-08-14 09:00:00"));
    return std::make_shared<PrimitiveOrder>(info);
  }

  auto make_entry(std::shared_ptr<Order> order) {
    return OrderLogModel::OrderEntry(std::move(order));
  }
}

TEST_SUITE("LocalBookViewModel") {
  TEST_CASE("update_bbo") {
    auto model = LocalBookViewModel(TICKER);
    model.update(make_default_bbo());
    auto& bbo = model.get_bbo_quote()->get();
    REQUIRE(bbo.m_bid.m_price == parse_money("10.00"));
    REQUIRE(bbo.m_ask.m_price == parse_money("10.01"));
  }

  TEST_CASE("update_book_quote") {
    auto model = LocalBookViewModel(TICKER);
    model.update(make_book_quote("TSX", parse_money("10.00"), 100, Side::BID));
    REQUIRE(model.get_bids()->get_size() == 1);
    REQUIRE(model.get_bids()->get(0).m_quote.m_price == parse_money("10.00"));
    model.update(make_book_quote("TSX", parse_money("10.01"), 200, Side::ASK));
    REQUIRE(model.get_asks()->get_size() == 1);
    REQUIRE(model.get_asks()->get(0).m_quote.m_price == parse_money("10.01"));
  }

  TEST_CASE("bid_quote_ordering_at_depth") {
    auto model = LocalBookViewModel(TICKER);
    auto price = [] (int level) {
      return parse_money("10.00") - level * 2 * Money::CENT;
    };
    for(auto i = 0; i != 20; ++i) {
      model.update(
        make_book_quote("M" + std::to_string(i), price(i), 100, Side::BID));
    }
    auto bids = model.get_bids();
    REQUIRE(bids->get_size() == 20);
    for(auto i = 0; i != 20; ++i) {
      REQUIRE(bids->get(i).m_quote.m_price == price(i));
    }
    model.update(make_book_quote("A", parse_money("9.99"), 100, Side::BID));
    REQUIRE(bids->get_size() == 21);
    REQUIRE(bids->get(1).m_mpid == "A");
    model.update(make_book_quote("B", parse_money("9.63"), 100, Side::BID));
    REQUIRE(bids->get_size() == 22);
    REQUIRE(bids->get(20).m_mpid == "B");
    model.update(make_book_quote("C", parse_money("10.50"), 100, Side::BID));
    REQUIRE(bids->get_size() == 23);
    REQUIRE(bids->get(0).m_mpid == "C");
    model.update(make_book_quote("D", parse_money("9.00"), 100, Side::BID));
    REQUIRE(bids->get_size() == 24);
    REQUIRE(bids->get(23).m_mpid == "D");
    model.update(make_book_quote("M10", price(10), 0, Side::BID));
    REQUIRE(bids->get_size() == 23);
    for(auto i = 1; i != bids->get_size(); ++i) {
      REQUIRE(bids->get(i - 1).m_quote.m_price >= bids->get(i).m_quote.m_price);
    }
  }

  TEST_CASE("ask_quote_ordering_at_depth") {
    auto model = LocalBookViewModel(TICKER);
    auto price = [] (int level) {
      return parse_money("10.00") + level * 2 * Money::CENT;
    };
    for(auto i = 0; i != 20; ++i) {
      model.update(
        make_book_quote("M" + std::to_string(i), price(i), 100, Side::ASK));
    }
    auto asks = model.get_asks();
    REQUIRE(asks->get_size() == 20);
    model.update(make_book_quote("A", parse_money("10.01"), 100, Side::ASK));
    REQUIRE(asks->get(1).m_mpid == "A");
    model.update(make_book_quote("B", parse_money("9.50"), 100, Side::ASK));
    REQUIRE(asks->get(0).m_mpid == "B");
    model.update(make_book_quote("C", parse_money("11.00"), 100, Side::ASK));
    REQUIRE(asks->get(asks->get_size() - 1).m_mpid == "C");
    for(auto i = 1; i != asks->get_size(); ++i) {
      REQUIRE(asks->get(i - 1).m_quote.m_price <= asks->get(i).m_quote.m_price);
    }
  }

  TEST_CASE("remove_book_quote") {
    auto model = LocalBookViewModel(TICKER);
    model.update(make_book_quote("TSX", parse_money("10.00"), 100, Side::BID));
    REQUIRE(model.get_bids()->get_size() == 1);
    model.update(make_book_quote("TSX", parse_money("10.00"), 0, Side::BID));
    REQUIRE(model.get_bids()->get_size() == 0);
  }

  TEST_CASE("update_time_and_sale") {
    auto model = LocalBookViewModel(TICKER);
    auto time_and_sale = TimeAndSale();
    time_and_sale.m_price = parse_money("10.00");
    time_and_sale.m_size = 100;
    time_and_sale.m_market_center = "TSE";
    model.update(time_and_sale);
    auto& technicals = model.get_session_technicals()->get();
    REQUIRE(technicals.m_open == parse_money("10.00"));
    REQUIRE(technicals.m_high == parse_money("10.00"));
    REQUIRE(technicals.m_low == parse_money("10.00"));
    REQUIRE(technicals.m_volume == 100);
  }

  TEST_CASE("add_limit_order") {
    auto model = LocalBookViewModel(TICKER);
    auto order = make_order(
      make_limit_order_fields(TICKER, Side::BID, 100, parse_money("10.00")));
    model.add(make_entry(order));
    REQUIRE(model.get_bid_orders()->get_size() == 1);
    auto& user_order = model.get_bid_orders()->get(0);
    REQUIRE(user_order.m_price == parse_money("10.00"));
    REQUIRE(user_order.m_size == 100);
  }

  TEST_CASE("remove_order") {
    auto model = LocalBookViewModel(TICKER);
    auto order = make_order(
      make_limit_order_fields(TICKER, Side::ASK, 200, parse_money("10.05")));
    auto entry = make_entry(order);
    model.add(entry);
    REQUIRE(model.get_ask_orders()->get_size() == 1);
    model.remove(entry);
    REQUIRE(model.get_ask_orders()->get_size() == 0);
  }

  TEST_CASE("execution_report") {
    auto model = LocalBookViewModel(TICKER);
    auto order = make_order(
      make_limit_order_fields(TICKER, Side::BID, 100, parse_money("10.00")));
    model.add(make_entry(order));
    auto report = ExecutionReport();
    report.m_id = order->get_info().m_id;
    report.m_status = OrderStatus::PARTIALLY_FILLED;
    report.m_last_quantity = 40;
    report.m_last_price = parse_money("10.00");
    model.update(report);
    REQUIRE(model.get_bid_orders()->get_size() == 1);
    auto& user_order = model.get_bid_orders()->get(0);
    REQUIRE(user_order.m_status == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(user_order.m_size == 60);
    report.m_status = OrderStatus::FILLED;
    report.m_last_quantity = 60;
    model.update(report);
    REQUIRE(model.get_bid_orders()->get_size() == 0);
  }

  TEST_CASE("pegged_order") {
    auto model = LocalBookViewModel(TICKER);
    model.update(make_default_bbo());
    auto order = make_order(make_pegged_order_fields(
      TICKER, Side::BID, 100, Money::ZERO, Money::ZERO));
    model.add(make_entry(order));
    REQUIRE(model.get_bid_orders()->get_size() == 1);
    REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("10.00"));
  }

  TEST_CASE("pegged_order_submitted_before_bbo") {
    auto model = LocalBookViewModel(TICKER);
    SUBCASE("bid") {
      auto order = make_order(make_pegged_order_fields(
        TICKER, Side::BID, 100, Money::ZERO, Money::ZERO));
      model.add(make_entry(order));
      model.update(make_default_bbo());
      REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("10.00"));
    }
    SUBCASE("ask") {
      auto order = make_order(make_pegged_order_fields(
        TICKER, Side::ASK, 100, Money::ZERO, Money::ZERO));
      model.add(make_entry(order));
      model.update(make_default_bbo());
      REQUIRE(model.get_ask_orders()->get(0).m_price == parse_money("10.01"));
    }
    SUBCASE("ask_with_limit") {
      auto order = make_order(make_pegged_order_fields(
        TICKER, Side::ASK, 100, parse_money("9.95"), Money::ZERO));
      model.add(make_entry(order));
      model.update(make_default_bbo());
      REQUIRE(model.get_ask_orders()->get(0).m_price == parse_money("10.01"));
    }
  }

  TEST_CASE("pegged_bid_price_without_bbo") {
    auto model = LocalBookViewModel(TICKER);
    auto order = make_order(make_pegged_order_fields(
      TICKER, Side::BID, 100, parse_money("10.00"), parse_money("0.05")));
    model.add(make_entry(order));
    REQUIRE(model.get_bid_orders()->get_size() == 1);
    REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("10.00"));
  }

  TEST_CASE("pegged_order_ratchet") {
    auto model = LocalBookViewModel(TICKER);
    model.update(make_default_bbo());
    auto order = make_order(make_pegged_order_fields(
      TICKER, Side::BID, 100, Money::ZERO, Money::ZERO));
    model.add(make_entry(order));
    model.update(make_bbo(parse_money("10.50"), parse_money("10.60")));
    REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("10.50"));
    model.update(make_bbo(parse_money("9.80"), parse_money("9.90")));
    REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("10.50"));
  }

  TEST_CASE("pegged_order_with_limit") {
    auto model = LocalBookViewModel(TICKER);
    model.update(make_bbo(parse_money("9.99"), parse_money("10.01")));
    auto order = make_order(make_pegged_order_fields(TICKER,
      Side::ASK, 100, parse_money("9.95"), Money::ZERO));
    model.add(make_entry(order));
    REQUIRE(model.get_ask_orders()->get(0).m_price == parse_money("10.01"));
    model.update(make_bbo(parse_money("9.80"), parse_money("9.90")));
    REQUIRE(model.get_ask_orders()->get(0).m_price == parse_money("9.95"));
  }

  TEST_CASE("pegged_order_with_peg_difference") {
    auto model = LocalBookViewModel(TICKER);
    model.update(make_default_bbo());
    auto order = make_order(make_pegged_order_fields(TICKER,
      Side::BID, 100, Money::ZERO, parse_money("0.03")));
    model.add(make_entry(order));
    REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("9.97"));
  }

  TEST_CASE("pegged_order_peg_type") {
    auto model = LocalBookViewModel(TICKER);
    SUBCASE("market") {
      model.update(make_bbo(parse_money("9.99"), parse_money("10.00")));
      auto order = make_order(make_pegged_order_fields(TICKER,
        Side::ASK, 100, Money::ZERO, Money::ZERO, PegType::MARKET));
      model.add(make_entry(order));
      REQUIRE(model.get_ask_orders()->get(0).m_price == parse_money("9.99"));
    }
    SUBCASE("midpoint") {
      model.update(make_bbo(parse_money("9.90"), parse_money("10.10")));
      auto order = make_order(make_pegged_order_fields(TICKER,
        Side::BID, 100, Money::ZERO, Money::ZERO, PegType::MID_POINT));
      model.add(make_entry(order));
      REQUIRE(model.get_bid_orders()->get(0).m_price == parse_money("10.00"));
    }
  }

  TEST_CASE("pegged_entry_removed_on_terminal") {
    auto model = LocalBookViewModel(TICKER);
    model.update(make_default_bbo());
    auto order = make_order(make_pegged_order_fields(
      TICKER, Side::BID, 100, Money::ZERO, Money::ZERO));
    model.add(make_entry(order));
    auto report = ExecutionReport();
    report.m_id = order->get_info().m_id;
    report.m_status = OrderStatus::FILLED;
    report.m_last_quantity = 100;
    model.update(report);
    REQUIRE(model.get_bid_orders()->get_size() == 0);
    model.update(make_bbo(parse_money("10.50"), parse_money("10.60")));
    REQUIRE(model.get_bid_orders()->get_size() == 0);
  }

  TEST_CASE("clear_orders") {
    auto model = LocalBookViewModel(TICKER);
    model.update(make_default_bbo());
    model.add(make_entry(make_order(make_limit_order_fields(
      TICKER, Side::BID, 100, parse_money("10.00")))));
    model.add(make_entry(make_order(make_pegged_order_fields(
      TICKER, Side::ASK, 200, Money::ZERO, Money::ZERO))));
    REQUIRE(model.get_bid_orders()->get_size() == 1);
    REQUIRE(model.get_ask_orders()->get_size() == 1);
    model.clear_orders();
    REQUIRE(model.get_bid_orders()->get_size() == 0);
    REQUIRE(model.get_ask_orders()->get_size() == 0);
  }

  TEST_CASE("add_order_with_quantity_and_status") {
    auto model = LocalBookViewModel(TICKER);
    SUBCASE("bid") {
      auto order = make_order(
        make_limit_order_fields(TICKER, Side::BID, 500, parse_money("10.00")));
      model.add(make_entry(order), 300, OrderStatus::NEW);
      REQUIRE(model.get_bid_orders()->get_size() == 1);
      auto& user_order = model.get_bid_orders()->get(0);
      REQUIRE(user_order.m_size == 300);
      REQUIRE(user_order.m_status == OrderStatus::NEW);
      REQUIRE(user_order.m_price == parse_money("10.00"));
    }
    SUBCASE("zero_remaining") {
      auto order = make_order(
        make_limit_order_fields(TICKER, Side::ASK, 100, parse_money("10.05")));
      model.add(make_entry(order), 0, OrderStatus::NEW);
      REQUIRE(model.get_ask_orders()->get_size() == 1);
      auto& user_order = model.get_ask_orders()->get(0);
      REQUIRE(user_order.m_size == 0);
      REQUIRE(user_order.m_status == OrderStatus::NEW);
    }
    SUBCASE("pending_new") {
      auto order = make_order(
        make_limit_order_fields(TICKER, Side::BID, 200, parse_money("10.00")));
      model.add(make_entry(order), 200, OrderStatus::PENDING_NEW);
      REQUIRE(model.get_bid_orders()->get_size() == 1);
      auto& user_order = model.get_bid_orders()->get(0);
      REQUIRE(user_order.m_size == 200);
      REQUIRE(user_order.m_status == OrderStatus::PENDING_NEW);
    }
    SUBCASE("pegged") {
      model.update(make_default_bbo());
      auto order = make_order(make_pegged_order_fields(
        TICKER, Side::BID, 500, Money::ZERO, Money::ZERO));
      model.add(make_entry(order), 300, OrderStatus::NEW);
      REQUIRE(model.get_bid_orders()->get_size() == 1);
      auto& user_order = model.get_bid_orders()->get(0);
      REQUIRE(user_order.m_size == 300);
      REQUIRE(user_order.m_status == OrderStatus::NEW);
      REQUIRE(user_order.m_price == parse_money("10.00"));
    }
  }

  TEST_CASE("clear_book_quotes") {
    auto model = LocalBookViewModel(TICKER);
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
    auto model = LocalBookViewModel(TICKER);
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
