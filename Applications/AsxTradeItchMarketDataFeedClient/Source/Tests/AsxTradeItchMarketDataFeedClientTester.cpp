#include <iostream>
#include <sstream>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/endian/conversion.hpp>
#include <doctest/doctest.h>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchMarketDataFeedClient.hpp"
#include "Nexus/Definitions/StandardVenues.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataFeedClient.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct Log {
    std::stringstream m_output;
    std::streambuf* m_buffer;

    Log()
      : m_buffer(std::cout.rdbuf(m_output.rdbuf())) {}

    ~Log() {
      std::cout.rdbuf(m_buffer);
    }
  };

  using FeedClient = Nexus::Tests::TestMarketDataFeedClient;
  const auto TIMESTAMP = time_from_string("2023-11-14 22:13:20.123456");
  constexpr auto NANOSECONDS = std::uint32_t(123456789);

  struct Encoder {
    SharedBuffer m_buffer;

    template<typename... T>
    void write(T... values) {
      (append(m_buffer, boost::endian::native_to_big(values)), ...);
    }

    void text(const std::string& value, std::size_t width) {
      append(m_buffer, value.data(), value.size());
      auto padding = std::string(width - value.size(), ' ');
      append(m_buffer, padding.data(), padding.size());
    }
  };

  SharedBuffer encode(const AsxTradeItchSeconds& message) {
    auto encoder = Encoder();
    encoder.write(message.TYPE, message.m_seconds);
    return encoder.m_buffer;
  }

  template<typename D> requires
    std::same_as<D, AsxTradeItchOrderBookDirectory> ||
      std::same_as<D, AsxTradeItchCombinationOrderBookDirectory>
  SharedBuffer encode(const D& message) {
    auto encoder = Encoder();
    encoder.write(message.TYPE, message.m_nanoseconds, message.m_order_book_id);
    encoder.text(message.m_symbol, AsxTradeItchDetails::SYMBOL_LENGTH);
    encoder.text(message.m_long_name, AsxTradeItchDetails::LONG_NAME_LENGTH);
    encoder.text(message.m_isin, AsxTradeItchDetails::ISIN_LENGTH);
    encoder.write(message.m_financial_product);
    encoder.text(message.m_currency, AsxTradeItchDetails::CURRENCY_LENGTH);
    encoder.write(message.m_price_decimals, message.m_nominal_value_decimals,
      message.m_odd_lot_size, message.m_round_lot_size,
      message.m_block_lot_size, message.m_nominal_value);
    if constexpr(std::same_as<D, AsxTradeItchCombinationOrderBookDirectory>) {
      for(auto& leg : message.m_legs) {
        encoder.text(leg.m_symbol, AsxTradeItchDetails::SYMBOL_LENGTH);
        encoder.write(leg.m_side, leg.m_ratio);
      }
    }
    return encoder.m_buffer;
  }

  Encoder encode_order_header(const auto& message) {
    auto encoder = Encoder();
    auto side = [&] {
      if(message.m_side == Side::BID) {
        return 'B';
      }
      return 'S';
    }();
    encoder.write(message.TYPE, message.m_nanoseconds, message.m_order_id,
      message.m_order_book_id, side);
    return encoder;
  }

  template<typename A> requires
    std::same_as<A, AsxTradeItchAddOrder> ||
      std::same_as<A, AsxTradeItchAddOrderWithParticipant> ||
      std::same_as<A, AsxTradeItchOrderReplace>
  SharedBuffer encode(const A& message) {
    auto encoder = encode_order_header(message);
    encoder.write(message.m_order_book_position, message.m_quantity,
      message.m_price, message.m_exchange_order_type);
    if constexpr(!std::same_as<A, AsxTradeItchOrderReplace>) {
      encoder.write(message.m_lot_type);
    }
    if constexpr(std::same_as<A, AsxTradeItchAddOrderWithParticipant>) {
      encoder.text(
        message.m_participant_id, AsxTradeItchDetails::PARTICIPANT_ID_LENGTH);
    }
    return encoder.m_buffer;
  }

  SharedBuffer encode(const AsxTradeItchOrderDelete& message) {
    return encode_order_header(message).m_buffer;
  }

  template<typename E> requires
    std::same_as<E, AsxTradeItchOrderExecuted> ||
      std::same_as<E, AsxTradeItchOrderExecutedAtPrice>
  SharedBuffer encode(const E& message) {
    auto encoder = encode_order_header(message);
    encoder.write(message.m_executed_quantity);
    for(auto word : message.m_match_id) {
      encoder.write(word);
    }
    encoder.text(message.m_owner, AsxTradeItchDetails::PARTICIPANT_ID_LENGTH);
    encoder.text(
      message.m_counterparty, AsxTradeItchDetails::PARTICIPANT_ID_LENGTH);
    if constexpr(std::same_as<E, AsxTradeItchOrderExecutedAtPrice>) {
      encoder.write(
        message.m_price, message.m_occurred_at_cross, message.m_printable);
    }
    return encoder.m_buffer;
  }

  SharedBuffer encode(const AsxTradeItchTrade& message) {
    auto encoder = Encoder();
    encoder.write(message.TYPE, message.m_nanoseconds);
    for(auto word : message.m_match_id) {
      encoder.write(word);
    }
    auto side = [&] {
      if(message.m_side == Side::BID) {
        return 'B';
      }
      if(message.m_side == Side::ASK) {
        return 'S';
      }
      return ' ';
    }();
    encoder.write(side, message.m_quantity, message.m_order_book_id,
      message.m_price);
    encoder.text(message.m_owner, AsxTradeItchDetails::PARTICIPANT_ID_LENGTH);
    encoder.text(
      message.m_counterparty, AsxTradeItchDetails::PARTICIPANT_ID_LENGTH);
    encoder.write(message.m_printable, message.m_occurred_at_cross);
    return encoder.m_buffer;
  }

  SharedBuffer encode(const AsxTradeItchOrderBookState& message) {
    auto encoder = Encoder();
    encoder.write(message.TYPE, message.m_nanoseconds, message.m_order_book_id);
    static constexpr auto STATE_LENGTH = std::size_t(20);
    encoder.text(message.m_state, STATE_LENGTH);
    return encoder.m_buffer;
  }

  SharedBuffer encode(const AsxTradeItchEquilibriumPriceUpdate& message) {
    auto encoder = Encoder();
    encoder.write(message.TYPE, message.m_nanoseconds, message.m_order_book_id,
      message.m_bid_quantity, message.m_ask_quantity,
      message.m_equilibrium_price, message.m_best_bid_price,
      message.m_best_ask_price, message.m_best_bid_quantity,
      message.m_best_ask_quantity);
    return encoder.m_buffer;
  }

  SharedBuffer encode(const AsxTradeItchSystemEvent& message) {
    auto encoder = Encoder();
    encoder.write(message.TYPE, message.m_nanoseconds, message.m_event_code);
    return encoder.m_buffer;
  }

  struct TestItchClient {
    Queue<SharedBuffer> m_messages;
    SharedBuffer m_payload;

    AsxTradeItchMessage read() {
      m_payload = m_messages.pop();
      return AsxTradeItchMessage::parse(
        std::string_view(m_payload.get_data(), m_payload.get_size()));
    }

    void close() {
      m_messages.close(std::make_exception_ptr(EndOfFileException()));
    }
  };

  struct Fixture {
    using Client =
      AsxTradeItchMarketDataFeedClient<FeedClient*, TestItchClient*>;
    std::shared_ptr<FeedClient::Queue> m_operations;
    FeedClient m_feed_client;
    TestItchClient m_itch_client;
    Client m_client;

    Fixture()
      : Fixture(false) {}

    explicit Fixture(bool is_logging_messages)
        : m_operations(std::make_shared<FeedClient::Queue>()),
          m_feed_client(m_operations),
          m_client([&] {
            auto config = AsxTradeItchConfiguration();
            config.m_is_logging_messages = is_logging_messages;
            config.m_primary_venue = Venues::ASX;
            config.m_disseminating_venue = Venues::ASX;
            return config;
          }(), &m_feed_client, &m_itch_client) {
      publish(AsxTradeItchSeconds(1700000000));
    }

    ~Fixture() {
      m_client.close();
    }

    void publish(const auto& message) {
      m_itch_client.m_messages.push(encode(message));
    }

    template<typename O>
    std::shared_ptr<O> take() {
      flush_pending_routines();
      auto operation = m_operations->try_pop();
      REQUIRE(operation.has_value());
      auto actual = std::get_if<O>(&**operation);
      REQUIRE(actual);
      auto result = std::shared_ptr<O>(*operation, actual);
      result->m_result.set();
      return result;
    }

    void require_empty() {
      flush_pending_routines();
      REQUIRE(!m_client.get_exception());
      REQUIRE(!m_operations->try_pop());
    }

    void directory(std::uint32_t book, const std::string& symbol) {
      auto message = AsxTradeItchOrderBookDirectory();
      message.m_nanoseconds = NANOSECONDS;
      message.m_order_book_id = book;
      message.m_symbol = symbol;
      message.m_long_name = symbol + " LTD";
      message.m_currency = "AUD";
      message.m_financial_product = 5;
      message.m_price_decimals = 2;
      message.m_round_lot_size = 1;
      publish(message);
      auto operation = take<FeedClient::AddOperation>();
      REQUIRE(operation->m_info ==
        TickerInfo(Ticker(symbol, Venues::ASX), symbol + " LTD", "", 1));
    }

    void add(std::uint64_t id, std::uint32_t book, const std::string& symbol,
        Side side, std::int32_t price, std::uint64_t quantity) {
      add(id, book, symbol, side, price, quantity, quantity);
    }

    void add(std::uint64_t id, std::uint32_t book, const std::string& symbol,
        Side side, std::int32_t price, std::uint64_t quantity,
        Quantity aggregate) {
      publish(AsxTradeItchAddOrder(
        NANOSECONDS, id, book, side, 1, quantity, price, 0, 2));
      if(quantity != 0) {
        require_book(symbol, side, Money(Quantity(price / 10000.0)), aggregate);
      }
    }

    void require_book(
        const std::string& symbol, Side side, Money price, Quantity size) {
      require_book(symbol, side, price, size, "AU000");
    }

    void require_book(const std::string& symbol, Side side, Money price,
        Quantity size, const std::string& mpid) {
      auto operation = take<FeedClient::PublishBookQuoteOperation>();
      REQUIRE(operation->m_quote == TickerBookQuote(BookQuote(mpid, false,
        Venues::ASX, Quote(price, size, side), TIMESTAMP),
        Ticker(symbol, Venues::ASX)));
    }

    void require_bbo(const std::string& symbol, Quote bid, Quote ask) {
      auto operation = take<FeedClient::PublishBboQuoteOperation>();
      REQUIRE(operation->m_quote == TickerBboQuote(
        BboQuote(bid, ask, TIMESTAMP), Ticker(symbol, Venues::ASX)));
    }
  };
}

TEST_SUITE("AsxTradeItchMarketDataFeedClient") {
  TEST_CASE("message_logging") {
    auto log = Log();
    auto is_logging = false;
    SUBCASE("disabled") {}
    SUBCASE("enabled") {
      is_logging = true;
    }
    auto fixture = Fixture(is_logging);
    fixture.m_itch_client.m_messages.push(SharedBuffer("?\x7f", 2));
    fixture.require_empty();
    if(is_logging) {
      REQUIRE(log.m_output.str() ==
        "(seconds 1700000000)\n(message ? 7f)\n");
    } else {
      REQUIRE(log.m_output.str().empty());
    }
    log.m_output.str("");
    fixture.m_itch_client.m_messages.push(SharedBuffer("A\x7f", 2));
    flush_pending_routines();
    REQUIRE(fixture.m_client.is_finished());
    REQUIRE(fixture.m_client.get_exception());
    REQUIRE(log.m_output.str() == "(message A 7f)\n");
  }

  TEST_CASE("excluded_product") {
    auto fixture = Fixture();
    auto directory = AsxTradeItchOrderBookDirectory();
    directory.m_nanoseconds = NANOSECONDS;
    directory.m_order_book_id = 1;
    directory.m_symbol = "BHP";
    directory.m_currency = "AUD";
    directory.m_price_decimals = 2;
    SUBCASE("option") {
      directory.m_financial_product = 1;
    }
    SUBCASE("future") {
      directory.m_financial_product = 3;
    }
    SUBCASE("combination") {
      directory.m_financial_product = 11;
    }
    SUBCASE("combination_directory") {
      auto combination = AsxTradeItchCombinationOrderBookDirectory();
      combination.m_nanoseconds = NANOSECONDS;
      combination.m_order_book_id = 1;
      combination.m_symbol = "BHP";
      combination.m_financial_product = 11;
      combination.m_currency = "AUD";
      combination.m_price_decimals = 2;
      for(auto& leg : combination.m_legs) {
        leg.m_side = '?';
      }
      fixture.publish(combination);
      fixture.require_empty();
      directory.m_financial_product = 11;
    }
    SUBCASE("unknown") {
      directory.m_financial_product = 255;
    }
    fixture.publish(directory);
    fixture.require_empty();
    fixture.publish(AsxTradeItchOrderBookState(NANOSECONDS, 1, "OPEN"));
    fixture.publish(AsxTradeItchAddOrder(
      NANOSECONDS, 1, 1, Side::BID, 1, 100, 1000, 0, 2));
    fixture.publish(AsxTradeItchAddOrderWithParticipant(
      NANOSECONDS, 2, 1, Side::ASK, 1, 100, 1100, 0, 2, "AU123"));
    fixture.publish(AsxTradeItchOrderExecuted(
      NANOSECONDS, 1, 1, Side::BID, 20, {}, "AU123", "AU456"));
    fixture.publish(AsxTradeItchOrderExecutedAtPrice(
      NANOSECONDS, 1, 1, Side::BID, 20, {}, "AU123", "AU456",
      1000, 'N', 'Y'));
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 1, 1, Side::BID, 1, 50, 1050, 0));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 1, 1, Side::BID));
    fixture.publish(AsxTradeItchTrade(
      NANOSECONDS, {}, Side::BID, 20, 1, 1000, "AU123", "AU456", 'Y', 'N'));
    auto imbalance = AsxTradeItchEquilibriumPriceUpdate();
    imbalance.m_nanoseconds = NANOSECONDS;
    imbalance.m_order_book_id = 1;
    fixture.publish(imbalance);
    fixture.require_empty();
    fixture.directory(1, "BHP");
    fixture.add(1, 1, "BHP", Side::BID, 1000, 100);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 100), make_ask(Money(), 0));
    fixture.publish(directory);
    fixture.require_book("BHP", Side::BID, 10 * Money::CENT, 0);
    fixture.require_bbo("BHP", make_bid(Money(), 0), make_ask(Money(), 0));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 1, 1, Side::BID));
    fixture.require_empty();
    fixture.directory(2, "CBA");
    fixture.add(1, 2, "CBA", Side::BID, 1000, 100);
    fixture.require_bbo(
      "CBA", make_bid(10 * Money::CENT, 100), make_ask(Money(), 0));
    fixture.require_empty();
  }

  TEST_CASE("order_mpid") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    auto mpid = std::string("AU000");
    SUBCASE("anonymous") {
      fixture.publish(AsxTradeItchAddOrder(
        NANOSECONDS, 1, 1, Side::BID, 1, 100, 1000, 0, 2));
    }
    SUBCASE("participant") {
      mpid = "AU550";
      fixture.publish(AsxTradeItchAddOrderWithParticipant(
        NANOSECONDS, 1, 1, Side::BID, 1, 100, 1000, 0, 2, mpid));
    }
    SUBCASE("empty_participant") {
      fixture.publish(AsxTradeItchAddOrderWithParticipant(
        NANOSECONDS, 1, 1, Side::BID, 1, 100, 1000, 0, 2, ""));
    }
    fixture.require_book("BHP", Side::BID, 10 * Money::CENT, 100, mpid);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 100), make_ask(Money(), 0));
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 1, 1, Side::BID, 1, 200, 1100, 0));
    fixture.require_book("BHP", Side::BID, 10 * Money::CENT, 0, mpid);
    fixture.require_book("BHP", Side::BID, 11 * Money::CENT, 200, mpid);
    fixture.require_bbo(
      "BHP", make_bid(11 * Money::CENT, 200), make_ask(Money(), 0));
    auto directory = AsxTradeItchOrderBookDirectory();
    directory.m_nanoseconds = NANOSECONDS;
    directory.m_order_book_id = 1;
    directory.m_symbol = "BHP";
    directory.m_financial_product = 5;
    directory.m_price_decimals = 3;
    fixture.publish(directory);
    fixture.take<FeedClient::AddOperation>();
    fixture.require_book("BHP", Side::BID, 11 * Money::CENT, 0, mpid);
    fixture.require_book("BHP", Side::BID, parse_money("0.011"), 200, mpid);
    fixture.require_bbo(
      "BHP", make_bid(parse_money("0.011"), 200), make_ask(Money(), 0));
    fixture.require_empty();
  }

  TEST_CASE("book_aggregation") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    fixture.add(1, 1, "BHP", Side::BID, 1000, 100);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 100), make_ask(Money(), 0));
    fixture.add(2, 1, "BHP", Side::BID, 1000, 200, 300);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 300), make_ask(Money(), 0));
    fixture.publish(AsxTradeItchAddOrderWithParticipant(
      NANOSECONDS, 3, 1, Side::BID, 1, 400, 1000, 0, 2, "AU550"));
    fixture.require_book("BHP", Side::BID, 10 * Money::CENT, 400, "AU550");
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 700), make_ask(Money(), 0));
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 1, 1, Side::BID, 1, 50, 1000, 0));
    fixture.require_book("BHP", Side::BID, 10 * Money::CENT, 250);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 650), make_ask(Money(), 0));
    fixture.publish(AsxTradeItchAddOrderWithParticipant(
      NANOSECONDS, 1, 1, Side::BID, 1, 50, 1000, 0, 2, "AU550"));
    fixture.require_book("BHP", Side::BID, 10 * Money::CENT, 200);
    fixture.require_book("BHP", Side::BID, 10 * Money::CENT, 450, "AU550");
    fixture.require_empty();
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 2, 1, Side::BID));
    fixture.require_book("BHP", Side::BID, 10 * Money::CENT, 0);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 450), make_ask(Money(), 0));
    fixture.publish(AsxTradeItchSystemEvent(NANOSECONDS, 'O'));
    fixture.require_book("BHP", Side::BID, 10 * Money::CENT, 0, "AU550");
    fixture.require_bbo("BHP", make_bid(Money(), 0), make_ask(Money(), 0));
    fixture.require_empty();
  }

  TEST_CASE("bbo_depth") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    fixture.add(1, 1, "BHP", Side::BID, 1000, 100);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 100), make_ask(Money(), 0));
    fixture.add(2, 1, "BHP", Side::BID, 1000, 200, 300);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 300), make_ask(Money(), 0));
    fixture.add(3, 1, "BHP", Side::BID, 900, 500);
    fixture.require_empty();
    fixture.add(1, 1, "BHP", Side::ASK, 1100, 50);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 300), make_ask(11 * Money::CENT, 50));
    fixture.add(2, 1, "BHP", Side::ASK, 1200, 400);
    fixture.require_empty();
    fixture.add(4, 1, "BHP", Side::BID, 1050, 80);
    fixture.require_bbo("BHP",
      make_bid(parse_money("0.105"), 80), make_ask(11 * Money::CENT, 50));
    fixture.add(3, 1, "BHP", Side::ASK, 1075, 90);
    fixture.require_bbo("BHP",
      make_bid(parse_money("0.105"), 80), make_ask(parse_money("0.1075"), 90));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 4, 1, Side::BID));
    fixture.require_book("BHP", Side::BID, parse_money("0.105"), 0);
    fixture.require_bbo("BHP",
      make_bid(10 * Money::CENT, 300), make_ask(parse_money("0.1075"), 90));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 3, 1, Side::ASK));
    fixture.require_book("BHP", Side::ASK, parse_money("0.1075"), 0);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 300), make_ask(11 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 1, 1, Side::BID));
    fixture.require_book("BHP", Side::BID, 10 * Money::CENT, 200);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 200), make_ask(11 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 2, 1, Side::BID));
    fixture.require_book("BHP", Side::BID, 10 * Money::CENT, 0);
    fixture.require_bbo(
      "BHP", make_bid(9 * Money::CENT, 500), make_ask(11 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 1, 1, Side::ASK));
    fixture.require_book("BHP", Side::ASK, 11 * Money::CENT, 0);
    fixture.require_bbo(
      "BHP", make_bid(9 * Money::CENT, 500), make_ask(12 * Money::CENT, 400));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 3, 1, Side::BID));
    fixture.require_book("BHP", Side::BID, 9 * Money::CENT, 0);
    fixture.require_bbo(
      "BHP", make_bid(Money(), 0), make_ask(12 * Money::CENT, 400));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 2, 1, Side::ASK));
    fixture.require_book("BHP", Side::ASK, 12 * Money::CENT, 0);
    fixture.require_bbo("BHP", make_bid(Money(), 0), make_ask(Money(), 0));
    fixture.require_empty();
  }

  TEST_CASE("replacement") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    fixture.directory(2, "RIO");
    fixture.add(1, 1, "BHP", Side::BID, 1000, 100);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 100), make_ask(Money(), 0));
    fixture.add(1, 2, "RIO", Side::BID, 2000, 400);
    fixture.require_bbo(
      "RIO", make_bid(20 * Money::CENT, 400), make_ask(Money(), 0));
    fixture.add(1, 1, "BHP", Side::ASK, 1200, 50);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 100), make_ask(12 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 1, 1, Side::BID, 1, 200, 1100, 0));
    fixture.require_book("BHP", Side::BID, 10 * Money::CENT, 0);
    fixture.require_book("BHP", Side::BID, 11 * Money::CENT, 200);
    fixture.require_bbo(
      "BHP", make_bid(11 * Money::CENT, 200), make_ask(12 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 1, 1, Side::BID, 1, 300, 1100, 0));
    fixture.require_book("BHP", Side::BID, 11 * Money::CENT, 300);
    fixture.require_bbo(
      "BHP", make_bid(11 * Money::CENT, 300), make_ask(12 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 1, 1, Side::BID, 1, 300, 1100, 0));
    fixture.require_empty();
    fixture.add(2, 1, "BHP", Side::BID, 900, 500);
    fixture.require_empty();
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 1, 1, Side::BID, 1, 250, 900, 0));
    fixture.require_book("BHP", Side::BID, 11 * Money::CENT, 0);
    fixture.require_book("BHP", Side::BID, 9 * Money::CENT, 750);
    fixture.require_bbo(
      "BHP", make_bid(9 * Money::CENT, 750), make_ask(12 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 1, 1, Side::BID, 1, 0, 900, 32));
    fixture.require_book("BHP", Side::BID, 9 * Money::CENT, 500);
    fixture.require_bbo(
      "BHP", make_bid(9 * Money::CENT, 500), make_ask(12 * Money::CENT, 50));
    fixture.add(3, 1, "BHP", Side::BID, 3000, 0);
    fixture.require_empty();
    auto quantity = std::uint64_t(1) << 33;
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 3, 1, Side::BID, 1, quantity, 3000, 0));
    fixture.require_book("BHP", Side::BID, 30 * Money::CENT, quantity);
    fixture.require_bbo("BHP",
      make_bid(30 * Money::CENT, quantity), make_ask(12 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 1, 2, Side::BID));
    fixture.require_book("RIO", Side::BID, 20 * Money::CENT, 0);
    fixture.require_bbo("RIO", make_bid(Money(), 0), make_ask(Money(), 0));
    fixture.require_empty();
  }


  TEST_CASE("execution") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    auto side = Side(Side::BID);
    SUBCASE("bid") {}
    SUBCASE("ask") {
      side = Side::ASK;
    }
    auto opposite = Quote(Money(), 0, get_opposite(side));
    auto require_size = [&] (Quantity size) {
      if(side == Side::BID) {
        fixture.require_bbo(
          "BHP", make_bid(10 * Money::CENT, size), opposite);
      } else {
        fixture.require_bbo(
          "BHP", opposite, make_ask(10 * Money::CENT, size));
      }
    };
    fixture.publish(AsxTradeItchAddOrderWithParticipant(
      NANOSECONDS, 1, 1, side, 1, 100, 1000, 0, 2, "AU123"));
    fixture.require_book("BHP", side, 10 * Money::CENT, 100, "AU123");
    require_size(100);
    fixture.add(2, 1, "BHP", side, 1000, 200);
    require_size(300);
    fixture.add(1, 1, "BHP", opposite.m_side, 1200, 50);
    opposite = Quote(12 * Money::CENT, 50, opposite.m_side);
    require_size(300);
    fixture.publish(AsxTradeItchOrderExecuted(
      NANOSECONDS, 1, 1, side, 40, {}, "AU123", "AU456"));
    fixture.require_book("BHP", side, 10 * Money::CENT, 60, "AU123");
    require_size(260);
    auto sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(sale->m_time_and_sale.get_index() == parse_ticker("BHP.ASX"));
    REQUIRE(sale->m_time_and_sale->m_price == 10 * Money::CENT);
    REQUIRE(sale->m_time_and_sale->m_size == 40);
    REQUIRE(sale->m_time_and_sale->m_timestamp == TIMESTAMP);
    if(side == Side::BID) {
      REQUIRE(sale->m_time_and_sale->m_buyer_mpid == "AU123");
      REQUIRE(sale->m_time_and_sale->m_seller_mpid == "AU456");
    } else {
      REQUIRE(sale->m_time_and_sale->m_buyer_mpid == "AU456");
      REQUIRE(sale->m_time_and_sale->m_seller_mpid == "AU123");
    }
    fixture.publish(AsxTradeItchOrderExecutedAtPrice(
      NANOSECONDS, 1, 1, side, 60, {}, "AU123", "AU456", 1050, 'Y', 'N'));
    fixture.require_book("BHP", side, 10 * Money::CENT, 0, "AU123");
    require_size(200);
    fixture.require_empty();
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 1, 1, side));
    fixture.require_empty();
    fixture.publish(AsxTradeItchOrderExecutedAtPrice(
      NANOSECONDS, 2, 1, side, 200, {}, "AU123", "AU456", 1050, 'N', 'Y'));
    fixture.require_book("BHP", side, 10 * Money::CENT, 0);
    if(side == Side::BID) {
      fixture.require_bbo("BHP", make_bid(Money(), 0), opposite);
    } else {
      fixture.require_bbo("BHP", opposite, make_ask(Money(), 0));
    }
    sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(sale->m_time_and_sale->m_price == parse_money("0.1050"));
    REQUIRE(sale->m_time_and_sale->m_size == 200);
    REQUIRE(sale->m_time_and_sale->m_condition.m_type ==
      TimeAndSale::Condition::Type::REGULAR);
    fixture.publish(
      AsxTradeItchOrderDelete(NANOSECONDS, 1, 1, opposite.m_side));
    fixture.require_book("BHP", opposite.m_side, 12 * Money::CENT, 0);
    fixture.require_bbo("BHP", make_bid(Money(), 0), make_ask(Money(), 0));
    fixture.require_empty();
  }


  TEST_CASE("trade_participants") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    auto side = Side(Side::BID);
    auto owner = std::string("AU550");
    auto counterparty = std::string("AU123");
    SUBCASE("bid") {}
    SUBCASE("ask") {
      side = Side::ASK;
    }
    SUBCASE("unknown_side") {
      side = Side::NONE;
    }
    SUBCASE("anonymous_owner") {
      owner.clear();
    }
    SUBCASE("anonymous_counterparty") {
      counterparty.clear();
    }
    fixture.publish(AsxTradeItchTrade(
      NANOSECONDS, {}, side, 20, 1, 1125, owner, counterparty, 'Y', 'N'));
    auto sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    fixture.require_empty();
    REQUIRE(sale->m_time_and_sale->m_market_center == "ASX");
    if(owner.empty()) {
      owner = "AU000";
    }
    if(counterparty.empty()) {
      counterparty = "AU000";
    }
    if(side == Side::BID) {
      REQUIRE(sale->m_time_and_sale->m_buyer_mpid == owner);
      REQUIRE(sale->m_time_and_sale->m_seller_mpid == counterparty);
    } else if(side == Side::ASK) {
      REQUIRE(sale->m_time_and_sale->m_buyer_mpid == counterparty);
      REQUIRE(sale->m_time_and_sale->m_seller_mpid == owner);
    } else {
      REQUIRE(sale->m_time_and_sale->m_buyer_mpid == "AU000");
      REQUIRE(sale->m_time_and_sale->m_seller_mpid == "AU000");
    }
  }

  TEST_CASE("trade") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    fixture.add(1, 1, "BHP", Side::BID, 1000, 100);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 100), make_ask(Money(), 0));
    fixture.add(2, 1, "BHP", Side::ASK, 1200, 0);
    fixture.require_empty();
    fixture.publish(AsxTradeItchOrderExecutedAtPrice(
      NANOSECONDS, 2, 1, Side::ASK, 50, {}, "AU123", "AU456", 1150, 'N', 'N'));
    fixture.require_empty();
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 2, 1, Side::ASK, 1, 100, 1200, 0));
    fixture.require_book("BHP", Side::ASK, 12 * Money::CENT, 100);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 100), make_ask(12 * Money::CENT, 100));
    auto trade = AsxTradeItchTrade(
      NANOSECONDS, {}, Side::NONE, 20, 1, 1125, "", "", 'N', 'N');
    fixture.publish(trade);
    fixture.require_empty();
    trade.m_printable = 'Y';
    fixture.publish(trade);
    auto sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(sale->m_time_and_sale == TickerTimeAndSale(
      TimeAndSale(TIMESTAMP, parse_money("0.1125"), 20,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "ASX", "AU000", "AU000"), parse_ticker("BHP.ASX")));
    fixture.require_empty();
    trade.m_occurred_at_cross = 'Y';
    trade.m_side = Side::ASK;
    trade.m_owner = "AU123";
    trade.m_counterparty = "AU456";
    using Type = TimeAndSale::Condition::Type;
    for(auto [state, type, code] : {
        std::tuple("CSPA", Type::CLOSE, "C"),
        std::tuple("PRE_CSPA", Type::CLOSE, "C"),
        std::tuple("PRE_OPEN", Type::OPEN, "O"),
        std::tuple("OPEN", Type::NONE, "AUCTION"),
        std::tuple("OSPA", Type::OPEN, "O"),
        std::tuple("UNKNOWN", Type::NONE, "AUCTION"),
        std::tuple("", Type::NONE, "AUCTION")}) {
      fixture.publish(AsxTradeItchOrderBookState(NANOSECONDS, 1, state));
      fixture.publish(trade);
      sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
      REQUIRE(sale->m_time_and_sale->m_condition ==
        TimeAndSale::Condition(type, code));
      REQUIRE(sale->m_time_and_sale->m_buyer_mpid == "AU456");
      REQUIRE(sale->m_time_and_sale->m_seller_mpid == "AU123");
      trade.m_occurred_at_cross = 'N';
      fixture.publish(trade);
      sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
      REQUIRE(sale->m_time_and_sale->m_condition ==
        TimeAndSale::Condition(Type::REGULAR, "@"));
      trade.m_occurred_at_cross = 'Y';
    }
    fixture.directory(2, "RIO");
    trade.m_order_book_id = 2;
    fixture.publish(trade);
    sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(sale->m_time_and_sale->m_condition ==
      TimeAndSale::Condition(Type::NONE, "AUCTION"));
    fixture.publish(AsxTradeItchOrderBookState(NANOSECONDS, 2, "PRE_OPEN"));
    fixture.publish(trade);
    sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(sale->m_time_and_sale->m_condition ==
      TimeAndSale::Condition(Type::OPEN, "O"));
    fixture.directory(2, "CBA");
    fixture.publish(trade);
    sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(sale->m_time_and_sale.get_index() == parse_ticker("CBA.ASX"));
    REQUIRE(sale->m_time_and_sale->m_condition ==
      TimeAndSale::Condition(Type::NONE, "AUCTION"));
    fixture.require_empty();
  }


  TEST_CASE("imbalance") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    auto message = AsxTradeItchEquilibriumPriceUpdate(
      NANOSECONDS, 1, 300, 100, 1050, 1000, 1100, 10, 20);
    auto require_imbalance = [&] (Side side, Quantity size, Money price) {
      fixture.publish(message);
      auto operation =
        fixture.take<FeedClient::PublishOrderImbalanceOperation>();
      REQUIRE(operation->m_imbalance == VenueOrderImbalance(OrderImbalance(
        parse_ticker("BHP.ASX"), side, size, price, TIMESTAMP),
        Venues::ASX));
      fixture.require_empty();
    };
    require_imbalance(Side::BID, 200, parse_money("0.1050"));
    message.m_ask_quantity = 400;
    require_imbalance(Side::ASK, 100, parse_money("0.1050"));
    message.m_ask_quantity = 300;
    require_imbalance(Side::NONE, 0, parse_money("0.1050"));
    message.m_bid_quantity = 0;
    message.m_ask_quantity = 0;
    message.m_equilibrium_price = std::numeric_limits<std::int32_t>::min();
    require_imbalance(Side::NONE, 0, Money());
  }


  TEST_CASE("directory") {
    auto fixture = Fixture();
    auto directory = AsxTradeItchOrderBookDirectory();
    directory.m_nanoseconds = NANOSECONDS;
    directory.m_order_book_id = 1;
    directory.m_symbol = "COMBO";
    directory.m_long_name = "COMBINATION";
    directory.m_currency = "AUD";
    directory.m_price_decimals = 2;
    directory.m_round_lot_size = 1;
    directory.m_financial_product = 5;
    fixture.publish(directory);
    REQUIRE(fixture.take<FeedClient::AddOperation>()->m_info ==
      TickerInfo(parse_ticker("COMBO.ASX"), "COMBINATION", "", 1));
    fixture.add(1, 1, "COMBO", Side::BID, -1000, 100);
    fixture.require_bbo(
      "COMBO", make_bid(-10 * Money::CENT, 100), make_ask(Money(), 0));
    fixture.publish(directory);
    fixture.take<FeedClient::AddOperation>();
    fixture.require_empty();
    directory.m_price_decimals = 3;
    fixture.publish(directory);
    fixture.take<FeedClient::AddOperation>();
    fixture.require_book("COMBO", Side::BID, -10 * Money::CENT, 0);
    fixture.require_book("COMBO", Side::BID, -1 * Money::CENT, 100);
    fixture.require_bbo(
      "COMBO", make_bid(-1 * Money::CENT, 100), make_ask(Money(), 0));
    fixture.directory(2, "BHP");
    fixture.publish(AsxTradeItchSeconds(1700000001));
    fixture.publish(
      AsxTradeItchAddOrder(NANOSECONDS, 1, 2, Side::ASK, 1, 50, 1234, 0, 2));
    auto order = fixture.take<FeedClient::PublishBookQuoteOperation>();
    auto timestamp = time_from_string("2023-11-14 22:13:21.123456");
    REQUIRE(order->m_quote->m_timestamp == timestamp);
    REQUIRE(order->m_quote->m_quote.m_price == parse_money("0.1234"));
    auto bbo = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(bbo->m_quote->m_timestamp == timestamp);
    fixture.publish(AsxTradeItchSeconds(1700000000));
    directory.m_symbol = "NEW";
    fixture.publish(directory);
    fixture.require_book("COMBO", Side::BID, -1 * Money::CENT, 0);
    fixture.require_bbo("COMBO", make_bid(Money(), 0), make_ask(Money(), 0));
    REQUIRE(fixture.take<FeedClient::AddOperation>()->m_info.m_ticker ==
      parse_ticker("NEW.ASX"));
    fixture.publish(AsxTradeItchSystemEvent(NANOSECONDS, 'O'));
    fixture.require_book("BHP", Side::ASK, parse_money("0.1234"), 0);
    fixture.require_bbo("BHP", make_bid(Money(), 0), make_ask(Money(), 0));
    fixture.require_empty();
  }


  TEST_CASE("repricing") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    fixture.add(1, 1, "BHP", Side::BID, 1000, 100);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 100), make_ask(Money(), 0));
    fixture.add(2, 1, "BHP", Side::BID, 2000, 200);
    fixture.require_bbo(
      "BHP", make_bid(20 * Money::CENT, 200), make_ask(Money(), 0));
    fixture.add(1, 1, "BHP", Side::ASK, 3000, 300);
    fixture.require_bbo(
      "BHP", make_bid(20 * Money::CENT, 200), make_ask(30 * Money::CENT, 300));
    fixture.publish(AsxTradeItchAddOrderWithParticipant(
      NANOSECONDS, 2, 1, Side::ASK, 1, 0, 4000, 0, 2, "AU777"));
    fixture.require_empty();
    auto directory = AsxTradeItchOrderBookDirectory();
    directory.m_nanoseconds = NANOSECONDS;
    directory.m_order_book_id = 1;
    directory.m_symbol = "BHP";
    directory.m_financial_product = 5;
    directory.m_price_decimals = 3;
    fixture.publish(directory);
    fixture.take<FeedClient::AddOperation>();
    auto quotes = std::vector<BookQuote>();
    auto expected = std::vector<BookQuote>();
    for(auto price : {10, 20, 30}) {
      auto side = Side(Side::BID);
      if(price == 30) {
        side = Side::ASK;
      }
      expected.emplace_back("AU000", false, Venues::ASX,
        Quote(price * Money::CENT, 0, side), TIMESTAMP);
      expected.emplace_back("AU000", false, Venues::ASX,
        Quote(price / 10 * Money::CENT, price * 10, side), TIMESTAMP);
    }
    for(auto i = std::size_t(0); i != expected.size(); ++i) {
      auto operation = fixture.take<FeedClient::PublishBookQuoteOperation>();
      REQUIRE(operation->m_quote.get_index() == parse_ticker("BHP.ASX"));
      quotes.push_back(*operation->m_quote);
    }
    REQUIRE(std::ranges::is_permutation(quotes, expected));
    fixture.require_bbo(
      "BHP", make_bid(2 * Money::CENT, 200), make_ask(3 * Money::CENT, 300));
    fixture.require_empty();
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 2, 1, Side::ASK, 1, 25, 4000, 0));
    fixture.require_book("BHP", Side::ASK, 4 * Money::CENT, 25, "AU777");
    fixture.require_empty();
  }

  TEST_CASE("completion") {
    auto fixture = Fixture();
    SUBCASE("end_of_session") {
      fixture.m_itch_client.m_messages.push(SharedBuffer("?", 1));
      fixture.m_itch_client.close();
      flush_pending_routines();
      REQUIRE(fixture.m_client.is_finished());
      REQUIRE(!fixture.m_client.get_exception());
    }
    SUBCASE("read_failure") {
      fixture.m_itch_client.m_messages.close(
        std::make_exception_ptr(IOException("Read failed.")));
      flush_pending_routines();
      REQUIRE(fixture.m_client.is_finished());
      auto error = fixture.m_client.get_exception();
      REQUIRE(error);
      REQUIRE_THROWS_AS(std::rethrow_exception(error), IOException);
    }
    SUBCASE("publication_failure") {
      auto failure =
        std::make_exception_ptr(IOException("Publication failed."));
      SUBCASE("io_error") {}
      SUBCASE("end_of_file") {
        failure = std::make_exception_ptr(EndOfFileException());
      }
      fixture.directory(1, "BHP");
      fixture.publish(
        AsxTradeItchAddOrder(NANOSECONDS, 1, 1, Side::BID, 1, 100, 1000, 0, 2));
      flush_pending_routines();
      auto operation = fixture.m_operations->try_pop();
      REQUIRE(operation.has_value());
      std::visit([&] (auto& operation) {
        operation.m_result.set(failure);
      }, **operation);
      flush_pending_routines();
      REQUIRE(fixture.m_client.is_finished());
      auto error = fixture.m_client.get_exception();
      REQUIRE(error);
      REQUIRE_THROWS_AS(std::rethrow_exception(error), IOException);
      REQUIRE(!fixture.m_operations->try_pop());
    }
    SUBCASE("missing_directory") {
      fixture.publish(
        AsxTradeItchAddOrder(NANOSECONDS, 1, 1, Side::BID, 1, 100, 1000, 0, 2));
      flush_pending_routines();
      REQUIRE(fixture.m_client.is_finished());
      auto error = fixture.m_client.get_exception();
      REQUIRE(error);
      REQUIRE_THROWS_AS(std::rethrow_exception(error), IOException);
      REQUIRE(!fixture.m_operations->try_pop());
    }
    SUBCASE("malformed_message") {
      auto log = Log();
      fixture.m_itch_client.m_messages.push(SharedBuffer("A\x7f", 2));
      flush_pending_routines();
      REQUIRE(fixture.m_client.is_finished());
      auto error = fixture.m_client.get_exception();
      REQUIRE(error);
      REQUIRE_THROWS_AS(
        std::rethrow_exception(error), AsxTradeItchParserException);
      REQUIRE(log.m_output.str() == "(message A 7f)\n");
    }
    SUBCASE("close") {
      fixture.m_client.close();
      REQUIRE(fixture.m_client.is_finished());
      REQUIRE(!fixture.m_client.get_exception());
    }
    SUBCASE("close_during_publication") {
      fixture.directory(1, "BHP");
      fixture.publish(
        AsxTradeItchAddOrder(NANOSECONDS, 1, 1, Side::BID, 1, 100, 1000, 0, 2));
      flush_pending_routines();
      auto operation = fixture.m_operations->try_pop();
      REQUIRE(operation.has_value());
      fixture.publish(
        AsxTradeItchAddOrder(NANOSECONDS, 2, 1, Side::ASK, 1, 200, 1100, 0, 2));
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
      REQUIRE(fixture.m_client.is_finished());
      REQUIRE(!fixture.m_client.get_exception());
      REQUIRE(!fixture.m_operations->try_pop());
    }
  }


  TEST_CASE("price_units") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    fixture.publish(
      AsxTradeItchAddOrder(NANOSECONDS, 1, 1, Side::BID, 1, 100, 55000, 0, 2));
    fixture.require_book("BHP", Side::BID, parse_money("5.50"), 100);
    fixture.require_bbo(
      "BHP", make_bid(parse_money("5.50"), 100), make_ask(Money(), 0));
    fixture.publish(
      AsxTradeItchOrderReplace(NANOSECONDS, 1, 1, Side::BID, 1, 90, 55100, 0));
    fixture.require_book("BHP", Side::BID, parse_money("5.50"), 0);
    fixture.require_book("BHP", Side::BID, parse_money("5.51"), 90);
    fixture.require_bbo(
      "BHP", make_bid(parse_money("5.51"), 90), make_ask(Money(), 0));
    fixture.publish(
      AsxTradeItchOrderReplace(NANOSECONDS, 1, 1, Side::BID, 1, 80, 410, 0));
    fixture.require_book("BHP", Side::BID, parse_money("5.51"), 0);
    fixture.require_book("BHP", Side::BID, parse_money("0.041"), 80);
    fixture.require_bbo(
      "BHP", make_bid(parse_money("0.041"), 80), make_ask(Money(), 0));
    fixture.require_empty();
  }

}
