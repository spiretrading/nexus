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
      : m_operations(std::make_shared<FeedClient::Queue>()),
        m_feed_client(m_operations),
        m_client([] {
          auto config = AsxTradeItchConfiguration();
          config.m_primary_venue = Venues::ASX;
          config.m_disseminating_venue = Venues::ASX;
          config.m_mpid = "ASX";
          return config;
        }(), &m_feed_client, &m_itch_client) {
      publish(AsxTradeItchSeconds(1700000000));
    }

    ~Fixture() {
      m_feed_client.close();
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

    std::string add(std::uint64_t id, std::uint32_t book,
        const std::string& symbol, Side side, std::int32_t price,
        std::uint64_t quantity) {
      publish(AsxTradeItchAddOrder(
        NANOSECONDS, id, book, side, 1, quantity, price, 0, 2));
      if(quantity == 0) {
        return {};
      }
      auto operation = take<FeedClient::AddOrderOperation>();
      REQUIRE(operation->m_ticker == Ticker(symbol, Venues::ASX));
      REQUIRE(operation->m_side == side);
      REQUIRE(operation->m_price == Money(Quantity(price / 10000.0)));
      REQUIRE(operation->m_size == Quantity(quantity));
      REQUIRE(operation->m_timestamp == TIMESTAMP);
      REQUIRE(operation->m_venue == Venues::ASX);
      REQUIRE(operation->m_mpid == "ASX");
      return operation->m_id;
    }

    void require_bbo(const std::string& symbol, Quote bid, Quote ask) {
      auto operation = take<FeedClient::PublishBboQuoteOperation>();
      REQUIRE(operation->m_quote == TickerBboQuote(
        BboQuote(bid, ask, TIMESTAMP), Ticker(symbol, Venues::ASX)));
    }
  };
}

TEST_SUITE("AsxTradeItchMarketDataFeedClient") {
  TEST_CASE("bbo_depth") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    auto first = fixture.add(1, 1, "BHP", Side::BID, 1000, 100);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 100), make_ask(Money(), 0));
    auto second = fixture.add(2, 1, "BHP", Side::BID, 1000, 200);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 300), make_ask(Money(), 0));
    fixture.add(3, 1, "BHP", Side::BID, 900, 500);
    fixture.require_empty();
    auto ask = fixture.add(1, 1, "BHP", Side::ASK, 1100, 50);
    REQUIRE(ask != first);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 300), make_ask(11 * Money::CENT, 50));
    fixture.add(2, 1, "BHP", Side::ASK, 1200, 400);
    fixture.require_empty();
    auto best_bid = fixture.add(4, 1, "BHP", Side::BID, 1050, 80);
    fixture.require_bbo("BHP",
      make_bid(parse_money("0.105"), 80), make_ask(11 * Money::CENT, 50));
    auto best_ask = fixture.add(3, 1, "BHP", Side::ASK, 1075, 90);
    fixture.require_bbo("BHP",
      make_bid(parse_money("0.105"), 80), make_ask(parse_money("0.1075"), 90));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 4, 1, Side::BID));
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == best_bid);
    fixture.require_bbo("BHP",
      make_bid(10 * Money::CENT, 300), make_ask(parse_money("0.1075"), 90));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 3, 1, Side::ASK));
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == best_ask);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 300), make_ask(11 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 1, 1, Side::BID));
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == first);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 200), make_ask(11 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 2, 1, Side::BID));
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == second);
    fixture.require_bbo(
      "BHP", make_bid(9 * Money::CENT, 500), make_ask(11 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 1, 1, Side::ASK));
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == ask);
    fixture.require_bbo(
      "BHP", make_bid(9 * Money::CENT, 500), make_ask(12 * Money::CENT, 400));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 3, 1, Side::BID));
    fixture.take<FeedClient::RemoveOrderOperation>();
    fixture.require_bbo(
      "BHP", make_bid(Money(), 0), make_ask(12 * Money::CENT, 400));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 2, 1, Side::ASK));
    fixture.take<FeedClient::RemoveOrderOperation>();
    fixture.require_bbo("BHP", make_bid(Money(), 0), make_ask(Money(), 0));
    fixture.require_empty();
  }

  TEST_CASE("replacement") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    fixture.directory(2, "RIO");
    auto bid = fixture.add(1, 1, "BHP", Side::BID, 1000, 100);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 100), make_ask(Money(), 0));
    auto other = fixture.add(1, 2, "RIO", Side::BID, 2000, 400);
    REQUIRE(other != bid);
    fixture.require_bbo(
      "RIO", make_bid(20 * Money::CENT, 400), make_ask(Money(), 0));
    fixture.add(1, 1, "BHP", Side::ASK, 1200, 50);
    fixture.require_bbo(
      "BHP", make_bid(10 * Money::CENT, 100), make_ask(12 * Money::CENT, 50));
    auto replace = [&] (std::int32_t price, std::uint64_t quantity) {
      fixture.publish(AsxTradeItchOrderReplace(
        NANOSECONDS, 1, 1, Side::BID, 1, quantity, price, 0));
      auto operation = fixture.take<FeedClient::AddOrderOperation>();
      REQUIRE(operation->m_id == bid);
      REQUIRE(operation->m_price == Money(Quantity(price / 10000.0)));
      REQUIRE(operation->m_size == Quantity(quantity));
    };
    replace(1100, 200);
    fixture.require_bbo(
      "BHP", make_bid(11 * Money::CENT, 200), make_ask(12 * Money::CENT, 50));
    replace(1100, 300);
    fixture.require_bbo(
      "BHP", make_bid(11 * Money::CENT, 300), make_ask(12 * Money::CENT, 50));
    replace(1100, 300);
    fixture.require_empty();
    fixture.add(2, 1, "BHP", Side::BID, 900, 500);
    fixture.require_empty();
    replace(900, 250);
    fixture.require_bbo(
      "BHP", make_bid(9 * Money::CENT, 750), make_ask(12 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 1, 1, Side::BID, 1, 0, 900, 32));
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == bid);
    fixture.require_bbo(
      "BHP", make_bid(9 * Money::CENT, 500), make_ask(12 * Money::CENT, 50));
    fixture.add(3, 1, "BHP", Side::BID, 3000, 0);
    fixture.require_empty();
    auto quantity = std::uint64_t(1) << 33;
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 3, 1, Side::BID, 1, quantity, 3000, 0));
    auto operation = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(operation->m_size == Quantity(quantity));
    fixture.require_bbo("BHP",
      make_bid(30 * Money::CENT, quantity), make_ask(12 * Money::CENT, 50));
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 1, 2, Side::BID));
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == other);
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
    auto require_size = [&] (Quantity size) {
      if(side == Side::BID) {
        fixture.require_bbo(
          "BHP", make_bid(10 * Money::CENT, size), make_ask(Money(), 0));
      } else {
        fixture.require_bbo(
          "BHP", make_bid(Money(), 0), make_ask(10 * Money::CENT, size));
      }
    };
    fixture.publish(AsxTradeItchAddOrderWithParticipant(
      NANOSECONDS, 1, 1, side, 1, 100, 1000, 0, 2, "AU123"));
    auto order = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(order->m_size == 100);
    REQUIRE(order->m_side == side);
    require_size(100);
    auto id = fixture.add(2, 1, "BHP", side, 1000, 200);
    require_size(300);
    fixture.publish(AsxTradeItchOrderExecuted(
      NANOSECONDS, 1, 1, side, 40, {}, "AU123", "AU456"));
    auto update = fixture.take<FeedClient::OffsetOrderSizeOperation>();
    REQUIRE(update->m_id == order->m_id);
    REQUIRE(update->m_delta == -40);
    REQUIRE(update->m_timestamp == TIMESTAMP);
    require_size(260);
    auto sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(sale->m_time_and_sale.get_index() == Ticker("BHP", Venues::ASX));
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
    update = fixture.take<FeedClient::OffsetOrderSizeOperation>();
    REQUIRE(update->m_id == order->m_id);
    REQUIRE(update->m_delta == -60);
    require_size(200);
    fixture.require_empty();
    fixture.publish(AsxTradeItchOrderDelete(NANOSECONDS, 1, 1, side));
    fixture.require_empty();
    fixture.publish(AsxTradeItchOrderExecutedAtPrice(
      NANOSECONDS, 2, 1, side, 200, {}, "AU123", "AU456", 1050, 'N', 'Y'));
    update = fixture.take<FeedClient::OffsetOrderSizeOperation>();
    REQUIRE(update->m_id == id);
    REQUIRE(update->m_delta == -200);
    fixture.require_bbo("BHP", make_bid(Money(), 0), make_ask(Money(), 0));
    sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(sale->m_time_and_sale->m_price == parse_money("0.1050"));
    REQUIRE(sale->m_time_and_sale->m_size == 200);
    REQUIRE(sale->m_time_and_sale->m_condition.m_type ==
      TimeAndSale::Condition::Type::REGULAR);
    fixture.require_empty();
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
    fixture.take<FeedClient::AddOrderOperation>();
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
        "ASX", "", ""), Ticker("BHP", Venues::ASX)));
    fixture.require_empty();
    fixture.publish(AsxTradeItchOrderBookState(NANOSECONDS, 1, "CSPA"));
    trade.m_occurred_at_cross = 'Y';
    trade.m_side = Side::ASK;
    trade.m_owner = "AU123";
    trade.m_counterparty = "AU456";
    fixture.publish(trade);
    sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(sale->m_time_and_sale->m_condition ==
      TimeAndSale::Condition(TimeAndSale::Condition::Type::CLOSE, "C"));
    REQUIRE(sale->m_time_and_sale->m_buyer_mpid == "AU456");
    REQUIRE(sale->m_time_and_sale->m_seller_mpid == "AU123");
    fixture.publish(AsxTradeItchOrderBookState(NANOSECONDS, 1, "PRE_OPEN"));
    fixture.publish(trade);
    sale = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(sale->m_time_and_sale->m_condition ==
      TimeAndSale::Condition(TimeAndSale::Condition::Type::OPEN, "O"));
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
      REQUIRE(operation->m_imbalance == VenueOrderImbalance(
        OrderImbalance(
          Ticker("BHP", Venues::ASX), side, size, price, TIMESTAMP),
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
    auto directory = AsxTradeItchCombinationOrderBookDirectory();
    directory.m_nanoseconds = NANOSECONDS;
    directory.m_order_book_id = 1;
    directory.m_symbol = "COMBO";
    directory.m_long_name = "COMBINATION";
    directory.m_currency = "AUD";
    directory.m_price_decimals = 2;
    directory.m_round_lot_size = 1;
    for(auto& leg : directory.m_legs) {
      leg.m_side = '?';
    }
    fixture.publish(directory);
    REQUIRE(fixture.take<FeedClient::AddOperation>()->m_info ==
      TickerInfo(Ticker("COMBO", Venues::ASX), "COMBINATION", "", 1));
    auto id = fixture.add(1, 1, "COMBO", Side::BID, -1000, 100);
    fixture.require_bbo(
      "COMBO", make_bid(-10 * Money::CENT, 100), make_ask(Money(), 0));
    fixture.publish(directory);
    fixture.take<FeedClient::AddOperation>();
    fixture.require_empty();
    directory.m_price_decimals = 3;
    fixture.publish(directory);
    fixture.take<FeedClient::AddOperation>();
    auto order = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(order->m_id == id);
    REQUIRE(order->m_price == -1 * Money::CENT);
    fixture.require_bbo(
      "COMBO", make_bid(-1 * Money::CENT, 100), make_ask(Money(), 0));
    fixture.directory(2, "BHP");
    fixture.publish(AsxTradeItchSeconds(1700000001));
    fixture.publish(AsxTradeItchAddOrder(
      NANOSECONDS, 1, 2, Side::ASK, 1, 50, 1234, 0, 2));
    order = fixture.take<FeedClient::AddOrderOperation>();
    auto timestamp = time_from_string("2023-11-14 22:13:21.123456");
    REQUIRE(order->m_timestamp == timestamp);
    REQUIRE(order->m_price == parse_money("0.1234"));
    auto bbo = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(bbo->m_quote->m_timestamp == timestamp);
    fixture.publish(AsxTradeItchSeconds(1700000000));
    directory.m_symbol = "NEW";
    fixture.publish(directory);
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == id);
    fixture.require_bbo("COMBO", make_bid(Money(), 0), make_ask(Money(), 0));
    REQUIRE(fixture.take<FeedClient::AddOperation>()->m_info.m_ticker ==
      Ticker("NEW", Venues::ASX));
    fixture.publish(AsxTradeItchSystemEvent(NANOSECONDS, 'O'));
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id ==
      order->m_id);
    fixture.require_bbo("BHP", make_bid(Money(), 0), make_ask(Money(), 0));
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
      fixture.directory(1, "BHP");
      fixture.publish(AsxTradeItchAddOrder(
        NANOSECONDS, 1, 1, Side::BID, 1, 100, 1000, 0, 2));
      flush_pending_routines();
      auto operation = fixture.m_operations->try_pop();
      REQUIRE(operation.has_value());
      std::visit([] (auto& operation) {
        operation.m_result.set(
          std::make_exception_ptr(IOException("Publication failed.")));
      }, **operation);
      flush_pending_routines();
      REQUIRE(fixture.m_client.is_finished());
      auto error = fixture.m_client.get_exception();
      REQUIRE(error);
      REQUIRE_THROWS_AS(std::rethrow_exception(error), IOException);
      REQUIRE(!fixture.m_operations->try_pop());
    }
    SUBCASE("missing_directory") {
      fixture.publish(AsxTradeItchAddOrder(
        NANOSECONDS, 1, 1, Side::BID, 1, 100, 1000, 0, 2));
      flush_pending_routines();
      REQUIRE(fixture.m_client.is_finished());
      auto error = fixture.m_client.get_exception();
      REQUIRE(error);
      REQUIRE_THROWS_AS(std::rethrow_exception(error), IOException);
      REQUIRE(!fixture.m_operations->try_pop());
    }
    SUBCASE("malformed_message") {
      fixture.m_itch_client.m_messages.push(SharedBuffer("A", 1));
      flush_pending_routines();
      REQUIRE(fixture.m_client.is_finished());
      auto error = fixture.m_client.get_exception();
      REQUIRE(error);
      REQUIRE_THROWS_AS(
        std::rethrow_exception(error), AsxTradeItchParserException);
    }
    SUBCASE("close") {
      fixture.m_client.close();
      REQUIRE(fixture.m_client.is_finished());
      REQUIRE(!fixture.m_client.get_exception());
    }
  }


  TEST_CASE("price_units") {
    auto fixture = Fixture();
    fixture.directory(1, "BHP");
    fixture.publish(AsxTradeItchAddOrder(
      NANOSECONDS, 1, 1, Side::BID, 1, 100, 55000, 0, 2));
    auto order = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(order->m_price == parse_money("5.50"));
    fixture.require_bbo(
      "BHP", make_bid(parse_money("5.50"), 100), make_ask(Money(), 0));
    fixture.publish(AsxTradeItchOrderReplace(
      NANOSECONDS, 1, 1, Side::BID, 1, 90, 55100, 0));
    order = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(order->m_price == parse_money("5.51"));
    fixture.require_bbo(
      "BHP", make_bid(parse_money("5.51"), 90), make_ask(Money(), 0));
    fixture.require_empty();
  }

}
