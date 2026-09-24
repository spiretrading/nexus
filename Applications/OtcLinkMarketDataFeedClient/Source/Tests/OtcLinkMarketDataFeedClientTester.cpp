#include <future>
#include <sstream>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataServiceTests/TestMarketDataFeedClient.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkMarketDataFeedClient.hpp"

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

  template<typename O>
  std::shared_ptr<O> require_operation(FeedClient::Queue& operations) {
    flush_pending_routines();
    auto operation = operations.try_pop();
    REQUIRE(operation.has_value());
    auto actual = std::get_if<O>(&**operation);
    REQUIRE(actual);
    auto result = std::shared_ptr<O>(*operation, actual);
    result->m_result.set();
    return result;
  }

  constexpr auto TIMESTAMP = std::uint64_t(1700000000123);

  struct Encoder {
    SharedBuffer m_buffer;

    template<typename... T>
    void write(T... values) {
      (append(m_buffer, boost::endian::native_to_big(values)), ...);
    }

    SharedBuffer finish(std::uint8_t type) {
      auto result = SharedBuffer();
      append(result, boost::endian::native_to_big(
        static_cast<std::uint16_t>(
          OtcLinkMessage::HEADER_LENGTH + m_buffer.get_size())));
      append(result, type);
      append(result, m_buffer);
      return result;
    }
  };

  template<typename S> requires std::same_as<S, OtcLinkSecurity> ||
    std::same_as<S, OtcLinkFractionalSecurity>
  SharedBuffer encode(const S& message) {
    auto encoder = Encoder();
    encoder.write(message.m_sequence);
    auto symbol = std::string(message.m_symbol);
    symbol.resize(10, ' ');
    append(encoder.m_buffer, symbol.data(), symbol.size());
    encoder.write(message.m_timestamp,
      static_cast<std::uint8_t>(message.m_action),
      static_cast<std::uint8_t>(message.m_asset_class), message.m_security,
      message.m_flags, static_cast<std::uint8_t>(message.m_tier),
      message.m_reporting_status, message.m_status);
    if constexpr(std::same_as<S, OtcLinkFractionalSecurity>) {
      encoder.write(
        message.m_minimum_notional_value, message.m_minimum_quote_size);
    }
    return encoder.finish(message.TYPE);
  }

  template<typename Q> requires std::same_as<Q, OtcLinkQuote> ||
    std::same_as<Q, OtcLinkFractionalQuote> ||
    std::same_as<Q, OtcLinkInside> ||
    std::same_as<Q, OtcLinkFractionalInside>
  SharedBuffer encode(const Q& message) {
    auto encoder = Encoder();
    encoder.write(message.m_sequence);
    if constexpr(requires { message.m_quote; }) {
      encoder.write(message.m_quote);
    } else {
      encoder.write(message.m_inside);
    }
    encoder.write(static_cast<std::uint8_t>(message.m_action),
      message.m_flags, message.m_security);
    if constexpr(requires { message.m_mpid; }) {
      append(encoder.m_buffer, message.m_mpid.data(), message.m_mpid.size());
    }
    encoder.write(message.m_ask_price, message.m_ask_size);
    if constexpr(requires { message.m_ask_adjustment; }) {
      encoder.write(message.m_ask_adjustment);
    }
    encoder.write(message.m_ask_timestamp, message.m_bid_price,
      message.m_bid_size);
    if constexpr(requires { message.m_bid_adjustment; }) {
      encoder.write(message.m_bid_adjustment);
    }
    encoder.write(message.m_bid_timestamp);
    if constexpr(requires { message.m_reference; }) {
      encoder.write(message.m_reference, message.m_extended_flags);
    } else {
      encoder.write(message.m_ask_participants, message.m_bid_participants);
    }
    return encoder.finish(message.TYPE);
  }

  template<typename Q> requires std::same_as<Q, OtcLinkQuoteUpdate> ||
    std::same_as<Q, OtcLinkFractionalQuoteUpdate> ||
    std::same_as<Q, OtcLinkInsideUpdate> ||
    std::same_as<Q, OtcLinkFractionalInsideUpdate>
  SharedBuffer encode(const Q& message) {
    auto encoder = Encoder();
    encoder.write(message.m_sequence);
    if constexpr(requires { message.m_quote; }) {
      encoder.write(message.m_quote);
    } else {
      encoder.write(message.m_inside);
    }
    encoder.write(message.m_flags, message.m_price, message.m_size);
    if constexpr(requires { message.m_adjustment; }) {
      encoder.write(message.m_adjustment);
    }
    encoder.write(message.m_timestamp);
    if constexpr(requires { message.m_reference; }) {
      encoder.write(message.m_reference, message.m_extended_flags);
    } else {
      encoder.write(message.m_participants);
    }
    return encoder.finish(message.TYPE);
  }

  SharedBuffer encode(const OtcLinkTrade& message) {
    auto encoder = Encoder();
    constexpr auto ADD = std::uint8_t(2);
    encoder.write(message.m_sequence, message.m_trade, ADD, message.m_flags,
      message.m_security, message.m_status);
    append(encoder.m_buffer, message.m_venue.data(), message.m_venue.size());
    append(encoder.m_buffer, "     ", 5);
    encoder.write(message.m_price, message.m_size, message.m_timestamp);
    return encoder.finish(message.TYPE);
  }

  OtcLinkTrade make_trade() {
    auto trade = OtcLinkTrade();
    trade.m_trade = 40;
    trade.m_security = 10;
    trade.m_venue = "ATS";
    trade.m_price = 1250000;
    trade.m_size = 123;
    trade.m_timestamp = TIMESTAMP;
    return trade;
  }

  SharedBuffer encode(const OtcLinkMarketClose& message) {
    auto encoder = Encoder();
    encoder.write(message.m_sequence, message.m_timestamp, message.m_count);
    return encoder.finish(message.TYPE);
  }

  SharedBuffer encode(const OtcLinkMarketOpen& message) {
    auto encoder = Encoder();
    encoder.write(message.m_sequence, message.m_timestamp, message.m_close);
    return encoder.finish(message.TYPE);
  }

  SharedBuffer encode(const OtcLinkSpinStart& message) {
    auto encoder = Encoder();
    encoder.write(message.m_sequence,
      static_cast<std::uint8_t>(message.m_type), message.m_timestamp,
      message.m_last_sequence);
    return encoder.finish(message.TYPE);
  }

  OtcLinkSecurity make_security() {
    auto message = OtcLinkSecurity();
    message.m_security = 10;
    message.m_symbol = "NLST";
    message.m_timestamp = TIMESTAMP;
    message.m_action = OtcLinkSecurityAction::SPIN;
    message.m_asset_class = OtcLinkAssetClass::EQUITY;
    message.m_tier = OtcLinkTier::OTCQB;
    message.m_status = 'A';
    return message;
  }

  OtcLinkQuote make_quote() {
    auto message = OtcLinkQuote();
    message.m_quote = 20;
    message.m_security = 10;
    message.m_action = OtcLinkQuoteAction::SPIN;
    message.m_flags = 0x4a;
    message.m_mpid = "CDEL";
    message.m_bid_price = 1250000;
    message.m_bid_size = 100;
    message.m_bid_timestamp = TIMESTAMP;
    message.m_ask_price = 1500000;
    message.m_ask_size = 200;
    message.m_ask_timestamp = TIMESTAMP;
    return message;
  }

  OtcLinkInside make_inside() {
    auto message = OtcLinkInside();
    message.m_inside = 30;
    message.m_security = 10;
    message.m_action = OtcLinkInsideAction::SPIN;
    message.m_flags = 0x4a;
    message.m_bid_price = 1250000;
    message.m_bid_size = 300;
    message.m_bid_timestamp = TIMESTAMP;
    message.m_ask_price = 1500000;
    message.m_ask_size = 400;
    message.m_ask_timestamp = TIMESTAMP;
    return message;
  }

  struct TestOtcClient {
    Queue<std::pair<SharedBuffer, std::uint64_t>> m_messages;
    SharedBuffer m_payload;

    OtcLinkMessage read() {
      auto session = std::uint64_t(0);
      return read(out(session));
    }

    OtcLinkMessage read(Out<std::uint64_t> session) {
      while(true) {
        if(auto message = read_event(session)) {
          return *message;
        }
      }
    }

    boost::optional<OtcLinkMessage> read_event(Out<std::uint64_t> session) {
      auto message = m_messages.pop();
      m_payload = std::move(message.first);
      *session = message.second;
      if(m_payload.get_size() == 0) {
        return boost::none;
      }
      return OtcLinkMessage::parse(
        std::string_view(m_payload.get_data(), m_payload.get_size()));
    }

    void close() {
      m_messages.close(std::make_exception_ptr(EndOfFileException()));
    }
  };

  boost::optional<OtcLinkSnapshot> make_reference() {
    return OtcLinkSnapshot(500000, {encode(make_security()),
      encode(make_quote()), encode(make_inside()), encode(make_trade())});
  }

  struct TradeFeed {};

  struct Fixture {
    std::shared_ptr<FeedClient::Queue> m_operations;
    FeedClient m_feed_client;
    TestOtcClient m_otc_client;
    FixedTimeClient m_time_client;
    OtcLinkMarketDataFeedClient<
      FeedClient*, TestOtcClient*, FixedTimeClient*> m_client;
    std::uint64_t m_session;

    Fixture()
        : m_operations(std::make_shared<FeedClient::Queue>()),
          m_feed_client(m_operations),
          m_time_client(time_from_string("2023-11-15 00:00:00")),
          m_client(&m_feed_client, &m_otc_client, &m_time_client),
          m_session(0) {}

    explicit Fixture(TradeFeed)
        : m_operations(std::make_shared<FeedClient::Queue>()),
          m_feed_client(m_operations),
          m_time_client(time_from_string("2023-11-15 00:00:00")),
          m_client(&m_feed_client, &m_otc_client, &m_time_client,
            make_reference()),
          m_session(0) {}

    Fixture(boost::optional<OtcLinkSnapshot> reference,
        bool is_logging_messages)
        : m_operations(std::make_shared<FeedClient::Queue>()),
          m_feed_client(m_operations),
          m_time_client(time_from_string("2023-11-15 00:00:00")),
          m_client(&m_feed_client, &m_otc_client, &m_time_client,
            std::move(reference), is_logging_messages),
          m_session(0) {}

    ~Fixture() {
      m_client.close();
    }

    void send(const auto& message) {
      m_otc_client.m_messages.push(std::pair(encode(message), m_session));
    }

    template<typename O>
    std::shared_ptr<O> take() {
      return require_operation<O>(*m_operations);
    }

    void security() {
      send(make_security());
      auto operation = take<FeedClient::AddOperation>();
      REQUIRE(operation->m_info.m_ticker == parse_ticker("NLST.OTCB"));
    }

    void require_empty() {
      flush_pending_routines();
      REQUIRE(!m_client.get_exception());
      REQUIRE(!m_operations->try_pop());
    }
  };
}

TEST_SUITE("OtcLinkMarketDataFeedClient") {
  TEST_CASE("message_logging") {
    auto log = Log();
    auto is_logging = false;
    SUBCASE("disabled") {}
    SUBCASE("enabled") {
      is_logging = true;
    }
    auto encoder = Encoder();
    encoder.write(std::uint8_t(0), std::uint8_t(0x80), std::uint8_t(0xff));
    auto payload = encoder.finish(255);
    auto reference = OtcLinkSnapshot(100, {payload});
    auto fixture = Fixture(reference, is_logging);
    fixture.m_otc_client.m_messages.push(std::pair(payload, 0));
    fixture.require_empty();
    if(is_logging) {
      REQUIRE(log.m_output.str() ==
        "(message 255 0080ff)\n(message 255 0080ff)\n");
    } else {
      REQUIRE(log.m_output.str().empty());
    }
  }

  TEST_CASE("parsed_message_logging") {
    auto log = Log();
    auto is_logging = false;
    SUBCASE("disabled") {}
    SUBCASE("enabled") {
      is_logging = true;
    }
    auto reference = OtcLinkSnapshot(100,
      {encode(OtcLinkMarketOpen(1, 2, 3))});
    auto fixture = Fixture(reference, is_logging);
    fixture.send(OtcLinkMarketClose(4, 5, 6));
    fixture.require_empty();
    if(is_logging) {
      REQUIRE(log.m_output.str() ==
        "(market_open 1 2 3)\n(market_close 4 5 6)\n");
    } else {
      REQUIRE(log.m_output.str().empty());
    }
  }

  TEST_CASE("malformed_reference_logging") {
    auto log = Log();
    auto payload = Encoder().finish(OtcLinkSecurity::TYPE);
    auto reference = OtcLinkSnapshot(100,
      {payload, encode(OtcLinkMarketOpen(1, 2, 3))});
    auto fixture = Fixture(reference, true);
    fixture.require_empty();
    REQUIRE(!fixture.m_client.is_finished());
    auto output = log.m_output.str();
    REQUIRE(output.starts_with("(message 9 )\n(bad_message 9 "));
    REQUIRE(output.ends_with("(market_open 1 2 3)\n"));
  }

  TEST_CASE("malformed_message_logging") {
    auto log = Log();
    auto is_logging = false;
    SUBCASE("disabled") {}
    SUBCASE("enabled") {
      is_logging = true;
    }
    auto fixture = Fixture(boost::none, is_logging);
    auto payload = Encoder().finish(OtcLinkTrade::TYPE);
    fixture.m_otc_client.m_messages.push(std::pair(payload, 0));
    fixture.m_otc_client.m_messages.push(
      std::pair(Encoder().finish(255), 0));
    fixture.require_empty();
    REQUIRE(!fixture.m_client.is_finished());
    auto output = log.m_output.str();
    auto message = std::string("(message 17 )\n");
    REQUIRE(output.starts_with(message));
    REQUIRE(output.find(message, message.size()) == std::string::npos);
    REQUIRE(output.find("(bad_message 17 ") == message.size());
    if(is_logging) {
      REQUIRE(output.ends_with("(message 255 )\n"));
    } else {
      REQUIRE(output.find("(message 255 ") == std::string::npos);
    }
  }

  TEST_CASE("unmapped_security") {
    auto log = Log();
    auto fixture = Fixture();
    auto trade = make_trade();
    SUBCASE("trade") {
      fixture.send(trade);
    }
    SUBCASE("book") {
      fixture.send(make_quote());
    }
    SUBCASE("inside") {
      fixture.send(make_inside());
    }
    fixture.require_empty();
    REQUIRE(log.m_output.str() ==
      "(unmapped_security 2023-Nov-15 00:00:00 10)\n");
    for(auto i = 0; i != 2; ++i) {
      fixture.send(trade);
      fixture.send(make_quote());
      fixture.send(make_inside());
    }
    ++trade.m_security;
    fixture.send(trade);
    fixture.send(trade);
    fixture.require_empty();
    auto expected = std::string(
      "(unmapped_security 2023-Nov-15 00:00:00 10)\n"
      "(unmapped_security 2023-Nov-15 00:00:00 11)\n");
    REQUIRE(log.m_output.str() == expected);
    fixture.security();
    trade = make_trade();
    fixture.send(trade);
    fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    auto security = make_security();
    security.m_action = OtcLinkSecurityAction::DELETE;
    fixture.send(security);
    fixture.send(trade);
    ++fixture.m_session;
    fixture.send(trade);
    fixture.require_empty();
    REQUIRE(log.m_output.str() == expected);
  }

  TEST_CASE("malformed_trade") {
    auto log = Log();
    auto fixture = Fixture(TradeFeed());
    fixture.take<FeedClient::AddOperation>();
    auto trade = make_trade();
    auto buffer = encode(trade);
    SUBCASE("action") {
      constexpr auto ACTION_OFFSET =
        OtcLinkMessage::HEADER_LENGTH + 2 * sizeof(std::uint32_t);
      buffer.get_mutable_data()[ACTION_OFFSET] = 3;
    }
    SUBCASE("timestamp") {
      trade.m_timestamp = std::numeric_limits<std::uint64_t>::max();
      buffer = encode(trade);
    }
    fixture.m_otc_client.m_messages.push(std::pair(std::move(buffer), 0));
    trade = make_trade();
    fixture.m_otc_client.m_messages.push(std::pair(encode(trade), 0));
    auto publication = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(publication->m_time_and_sale->m_size == trade.m_size);
    REQUIRE(!log.m_output.str().empty());
    fixture.require_empty();
  }

  TEST_CASE("trade_channel") {
    auto fixture = Fixture(TradeFeed());
    auto trade = make_trade();
    fixture.send(trade);
    auto definition = fixture.take<FeedClient::AddOperation>();
    REQUIRE(definition->m_info.m_ticker == parse_ticker("NLST.OTCB"));
    auto publication = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(publication->m_time_and_sale.get_index() ==
      parse_ticker("NLST.OTCB"));
    fixture.require_empty();
    REQUIRE(!fixture.m_client.is_finished());
    fixture.send(make_quote());
    fixture.send(make_inside());
    fixture.require_empty();
    ++fixture.m_session;
    fixture.send(OtcLinkMarketOpen());
    fixture.send(trade);
    publication = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(publication->m_time_and_sale.get_index() ==
      parse_ticker("NLST.OTCB"));
    fixture.require_empty();
    fixture.m_client.close();
    REQUIRE(fixture.m_client.is_finished());
    REQUIRE(!fixture.m_client.get_exception());
  }

  TEST_CASE("reference_completion") {
    auto fixture = Fixture(TradeFeed());
    flush_pending_routines();
    SUBCASE("close_during_publication") {
      fixture.m_client.close();
      REQUIRE(fixture.m_client.is_finished());
      REQUIRE(!fixture.m_client.get_exception());
    }
    SUBCASE("publication_failure") {
      auto operation = fixture.m_operations->try_pop();
      REQUIRE(operation.has_value());
      auto add = std::get_if<FeedClient::AddOperation>(&**operation);
      REQUIRE(add);
      add->m_result.set(
        std::make_exception_ptr(IOException("Publish failed.")));
      flush_pending_routines();
      REQUIRE(fixture.m_client.is_finished());
      REQUIRE_THROWS_AS(
        std::rethrow_exception(fixture.m_client.get_exception()), IOException);
    }
  }

  TEST_CASE("trades") {
    auto fixture = Fixture();
    auto trade = make_trade();
    fixture.send(trade);
    fixture.require_empty();
    fixture.security();
    SUBCASE("regular") {}
    SUBCASE("irregular") {
      trade.m_status = 0x01;
    }
    fixture.send(trade);
    auto publication = fixture.take<FeedClient::PublishTimeAndSaleOperation>();
    auto& sale = publication->m_time_and_sale;
    REQUIRE(sale.get_index() == parse_ticker("NLST.OTCB"));
    REQUIRE(sale->m_timestamp == time_from_string("2023-11-14 22:13:20.123"));
    REQUIRE(sale->m_price == Money(Quantity(1.25)));
    REQUIRE(sale->m_size == 123);
    REQUIRE(sale->m_market_center == "ATS");
    REQUIRE(sale->m_buyer_mpid.empty());
    REQUIRE(sale->m_seller_mpid.empty());
    if(trade.m_status == 0) {
      REQUIRE(sale->m_condition.m_type ==
        TimeAndSale::Condition::Type::REGULAR);
      REQUIRE(sale->m_condition.m_code == "@");
    } else {
      REQUIRE(sale->m_condition.m_type == TimeAndSale::Condition::Type::NONE);
      REQUIRE(sale->m_condition.m_code == "I");
    }
    auto security = make_security();
    security.m_action = OtcLinkSecurityAction::DELETE;
    fixture.send(security);
    fixture.send(trade);
    fixture.require_empty();
  }

  TEST_CASE("participant_quotes") {
    auto fixture = Fixture();
    fixture.security();
    auto quote = make_quote();
    fixture.send(quote);
    auto bid = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(bid->m_ticker == parse_ticker("NLST.OTCB"));
    REQUIRE(bid->m_venue == Venues::OTCM);
    REQUIRE(bid->m_mpid == "CDEL");
    REQUIRE(!bid->m_is_primary_mpid);
    REQUIRE(bid->m_id == "20:B");
    REQUIRE(bid->m_side == Side::BID);
    REQUIRE(bid->m_price == Money(Quantity(1.25)));
    REQUIRE(bid->m_size == 100);
    REQUIRE(bid->m_timestamp ==
      time_from_string("2023-11-14 22:13:20.123"));
    auto ask = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(ask->m_id == "20:A");
    REQUIRE(ask->m_side == Side::ASK);
    REQUIRE(ask->m_price == Money(Quantity(1.5)));
    REQUIRE(ask->m_size == 200);
    fixture.send(quote);
    fixture.require_empty();
    auto update = OtcLinkQuoteUpdate();
    update.m_quote = quote.m_quote;
    update.m_flags = 0x4b;
    update.m_price = 1750000;
    update.m_size = 250;
    update.m_timestamp = TIMESTAMP + 1;
    fixture.send(update);
    ask = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(ask->m_id == "20:A");
    REQUIRE(ask->m_price == Money(Quantity(1.75)));
    REQUIRE(ask->m_size == 250);
    fixture.require_empty();
    update.m_flags = 0x49;
    fixture.send(update);
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == "20:B");
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == "20:A");
    update.m_flags = 0x43;
    fixture.send(update);
    bid = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(bid->m_id == "20:B");
    REQUIRE(bid->m_size == 100);
    fixture.require_empty();
    quote.m_action = OtcLinkQuoteAction::DELETE;
    fixture.send(quote);
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == "20:B");
    fixture.send(update);
    fixture.require_empty();
  }

  TEST_CASE("inside_quotes") {
    auto fixture = Fixture();
    fixture.security();
    auto inside = make_inside();
    fixture.send(inside);
    auto publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote.get_index() == parse_ticker("NLST.OTCB"));
    REQUIRE(publication->m_quote->m_bid ==
      make_bid(Money(Quantity(1.25)), 300));
    REQUIRE(publication->m_quote->m_ask ==
      make_ask(Money(Quantity(1.5)), 400));
    REQUIRE(publication->m_quote->m_timestamp ==
      time_from_string("2023-11-14 22:13:20.123"));
    fixture.send(inside);
    fixture.require_empty();
    auto update = OtcLinkInsideUpdate();
    update.m_inside = inside.m_inside;
    update.m_flags = 0x4b;
    update.m_price = 1750000;
    update.m_size = 600;
    update.m_timestamp = TIMESTAMP + 1;
    fixture.send(update);
    publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_bid ==
      make_bid(Money(Quantity(1.25)), 300));
    REQUIRE(publication->m_quote->m_ask ==
      make_ask(Money(Quantity(1.75)), 600));
    REQUIRE(publication->m_quote->m_timestamp ==
      time_from_string("2023-11-14 22:13:20.124"));
    update.m_flags = 0x0a;
    update.m_price = 0;
    update.m_size = 0;
    fixture.send(update);
    publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_bid == make_bid(Money(), Quantity()));
    REQUIRE(publication->m_quote->m_ask.m_size == 600);
    inside.m_action = OtcLinkInsideAction::DELETE;
    fixture.send(inside);
    publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_bid.m_size == 0);
    REQUIRE(publication->m_quote->m_ask.m_size == 0);
    fixture.send(update);
    fixture.require_empty();
  }

  TEST_CASE("fractional_quotes") {
    auto fixture = Fixture();
    fixture.security();
    auto quote = OtcLinkFractionalQuote();
    quote.m_quote = 20;
    quote.m_security = 10;
    quote.m_action = OtcLinkQuoteAction::SPIN;
    quote.m_flags = 0x4a;
    quote.m_mpid = "CDEL";
    quote.m_bid_price = 125000025;
    quote.m_bid_size = 10000000125;
    quote.m_bid_timestamp = TIMESTAMP;
    quote.m_ask_price = 150000075;
    quote.m_ask_size = 20000000225;
    quote.m_ask_timestamp = TIMESTAMP;
    fixture.send(quote);
    auto bid = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(bid->m_price ==
      Money(Quantity::from_representation(1250000.25)));
    REQUIRE(bid->m_size == Quantity::from_representation(100000001.25));
    auto ask = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(ask->m_price ==
      Money(Quantity::from_representation(1500000.75)));
    REQUIRE(ask->m_size == Quantity::from_representation(200000002.25));
    auto update = OtcLinkFractionalQuoteUpdate();
    update.m_quote = quote.m_quote;
    update.m_flags = 0x4a;
    update.m_price = 130000050;
    update.m_size = 35000000025;
    update.m_timestamp = TIMESTAMP + 1;
    fixture.send(update);
    bid = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(bid->m_price ==
      Money(Quantity::from_representation(1300000.5)));
    REQUIRE(bid->m_size == Quantity::from_representation(350000000.25));
    fixture.require_empty();
    auto inside = OtcLinkFractionalInside();
    inside.m_inside = 30;
    inside.m_security = 10;
    inside.m_action = OtcLinkInsideAction::SPIN;
    inside.m_flags = 0x4a;
    inside.m_bid_price = quote.m_bid_price;
    inside.m_bid_size = quote.m_bid_size;
    inside.m_bid_timestamp = TIMESTAMP;
    inside.m_ask_price = quote.m_ask_price;
    inside.m_ask_size = quote.m_ask_size;
    inside.m_ask_timestamp = TIMESTAMP;
    fixture.send(inside);
    auto publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_bid.m_price ==
      Money(Quantity::from_representation(1250000.25)));
    REQUIRE(publication->m_quote->m_bid.m_size ==
      Quantity::from_representation(100000001.25));
    auto inside_update = OtcLinkFractionalInsideUpdate();
    inside_update.m_inside = inside.m_inside;
    inside_update.m_flags = 0x4b;
    inside_update.m_price = 160000050;
    inside_update.m_size = 32000000025;
    inside_update.m_timestamp = TIMESTAMP + 1;
    fixture.send(inside_update);
    publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_ask.m_price ==
      Money(Quantity::from_representation(1600000.5)));
    REQUIRE(publication->m_quote->m_ask.m_size ==
      Quantity::from_representation(320000000.25));
    fixture.require_empty();
  }

  TEST_CASE("fractional_security") {
    auto security = OtcLinkFractionalSecurity();
    static_cast<OtcLinkSecurity&>(security) = make_security();
    security.m_minimum_notional_value = 100;
    security.m_minimum_quote_size = 25;
    auto buffer = encode(security);
    REQUIRE(OtcLinkMessage::parse(
      std::string_view(buffer.get_data(), buffer.get_size())).m_type == 23);
    SUBCASE("live") {
      auto fixture = Fixture();
      fixture.send(security);
      REQUIRE(fixture.take<FeedClient::AddOperation>()->m_info.m_ticker ==
        parse_ticker("NLST.OTCB"));
      fixture.send(make_quote());
      REQUIRE(fixture.take<FeedClient::AddOrderOperation>()->m_ticker ==
        parse_ticker("NLST.OTCB"));
      REQUIRE(fixture.take<FeedClient::AddOrderOperation>()->m_ticker ==
        parse_ticker("NLST.OTCB"));
      fixture.require_empty();
    }
    SUBCASE("reference") {
      auto fixture = Fixture(OtcLinkSnapshot(100, {buffer}), false);
      REQUIRE(fixture.take<FeedClient::AddOperation>()->m_info.m_ticker ==
        parse_ticker("NLST.OTCB"));
      fixture.send(make_trade());
      auto publication =
        fixture.take<FeedClient::PublishTimeAndSaleOperation>();
      REQUIRE(publication->m_time_and_sale.get_index() ==
        parse_ticker("NLST.OTCB"));
      fixture.require_empty();
    }
  }

  TEST_CASE("security_changes") {
    auto fixture = Fixture();
    fixture.security();
    auto security = make_security();
    auto quote = make_quote();
    auto inside = make_inside();
    fixture.send(quote);
    fixture.take<FeedClient::AddOrderOperation>();
    fixture.take<FeedClient::AddOrderOperation>();
    fixture.send(inside);
    fixture.take<FeedClient::PublishBboQuoteOperation>();
    auto other_security = make_security();
    other_security.m_security = 11;
    other_security.m_symbol = "OTHER";
    fixture.send(other_security);
    REQUIRE(fixture.take<FeedClient::AddOperation>()->m_info.m_ticker ==
      parse_ticker("OTHER.OTCB"));
    auto other_quote = make_quote();
    other_quote.m_security = other_security.m_security;
    other_quote.m_quote = 21;
    fixture.send(other_quote);
    fixture.take<FeedClient::AddOrderOperation>();
    fixture.take<FeedClient::AddOrderOperation>();
    auto other_inside = make_inside();
    other_inside.m_security = other_security.m_security;
    other_inside.m_inside = 31;
    fixture.send(other_inside);
    fixture.take<FeedClient::PublishBboQuoteOperation>();
    fixture.send(security);
    fixture.require_empty();
    security.m_action = OtcLinkSecurityAction::UPDATE;
    security.m_symbol = "NEW";
    security.m_tier = OtcLinkTier::OTCQX_US;
    fixture.send(security);
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == "20:B");
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_id == "20:A");
    auto publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote.get_index() == parse_ticker("NLST.OTCB"));
    REQUIRE(publication->m_quote->m_bid.m_size == 0);
    REQUIRE(publication->m_quote->m_ask.m_size == 0);
    REQUIRE(fixture.take<FeedClient::AddOperation>()->m_info.m_ticker ==
      parse_ticker("NEW.OTCQ"));
    REQUIRE(fixture.take<FeedClient::AddOrderOperation>()->m_ticker ==
      parse_ticker("NEW.OTCQ"));
    REQUIRE(fixture.take<FeedClient::AddOrderOperation>()->m_ticker ==
      parse_ticker("NEW.OTCQ"));
    publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote.get_index() == parse_ticker("NEW.OTCQ"));
    REQUIRE(publication->m_quote->m_bid.m_size == 300);
    REQUIRE(publication->m_quote->m_ask.m_size == 400);
    SUBCASE("remove") {
      security.m_action = OtcLinkSecurityAction::DELETE;
    }
    SUBCASE("deleted_status") {
      security.m_status = 'D';
    }
    SUBCASE("fixed_income") {
      security.m_asset_class = OtcLinkAssetClass::FIXED_INCOME;
    }
    SUBCASE("unsupported_tier") {
      security.m_tier = static_cast<OtcLinkTier>(99);
    }
    fixture.send(security);
    fixture.take<FeedClient::RemoveOrderOperation>();
    fixture.take<FeedClient::RemoveOrderOperation>();
    publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote.get_index() == parse_ticker("NEW.OTCQ"));
    REQUIRE(publication->m_quote->m_bid.m_size == 0);
    REQUIRE(publication->m_quote->m_ask.m_size == 0);
    fixture.send(quote);
    fixture.send(inside);
    fixture.require_empty();
    auto update = OtcLinkQuoteUpdate();
    update.m_quote = other_quote.m_quote;
    update.m_flags = 0x4b;
    update.m_price = 1750000;
    update.m_size = 250;
    update.m_timestamp = TIMESTAMP + 1;
    fixture.send(update);
    auto order = fixture.take<FeedClient::AddOrderOperation>();
    REQUIRE(order->m_id == "21:A");
    REQUIRE(order->m_ticker == parse_ticker("OTHER.OTCB"));
    REQUIRE(order->m_price == Money(Quantity(1.75)));
    REQUIRE(order->m_size == 250);
    auto inside_update = OtcLinkInsideUpdate();
    inside_update.m_inside = other_inside.m_inside;
    inside_update.m_flags = 0x4b;
    inside_update.m_price = 1750000;
    inside_update.m_size = 600;
    inside_update.m_timestamp = TIMESTAMP + 1;
    fixture.send(inside_update);
    publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote.get_index() == parse_ticker("OTHER.OTCB"));
    REQUIRE(publication->m_quote->m_bid ==
      make_bid(Money(Quantity(1.25)), 300));
    REQUIRE(publication->m_quote->m_ask ==
      make_ask(Money(Quantity(1.75)), 600));
    fixture.require_empty();
  }

  TEST_CASE("reset_heartbeat") {
    auto server = LocalServerConnection();
    auto time_client = FixedTimeClient(
      time_from_string("2023-11-15 00:00:00"));
    auto timer = TriggerTimer();
    using ProtocolClient =
      OtcLinkProtocolClient<LocalClientChannel*, FixedTimeClient*>;
    auto channels = std::vector<std::unique_ptr<LocalClientChannel>>();
    auto servers = std::vector<std::unique_ptr<LocalServerChannel>>();
    auto protocols = std::vector<std::unique_ptr<ProtocolClient>>();
    auto clients = std::vector<ProtocolClient*>();
    for(auto i = 0; i != 2; ++i) {
      auto accepted = std::async(std::launch::async, [&] {
        return server.accept();
      });
      channels.push_back(
        std::make_unique<LocalClientChannel>("otc_link", server));
      servers.push_back(accepted.get());
      protocols.push_back(std::make_unique<ProtocolClient>(
        channels.back().get(), &time_client));
      clients.push_back(protocols.back().get());
    }
    auto client = OtcLinkClient(
      seconds(1), seconds(1), clients, &time_client, &timer);
    auto operations = std::make_shared<FeedClient::Queue>();
    auto feed_client = FeedClient(operations);
    auto publisher = OtcLinkMarketDataFeedClient(
      &feed_client, &client, &time_client);
    auto publish = [&] (int feed, std::uint8_t flags,
        const std::vector<SharedBuffer>& messages) {
      auto length = OtcLinkHeader::LENGTH;
      for(auto& message : messages) {
        length += message.get_size();
      }
      auto timestamp = boost::local_time::local_date_time(
        time_client.get_time(),
        TIME_ZONES.time_zone_from_region("America/New_York")).local_time();
      auto encoder = Encoder();
      encoder.write(static_cast<std::uint16_t>(length), std::uint32_t(1),
        flags, static_cast<std::uint8_t>(messages.size()),
        static_cast<std::uint32_t>(
          timestamp.time_of_day().total_milliseconds()));
      for(auto& message : messages) {
        append(encoder.m_buffer, message.get_data(), message.get_size());
      }
      servers[feed]->get_writer().write(encoder.m_buffer);
      flush_pending_routines();
    };
    auto security = make_security();
    security.m_sequence = 1;
    auto quote = make_quote();
    quote.m_sequence = 2;
    auto inside = make_inside();
    inside.m_sequence = 3;
    publish(0, 0, {encode(security), encode(quote), encode(inside)});
    publish(1, 0, {encode(security), encode(quote), encode(inside)});
    require_operation<FeedClient::AddOperation>(*operations);
    require_operation<FeedClient::AddOrderOperation>(*operations);
    require_operation<FeedClient::AddOrderOperation>(*operations);
    require_operation<FeedClient::PublishBboQuoteOperation>(*operations);
    publish(0, static_cast<std::uint8_t>(
      OtcLinkHeader::Flag::SEQUENCE_RESET), {});
    publish(0, static_cast<std::uint8_t>(OtcLinkHeader::Flag::HEARTBEAT), {});
    require_operation<FeedClient::RemoveOrderOperation>(*operations);
    require_operation<FeedClient::RemoveOrderOperation>(*operations);
    auto cleared =
      require_operation<FeedClient::PublishBboQuoteOperation>(*operations);
    REQUIRE(cleared->m_quote->m_bid.m_size == 0);
    REQUIRE(cleared->m_quote->m_ask.m_size == 0);
    REQUIRE(cleared->m_quote->m_timestamp == time_client.get_time());
    publish(1, static_cast<std::uint8_t>(
      OtcLinkHeader::Flag::SEQUENCE_RESET), {});
    publish(1, static_cast<std::uint8_t>(OtcLinkHeader::Flag::HEARTBEAT), {});
    REQUIRE(!publisher.get_exception());
    REQUIRE(!operations->try_pop());
  }

  TEST_CASE("reset") {
    auto fixture = Fixture();
    fixture.security();
    fixture.send(make_quote());
    fixture.take<FeedClient::AddOrderOperation>();
    fixture.take<FeedClient::AddOrderOperation>();
    fixture.send(make_inside());
    fixture.take<FeedClient::PublishBboQuoteOperation>();
    auto timestamp = time_from_string("2023-11-14 22:13:20.123");
    auto is_session_reset = false;
    SUBCASE("channel") {
      is_session_reset = true;
      ++fixture.m_session;
      timestamp = fixture.m_time_client.get_time();
      fixture.send(make_security());
    }
    SUBCASE("opening_spin") {
      fixture.send(OtcLinkSpinStart(
        0, OtcLinkSpinType::OPENING, TIMESTAMP, 0));
    }
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_timestamp ==
      timestamp);
    REQUIRE(fixture.take<FeedClient::RemoveOrderOperation>()->m_timestamp ==
      timestamp);
    auto publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_timestamp == timestamp);
    REQUIRE(publication->m_quote->m_bid.m_size == 0);
    REQUIRE(publication->m_quote->m_ask.m_size == 0);
    if(is_session_reset) {
      fixture.take<FeedClient::AddOperation>();
    }
    auto stale = OtcLinkQuoteUpdate();
    stale.m_quote = make_quote().m_quote;
    stale.m_flags = 0x4a;
    stale.m_price = 1100000;
    stale.m_size = 500;
    stale.m_timestamp = TIMESTAMP;
    fixture.send(stale);
    fixture.require_empty();
    fixture.send(make_quote());
    REQUIRE(fixture.take<FeedClient::AddOrderOperation>()->m_size == 100);
    REQUIRE(fixture.take<FeedClient::AddOrderOperation>()->m_size == 200);
    fixture.send(make_inside());
    publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_bid.m_size == 300);
    REQUIRE(publication->m_quote->m_ask.m_size == 400);
    fixture.require_empty();
  }

  TEST_CASE("market_state") {
    auto fixture = Fixture();
    fixture.security();
    fixture.send(make_quote());
    fixture.take<FeedClient::AddOrderOperation>();
    fixture.take<FeedClient::AddOrderOperation>();
    fixture.send(make_inside());
    fixture.take<FeedClient::PublishBboQuoteOperation>();
    fixture.send(OtcLinkMarketClose(0, TIMESTAMP, 0));
    fixture.take<FeedClient::RemoveOrderOperation>();
    fixture.take<FeedClient::RemoveOrderOperation>();
    auto publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_bid.m_size == 0);
    REQUIRE(publication->m_quote->m_ask.m_size == 0);
    auto quote = make_quote();
    quote.m_bid_size = 500;
    fixture.send(quote);
    auto inside = make_inside();
    inside.m_bid_size = 700;
    fixture.send(inside);
    fixture.require_empty();
    fixture.send(OtcLinkMarketOpen(0, TIMESTAMP + 1, TIMESTAMP + 1000));
    REQUIRE(fixture.take<FeedClient::AddOrderOperation>()->m_size == 500);
    REQUIRE(fixture.take<FeedClient::AddOrderOperation>()->m_size == 200);
    publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_bid.m_size == 700);
    REQUIRE(publication->m_quote->m_ask.m_size == 400);
    fixture.require_empty();
  }

  TEST_CASE("security_tiers") {
    auto fixture = Fixture();
    auto security = make_security();
    for(auto [tier, venue] : {
        std::pair(OtcLinkTier::OTCQX_US, Venues::OTCQ),
        std::pair(OtcLinkTier::OTCQX_INTERNATIONAL, Venues::OTCQ),
        std::pair(OtcLinkTier::OTCQB, Venues::OTCB),
        std::pair(OtcLinkTier::OTCID, Venues::OTCD),
        std::pair(OtcLinkTier::PINK_LIMITED, Venues::PINL),
        std::pair(OtcLinkTier::EXPERT_MARKET, Venues::EXPM),
        std::pair(OtcLinkTier::GREY_MARKET, Venues::PSGM)}) {
      ++security.m_security;
      security.m_tier = tier;
      fixture.send(security);
      REQUIRE(fixture.take<FeedClient::AddOperation>()->m_info.m_ticker ==
        Ticker("NLST", venue));
    }
    security.m_security = 10;
    security.m_asset_class = OtcLinkAssetClass::FIXED_INCOME;
    fixture.send(security);
    fixture.send(make_quote());
    fixture.send(make_inside());
    fixture.require_empty();
    security.m_asset_class = OtcLinkAssetClass::EQUITY;
    security.m_tier = OtcLinkTier::NONE;
    fixture.send(security);
    fixture.require_empty();
    security.m_tier = OtcLinkTier::OTCQB;
    security.m_symbol = "";
    fixture.send(security);
    fixture.require_empty();
  }

  TEST_CASE("malformed_message") {
    auto log = Log();
    auto fixture = Fixture();
    fixture.security();
    SUBCASE("truncated") {
      auto encoder = Encoder();
      encoder.write(std::uint32_t(0x01020304));
      fixture.m_otc_client.m_messages.push(
        std::pair(encoder.finish(OtcLinkQuote::TYPE), fixture.m_session));
    }
    SUBCASE("invalid_timestamp") {
      auto quote = make_quote();
      quote.m_bid_timestamp = UINT64_MAX;
      fixture.send(quote);
    }
    fixture.send(make_quote());
    REQUIRE(fixture.take<FeedClient::AddOrderOperation>()->m_size == 100);
    REQUIRE(fixture.take<FeedClient::AddOrderOperation>()->m_size == 200);
    fixture.require_empty();
    REQUIRE(log.m_output.str().find("(bad_message 1 ") != std::string::npos);
    REQUIRE(!fixture.m_client.is_finished());
  }

  TEST_CASE("quote_identity") {
    auto fixture = Fixture();
    fixture.security();
    auto quote = make_quote();
    fixture.send(quote);
    fixture.take<FeedClient::AddOrderOperation>();
    fixture.take<FeedClient::AddOrderOperation>();
    SUBCASE("participant") {
      quote.m_mpid = "NITE";
      fixture.send(quote);
      fixture.take<FeedClient::RemoveOrderOperation>();
      fixture.take<FeedClient::RemoveOrderOperation>();
      REQUIRE(fixture.take<FeedClient::AddOrderOperation>()->m_mpid == "NITE");
      REQUIRE(fixture.take<FeedClient::AddOrderOperation>()->m_mpid == "NITE");
    }
    SUBCASE("unknown_security") {
      ++quote.m_security;
      fixture.send(quote);
      fixture.take<FeedClient::RemoveOrderOperation>();
      fixture.take<FeedClient::RemoveOrderOperation>();
    }
    fixture.require_empty();
  }

  TEST_CASE("completion") {
    auto fixture = Fixture();
    SUBCASE("end_of_file") {
      fixture.m_otc_client.close();
      flush_pending_routines();
      REQUIRE(fixture.m_client.is_finished());
      REQUIRE(!fixture.m_client.get_exception());
    }
    SUBCASE("read_failure") {
      fixture.m_otc_client.m_messages.close(
        std::make_exception_ptr(IOException("Read failed.")));
      flush_pending_routines();
      REQUIRE(fixture.m_client.is_finished());
      REQUIRE(fixture.m_client.get_exception());
      REQUIRE_THROWS_AS(
        std::rethrow_exception(fixture.m_client.get_exception()), IOException);
    }
    SUBCASE("publish_failure") {
      fixture.send(make_security());
      flush_pending_routines();
      auto operation = fixture.m_operations->try_pop();
      REQUIRE(operation.has_value());
      auto add = std::get_if<FeedClient::AddOperation>(&**operation);
      REQUIRE(add);
      add->m_result.set(
        std::make_exception_ptr(IOException("Publish failed.")));
      flush_pending_routines();
      REQUIRE(fixture.m_client.is_finished());
      REQUIRE(fixture.m_client.get_exception());
      REQUIRE_THROWS_AS(
        std::rethrow_exception(fixture.m_client.get_exception()), IOException);
    }
    SUBCASE("close_during_read") {
      fixture.m_client.close();
      REQUIRE(fixture.m_client.is_finished());
      REQUIRE(!fixture.m_client.get_exception());
    }
    SUBCASE("close_during_publish") {
      fixture.send(make_security());
      flush_pending_routines();
      fixture.m_client.close();
      REQUIRE(fixture.m_client.is_finished());
      REQUIRE(!fixture.m_client.get_exception());
    }
  }

  TEST_CASE("inside_entries") {
    auto fixture = Fixture();
    fixture.security();
    auto bid = make_inside();
    bid.m_flags = 0x42;
    bid.m_ask_size = 0;
    fixture.send(bid);
    auto publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_bid.m_size == 300);
    REQUIRE(publication->m_quote->m_ask.m_size == 0);
    auto ask = make_inside();
    ++ask.m_inside;
    ask.m_flags = 0x0a;
    ask.m_bid_size = 0;
    fixture.send(ask);
    publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_bid.m_size == 300);
    REQUIRE(publication->m_quote->m_ask.m_size == 400);
    bid.m_action = OtcLinkInsideAction::DELETE;
    fixture.send(bid);
    publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_bid.m_size == 0);
    REQUIRE(publication->m_quote->m_ask.m_size == 400);
    ask.m_action = OtcLinkInsideAction::DELETE;
    fixture.send(ask);
    publication = fixture.take<FeedClient::PublishBboQuoteOperation>();
    REQUIRE(publication->m_quote->m_bid.m_size == 0);
    REQUIRE(publication->m_quote->m_ask.m_size == 0);
    fixture.require_empty();
  }
}
