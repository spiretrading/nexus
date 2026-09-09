#include <memory>
#include <string>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchMarketDataFeedClient.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct StubClient {
    std::shared_ptr<Queue<std::string>> m_messages;
    std::string m_payload;

    StubClient()
      : m_messages(std::make_shared<Queue<std::string>>()) {}

    CxaPitchMessage read() {
      m_payload = m_messages->pop();
      return CxaPitchMessage::parse(m_payload);
    }

    void close() {
      m_messages->close();
    }
  };

  struct StubMarketDataFeedClient {
    std::shared_ptr<Queue<std::string>> m_operations;
    std::shared_ptr<Queue<TickerTimeAndSale>> m_time_and_sales;
    std::shared_ptr<Queue<VenueOrderImbalance>> m_imbalances;

    StubMarketDataFeedClient()
      : m_operations(std::make_shared<Queue<std::string>>()),
        m_time_and_sales(std::make_shared<Queue<TickerTimeAndSale>>()),
        m_imbalances(std::make_shared<Queue<VenueOrderImbalance>>()) {}

    void add(const TickerInfo&) {}

    void publish(const VenueOrderImbalance& imbalance) {
      m_imbalances->push(imbalance);
    }

    void publish(const TickerBboQuote&) {}

    void publish(const TickerBookQuote&) {}

    void publish(const TickerTimeAndSale& time_and_sale) {
      m_time_and_sales->push(time_and_sale);
    }

    void publish(const IndexedTickerStatus&) {}

    void add_order(const Ticker& ticker, Venue, const std::string&, bool,
        const std::string& id, Side side, Money price, Quantity size, ptime) {
      m_operations->push("(add " + id + ' ' + to_string(ticker) + ' ' +
        to_string(side) + ' ' + to_string(price) + ' ' + to_string(size) + ')');
    }

    void modify_order_size(const std::string& id, Quantity size, ptime) {
      m_operations->push("(size " + id + ' ' + to_string(size) + ')');
    }

    void offset_order_size(const std::string& id, Quantity delta, ptime) {
      m_operations->push("(offset " + id + ' ' + to_string(delta) + ')');
    }

    void modify_order_price(const std::string& id, Money price, ptime) {
      m_operations->push("(price " + id + ' ' + to_string(price) + ')');
    }

    void remove_order(const std::string& id, ptime) {
      m_operations->push("(remove " + id + ')');
    }

    void close() {
      m_operations->close();
      m_time_and_sales->close();
      m_imbalances->close();
    }
  };

  using Client =
    CxaPitchMarketDataFeedClient<StubMarketDataFeedClient*, StubClient*>;
}

TEST_SUITE("CxaPitchMarketDataFeedClient") {
  static const auto VENUE = Venue("CXA");
  static const auto TICKER = Ticker("ZVZT", VENUE);
  static const auto ORDER_ID = std::string("800891482924597253");
  static const auto PRICE = Money(Quantity(10));
  static const auto EXECUTION_PRICE = parse_money("12.3456789");
  static const auto CONFIGURATION = [] {
    auto configuration = CxaPitchConfiguration();
    configuration.m_unit = 1;
    configuration.m_primary_venue = VENUE;
    configuration.m_disseminating_venue = VENUE;
    configuration.m_mpid = "CXA";
    return configuration;
  }();
  static const auto UNIT_CLEAR = std::string("\x06\x97\x20\x20\x20\x20", 6);
  static const auto TRADING_STATUS = std::string(
    "\x16\x3b"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "ZVZT  "
    "T"
    "AUS "
    "\x00", 22);
  static const auto ADD_ORDER = std::string(
    "\x2a\x37"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "B"
    "\xbc\x02\x00\x00"
    "ZVZT  "
    "\x00\xe1\xf5\x05\x00\x00\x00\x00"
    "1234"
    "\x00", 42);
  static const auto UNDISCLOSED_ADD_ORDER = std::string(
    "\x2a\x37"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "S"
    "\x00\x00\x00\x00"
    "ZVZT  "
    "\x00\xe1\xf5\x05\x00\x00\x00\x00"
    "    "
    "\x00", 42);
  static const auto ORDER_EXECUTED = std::string(
    "\x2b\x38"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "\xbc\x02\x00\x00"
    "\x34\x2b\x46\xe0\xbb\x00\x00\x00"
    "\x06\x40\x5b\x77\x8f\x56\x1d\x0b"
    "5678"
    "\x00", 43);
  static const auto ORDER_EXECUTED_AT_PRICE = std::string(
    "\x34\x58"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "\xbc\x02\x00\x00"
    "\x34\x2b\x46\xe0\xbb\x00\x00\x00"
    "\x06\x40\x5b\x77\x8f\x56\x1d\x0b"
    "5678"
    "C"
    "\x15\xcd\x5b\x07\x00\x00\x00\x00"
    "\x00", 52);
  static const auto REDUCE_SIZE = std::string(
    "\x16\x39"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "\xbc\x02\x00\x00", 22);
  static const auto MODIFY_ORDER = std::string(
    "\x1f\x3a"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "\x2c\x01\x00\x00"
    "\x15\xcd\x5b\x07\x00\x00\x00\x00"
    "\x00", 31);
  static const auto DELETE_ORDER = std::string(
    "\x12\x3c"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b", 18);
  static const auto TRADE = std::string(
    "\x48\x3d"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "ZVZT  "
    "\xbc\x02\x00\x00"
    "\x15\xcd\x5b\x07\x00\x00\x00\x00"
    "\x34\x2b\x46\xe0\xbb\x00\x00\x00"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "\x06\x40\x5b\x77\x8f\x56\x1d\x0b"
    "1234"
    "5678"
    "N"
    "C"
    " "
    "\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00", 72);
  static const auto OFF_EXCHANGE_TRADE = std::string(
    "\x48\x3d"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "ZVZT  "
    "\xbc\x02\x00\x00"
    "\x15\xcd\x5b\x07\x00\x00\x00\x00"
    "\x34\x2b\x46\xe0\xbb\x00\x00\x00"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "\x06\x40\x5b\x77\x8f\x56\x1d\x0b"
    "1234"
    "    "
    " "
    " "
    "P"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x02", 72);
  static const auto AUCTION_UPDATE = std::string(
    "\x22\x59"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "ZVZT  "
    "O"
    "\xbc\x02\x00\x00"
    "\x2c\x01\x00\x00"
    "\x15\xcd\x5b\x07\x00\x00\x00\x00"
    "\x00", 34);

  TEST_CASE("add_an_order") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(ADD_ORDER);
    REQUIRE(feed.m_operations->pop() == "(add " + ORDER_ID + ' ' +
      to_string(TICKER) + ' ' + to_string(Side::BID) + ' ' + to_string(PRICE) +
      " 700)");
  }

  TEST_CASE("add_an_undisclosed_order") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(UNDISCLOSED_ADD_ORDER);
    pitch.m_messages->push(TRADE);
    auto sale = feed.m_time_and_sales->pop();
    REQUIRE(sale->m_buyer_mpid == "5678");
    REQUIRE(sale->m_seller_mpid == "1234");
    REQUIRE(!feed.m_operations->try_pop());
  }

  TEST_CASE("execute_an_order") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(ADD_ORDER);
    feed.m_operations->pop();
    pitch.m_messages->push(ORDER_EXECUTED);
    REQUIRE(feed.m_operations->pop() == "(offset " + ORDER_ID + " -700)");
    auto sale = feed.m_time_and_sales->pop();
    REQUIRE(sale.get_index() == TICKER);
    REQUIRE(sale->m_price == PRICE);
    REQUIRE(sale->m_size == 700);
    REQUIRE(sale->m_condition.m_code == "@");
    REQUIRE(sale->m_condition.m_type == TimeAndSale::Condition::Type::REGULAR);
    REQUIRE(sale->m_market_center == "CXA");
    REQUIRE(sale->m_buyer_mpid == "1234");
    REQUIRE(sale->m_seller_mpid == "5678");
  }

  TEST_CASE("execute_an_order_at_price") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(ADD_ORDER);
    feed.m_operations->pop();
    pitch.m_messages->push(ORDER_EXECUTED_AT_PRICE);
    REQUIRE(feed.m_operations->pop() == "(offset " + ORDER_ID + " -700)");
    auto sale = feed.m_time_and_sales->pop();
    REQUIRE(sale->m_price == EXECUTION_PRICE);
    REQUIRE(sale->m_condition.m_code == "C");
    REQUIRE(sale->m_condition.m_type == TimeAndSale::Condition::Type::CLOSE);
  }

  TEST_CASE("reduce_an_order") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(REDUCE_SIZE);
    REQUIRE(feed.m_operations->pop() == "(offset " + ORDER_ID + " -700)");
  }

  TEST_CASE("modify_an_order") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(ADD_ORDER);
    feed.m_operations->pop();
    pitch.m_messages->push(MODIFY_ORDER);
    REQUIRE(feed.m_operations->pop() == "(add " + ORDER_ID + ' ' +
      to_string(TICKER) + ' ' + to_string(Side::BID) + ' ' +
      to_string(EXECUTION_PRICE) + " 300)");
    flush_pending_routines();
    REQUIRE(!feed.m_operations->try_pop());
    pitch.m_messages->push(ORDER_EXECUTED);
    feed.m_operations->pop();
    REQUIRE(feed.m_time_and_sales->pop()->m_price == EXECUTION_PRICE);
  }

  TEST_CASE("delete_an_order") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(ADD_ORDER);
    feed.m_operations->pop();
    pitch.m_messages->push(DELETE_ORDER);
    REQUIRE(feed.m_operations->pop() == "(remove " + ORDER_ID + ')');
    pitch.m_messages->push(ORDER_EXECUTED);
    flush_pending_routines();
    REQUIRE(feed.m_operations->pop() == "(offset " + ORDER_ID + " -700)");
    REQUIRE(!feed.m_time_and_sales->try_pop());
  }

  TEST_CASE("forget_a_fully_executed_order") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(ADD_ORDER);
    feed.m_operations->pop();
    pitch.m_messages->push(ORDER_EXECUTED);
    REQUIRE(feed.m_operations->pop() == "(offset " + ORDER_ID + " -700)");
    feed.m_time_and_sales->pop();
    pitch.m_messages->push(UNIT_CLEAR);
    pitch.m_messages->push(ADD_ORDER);
    REQUIRE(feed.m_operations->pop() == "(add " + ORDER_ID + ' ' +
      to_string(TICKER) + ' ' + to_string(Side::BID) + ' ' + to_string(PRICE) +
      " 700)");
  }

  TEST_CASE("disclose_an_undisclosed_order") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(UNDISCLOSED_ADD_ORDER);
    pitch.m_messages->push(MODIFY_ORDER);
    REQUIRE(feed.m_operations->pop() == "(add " + ORDER_ID + ' ' +
      to_string(TICKER) + ' ' + to_string(Side::ASK) + ' ' +
      to_string(EXECUTION_PRICE) + " 300)");
  }

  TEST_CASE("report_a_trade") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(TRADE);
    auto sale = feed.m_time_and_sales->pop();
    REQUIRE(sale.get_index() == TICKER);
    REQUIRE(sale->m_price == EXECUTION_PRICE);
    REQUIRE(sale->m_size == 700);
    REQUIRE(sale->m_condition.m_code == "@");
    REQUIRE(sale->m_condition.m_type == TimeAndSale::Condition::Type::REGULAR);
    REQUIRE(sale->m_buyer_mpid.empty());
    REQUIRE(sale->m_seller_mpid.empty());
    pitch.m_messages->push(OFF_EXCHANGE_TRADE);
    auto report = feed.m_time_and_sales->pop();
    REQUIRE(report->m_condition.m_code == "P");
    REQUIRE(report->m_buyer_mpid.empty());
    REQUIRE(report->m_seller_mpid.empty());
  }

  TEST_CASE("report_auction_trades") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    for(auto code : {'O', 'C', 'H'}) {
      auto trade = TRADE;
      trade[60] = code;
      pitch.m_messages->push(trade);
      auto sale = feed.m_time_and_sales->pop();
      REQUIRE(sale->m_condition.m_code == std::string(1, code));
      if(code == 'C') {
        REQUIRE(
          sale->m_condition.m_type == TimeAndSale::Condition::Type::CLOSE);
      } else {
        REQUIRE(sale->m_condition.m_type == TimeAndSale::Condition::Type::OPEN);
      }
    }
  }

  TEST_CASE("offset_large_quantities") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    for(auto message : {ORDER_EXECUTED, ORDER_EXECUTED_AT_PRICE, REDUCE_SIZE}) {
      message.replace(18, 4, "\x00\x00\x00\x80", 4);
      pitch.m_messages->push(message);
      REQUIRE(feed.m_operations->pop() ==
        "(offset " + ORDER_ID + " -2147483648)");
      message.replace(18, 4, "\xff\xff\xff\xff", 4);
      pitch.m_messages->push(message);
      REQUIRE(feed.m_operations->pop() ==
        "(offset " + ORDER_ID + " -4294967295)");
    }
  }

  TEST_CASE("modify_to_undisclosed_and_back") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(ADD_ORDER);
    feed.m_operations->pop();
    auto message = MODIFY_ORDER;
    message.replace(18, 4, 4, char(0));
    pitch.m_messages->push(message);
    REQUIRE(feed.m_operations->pop() == "(remove " + ORDER_ID + ')');
    pitch.m_messages->push(TRADE);
    auto sale = feed.m_time_and_sales->pop();
    REQUIRE(sale->m_buyer_mpid == "1234");
    REQUIRE(sale->m_seller_mpid == "5678");
    pitch.m_messages->push(MODIFY_ORDER);
    REQUIRE(feed.m_operations->pop() == "(add " + ORDER_ID + ' ' +
      to_string(TICKER) + ' ' + to_string(Side::BID) + ' ' +
      to_string(EXECUTION_PRICE) + " 300)");
  }

  TEST_CASE("report_an_auction_update") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(AUCTION_UPDATE);
    auto imbalance = feed.m_imbalances->pop();
    REQUIRE(imbalance.get_index() == VENUE);
    REQUIRE(imbalance->m_ticker == TICKER);
    REQUIRE(imbalance->m_side == Side::BID);
    REQUIRE(imbalance->m_size == 400);
    REQUIRE(imbalance->m_reference_price == EXECUTION_PRICE);
    auto message = AUCTION_UPDATE;
    message.replace(17, 4, message, 21, 4);
    pitch.m_messages->push(message);
    imbalance = feed.m_imbalances->pop();
    REQUIRE(imbalance->m_side == Side::NONE);
    REQUIRE(imbalance->m_size == 0);
    REQUIRE(imbalance->m_reference_price == EXECUTION_PRICE);
  }

  TEST_CASE("clear_a_unit") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(ADD_ORDER);
    feed.m_operations->pop();
    pitch.m_messages->push(UNIT_CLEAR);
    REQUIRE(feed.m_operations->pop() == "(remove " + ORDER_ID + ')');
    pitch.m_messages->push(ORDER_EXECUTED);
    flush_pending_routines();
    REQUIRE(feed.m_operations->pop() == "(offset " + ORDER_ID + " -700)");
    REQUIRE(!feed.m_time_and_sales->try_pop());
  }

  TEST_CASE("ignore_an_unhandled_message") {
    auto feed = StubMarketDataFeedClient();
    auto pitch = StubClient();
    auto client = Client(CONFIGURATION, &feed, &pitch);
    pitch.m_messages->push(TRADING_STATUS);
    pitch.m_messages->push(DELETE_ORDER);
    REQUIRE(feed.m_operations->pop() == "(remove " + ORDER_ID + ')');
  }
}
