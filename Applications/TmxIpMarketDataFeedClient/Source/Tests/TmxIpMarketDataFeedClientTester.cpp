#include <Beam/Queues/Queue.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataFeedClient.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpMarketDataFeedClient.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct MessageClient {
    Queue<std::string> m_messages;
    std::string m_payload;
    std::atomic_int m_count = 0;
    bool m_is_closed = false;

    StampMessage read() {
      m_payload = m_messages.pop();
      ++m_count;
      return StampMessage::parse(m_payload);
    }

    int get_count() const {
      return m_count;
    }

    void close() {
      m_is_closed = true;
      m_messages.close();
    }
  };
  using FeedClient = Nexus::Tests::TestMarketDataFeedClient;
  using DataClient = Nexus::Tests::TestMarketDataClient;

  struct Fixture {
    using Client = TmxIpMarketDataFeedClient<
      MessageClient*, DataClient*, FixedTimeClient*, FeedClient*>;
    std::shared_ptr<FeedClient::Queue> m_feed_operations;
    std::shared_ptr<DataClient::Queue> m_data_operations;
    FeedClient m_feed;
    DataClient m_data;
    MessageClient m_source;
    FixedTimeClient m_time;
    Client m_client;

    Fixture()
      : Fixture({TickerInfo(parse_ticker("ABX.TSX"), "Barrick", "", 100)}) {}

    explicit Fixture(std::vector<TickerInfo> tickers)
        : m_feed_operations(std::make_shared<FeedClient::Queue>()),
          m_data_operations(std::make_shared<DataClient::Queue>()),
          m_feed(m_feed_operations),
          m_data(m_data_operations),
          m_time(time_from_string("2026-09-21 13:00:00")),
          m_client([] {
            auto config = TmxIpConfiguration();
            config.m_country = Countries::CA;
            return config;
          }(), parse_trading_schedule(YAML::Load(R"(
- venues: [XTSE, XTSX, XCNQ, NEOE]
  time:
    weekdays: [Sat, Sun]
- venues: [XTSE, XTSX, XCNQ, NEOE]
  dates: [2026-12-25]
- venues: [XTSE]
  dates: [2026-12-24]
  events:
    - code: OPEN
      time: "09:30:00"
    - code: CLOSE
      time: "13:00:00"
- venues: [XTSE, XTSX, XCNQ, NEOE]
  events:
    - code: OPEN
      time: "09:30:00"
    - code: CLOSE
      time: "16:00:00"
)")), &m_source, &m_data, &m_time, &m_feed) {
      auto operation = query();
      REQUIRE(operation->m_query.get_index() == Scope(Countries::CA));
      REQUIRE(operation->m_query.get_snapshot_limit() ==
        SnapshotLimit::UNLIMITED);
      operation->m_result.set(tickers);
      flush_pending_routines();
    }

    ~Fixture() {
      m_client.close();
    }

    std::shared_ptr<DataClient::TickerInfoQueryOperation> query() {
      flush_pending_routines();
      auto operation = m_data_operations->try_pop();
      REQUIRE(operation.has_value());
      auto actual =
        std::get_if<DataClient::TickerInfoQueryOperation>(&**operation);
      REQUIRE(actual);
      return std::shared_ptr<DataClient::TickerInfoQueryOperation>(
        *operation, actual);
    }

    void publish(std::string_view fields) {
      publish(fields, "|56=20260921090000000|17=FFFFFFFF|54=0123abcd|50=1");
    }

    void publish(std::string_view fields, std::string_view control) {
      auto message = std::string("") + std::string(control) + '' +
        std::string(fields);
      std::ranges::replace(message, '|', '');
      m_source.m_messages.push(std::move(message));
    }

    void publish_cbbo() {
      publish("|6=Quote|5=Quote|55=ABX|196=25.50|196.1=25.51"
        "|64=123|64.1=456");
    }

    TickerBboQuote quote() {
      flush_pending_routines();
      auto operation = m_feed_operations->try_pop();
      REQUIRE(operation.has_value());
      auto actual = std::get_if<FeedClient::PublishBboQuoteOperation>(
        &**operation);
      REQUIRE(actual);
      auto quote = actual->m_quote;
      actual->m_result.set();
      flush_pending_routines();
      REQUIRE_FALSE(m_client.get_exception());
      return quote;
    }

    void require_empty() {
      flush_pending_routines();
      REQUIRE_FALSE(m_client.is_finished());
      REQUIRE_FALSE(m_client.get_exception());
      REQUIRE_FALSE(m_feed_operations->try_pop().has_value());
    }
  };

}

TEST_SUITE("TmxIpMarketDataFeedClient") {
  TEST_CASE("close") {
    auto fixture = Fixture();
    auto& source = fixture.m_source;
    auto& client = fixture.m_client;
    flush_pending_routines();
    REQUIRE_FALSE(client.is_finished());
    client.close();
    REQUIRE(client.is_finished());
    REQUIRE_FALSE(client.get_exception());
    REQUIRE_NOTHROW(client.close());
    REQUIRE(source.m_is_closed);
    REQUIRE(source.get_count() == 0);
  }

  TEST_CASE("source_error") {
    auto fixture = Fixture();
    auto& source = fixture.m_source;
    auto& client = fixture.m_client;
    flush_pending_routines();
    auto exception = std::make_exception_ptr(IOException("Feed failed."));
    source.m_messages.close(exception);
    flush_pending_routines();
    REQUIRE(client.is_finished());
    exception = client.get_exception();
    REQUIRE(exception);
    REQUIRE_THROWS_AS(std::rethrow_exception(exception), IOException);
    client.close();
    REQUIRE(source.m_is_closed);
    REQUIRE(client.get_exception() == exception);
  }

  TEST_CASE("parse_error") {
    auto fixture = Fixture();
    auto& source = fixture.m_source;
    auto& client = fixture.m_client;
    SUBCASE("stamp_framing") {
      source.m_messages.push("not a STAMP message");
    }
    SUBCASE("required_field") {
      source.m_messages.push("\x01\x1e" "56=20260920090000000\x1c"
        "\x1e" "6=MBXMessage\x1e" "5=AssignCOP\x1e" "191=25.50"
        "\x1e" "57=20260920090000000");
    }
    SUBCASE("price") {
      source.m_messages.push("\x01\x1e" "56=20260920090000000\x1c"
        "\x1e" "6=MBXMessage\x1e" "5=AssignCOP\x1e" "55=ABX"
        "\x1e" "191=invalid\x1e" "57=20260920090000000");
    }
    source.m_messages.push("\x01\x1e" "56=20260920090000000\x1c"
      "\x1e" "6=FutureMessage");
    flush_pending_routines();
    REQUIRE(client.is_finished());
    REQUIRE(source.get_count() == 1);
    auto exception = client.get_exception();
    REQUIRE(exception);
    REQUIRE_THROWS_AS(std::rethrow_exception(exception), std::runtime_error);
  }

  TEST_CASE("reception") {
    auto fixture = Fixture();
    auto& source = fixture.m_source;
    auto& client = fixture.m_client;
    source.m_messages.push("\x01\x1e" "56=20260920090000000\x1c"
      "\x1e" "6=StockStatus\x1e" "57=20260920090000000");
    source.m_messages.push("\x01\x1e" "56=20260920090000000\x1c"
      "\x1e" "6=MBXMessage\x1e" "5=AssignCOP\x1e" "55=ABX"
      "\x1e" "191=25.50\x1e" "57=20260920090000000");
    source.m_messages.push("\x01\x1e" "56=20260920090000000\x1c"
      "\x1e" "6=FutureMessage");
    flush_pending_routines();
    REQUIRE(source.get_count() == 3);
    REQUIRE_FALSE(client.is_finished());
    REQUIRE_FALSE(client.get_exception());
    client.close();
    REQUIRE(client.is_finished());
    REQUIRE(source.m_is_closed);
    REQUIRE_FALSE(client.get_exception());
  }

  TEST_CASE("opening_quote") {
    auto fixture = Fixture();
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.50"
      "|57=20260921090000000");
    REQUIRE(fixture.quote() == TickerBboQuote(BboQuote(
      make_bid(Money(25.50), 0), make_ask(Money(25.50), 0),
      time_from_string("2026-09-21 13:00:00")), parse_ticker("ABX.TSX")));
    fixture.publish("|6=OpeningAuction|5=PairedVolume|55=ABX|247=TSE"
      "|191=25.50|578=1200|57=20260921090001000");
    REQUIRE(fixture.quote() == TickerBboQuote(BboQuote(
      make_bid(Money(25.50), 1200), make_ask(Money(25.50), 1200),
      time_from_string("2026-09-21 13:00:01")), parse_ticker("ABX.TSX")));
    SUBCASE("buy_imbalance") {
      fixture.publish("|6=OpeningAuction|5=OddlotImbalance|55=ABX|247=TSE"
        "|572=Buyside|573=30|57=20260921090002000");
      auto quote = fixture.quote();
      REQUIRE(quote->m_bid == make_bid(Money(25.50), 1230));
      REQUIRE(quote->m_ask == make_ask(Money(25.50), 1200));
    }
    SUBCASE("sell_imbalance") {
      fixture.publish("|6=OpeningAuction|5=OddlotImbalance|55=ABX|247=TSE"
        "|572=Sellside|573=40|57=20260921090002000");
      auto quote = fixture.quote();
      REQUIRE(quote->m_bid == make_bid(Money(25.50), 1200));
      REQUIRE(quote->m_ask == make_ask(Money(25.50), 1240));
      fixture.publish("|6=OpeningAuction|5=OddlotImbalance|55=ABX|247=TSE"
        "|572=NA|573=0|57=20260921090003000");
      quote = fixture.quote();
      REQUIRE(quote->m_bid.m_size == 1200);
      REQUIRE(quote->m_ask.m_size == 1200);
    }
    SUBCASE("new_price") {
      fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.60"
        "|57=20260921090002000");
      auto quote = fixture.quote();
      REQUIRE(quote->m_bid == make_bid(Money(25.60), 0));
      REQUIRE(quote->m_ask == make_ask(Money(25.60), 0));
    }
    SUBCASE("next_day") {
      fixture.m_time.set(time_from_string("2026-09-22 13:00:00"));
      fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.50"
        "|57=20260922090000000");
      auto quote = fixture.quote();
      REQUIRE(quote->m_bid.m_size == 0);
      REQUIRE(quote->m_ask.m_size == 0);
    }
  }

  TEST_CASE("opening_quantities") {
    auto fixture = Fixture();
    fixture.publish("|6=OpeningAuction|5=PairedVolume|55=ABX|247=TSE"
      "|578=500|57=20260921090000000");
    fixture.require_empty();
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.50"
      "|57=20260921090001000");
    auto quote = fixture.quote();
    REQUIRE(quote->m_bid.m_size == 500);
    REQUIRE(quote->m_ask.m_size == 500);
  }

  TEST_CASE("listing_venue") {
    auto fixture = Fixture({
      TickerInfo(parse_ticker("ABX.TSX"), "", "", 100),
      TickerInfo(parse_ticker("CSE.CSE"), "", "", 100),
      TickerInfo(parse_ticker("NEO.NEOE"), "", "", 100)});
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=CNQ|191=25.50"
      "|57=20260921090000000|654=700");
    fixture.require_empty();
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=CSE|247=CNQ|191=10"
      "|57=20260921090000000|654=700");
    auto quote = fixture.quote();
    REQUIRE(quote.get_index() == parse_ticker("CSE.CSE"));
    REQUIRE(quote->m_bid.m_size == 700);
    REQUIRE(quote->m_ask.m_size == 700);
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=NEO|247=AQL|191=12"
      "|57=20260921090000000|698=900|492=SellSide|493=200");
    quote = fixture.quote();
    REQUIRE(quote.get_index() == parse_ticker("NEO.NEOE"));
    REQUIRE(quote->m_bid.m_size == 900);
    REQUIRE(quote->m_ask.m_size == 1100);
  }

  TEST_CASE("opening_session") {
    auto fixture = Fixture();
    auto timestamp = std::string("20260921093000000");
    SUBCASE("opening") {
      fixture.m_time.set(time_from_string("2026-09-21 13:30:00"));
    }
    SUBCASE("mid_session_start") {
      fixture.m_time.set(time_from_string("2026-09-21 16:00:00"));
    }
    SUBCASE("closed") {
      fixture.m_time.set(time_from_string("2026-09-21 20:00:00"));
    }
    SUBCASE("holiday") {
      fixture.m_time.set(time_from_string("2026-12-25 14:00:00"));
      timestamp = "20261225090000000";
    }
    SUBCASE("weekend") {
      fixture.m_time.set(time_from_string("2026-09-20 13:00:00"));
      timestamp = "20260920090000000";
    }
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.50"
      "|57=" + timestamp);
    fixture.require_empty();
  }

  TEST_CASE("unknown_symbol") {
    auto fixture = Fixture(std::vector<TickerInfo>());
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.50"
      "|57=20260921090000000");
    auto query = fixture.query();
    REQUIRE(query->m_query.get_index() == Scope(Countries::CA));
    REQUIRE(query->m_query.get_snapshot_limit() == SnapshotLimit::UNLIMITED);
    SUBCASE("found") {
      query->m_result.set({TickerInfo(parse_ticker("ABX.TSX"), "", "", 100)});
      REQUIRE(fixture.quote().get_index() == parse_ticker("ABX.TSX"));
      fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.60"
        "|57=20260921090001000");
      REQUIRE(fixture.quote()->m_bid.m_price == Money(25.60));
    }
    SUBCASE("missing") {
      query->m_result.set(std::vector<TickerInfo>());
      fixture.require_empty();
    }
    SUBCASE("failure") {
      query->m_result.set(std::make_exception_ptr(IOException("Unavailable.")));
      fixture.require_empty();
    }
    SUBCASE("ambiguous") {
      query->m_result.set({
        TickerInfo(parse_ticker("ABX.TSX"), "", "", 100),
        TickerInfo(parse_ticker("ABX.TSXV"), "", "", 100)});
      fixture.require_empty();
    }
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=CNQ|191=25.50"
      "|57=20260921090000000");
    fixture.require_empty();
    REQUIRE_FALSE(fixture.m_data_operations->try_pop().has_value());
  }

  TEST_CASE("lookup_shutdown") {
    auto fixture = Fixture(std::vector<TickerInfo>());
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.50"
      "|57=20260921090000000");
    auto query = fixture.query();
    fixture.m_client.close();
    REQUIRE(fixture.m_client.is_finished());
    REQUIRE_FALSE(fixture.m_client.get_exception());
    REQUIRE_FALSE(fixture.m_feed_operations->try_pop().has_value());
  }

  TEST_CASE("cbbo_session") {
    auto fixture = Fixture();
    auto is_publishing = false;
    auto timestamp = time_from_string("2026-09-21 13:00:00");
    SUBCASE("pre_open") {}
    SUBCASE("opening") {
      timestamp = time_from_string("2026-09-21 13:30:00");
      is_publishing = true;
    }
    SUBCASE("mid_session_start") {
      timestamp = time_from_string("2026-09-21 16:00:00");
      is_publishing = true;
    }
    SUBCASE("closing") {
      timestamp = time_from_string("2026-09-21 20:00:00");
    }
    SUBCASE("holiday") {
      timestamp = time_from_string("2026-12-25 15:00:00");
    }
    SUBCASE("weekend") {
      timestamp = time_from_string("2026-09-20 16:00:00");
    }
    SUBCASE("early_close") {
      timestamp = time_from_string("2026-12-24 18:00:00");
    }
    SUBCASE("winter_opening") {
      timestamp = time_from_string("2026-12-24 14:30:00");
      is_publishing = true;
    }
    SUBCASE("winter_pre_open") {
      timestamp = time_from_string("2026-12-24 14:00:00");
    }
    fixture.m_time.set(timestamp);
    fixture.publish_cbbo();
    if(is_publishing) {
      REQUIRE(fixture.quote() == TickerBboQuote(BboQuote(
        make_bid(Money(25.50), 123), make_ask(Money(25.51), 456),
        timestamp), parse_ticker("ABX.TSX")));
    } else {
      fixture.require_empty();
    }
  }

  TEST_CASE("quote_source") {
    auto fixture = Fixture();
    fixture.publish_cbbo();
    fixture.require_empty();
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.50"
      "|57=20260921090000000");
    REQUIRE(fixture.quote()->m_bid.m_price == Money(25.50));
    fixture.m_time.set(time_from_string("2026-09-21 13:30:00"));
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.60"
      "|57=20260921093000000");
    fixture.require_empty();
    fixture.publish_cbbo();
    auto quote = fixture.quote();
    REQUIRE(quote->m_bid == make_bid(Money(25.50), 123));
    REQUIRE(quote->m_ask == make_ask(Money(25.51), 456));
    fixture.m_time.set(time_from_string("2026-09-21 20:00:00"));
    fixture.publish_cbbo();
    fixture.require_empty();
  }

  TEST_CASE("cbbo_timestamp") {
    auto fixture = Fixture();
    fixture.m_time.set(time_from_string("2026-09-21 16:00:00"));
    fixture.publish("|6=Quote|5=Quote|55=ABX|196=25.50|196.1=25.51"
      "|64=123|64.1=456", "|17=FFFFFFFF|54=0123abcd|50=1"
      "|501=20260921115959000|514=20260921115959123");
    REQUIRE(fixture.quote()->m_timestamp ==
      time_from_string("2026-09-21 15:59:59.123"));
    fixture.publish("|6=Quote|5=Quote|55=ABX|196=25.50|196.1=25.51"
      "|64=123|64.1=456", "|17=FFFFFFFF|54=0123abcd|50=1"
      "|501=20260918120000000");
    fixture.require_empty();
  }

  TEST_CASE("empty_cbbo") {
    auto fixture = Fixture();
    fixture.m_time.set(time_from_string("2026-09-21 16:00:00"));
    fixture.publish("|6=Quote|5=Quote|55=ABX|196=0|196.1=0|64=0|64.1=0");
    auto quote = fixture.quote();
    REQUIRE(quote->m_bid == make_bid(Money::ZERO, 0));
    REQUIRE(quote->m_ask == make_ask(Money::ZERO, 0));
  }
}
