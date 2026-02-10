#include <doctest/doctest.h>
#include "Nexus/Accounting/ShortingModel.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

TEST_SUITE("ShortingModel") {
  TEST_CASE("submit_bid_order_not_short") {
    auto model = ShortingModel();
    auto ticker = parse_ticker("TST.TSX");
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    REQUIRE(!model.submit(1, fields));
  }

  TEST_CASE("submit_ask_order_not_short_if_position_sufficient") {
    auto model = ShortingModel();
    auto ticker = parse_ticker("TST.TSX");
    auto bid_fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    model.submit(1, bid_fields);
    auto bid_report = ExecutionReport(1, boost::posix_time::not_a_date_time);
    bid_report.m_last_quantity = 100;
    bid_report.m_status = OrderStatus::FILLED;
    model.update(bid_report);
    auto ask_fields =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 50, Money::ONE);
    REQUIRE(!model.submit(2, ask_fields));
  }

  TEST_CASE("submit_ask_order_short_if_position_insufficient") {
    auto model = ShortingModel();
    auto ticker = parse_ticker("TST.TSX");
    auto ask_fields =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 50, Money::ONE);
    REQUIRE(model.submit(1, ask_fields));
  }

  TEST_CASE("multiple_ask_orders_accumulate_pending_and_trigger_short") {
    auto model = ShortingModel();
    auto ticker = parse_ticker("TST.TSX");
    auto bid_fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    model.submit(1, bid_fields);
    auto bid_report = ExecutionReport(1, boost::posix_time::not_a_date_time);
    bid_report.m_last_quantity = 100;
    bid_report.m_status = OrderStatus::FILLED;
    model.update(bid_report);
    auto ask_fields1 =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 60, Money::ONE);
    REQUIRE(!model.submit(2, ask_fields1));
    auto ask_fields2 =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 50, Money::ONE);
    REQUIRE(model.submit(3, ask_fields2));
  }

  TEST_CASE("update_bid_order_increases_position") {
    auto model = ShortingModel();
    auto ticker = parse_ticker("TST.TSX");
    auto bid_fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    model.submit(1, bid_fields);
    auto bid_report = ExecutionReport(1, boost::posix_time::not_a_date_time);
    bid_report.m_last_quantity = 100;
    bid_report.m_status = OrderStatus::FILLED;
    model.update(bid_report);
    auto ask_fields =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 100, Money::ONE);
    REQUIRE(!model.submit(2, ask_fields));
  }

  TEST_CASE("update_ask_order_decreases_position_and_pending") {
    auto model = ShortingModel();
    auto ticker = parse_ticker("TST.TSX");
    auto bid_fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    model.submit(1, bid_fields);
    auto bid_report = ExecutionReport(1, boost::posix_time::not_a_date_time);
    bid_report.m_last_quantity = 100;
    bid_report.m_status = OrderStatus::FILLED;
    model.update(bid_report);
    auto ask_fields =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 80, Money::ONE);
    model.submit(2, ask_fields);
    auto ask_report = ExecutionReport(2, boost::posix_time::not_a_date_time);
    ask_report.m_last_quantity = 80;
    ask_report.m_status = OrderStatus::FILLED;
    model.update(ask_report);
    auto ask_fields2 =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 30, Money::ONE);
    REQUIRE(model.submit(3, ask_fields2));
  }

  TEST_CASE("update_terminal_status_clears_pending") {
    auto model = ShortingModel();
    auto ticker = parse_ticker("TST.TSX");
    auto ask_fields =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 50, Money::ONE);
    model.submit(1, ask_fields);
    auto ask_report = ExecutionReport(1, boost::posix_time::not_a_date_time);
    ask_report.m_last_quantity = 25;
    ask_report.m_status = OrderStatus::FILLED;
    model.update(ask_report);
    auto ask_report_terminal =
      ExecutionReport(1, boost::posix_time::not_a_date_time);
    ask_report_terminal.m_last_quantity = 25;
    ask_report_terminal.m_status = OrderStatus::CANCELED;
    model.update(ask_report_terminal);
    auto ask_fields2 =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 10, Money::ONE);
    REQUIRE(model.submit(2, ask_fields2));
  }

  TEST_CASE("submit_and_update_multiple_orders_mixed_sides") {
    auto model = ShortingModel();
    auto ticker = parse_ticker("TST.TSX");
    auto bid_fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    model.submit(1, bid_fields);
    auto bid_report = ExecutionReport(1, boost::posix_time::not_a_date_time);
    bid_report.m_last_quantity = 100;
    bid_report.m_status = OrderStatus::FILLED;
    model.update(bid_report);
    auto ask_fields =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 50, Money::ONE);
    model.submit(2, ask_fields);
    auto ask_report = ExecutionReport(2, boost::posix_time::not_a_date_time);
    ask_report.m_last_quantity = 50;
    ask_report.m_status = OrderStatus::FILLED;
    model.update(ask_report);
    auto bid_fields2 =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 25, Money::ONE);
    model.submit(3, bid_fields2);
    auto bid_report2 = ExecutionReport(3, boost::posix_time::not_a_date_time);
    bid_report2.m_last_quantity = 25;
    bid_report2.m_status = OrderStatus::FILLED;
    model.update(bid_report2);
    auto ask_fields2 =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 80, Money::ONE);
    REQUIRE(model.submit(4, ask_fields2));
  }

  TEST_CASE("submit_ask_order_then_update_bid_order") {
    auto model = ShortingModel();
    auto ticker = parse_ticker("TST.TSX");
    auto ask_fields =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 100, Money::ONE);
    REQUIRE(model.submit(1, ask_fields));
    auto bid_fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    model.submit(2, bid_fields);
    auto bid_report = ExecutionReport(2, boost::posix_time::not_a_date_time);
    bid_report.m_last_quantity = 200;
    bid_report.m_status = OrderStatus::FILLED;
    model.update(bid_report);
    auto ask_fields2 = make_limit_order_fields(
      ticker, CAD, Side::ASK, "TSX", 50, 2 * Money::ONE);
    REQUIRE(!model.submit(3, ask_fields2));
  }

  TEST_CASE("update_with_unknown_order_id_no_effect") {
    auto model = ShortingModel();
    auto ticker = parse_ticker("TST.TSX");
    auto ask_fields =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 100, Money::ONE);
    model.submit(1, ask_fields);
    auto unknown_report =
      ExecutionReport(999, boost::posix_time::not_a_date_time);
    unknown_report.m_last_quantity = 100;
    unknown_report.m_status = OrderStatus::FILLED;
    model.update(unknown_report);
    auto ask_fields2 =
      make_limit_order_fields(ticker, CAD, Side::ASK, "TSX", 1, Money::ONE);
    REQUIRE(model.submit(2, ask_fields2));
  }
}
