#include <atomic>
#include <tuple>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchMarketDataFeedClient.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataFeedClient.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;
using namespace std::string_view_literals;

namespace {
  struct TestPitchClient {
    Queue<std::string> m_messages;
    std::string m_payload;

    CxaPitchMessage read() {
      m_payload = m_messages.pop();
      return CxaPitchMessage::parse(m_payload);
    }

    void close() {
      m_messages.close();
    }
  };

  using FeedClient = Nexus::Tests::TestMarketDataFeedClient;

  const auto TIMESTAMP = time_from_string("2021-02-10 14:45:48.641622");
  constexpr auto QUANTITY_OFFSET =
    CxaPitchMessage::HEADER_LENGTH + 2 * sizeof(std::uint64_t);
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
  static const auto UNIT_CLEAR = std::string("\x06\x97\x20\x20\x20\x20"sv);
  static const auto ADD_ORDER = std::string(
    "\x2a\x37"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "B"
    "\xbc\x02\x00\x00"
    "ZVZT  "
    "\x00\xe1\xf5\x05\x00\x00\x00\x00"
    "1234"
    "\x00"sv);
  static const auto UNDISCLOSED_ADD_ORDER = std::string(
    "\x2a\x37"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "S"
    "\x00\x00\x00\x00"
    "ZVZT  "
    "\x00\xe1\xf5\x05\x00\x00\x00\x00"
    "    "
    "\x00"sv);
  static const auto ORDER_EXECUTED = std::string(
    "\x2b\x38"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "\xbc\x02\x00\x00"
    "\x34\x2b\x46\xe0\xbb\x00\x00\x00"
    "\x06\x40\x5b\x77\x8f\x56\x1d\x0b"
    "5678"
    "\x00"sv);
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
    "\x00"sv);
  static const auto REDUCE_SIZE = std::string(
    "\x16\x39"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "\xbc\x02\x00\x00"sv);
  static const auto MODIFY_ORDER = std::string(
    "\x1f\x3a"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "\x2c\x01\x00\x00"
    "\x15\xcd\x5b\x07\x00\x00\x00\x00"
    "\x00"sv);
  static const auto DELETE_ORDER = std::string(
    "\x12\x3c"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"sv);
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
    "\x00"sv);
  struct Fixture {
    using Client = CxaPitchMarketDataFeedClient<FeedClient*, TestPitchClient*>;

    std::shared_ptr<FeedClient::Queue> m_operations;
    FeedClient m_feed_client;
    TestPitchClient m_pitch_client;
    Client m_client;

    Fixture()
      : m_operations(std::make_shared<FeedClient::Queue>()),
        m_feed_client(m_operations),
        m_client(CONFIGURATION, &m_feed_client, &m_pitch_client) {}

    ~Fixture() {
      m_client.close();
    }

    void publish(const std::string& message) {
      m_pitch_client.m_messages.push(message);
    }

    template<typename O>
    std::shared_ptr<O> pop_operation() {
      auto operation = m_operations->pop();
      auto actual = std::get_if<O>(&*operation);
      REQUIRE(actual);
      return std::shared_ptr<O>(operation, actual);
    }

    template<typename O>
    void require_operation(const auto&... args) {
      auto operation = pop_operation<O>();
      auto fields = [&] {
        if constexpr(std::same_as<O, FeedClient::AddOrderOperation>) {
          return std::tie(operation->m_ticker, operation->m_venue,
            operation->m_mpid, operation->m_is_primary_mpid, operation->m_id,
            operation->m_side, operation->m_price, operation->m_size,
            operation->m_timestamp);
        } else if constexpr(
            std::same_as<O, FeedClient::OffsetOrderSizeOperation>) {
          return std::tie(
            operation->m_id, operation->m_delta, operation->m_timestamp);
        } else {
          return std::tie(operation->m_id, operation->m_timestamp);
        }
      }();
      REQUIRE(fields == std::tie(args...));
      operation->m_result.set();
    }

    TickerTimeAndSale read_time_and_sale() {
      auto operation = pop_operation<FeedClient::PublishTimeAndSaleOperation>();
      auto sale = operation->m_time_and_sale;
      operation->m_result.set();
      return sale;
    }

    VenueOrderImbalance read_imbalance() {
      auto operation =
        pop_operation<FeedClient::PublishOrderImbalanceOperation>();
      auto imbalance = operation->m_imbalance;
      operation->m_result.set();
      return imbalance;
    }

    void add_order() {
      publish(ADD_ORDER);
      require_operation<FeedClient::AddOrderOperation>(
        TICKER, VENUE, "CXA", false, ORDER_ID, Side::BID, PRICE, 700,
        TIMESTAMP);
    }
  };
}

TEST_SUITE("CxaPitchMarketDataFeedClient") {
  TEST_CASE("close_during_publication") {
    auto fixture = Fixture();
    fixture.publish(ADD_ORDER);
    auto operation = fixture.pop_operation<FeedClient::AddOrderOperation>();
    fixture.publish(DELETE_ORDER);
    auto is_closed = std::atomic_bool(false);
    auto closer = RoutineHandler(spawn([&] {
      fixture.m_client.close();
      is_closed = true;
    }));
    flush_pending_routines();
    auto was_closed = bool(is_closed);
    fixture.m_feed_client.close();
    closer.wait();
    REQUIRE(was_closed);
    REQUIRE(!fixture.m_client.get_exception());
    REQUIRE(!fixture.m_operations->try_pop());
  }

  TEST_CASE("publication_failure") {
    auto fixture = Fixture();
    auto failure = std::make_exception_ptr(IOException("Publication failed."));
    SUBCASE("io_error") {}
    SUBCASE("end_of_file") {
      failure = std::make_exception_ptr(EndOfFileException());
    }
    fixture.publish(ADD_ORDER);
    auto operation = fixture.pop_operation<FeedClient::AddOrderOperation>();
    fixture.publish(DELETE_ORDER);
    operation->m_result.set(failure);
    flush_pending_routines();
    auto error = fixture.m_client.get_exception();
    REQUIRE(error);
    REQUIRE_THROWS_AS(std::rethrow_exception(error), IOException);
    REQUIRE(!fixture.m_operations->try_pop());
  }

  TEST_CASE("malformed_message") {
    auto fixture = Fixture();
    fixture.publish(std::string("\x02\x37", 2));
    fixture.add_order();
    flush_pending_routines();
    REQUIRE(!fixture.m_client.get_exception());
  }

  TEST_CASE("read_failure") {
    auto fixture = Fixture();
    REQUIRE(!fixture.m_client.get_exception());
    SUBCASE("failure") {
      fixture.m_pitch_client.m_messages.close(
        std::make_exception_ptr(IOException("Feed failed.")));
      flush_pending_routines();
      auto exception = fixture.m_client.get_exception();
      REQUIRE(exception);
      REQUIRE_THROWS_AS(std::rethrow_exception(exception), IOException);
    }
    SUBCASE("shutdown") {
      fixture.m_client.close();
      flush_pending_routines();
      REQUIRE(!fixture.m_client.get_exception());
    }
  }

  TEST_CASE("add_order") {
    auto fixture = Fixture();
    fixture.publish(ADD_ORDER);
    fixture.require_operation<FeedClient::AddOrderOperation>(
      TICKER, VENUE, "CXA", false, ORDER_ID, Side::BID, PRICE, 700, TIMESTAMP);
  }

  TEST_CASE("order_visibility") {
    auto fixture = Fixture();
    auto side = Side(Side::ASK);
    auto buyer = std::string("5678");
    auto seller = std::string("1234");
    SUBCASE("undisclosed_order") {
      fixture.publish(UNDISCLOSED_ADD_ORDER);
    }
    SUBCASE("disclosed_order") {
      fixture.add_order();
      auto message = MODIFY_ORDER;
      message.replace(QUANTITY_OFFSET, sizeof(std::uint32_t),
        sizeof(std::uint32_t), char(0));
      fixture.publish(message);
      fixture.require_operation<FeedClient::RemoveOrderOperation>(
        ORDER_ID, TIMESTAMP);
      side = Side::BID;
      std::swap(buyer, seller);
    }
    fixture.publish(TRADE);
    auto sale = fixture.read_time_and_sale();
    REQUIRE(sale->m_buyer_mpid == buyer);
    REQUIRE(sale->m_seller_mpid == seller);
    flush_pending_routines();
    REQUIRE(!fixture.m_operations->try_pop());
    fixture.publish(MODIFY_ORDER);
    fixture.require_operation<FeedClient::AddOrderOperation>(
      TICKER, VENUE, "CXA", false,
      ORDER_ID, side, EXECUTION_PRICE, 300, TIMESTAMP);
  }

  TEST_CASE("order_execution") {
    auto fixture = Fixture();
    fixture.add_order();
    auto message = ORDER_EXECUTED;
    auto price = PRICE;
    auto code = std::string("@");
    auto condition = TimeAndSale::Condition::Type::REGULAR;
    SUBCASE("resting_price") {}
    SUBCASE("execution_price") {
      message = ORDER_EXECUTED_AT_PRICE;
      price = EXECUTION_PRICE;
      code = "C";
      condition = TimeAndSale::Condition::Type::CLOSE;
    }
    fixture.publish(message);
    fixture.require_operation<FeedClient::OffsetOrderSizeOperation>(
      ORDER_ID, -700, TIMESTAMP);
    auto sale = fixture.read_time_and_sale();
    REQUIRE(sale.get_index() == TICKER);
    REQUIRE(sale->m_timestamp == TIMESTAMP);
    REQUIRE(sale->m_price == price);
    REQUIRE(sale->m_size == 700);
    REQUIRE(sale->m_condition.m_code == code);
    REQUIRE(sale->m_condition.m_type == condition);
    REQUIRE(sale->m_market_center == "CXA");
    REQUIRE(sale->m_buyer_mpid == "1234");
    REQUIRE(sale->m_seller_mpid == "5678");
    fixture.publish(UNIT_CLEAR);
    fixture.add_order();
  }

  TEST_CASE("size_reduction") {
    auto fixture = Fixture();
    fixture.publish(REDUCE_SIZE);
    fixture.require_operation<FeedClient::OffsetOrderSizeOperation>(
      ORDER_ID, -700, TIMESTAMP);
  }

  TEST_CASE("order_modification") {
    auto fixture = Fixture();
    fixture.add_order();
    fixture.publish(MODIFY_ORDER);
    fixture.require_operation<FeedClient::AddOrderOperation>(
      TICKER, VENUE, "CXA", false,
      ORDER_ID, Side::BID, EXECUTION_PRICE, 300, TIMESTAMP);
    flush_pending_routines();
    REQUIRE(!fixture.m_operations->try_pop());
    fixture.publish(ORDER_EXECUTED);
    fixture.require_operation<FeedClient::OffsetOrderSizeOperation>(
      ORDER_ID, -700, TIMESTAMP);
    REQUIRE(fixture.read_time_and_sale()->m_price == EXECUTION_PRICE);
  }

  TEST_CASE("order_removal") {
    auto fixture = Fixture();
    fixture.add_order();
    auto message = DELETE_ORDER;
    SUBCASE("delete_order") {}
    SUBCASE("unit_clear") {
      message = UNIT_CLEAR;
    }
    fixture.publish(message);
    fixture.require_operation<FeedClient::RemoveOrderOperation>(
      ORDER_ID, TIMESTAMP);
    fixture.publish(ORDER_EXECUTED);
    flush_pending_routines();
    fixture.require_operation<FeedClient::OffsetOrderSizeOperation>(
      ORDER_ID, -700, TIMESTAMP);
    flush_pending_routines();
    REQUIRE(!fixture.m_operations->try_pop());
  }

  TEST_CASE("trade") {
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
      "\x02"sv);
    auto fixture = Fixture();
    fixture.publish(TRADE);
    auto sale = fixture.read_time_and_sale();
    REQUIRE(sale.get_index() == TICKER);
    REQUIRE(sale->m_price == EXECUTION_PRICE);
    REQUIRE(sale->m_size == 700);
    REQUIRE(sale->m_condition.m_code == "@");
    REQUIRE(sale->m_condition.m_type == TimeAndSale::Condition::Type::REGULAR);
    REQUIRE(sale->m_buyer_mpid.empty());
    REQUIRE(sale->m_seller_mpid.empty());
    fixture.publish(OFF_EXCHANGE_TRADE);
    auto report = fixture.read_time_and_sale();
    REQUIRE(report->m_condition.m_code == "P");
    REQUIRE(report->m_buyer_mpid.empty());
    REQUIRE(report->m_seller_mpid.empty());
  }

  TEST_CASE("auction_trade") {
    constexpr auto TRADE_TYPE_OFFSET = std::size_t(60);
    auto fixture = Fixture();
    for(auto code : {'O', 'C', 'H'}) {
      auto trade = TRADE;
      trade[TRADE_TYPE_OFFSET] = code;
      fixture.publish(trade);
      auto sale = fixture.read_time_and_sale();
      REQUIRE(sale->m_condition.m_code == std::string(1, code));
      if(code == 'C') {
        REQUIRE(
          sale->m_condition.m_type == TimeAndSale::Condition::Type::CLOSE);
      } else {
        REQUIRE(sale->m_condition.m_type == TimeAndSale::Condition::Type::OPEN);
      }
    }
  }

  TEST_CASE("large_quantity_offset") {
    auto fixture = Fixture();
    for(auto message : {ORDER_EXECUTED, ORDER_EXECUTED_AT_PRICE, REDUCE_SIZE}) {
      message.replace(
        QUANTITY_OFFSET, sizeof(std::uint32_t), "\x00\x00\x00\x80"sv);
      fixture.publish(message);
      fixture.require_operation<FeedClient::OffsetOrderSizeOperation>(
        ORDER_ID, std::int64_t(-2147483648LL), TIMESTAMP);
      message.replace(
        QUANTITY_OFFSET, sizeof(std::uint32_t), "\xff\xff\xff\xff"sv);
      fixture.publish(message);
      fixture.require_operation<FeedClient::OffsetOrderSizeOperation>(
        ORDER_ID, std::int64_t(-4294967295LL), TIMESTAMP);
    }
  }

  TEST_CASE("auction_update") {
    static const auto AUCTION_UPDATE = std::string(
      "\x22\x59"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "ZVZT  "
      "O"
      "\xbc\x02\x00\x00"
      "\x2c\x01\x00\x00"
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "\x00"sv);
    constexpr auto BUY_SHARES_OFFSET = std::size_t(17);
    constexpr auto SELL_SHARES_OFFSET =
      BUY_SHARES_OFFSET + sizeof(std::uint32_t);
    auto fixture = Fixture();
    fixture.publish(AUCTION_UPDATE);
    auto imbalance = fixture.read_imbalance();
    REQUIRE(imbalance.get_index() == VENUE);
    REQUIRE(imbalance->m_ticker == TICKER);
    REQUIRE(imbalance->m_side == Side::BID);
    REQUIRE(imbalance->m_size == 400);
    REQUIRE(imbalance->m_reference_price == EXECUTION_PRICE);
    auto message = AUCTION_UPDATE;
    message.replace(BUY_SHARES_OFFSET, sizeof(std::uint32_t), message,
      SELL_SHARES_OFFSET, sizeof(std::uint32_t));
    fixture.publish(message);
    imbalance = fixture.read_imbalance();
    REQUIRE(imbalance->m_side == Side::NONE);
    REQUIRE(imbalance->m_size == 0);
    REQUIRE(imbalance->m_reference_price == EXECUTION_PRICE);
  }

  TEST_CASE("unhandled_message") {
    static const auto TRADING_STATUS = std::string(
      "\x16\x3b"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "ZVZT  "
      "T"
      "AUS "
      "\x00"sv);
    auto fixture = Fixture();
    fixture.publish(TRADING_STATUS);
    fixture.publish(DELETE_ORDER);
    fixture.require_operation<FeedClient::RemoveOrderOperation>(
      ORDER_ID, TIMESTAMP);
  }
}
