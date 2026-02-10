#include <doctest/doctest.h>
#include "Nexus/Accounting/PositionOrderBook.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

TEST_SUITE("PositionOrderBook") {
  TEST_CASE("constructor") {
    auto book = PositionOrderBook();
    REQUIRE(book.get_positions().empty());
    REQUIRE(book.get_live_orders().empty());
    REQUIRE(book.get_opening_orders().empty());
  }

  TEST_CASE("initial_positions_population") {
    auto ticker = parse_ticker("TST.TSX");
    auto inventory = Inventory(Position(
      ticker, CAD, 100, 100 * Money::ONE), Money::ZERO, Money::ZERO, 100, 1);
    auto book = PositionOrderBook(View(std::vector{inventory}));
    auto positions = book.get_positions();
    REQUIRE(positions.size() == 1);
    REQUIRE(positions.front().m_ticker == ticker);
    REQUIRE(positions.front().m_quantity == 100);
  }

  TEST_CASE("add_order_increases_live_orders") {
    auto book = PositionOrderBook();
    auto ticker = parse_ticker("TST.TSX");
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 50, Money::ONE);
    auto info =
      OrderInfo(fields, 1, false, time_from_string("2024-07-21 10:15:00.000"));
    auto order = std::make_shared<PrimitiveOrder>(info);
    book.add(order);
    auto live_orders = book.get_live_orders();
    REQUIRE(live_orders.size() == 1);
    REQUIRE(live_orders.front() == order);
  }

  TEST_CASE("add_order_opening_vs_closing") {
    auto ticker = parse_ticker("TST.TSX");
    auto inventory = Inventory(Position(
      ticker, CAD, 100, 100 * Money::ONE), Money::ZERO, Money::ZERO, 100, 1);
    auto book = PositionOrderBook(View(std::vector{inventory}));
    auto opening_fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 50, Money::ONE);
    auto closing_fields =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 50, Money::ONE);
    REQUIRE(book.test_opening_order_submission(opening_fields));
    REQUIRE(!book.test_opening_order_submission(closing_fields));
  }

  TEST_CASE("update_order_execution_report_changes_position") {
    auto ticker = parse_ticker("TST.TSX");
    auto book = PositionOrderBook();
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 50, Money::ONE);
    auto info =
      OrderInfo(fields, 1, false, time_from_string("2024-07-21 10:15:00.000"));
    auto order = std::make_shared<PrimitiveOrder>(info);
    book.add(order);
    book.update(accept(*order));
    book.update(fill(*order, 50));
    auto positions = book.get_positions();
    REQUIRE(positions.size() == 1);
    REQUIRE(positions.front().m_ticker == ticker);
    REQUIRE(positions.front().m_quantity == 50);
    auto live_orders = book.get_live_orders();
    REQUIRE(live_orders.empty());
  }

  TEST_CASE("multiple_orders_same_ticker") {
    auto ticker = parse_ticker("TST.TSX");
    auto book = PositionOrderBook();
    auto fields1 =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 30, Money::ONE);
    auto info1 =
      OrderInfo(fields1, 1, false, time_from_string("2024-07-21 10:15:00.000"));
    auto order1 = std::make_shared<PrimitiveOrder>(info1);
    book.add(order1);
    auto fields2 =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 20, Money::ONE);
    auto info2 =
      OrderInfo(fields2, 2, false, time_from_string("2024-07-21 10:25:00.000"));
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    book.add(order2);
    book.update(accept(*order1));
    book.update(fill(*order1, 30));
    book.update(accept(*order2));
    book.update(fill(*order2, 20));
    auto positions = book.get_positions();
    REQUIRE(positions.size() == 1);
    REQUIRE(positions.front().m_ticker == ticker);
    REQUIRE(positions.front().m_quantity == 50);
    auto live_orders = book.get_live_orders();
    REQUIRE(live_orders.empty());
  }

  TEST_CASE("get_opening_orders_returns_correct_orders") {
    auto ticker = parse_ticker("TST.TSX");
    auto book = PositionOrderBook();
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 50, Money::ONE);
    auto info =
      OrderInfo(fields, 1, false, time_from_string("2024-07-21 10:15:00.000"));
    auto order = std::make_shared<PrimitiveOrder>(info);
    book.add(order);
    book.update(accept(*order));
    auto opening_orders = book.get_opening_orders();
    REQUIRE(opening_orders.size() == 1);
    REQUIRE(opening_orders.front() == order);
  }

  TEST_CASE("get_positions_returns_nonzero_positions") {
    auto ticker1 = parse_ticker("TST.TSX");
    auto ticker2 = parse_ticker("ABC.TSX");
    auto book = PositionOrderBook();
    auto fields1 =
      make_limit_order_fields(ticker1, CAD, Side::BID, "TSX", 50, Money::ONE);
    auto info1 =
      OrderInfo(fields1, 1, false, time_from_string("2024-07-21 10:15:00.000"));
    auto order1 = std::make_shared<PrimitiveOrder>(info1);
    book.add(order1);
    book.update(accept(*order1));
    book.update(fill(*order1, 50));
    auto fields2 =
      make_limit_order_fields(ticker2, CAD, Side::BID, "TSX", 0, Money::ONE);
    auto info2 =
      OrderInfo(fields2, 2, false, time_from_string("2024-07-21 10:35:00.000"));
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    book.add(order2);
    book.update(accept(*order2));
    auto positions = book.get_positions();
    REQUIRE(positions.size() == 1);
    REQUIRE(positions.front().m_ticker == ticker1);
    REQUIRE(positions.front().m_quantity == 50);
  }

  TEST_CASE("partial_fill_updates_remaining_quantity") {
    auto ticker = parse_ticker("TST.TSX");
    auto book = PositionOrderBook();
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info =
      OrderInfo(fields, 1, false, time_from_string("2024-07-21 10:15:00.000"));
    auto order = std::make_shared<PrimitiveOrder>(info);
    book.add(order);
    book.update(accept(*order));
    book.update(fill(*order, 40));
    auto live_orders = book.get_live_orders();
    REQUIRE(live_orders.size() == 1);
    book.update(fill(*order, 60));
    live_orders = book.get_live_orders();
    REQUIRE(live_orders.empty());
    auto positions = book.get_positions();
    REQUIRE(positions.size() == 1);
    REQUIRE(positions.front().m_quantity == 100);
  }

  TEST_CASE("terminal_status_removes_order") {
    auto ticker = parse_ticker("TST.TSX");
    auto book = PositionOrderBook();
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 50, Money::ONE);
    auto info =
      OrderInfo(fields, 1, false, time_from_string("2024-07-21 10:15:00.000"));
    auto order = std::make_shared<PrimitiveOrder>(info);
    book.add(order);
    book.update(accept(*order));
    book.update(reject(*order));
    auto live_orders = book.get_live_orders();
    REQUIRE(live_orders.empty());
  }

  TEST_CASE("side_switching_orders") {
    auto ticker = parse_ticker("TST.TSX");
    auto book = PositionOrderBook();
    auto fields_long =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info_long = OrderInfo(
      fields_long, 1, false, time_from_string("2024-07-21 10:15:00.000"));
    auto order_long = std::make_shared<PrimitiveOrder>(info_long);
    book.add(order_long);
    book.update(accept(*order_long));
    book.update(fill(*order_long, 100));
    auto fields_short =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 150, Money::ONE);
    auto info_short = OrderInfo(
      fields_short, 2, false, time_from_string("2024-07-21 10:25:00.000"));
    auto order_short = std::make_shared<PrimitiveOrder>(info_short);
    book.add(order_short);
    book.update(accept(*order_short));
    book.update(fill(*order_short, 150));
    auto positions = book.get_positions();
    REQUIRE(positions.size() == 1);
    REQUIRE(positions.front().m_quantity == -50);
  }

  TEST_CASE("open_quantity_tracking") {
    auto ticker = parse_ticker("TST.TSX");
    auto book = PositionOrderBook();
    auto fields_bid =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info_bid = OrderInfo(
      fields_bid, 1, false, time_from_string("2024-07-21 10:15:00.000"));
    auto order_bid = std::make_shared<PrimitiveOrder>(info_bid);
    book.add(order_bid);
    book.update(accept(*order_bid));
    book.update(fill(*order_bid, 100));
    auto fields_ask =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 50, Money::ONE);
    auto info_ask = OrderInfo(
      fields_ask, 2, false, time_from_string("2024-07-21 10:16:00.000"));
    auto order_ask = std::make_shared<PrimitiveOrder>(info_ask);
    book.add(order_ask);
    book.update(accept(*order_ask));
    book.update(fill(*order_ask, 50));
    auto positions = book.get_positions();
    REQUIRE(positions.size() == 1);
    REQUIRE(positions.front().m_quantity == 50);
  }

  TEST_CASE("order_sequence_numbering") {
    auto ticker = parse_ticker("TST.TSX");
    auto book = PositionOrderBook();
    auto fields1 =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 10, Money::ONE);
    auto info1 =
      OrderInfo(fields1, 1, false, time_from_string("2024-07-21 10:15:00.000"));
    auto order1 = std::make_shared<PrimitiveOrder>(info1);
    book.add(order1);
    auto fields2 =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 20, Money::ONE);
    auto info2 =
      OrderInfo(fields2, 2, false, time_from_string("2024-07-21 10:25:00.000"));
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    book.add(order2);
    auto live_orders = book.get_live_orders();
    REQUIRE(live_orders.size() == 2);
    REQUIRE(live_orders[0] == order1);
    REQUIRE(live_orders[1] == order2);
  }
}
