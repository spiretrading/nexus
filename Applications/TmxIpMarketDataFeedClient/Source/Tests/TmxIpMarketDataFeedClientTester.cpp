#include <tuple>
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
    struct Message {
      std::string m_payload;
      std::uint64_t m_session;
    };
    Queue<Message> m_messages;
    std::string m_payload;
    std::atomic_int m_count = 0;
    bool m_is_closed = false;

    StampMessage read() {
      auto session = std::uint64_t();
      return read(out(session));
    }

    StampMessage read(Out<std::uint64_t> session) {
      auto message = m_messages.pop();
      m_payload = std::move(message.m_payload);
      *session = message.m_session;
      ++m_count;
      return StampMessage::parse(m_payload);
    }

    void push(std::string payload) {
      push(std::move(payload), 0);
    }

    void push(std::string payload, std::uint64_t session) {
      m_messages.push(Message(std::move(payload), session));
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
      : Fixture(Venues::TSX) {}

    explicit Fixture(Venue venue)
      : Fixture(
          venue, {TickerInfo(parse_ticker("ABX.TSX"), "Barrick", "", 100)}) {}

    explicit Fixture(std::vector<TickerInfo> tickers)
      : Fixture(Venues::TSX, std::move(tickers)) {}

    Fixture(Venue venue, std::vector<TickerInfo> tickers)
      : Fixture(venue, std::move(tickers), {}) {}

    Fixture(Venue venue, std::vector<TickerInfo> tickers,
        std::unordered_map<std::uint64_t, std::string> mappings)
        : m_feed_operations(std::make_shared<FeedClient::Queue>()),
          m_data_operations(std::make_shared<DataClient::Queue>()),
          m_feed(m_feed_operations),
          m_data(m_data_operations),
          m_time(time_from_string("2026-09-21 13:00:00")),
          m_client([&] {
            auto config = TmxIpConfiguration();
            config.m_country = Countries::CA;
            config.m_venue = venue;
            config.m_mpid_mappings = std::move(mappings);
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
      publish(fields, 0);
    }

    void publish(std::string_view fields, std::uint64_t session) {
      publish(fields, "|56=20260921090000000|17=FFFFFFFF|54=0123abcd|50=1",
        session);
    }

    void publish(std::string_view fields, std::string_view control) {
      publish(fields, control, 0);
    }

    void publish(std::string_view fields, std::string_view control,
        std::uint64_t session) {
      auto message = std::string("") + std::string(control) + '' +
        std::string(fields);
      std::ranges::replace(message, '|', '');
      m_source.push(std::move(message), session);
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

    template<typename T>
    std::shared_ptr<T> operation() {
      flush_pending_routines();
      auto operation = m_feed_operations->try_pop();
      REQUIRE(operation.has_value());
      auto actual = std::get_if<T>(&**operation);
      REQUIRE(actual);
      auto result = std::shared_ptr<T>(*operation, actual);
      actual->m_result.set();
      flush_pending_routines();
      REQUIRE_FALSE(m_client.get_exception());
      return result;
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
  TEST_CASE("broker_mpids") {
    auto venue = Venues::TSX;
    auto ticker = parse_ticker("ABX.TSX");
    auto expected = std::string("CIBC");
    auto broker = std::string("|70=079");
    SUBCASE("tsx") {}
    SUBCASE("tsxv") {
      venue = Venues::TSXV;
      ticker = parse_ticker("ABX.TSXV");
    }
    SUBCASE("cse") {
      venue = Venues::CSE;
      ticker = parse_ticker("ABX.CSE");
    }
    SUBCASE("neo") {
      venue = Venues::NEOE;
      ticker = Ticker("ABX", venue);
    }
    SUBCASE("secondary") {
      venue = Venues::XATS;
      expected = VENUES.from(venue).m_display_name;
    }
    SUBCASE("unknown") {
      broker = "|70=049";
      expected = "049";
    }
    SUBCASE("absent") {
      venue = Venues::CSE;
      ticker = parse_ticker("ABX.CSE");
      broker.clear();
      expected = "CSE";
    }
    auto fixture = Fixture(venue, {TickerInfo(ticker, "", "", 100)},
      {{79, "CIBC"}, {1, "ANON"}});
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX"
      "|57=20260921090000000|40=123|196=25.50|64=300" + broker);
    auto order = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(order->m_mpid == expected);
    fixture.publish("|6=TradeReport|5=Trade|55=ABX|41=25.50|64=100"
      "|57=20260921090001000|40=123" + broker);
    auto update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == order->m_id);
    REQUIRE(update->m_mpid == expected);
    REQUIRE(update->m_size == 200);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Cancelled|55=ABX"
      "|57=20260921090002000|40=123|196=25.50|64=200" + broker);
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      order->m_id);
    fixture.require_empty();
  }

  TEST_CASE("broker_changes") {
    auto fixture = Fixture(Venues::NEOE,
      {TickerInfo(parse_ticker("ABX.NEOE"), "", "", 100)},
      {{79, "CIBC"}, {2, "RBCC"}});
    fixture.publish("|6=OrderInfo|5=OrderBook"
      "|57=20260921070000000|55=ABX|40=123|70=79|197=Buy|64=300|196=25.50"
      "|55.1=ABX|40.1=456|70.1=2|197.1=Buy|64.1=200|196.1=25.50"
      "|55.2=ABX|40.2=789|70.2=79|197.2=Buy|64.2=100|196.2=25.50");
    auto order = fixture.operation<FeedClient::AddOrderOperation>();
    auto second = fixture.operation<FeedClient::AddOrderOperation>();
    auto third = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(order->m_mpid == "CIBC");
    REQUIRE(second->m_mpid == "RBCC");
    REQUIRE(third->m_mpid == order->m_mpid);
    REQUIRE(order->m_id != second->m_id);
    REQUIRE(order->m_id != third->m_id);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX"
      "|57=20260921090000000|40=123|70=2|196=25.50|64=300");
    auto update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == order->m_id);
    REQUIRE(update->m_mpid == "RBCC");
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX"
      "|57=20260921090001000|40=123|196=25.50|64=200");
    update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == order->m_id);
    REQUIRE(update->m_mpid == "RBCC");
    REQUIRE(update->m_size == 200);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=PriceAssigned|55=ABX"
      "|57=20260921090002000|40=123|196=25.51|64=0");
    update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == order->m_id);
    REQUIRE(update->m_mpid == "RBCC");
    REQUIRE(update->m_price == Money(25.51));
    fixture.require_empty();
  }

  TEST_CASE("cls_trade") {
    auto exchange = std::string("CHI");
    auto venue = std::string("CHIC");
    auto brokers = std::string("|70=001|70.1=002");
    auto buyer = std::string("ANON");
    auto seller = std::string("RBCC");
    SUBCASE("primary_venue") {
      exchange = "TSE";
      venue = "TSX";
    }
    SUBCASE("secondary_venue") {}
    SUBCASE("unknown_brokers") {
      brokers = "|70=49|70.1=999";
      buyer = "049";
      seller = "999";
    }
    SUBCASE("missing_buyer") {
      brokers = "|70.1=002";
      buyer.clear();
    }
    SUBCASE("missing_seller") {
      brokers = "|70=001";
      seller.clear();
    }
    auto fixture = Fixture(Venue(),
      {TickerInfo(parse_ticker("ABX.TSX"), "Barrick", "", 100)},
      {{1, "ANON"}, {2, "RBCC"}});
    fixture.publish("|6=TradeReport|5=Trade|55=ABX|41=23.10|64=125"
      "|57=20260921100100123|247=" + exchange + brokers);
    auto operation =
      fixture.operation<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(operation->m_time_and_sale == TickerTimeAndSale(TimeAndSale(
      time_from_string("2026-09-21 14:01:00.123"), parse_money("23.10"), 125,
      TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
      venue, buyer, seller), parse_ticker("ABX.TSX")));
    fixture.require_empty();
  }

  TEST_CASE("cls_venues") {
    auto fixture = Fixture(Venue());
    for(auto [exchange, book, venue] : {
        std::tuple("TSE", "", Venues::TSX), {"CDX", "", Venues::TSXV},
        {"ALP", "", Venues::XATS}, {"ALX", "", Venues::ALX},
        {"ALD", "", Venues::ALD}, {"CHI", "", Venues::CHIC},
        {"CHT", "", Venues::XCX2}, {"CHD", "", Venues::CXD},
        {"CNQ", "", Venues::CSE}, {"PUR", "", Venues::PURE},
        {"CS2", "", Venues::CSE2}, {"ICX", "", Venues::ICX},
        {"LIQ", "", Venues::LIQ}, {"LYX", "", Venues::LYNX},
        {"OMG", "", Venues::OMGA}, {"TCM", "", Venues::MATN},
        {"AQL", "", Venues::NEOE}, {"AQN", "", Venues::NEON},
        {"AQL", "AQL", Venues::NEOE}, {"AQL", "AQN", Venues::NEON},
        {"AQL", "AQD", Venues::NEOE}, {"AQL", "AQS", Venues::NEOE},
        {"AQL", "AQC", Venues::NEOE}}) {
      CAPTURE(exchange);
      CAPTURE(book);
      auto fields = std::string(
        "|6=TradeReport|5=Trade|55=ABX|41=10|64=1"
        "|57=20260921100000000|247=") + exchange;
      if(!std::string_view(book).empty()) {
        fields += std::string("|636=") + book;
      }
      fixture.publish(fields);
      auto operation =
        fixture.operation<FeedClient::PublishTimeAndSaleOperation>();
      auto& trade = operation->m_time_and_sale;
      REQUIRE(trade.get_index() == parse_ticker("ABX.TSX"));
      REQUIRE(trade->m_market_center == VENUES.from(venue).m_display_name);
      REQUIRE(trade->m_buyer_mpid.empty());
      REQUIRE(trade->m_seller_mpid.empty());
    }
    for(auto fields : {"", "|247=BAD", "|247=AQL|636=BAD"}) {
      fixture.publish(std::string(
        "|6=TradeReport|5=Trade|55=ABX|41=10|64=1"
        "|57=20260921100000000") + fields);
      fixture.require_empty();
    }
  }

  TEST_CASE("cls_conditions") {
    using Type = TimeAndSale::Condition::Type;
    auto fixture = Fixture(Venue());
    for(auto [fields, type, code] : {
        std::tuple("", Type::REGULAR, "@"),
        {"|574=O", Type::OPEN, "O"}, {"|574=R", Type::REOPEN, "R"},
        {"|494=Y", Type::CLOSE, "C"},
        {"|390=Regular", Type::REGULAR, "@"},
        {"|390=Basis", Type::NONE, "BA"},
        {"|390=Contgt", Type::NONE, "CG"},
        {"|390=Intrnl", Type::NONE, "I"},
        {"|390=NAV", Type::NONE, "NV"},
        {"|390=STS", Type::NONE, "ST"},
        {"|390=VWAP", Type::NONE, "V"},
        {"|390=NC", Type::NONE, "NC"},
        {"|390=Intentional", Type::NONE, "IC"},
        {"|390=Derivative", Type::NONE, "DR"},
        {"|390=CCP-Closing Price", Type::NONE, "CP"},
        {"|390=CPP", Type::NONE, "PP"},
        {"|390=Unknown", Type::NONE, "CX"},
        {"|53=Cash", Type::NONE, "CA"},
        {"|53=CT", Type::NONE, "CT"},
        {"|53=MS", Type::NONE, "MS"},
        {"|53=NN", Type::NONE, "NN"},
        {"|53=Future", Type::NONE, "F"},
        {"|53=ND", Type::NONE, "ND"},
        {"|53=20260924", Type::NONE, "DD"},
        {"|53=Unknown", Type::NONE, "S"},
        {"|76=Y", Type::NONE, "E"},
        {"|503=Y", Type::NONE, "B"},
        {"|168=Y", Type::NONE, "N"},
        {"|617=Y", Type::NONE, "D"},
        {"|684=Y", Type::NONE, "M"},
        {"|688=Y", Type::NONE, "CO"},
        {"|689=L", Type::NONE, "L"},
        {"|703=P", Type::NONE, "P"},
        {"|76=N|503=N|168=N|617=N|684=N|688=N", Type::REGULAR, "@"},
        {"|76=Y|503=Y|168=Y", Type::NONE, "E;B;N"},
        {"|617=Y|684=Y|688=Y|689=L|703=P", Type::NONE,
          "D;M;CO;L;P"},
        {"|574=O|390=Intrnl", Type::OPEN, "O;I"},
        {"|574=R|390=VWAP", Type::REOPEN, "R;V"}}) {
      CAPTURE(fields);
      fixture.publish(std::string(
        "|6=TradeReport|5=Trade|55=ABX|41=10|64=37"
        "|57=20260921100000000|247=TSE") + fields);
      auto operation =
        fixture.operation<FeedClient::PublishTimeAndSaleOperation>();
      REQUIRE(operation->m_time_and_sale->m_condition ==
        TimeAndSale::Condition(type, code));
      REQUIRE(operation->m_time_and_sale->m_size == 37);
    }
    fixture.require_empty();
  }

  TEST_CASE("cls_corrections") {
    using Type = TimeAndSale::Condition::Type;
    auto fixture = Fixture(Venue());
    for(auto [fields, type, code] : {
        std::tuple("|5=Cancelled", Type::CANCELLATION, "X"),
        {"|5=Cancelled|183=Y|506=ORIGINAL|574=O", Type::CANCELLATION, "X"},
        {"|5=Cancelled|76=Y", Type::CANCELLATION, "X;E"},
        {"|5=Trade|183=Y|506=ORIGINAL", Type::CORRECTION, "U"},
        {"|5=Trade|506=ORIGINAL", Type::CORRECTION, "U"},
        {"|5=Trade|183=Y|494=Y", Type::CORRECTION, "U"},
        {"|5=Trade|183=Y|390=VWAP", Type::CORRECTION, "U;V"},
        {"|5=AuctionTradeIndividual|183=Y|220=TRADE", Type::CORRECTION, "U"},
        {"|5=AuctionTradeIndividual|183=Y|220=TRADE", Type::CORRECTION, "U"}}) {
      CAPTURE(fields);
      fixture.publish(std::string(
        "|6=TradeReport|55=ABX|41=10|64=100|57=20260921100000000"
        "|247=AQL") + fields);
      auto operation =
        fixture.operation<FeedClient::PublishTimeAndSaleOperation>();
      REQUIRE(operation->m_time_and_sale->m_condition ==
        TimeAndSale::Condition(type, code));
      REQUIRE(operation->m_time_and_sale->m_price == parse_money("10"));
      REQUIRE(operation->m_time_and_sale->m_size == 100);
      fixture.require_empty();
    }
    for(auto fields : {"|264=20260921093000123",
        "|264.1=20260921093000123",
        "|264=20260921093000123|264.1=20260921093000123"}) {
      fixture.publish(std::string(
        "|6=TradeReport|5=Trade|55=ABX|41=10|64=100|183=Y"
        "|57=20260921100000000|247=TSE") + fields);
      auto operation =
        fixture.operation<FeedClient::PublishTimeAndSaleOperation>();
      REQUIRE(operation->m_time_and_sale->m_timestamp ==
        time_from_string("2026-09-21 13:30:00.123"));
      REQUIRE(operation->m_time_and_sale->m_size == 100);
      REQUIRE(operation->m_time_and_sale->m_condition ==
        TimeAndSale::Condition(Type::CORRECTION, "U"));
    }
    fixture.publish("|6=TradeReport|5=Trade|55=ABX|41=10|64=100|183=Y"
      "|57=20261221100000000|247=TSE");
    auto operation =
      fixture.operation<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(operation->m_time_and_sale->m_timestamp ==
      time_from_string("2026-12-21 15:00:00"));
    fixture.require_empty();
  }

  TEST_CASE("cls_auction") {
    auto fixture = Fixture(Venue(), {
      TickerInfo(parse_ticker("ABX.TSX"), "Barrick", "", 100),
      TickerInfo(parse_ticker("FOO.CSE"), "Foo", "", 100)});
    auto next_date = std::string("20260922");
    auto next_session = std::uint64_t(0);
    SUBCASE("next_day") {}
    SUBCASE("source_session") {
      next_date = "20260921";
      next_session = 1;
    }
    for(auto& [date, session] : {
        std::pair(std::string("20260921"), std::uint64_t(0)),
        {next_date, next_session}}) {
      for(auto symbol : {"ABX", "FOO"}) {
        auto fields = std::string(
          "|6=TradeReport|5=AuctionTradeIndividual|55=") + symbol +
          "|41=10|64=100|247=AQL|636=AQL|220=TRADE|57=" + date +
          "093000000";
        fixture.publish(fields, session);
        auto operation =
          fixture.operation<FeedClient::PublishTimeAndSaleOperation>();
        REQUIRE(operation->m_time_and_sale->m_size == 100);
        REQUIRE(operation->m_time_and_sale->m_condition ==
          TimeAndSale::Condition(TimeAndSale::Condition::Type::AUCTION, "A"));
        fixture.publish(fields, session);
        fixture.require_empty();
      }
    }
    fixture.publish("|6=TradeReport|5=AuctionTradeIndividual|55=ABX"
      "|41=10|64=100|247=AQL|636=AQL|57=" + next_date + "093000000",
      next_session);
    fixture.require_empty();
    fixture.publish("|6=TradeReport|5=Trade|55=ABX|41=10|64=100"
      "|247=AQL|220=TRADE|57=" + next_date + "093100000", next_session);
    auto operation =
      fixture.operation<FeedClient::PublishTimeAndSaleOperation>();
    REQUIRE(operation->m_time_and_sale->m_condition.m_code == "@");
    fixture.require_empty();
  }

  TEST_CASE("cls_ticker") {
    auto fixture = Fixture(Venue(), {});
    fixture.publish("|6=TradeReport|5=Trade|55=ABX|41=10|64=1"
      "|57=20260921100000000|247=CHI");
    auto query = fixture.query();
    auto tickers = std::vector<TickerInfo>();
    SUBCASE("found") {
      tickers.emplace_back(parse_ticker("ABX.TSX"), "Barrick", "", 100);
    }
    SUBCASE("missing") {}
    query->m_result.set(tickers);
    for(auto i = 0; i != 2; ++i) {
      if(!tickers.empty()) {
        auto operation =
          fixture.operation<FeedClient::PublishTimeAndSaleOperation>();
        REQUIRE(operation->m_time_and_sale.get_index() ==
          parse_ticker("ABX.TSX"));
      }
      fixture.require_empty();
      REQUIRE_FALSE(fixture.m_data_operations->try_pop().has_value());
      if(i == 0) {
        fixture.publish("|6=TradeReport|5=Trade|55=ABX|41=10|64=1"
          "|57=20260921100000000|247=CHI");
      }
    }
  }

  TEST_CASE("configured_venue") {
    auto fixture = Fixture();
    auto exchange = std::string();
    SUBCASE("absent_exchange") {}
    SUBCASE("different_exchange") {
      exchange = "|247=CNQ";
    }
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX"
      "|57=20260921090000000|40=123|70=79|196=25.50|64=300" + exchange);
    auto order = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(order->m_ticker == parse_ticker("ABX.TSX"));
    REQUIRE(order->m_venue == Venues::TSX);
    REQUIRE(order->m_mpid == "079");
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Cancelled|55=ABX"
      "|57=20260921090001000|40=123|70=79|196=25.50|64=300");
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      order->m_id);
    fixture.require_empty();
  }

  TEST_CASE("consolidated_service") {
    auto fixture = Fixture(Venue());
    fixture.m_time.set(time_from_string("2026-09-21 16:00:00"));
    fixture.publish_cbbo();
    REQUIRE(fixture.quote().get_index() == parse_ticker("ABX.TSX"));
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=TSE"
      "|57=20260921120000000|40=123|70=79|196=25.50|64=300");
    fixture.require_empty();
  }

  TEST_CASE("cse_venue") {
    auto fixture = Fixture(Venues::CSE, {
      TickerInfo(parse_ticker("LISTED.CSE"), "", "", 100),
      TickerInfo(parse_ticker("ABX.TSX"), "", "", 100)});
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=LISTED"
      "|57=20260921090000000|40=123|196=10|64=300|247=TSE");
    auto listed = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(listed->m_ticker == parse_ticker("LISTED.CSE"));
    REQUIRE(listed->m_venue == Venues::CSE);
    REQUIRE(listed->m_mpid == "CSE");
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX"
      "|57=20260921090000000|40=123|196=25.50|64=200");
    auto other = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(other->m_ticker == parse_ticker("ABX.TSX"));
    REQUIRE(other->m_venue == Venues::PURE);
    REQUIRE(other->m_mpid == "PURE");
    REQUIRE(other->m_id != listed->m_id);
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=LISTED|191=10"
      "|57=20260921090001000|654=500");
    auto quote = fixture.quote();
    REQUIRE(quote.get_index() == listed->m_ticker);
    REQUIRE(quote->m_bid == make_bid(Money(10), 500));
    REQUIRE(quote->m_ask == make_ask(Money(10), 500));
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|191=25.50"
      "|57=20260921090001000|654=500");
    fixture.require_empty();
    fixture.publish("|6=ClearOrderInfo|5=ClearOrderBook|55=ABX"
      "|57=20260921090002000");
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      other->m_id);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Cancelled|55=LISTED"
      "|57=20260921090003000|40=123|196=10|64=300");
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      listed->m_id);
    fixture.require_empty();
  }

  TEST_CASE("book_updates") {
    auto fixture = Fixture();
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=TSE"
      "|57=20260921090000000|40=123|70=79|196=25.50|64=350");
    auto order = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(order->m_ticker == parse_ticker("ABX.TSX"));
    REQUIRE(order->m_venue == Venues::TSX);
    REQUIRE(order->m_mpid == "079");
    REQUIRE_FALSE(order->m_is_primary_mpid);
    REQUIRE(order->m_side == Side::BID);
    REQUIRE(order->m_price == Money(25.50));
    REQUIRE(order->m_size == 300);
    REQUIRE(order->m_timestamp == time_from_string("2026-09-21 13:00:00"));
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=TSE"
      "|57=20260921090001000|40=123|70=79|196=25.51|64=200");
    auto update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == order->m_id);
    REQUIRE(update->m_price == Money(25.51));
    REQUIRE(update->m_size == 200);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Cancelled|55=ABX|247=TSE"
      "|57=20260921090002000|40=123|70=79|196=25.51|64=200");
    auto removal = fixture.operation<FeedClient::RemoveOrderOperation>();
    REQUIRE(removal->m_id == order->m_id);
    REQUIRE(removal->m_timestamp == time_from_string("2026-09-21 13:00:02"));
    fixture.require_empty();
  }

  TEST_CASE("book_venues") {
    auto venues = {Venues::TSX, Venues::TSXV, Venues::XATS, Venues::ALX,
      Venues::CHIC, Venues::XCX2, Venues::CSE, Venues::PURE, Venues::CSE2,
      Venues::OMGA, Venues::LYNX, Venues::NEOE, Venues::NEON};
    auto ids = std::vector<std::string>();
    for(auto venue : venues) {
      CAPTURE(venue);
      auto ticker = parse_ticker("ABX.TSX");
      if(venue == Venues::CSE) {
        ticker = parse_ticker("ABX.CSE");
      }
      auto fixture = Fixture(venue, {TickerInfo(ticker, "", "", 100)});
      fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX"
        "|57=20260921090000000|40=123|70=79|196=25.50|64=300");
      auto order = fixture.operation<FeedClient::AddOrderOperation>();
      REQUIRE(order->m_ticker == ticker);
      REQUIRE(order->m_venue == venue);
      if(venue == ticker.get_venue()) {
        REQUIRE(order->m_mpid == "079");
      } else {
        REQUIRE(order->m_mpid == VENUES.from(venue).m_display_name);
      }
      REQUIRE(std::ranges::find(ids, order->m_id) == ids.end());
      ids.push_back(order->m_id);
      fixture.require_empty();
    }
  }

  TEST_CASE("partial_cancellation") {
    for(auto source : std::array<std::string_view, 4>{
        "CHI", "CHT", "OMG", "LYX"}) {
      CAPTURE(source);
      auto fixture = Fixture(from_market_center(source).m_venue);
      fixture.publish("|6=OrderCancelResp|5=Sell|16=Booked|55=ABX|247=" +
        std::string(source) + "|57=20260921090000000|40=123"
        "|196=25.50|64=550");
      auto order = fixture.operation<FeedClient::AddOrderOperation>();
      REQUIRE(order->m_size == 500);
      fixture.publish("|6=OrderCancelResp|5=Sell|16=Cancelled|55=ABX|247=" +
        std::string(source) + "|57=20260921090001000|40=123"
        "|196=25.50|64=100");
      auto update = fixture.operation<FeedClient::AddOrderOperation>();
      REQUIRE(update->m_id == order->m_id);
      REQUIRE(update->m_side == Side::ASK);
      REQUIRE(update->m_price == order->m_price);
      if(source == "OMG" || source == "LYX") {
        REQUIRE(update->m_size == 400);
      } else {
        REQUIRE(update->m_size == 100);
      }
      fixture.publish("|6=OrderCancelResp|5=Sell|16=Cancelled|55=ABX|247=" +
        std::string(source) + "|57=20260921090002000|40=123"
        "|196=25.50|64=0");
      REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
        order->m_id);
      fixture.require_empty();
    }
  }

  TEST_CASE("order_replacement") {
    auto fixture = Fixture(Venues::OMGA);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=OMG"
      "|57=20260921090000000|40=OLD|196=25.50|64=400");
    auto order = fixture.operation<FeedClient::AddOrderOperation>();
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=OMG"
      "|57=20260921090001000|40=NEW|11=OLD|196=25.51|64=600");
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      order->m_id);
    auto replacement = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(replacement->m_id != order->m_id);
    REQUIRE(replacement->m_price == Money(25.51));
    REQUIRE(replacement->m_size == 600);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=OMG"
      "|57=20260921090002000|40=NEW|11=NEW|196=25.52|64=500");
    auto update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == replacement->m_id);
    REQUIRE(update->m_price == Money(25.52));
    REQUIRE(update->m_size == 500);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=PriceAssigned|55=ABX"
      "|247=OMG|57=20260921090003000|40=NEW|196=25.53|64=0");
    update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == replacement->m_id);
    REQUIRE(update->m_price == Money(25.53));
    REQUIRE(update->m_size == 500);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=AssignTimePriority|55=ABX"
      "|247=OMG|57=20260921090004000|40=NEW|196=25.53|64=500");
    fixture.require_empty();
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Killed|55=ABX|247=OMG"
      "|57=20260921090005000|40=NEW|196=25.53|64=500");
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      replacement->m_id);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Cancelled|55=ABX|247=OMG"
      "|57=20260921090006000|40=OLD|196=25.50|64=0");
    fixture.require_empty();
  }

  TEST_CASE("initial_book") {
    auto fixture = Fixture({
      TickerInfo(parse_ticker("ABX.TSX"), "", "", 100),
      TickerInfo(parse_ticker("BBD.TSX"), "", "", 100)});
    fixture.publish("|6=OrderInfo|5=OrderBook|247=TSE"
      "|57=20260921070000000|55=ABX|40=123|70=1|197=Buy|64=300|196=25.50"
      "|55.1=ABX|40.1=123|70.1=2|197.1=Buy|64.1=200|41.1=25.51"
      "|55.2=ABX|40.2=123|70.2=1|197.2=Sell|64.2=100|196.2=25.52"
      "|55.3=BBD|40.3=123|70.3=1|197.3=Buy|64.3=400|196.3=30|113=Y");
    auto ids = std::vector<std::string>();
    for(auto price : {Money(25.50), Money(25.51), Money(25.52), Money(30)}) {
      auto order = fixture.operation<FeedClient::AddOrderOperation>();
      REQUIRE(order->m_price == price);
      REQUIRE(order->m_timestamp == time_from_string("2026-09-21 11:00:00"));
      REQUIRE(std::ranges::find(ids, order->m_id) == ids.end());
      ids.push_back(order->m_id);
    }
    auto alpha_fixture = Fixture(Venues::XATS);
    alpha_fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=ALP"
      "|57=20260921090000000|40=123|70=1|196=25.50|64=500");
    auto alpha = alpha_fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(std::ranges::find(ids, alpha->m_id) == ids.end());
    fixture.publish("|6=ClearOrderInfo|5=ClearOrderBook|55=ABX|247=TSE"
      "|57=20260921090001000");
    auto removals = std::vector<std::string>();
    for(auto i = 0; i != 3; ++i) {
      auto removal = fixture.operation<FeedClient::RemoveOrderOperation>();
      REQUIRE(removal->m_timestamp ==
        time_from_string("2026-09-21 13:00:01"));
      removals.push_back(removal->m_id);
    }
    auto expected = std::vector(ids.begin(), ids.begin() + 3);
    std::ranges::sort(expected);
    std::ranges::sort(removals);
    REQUIRE(removals == expected);
    fixture.publish("|6=ClearOrderInfo|5=ClearOrderBook|55=ABX|247=TSE"
      "|57=20260921090002000");
    fixture.require_empty();
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Cancelled|55=BBD|247=TSE"
      "|57=20260921090003000|40=123|70=1|196=30|64=400");
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      ids.back());
    alpha_fixture.publish("|6=OrderCancelResp|5=Buy|16=Cancelled|55=ABX|247=ALP"
      "|57=20260921090004000|40=123|70=1|196=25.50|64=500");
    REQUIRE(alpha_fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      alpha->m_id);
    alpha_fixture.require_empty();
    fixture.require_empty();
  }

  TEST_CASE("source_session") {
    auto fixture = Fixture(Venues::NEOE, {
      TickerInfo(parse_ticker("ABX.TSX"), "", "", 100),
      TickerInfo(parse_ticker("BBD.TSX"), "", "", 100)});
    auto bid = std::string(
      "|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=AQL"
      "|57=20260921090000000|40=123|196=25.50|64=600");
    fixture.publish(bid, 0);
    fixture.publish("|6=OrderCancelResp|5=Sell|16=Booked|55=ABX|247=AQL"
      "|57=20260921090000000|40=123|196=25.51|64=400", 0);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=BBD|247=AQL"
      "|57=20260921090000000|40=123|196=30|64=200", 0);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=BBD|247=AQL"
      "|57=20260921090000000|40=MARKET|196=0|64=500", 0);
    fixture.publish(bid, 0);
    fixture.publish(bid, 1);
    auto ids = std::vector<std::string>();
    for(auto i = 0; i != 3; ++i) {
      auto order = fixture.operation<FeedClient::AddOrderOperation>();
      ids.push_back(order->m_id);
    }
    auto removals = std::vector<std::string>();
    for(auto i = 0; i != 3; ++i) {
      flush_pending_routines();
      auto operation = fixture.m_feed_operations->try_pop();
      REQUIRE(operation.has_value());
      std::visit([] (auto& operation) { operation.m_result.set(); },
        **operation);
      auto removal = std::get_if<FeedClient::RemoveOrderOperation>(
        &**operation);
      REQUIRE(removal);
      REQUIRE(removal->m_timestamp == fixture.m_time.get_time());
      removals.push_back(removal->m_id);
    }
    auto id = ids.front();
    std::ranges::sort(ids);
    std::ranges::sort(removals);
    REQUIRE(removals == ids);
    auto order = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(order->m_id == id);
    REQUIRE(order->m_size == 600);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=PriceAssigned|55=BBD"
      "|247=AQL|57=20260921090001000|40=123|196=31|64=0", 1);
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=BBD|247=AQL|191=32"
      "|57=20260921090002000", 1);
    fixture.publish(bid, 1);
    fixture.require_empty();
    fixture.publish(bid, 2);
    flush_pending_routines();
    auto operation = fixture.m_feed_operations->try_pop();
    REQUIRE(operation.has_value());
    std::visit([] (auto& operation) { operation.m_result.set(); }, **operation);
    auto removal = std::get_if<FeedClient::RemoveOrderOperation>(&**operation);
    REQUIRE(removal);
    REQUIRE(removal->m_id == id);
    order = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(order->m_id == id);
    REQUIRE(order->m_size == 600);
    fixture.require_empty();
  }

  TEST_CASE("auction_sides") {
    auto fixture = Fixture(Venues::NEOE);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=AQL"
      "|57=20260921090000000|40=123|70=79|196=25.50|64=600");
    auto bid = fixture.operation<FeedClient::AddOrderOperation>();
    fixture.publish("|6=OrderCancelResp|5=Sell|16=Booked|55=ABX|247=AQL"
      "|57=20260921090000000|40=123|70=79|196=25.51|64=500");
    auto ask = fixture.operation<FeedClient::AddOrderOperation>();
    auto reports = std::vector{
      std::tuple("|40=123|70=79", bid->m_id, 500),
      std::tuple("|40.1=123|70.1=79", ask->m_id, 400)};
    SUBCASE("buyer_first") {}
    SUBCASE("seller_first") {
      std::ranges::reverse(reports);
    }
    for(auto& [fields, id, quantity] : reports) {
      fixture.publish(std::string(
        "|6=TradeReport|5=AuctionTradeIndividual|55=ABX|247=AQL"
        "|57=20260921090001000|220=AUCTION|41=25.50|64=100") + fields);
      auto update = fixture.operation<FeedClient::AddOrderOperation>();
      REQUIRE(update->m_id == id);
      REQUIRE(update->m_size == quantity);
      fixture.require_empty();
    }
  }

  TEST_CASE("book_trades") {
    auto source = std::string("TSE");
    auto action = std::string("Trade");
    auto display = std::string("|150=800|150.1=0");
    SUBCASE("display_quantity") {}
    SUBCASE("trade_quantity") {
      source = "CHI";
      display.clear();
    }
    SUBCASE("auction_trade") {
      source = "AQL";
      action = "AuctionTradeIndividual";
      display.clear();
    }
    auto fixture = Fixture(from_market_center(source).m_venue);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=" +
      source + "|57=20260921090000000|40=123|70=79|196=25.50|64=600");
    auto bid = fixture.operation<FeedClient::AddOrderOperation>();
    fixture.publish("|6=OrderCancelResp|5=Sell|16=Booked|55=ABX|247=" +
      source + "|57=20260921090000000|40=123|70=79|196=25.51|64=500");
    auto ask = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(bid->m_id != ask->m_id);
    auto trade = "|6=TradeReport|55=ABX|247=" + source +
      "|57=20260921090001000|40=123|70=79|40.1=123|70.1=79"
      "|41=25.50|64=100";
    fixture.publish(trade + "|5=Cancelled");
    fixture.publish(trade + "|5=Trade|183=Y");
    fixture.require_empty();
    for(auto correction : {"", "|183=N"}) {
      CAPTURE(correction);
      fixture.publish(trade + "|5=" + action + display +
        "|506=ORIGINAL" + correction);
      fixture.require_empty();
    }
    fixture.publish(trade + "|5=" + action + display);
    auto update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == bid->m_id);
    REQUIRE(update->m_price == bid->m_price);
    REQUIRE(update->m_timestamp == time_from_string("2026-09-21 13:00:01"));
    if(display.empty()) {
      REQUIRE(update->m_size == 500);
      update = fixture.operation<FeedClient::AddOrderOperation>();
      REQUIRE(update->m_id == ask->m_id);
      REQUIRE(update->m_price == ask->m_price);
      REQUIRE(update->m_size == 400);
    } else {
      REQUIRE(update->m_size == 800);
      REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
        ask->m_id);
    }
    fixture.require_empty();
    fixture.publish("|6=TradeReport|5=Trade|55=ABX|247=" + source +
      "|57=20260921090002000|40=123|70=79|41=25.50|64=800|150=0");
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      bid->m_id);
    fixture.publish("|6=TradeReport|5=Trade|55=ABX|247=" + source +
      "|57=20260921090003000|40=UNKNOWN|70=79|41=25.50|64=100");
    fixture.require_empty();
  }

  TEST_CASE("price_levels") {
    auto fixture = Fixture(Venues::NEON);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=AQN"
      "|636=AQN|57=20260921090000000|196=25.50|64=600");
    auto bid = fixture.operation<FeedClient::AddOrderOperation>();
    fixture.publish("|6=OrderCancelResp|5=Sell|16=Booked|55=ABX|247=AQN"
      "|636=AQN|57=20260921090000000|196=25.50|64=500");
    auto ask = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(bid->m_id != ask->m_id);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=AQN"
      "|636=AQN|57=20260921090001000|196=25.50000|64=700");
    auto update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == bid->m_id);
    REQUIRE(update->m_size == 700);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=AQN"
      "|636=AQN|57=20260921090002000|642=25.50|196=25.51|64=400");
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      bid->m_id);
    update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id != bid->m_id);
    REQUIRE(update->m_price == Money(25.51));
    REQUIRE(update->m_size == 400);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Cancelled|55=ABX|247=AQN"
      "|636=AQN|57=20260921090003000|196=25.51|64=400");
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      update->m_id);
    fixture.publish("|6=ClearOrderInfo|5=ClearOrderBook|55=ABX|247=AQN"
      "|636=AQN|57=20260921090004000");
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      ask->m_id);
    fixture.require_empty();
  }

  TEST_CASE("price_level_trades") {
    auto fixture = Fixture(Venues::NEON);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=AQN"
      "|636=AQN|57=20260921090000000|196=25.50|64=600");
    auto bid = fixture.operation<FeedClient::AddOrderOperation>();
    fixture.publish("|6=TradeReport|5=Trade|55=ABX|247=AQN"
      "|57=20260921090001000|41=25.50|64=200");
    auto update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == bid->m_id);
    REQUIRE(update->m_size == 400);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=AQN"
      "|636=AQN|57=20260921090002000|196=25.50|64=300");
    update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == bid->m_id);
    REQUIRE(update->m_size == 300);
    fixture.publish("|6=TradeReport|5=Trade|55=ABX|247=AQN"
      "|57=20260921090003000|41=25.50|64=300");
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      bid->m_id);
    fixture.require_empty();
  }

  TEST_CASE("order_repricing") {
    auto fixture = Fixture();
    fixture.m_time.set(time_from_string("2026-09-21 16:00:00"));
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=TSE"
      "|57=20260921090000000|40=123|70=79|196=OPG|64=500");
    fixture.require_empty();
    fixture.publish("|6=OrderCancelResp|5=Sell|16=Booked|55=ABX|247=TSE"
      "|57=20260921090000000|40=124|70=79|196=26|64=200");
    auto ask = fixture.operation<FeedClient::AddOrderOperation>();
    fixture.m_source.push("\x01\x1e" "56=20260921090001000\x1c"
      "\x1e" "6=MBXMessage\x1e" "5=AssignCOP\x1e" "55=ABX"
      "\x1e" "247=TSE\x1e" "57=20260921090001000\x1e" "191=25.50"
      "\x1e" "192=079|123\x1e" "192.1=079|124");
    auto bid = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(bid->m_side == Side::BID);
    REQUIRE(bid->m_price == Money(25.50));
    REQUIRE(bid->m_size == 500);
    auto update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == ask->m_id);
    REQUIRE(update->m_price == Money(25.50));
    REQUIRE(update->m_size == 200);
    fixture.m_source.push("\x01\x1e" "56=20260921090002000\x1c"
      "\x1e" "6=MBXMessage\x1e" "5=AssignLimit\x1e" "55=ABX"
      "\x1e" "247=TSE\x1e" "57=20260921090002000\x1e" "191=25.50"
      "\x1e" "192=079|123\x1e" "41=25\x1e" "192.1=079|124"
      "\x1e" "41.1=26");
    update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == bid->m_id);
    REQUIRE(update->m_price == Money(25));
    REQUIRE(update->m_size == 500);
    update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == ask->m_id);
    REQUIRE(update->m_price == Money(26));
    REQUIRE(update->m_size == 200);
    fixture.require_empty();
  }

  TEST_CASE("market_order_repricing") {
    auto fixture = Fixture(Venues::NEOE);
    fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=AQL"
      "|57=20260921090000000|40=MARKET|196=0|64=500");
    fixture.require_empty();
    fixture.publish("|6=OrderCancelResp|5=Sell|16=Booked|55=ABX|247=AQL"
      "|57=20260921090000000|40=LIMIT|196=26|64=200");
    auto limit = fixture.operation<FeedClient::AddOrderOperation>();
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=AQL|191=25.50"
      "|57=20260921090001000");
    auto market = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(market->m_id != limit->m_id);
    REQUIRE(market->m_price == Money(25.50));
    REQUIRE(market->m_size == 500);
    fixture.require_empty();
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=AQL|191=25.60"
      "|57=20260921090002000");
    auto update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == market->m_id);
    REQUIRE(update->m_price == Money(25.60));
    fixture.require_empty();
    fixture.publish("|6=OrderCancelResp|5=Buy|16=PriceAssigned|55=ABX"
      "|247=AQL|57=20260921090003000|40=MARKET|196=0|64=500");
    REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
      market->m_id);
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=AQL|191=25.70"
      "|57=20260921090004000");
    update = fixture.operation<FeedClient::AddOrderOperation>();
    REQUIRE(update->m_id == market->m_id);
    REQUIRE(update->m_price == Money(25.70));
    REQUIRE(update->m_size == 500);
    fixture.require_empty();
  }

  TEST_CASE("regular_book") {
    auto price = std::string("25.50");
    auto quantity = std::string("300");
    auto terms = std::string();
    SUBCASE("odd_lot") {
      quantity = "99";
    }
    SUBCASE("market_order") {
      price = "MKT";
    }
    SUBCASE("zero_price") {
      price = "0";
    }
    SUBCASE("nonresident") {
      terms = "|168=Y";
    }
    SUBCASE("settlement_terms") {
      terms = "|53=Cash";
    }
    SUBCASE("other_book") {
      terms = "|636=AQD";
    }
    for(auto is_initial : {false, true}) {
      CAPTURE(is_initial);
      auto fixture = Fixture(Venues::NEOE);
      auto fields = std::string();
      if(is_initial) {
        fields = "|6=OrderInfo|5=OrderBook|197=Buy";
      } else {
        fields = "|6=OrderCancelResp|5=Buy|16=Booked";
      }
      fixture.publish(fields + "|55=ABX|247=AQL|57=20260921090000000"
        "|40=123|70=79|196=" + price + "|64=" + quantity + terms);
      fixture.require_empty();
      fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=AQL"
        "|57=20260921090001000|40=123|196=25.50|64=300");
      auto order = fixture.operation<FeedClient::AddOrderOperation>();
      REQUIRE(order->m_size == 300);
      fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=AQL"
        "|57=20260921090002000|40=123|196=25.50|64=350");
      fixture.require_empty();
      fixture.publish("|6=OrderCancelResp|5=Buy|16=Booked|55=ABX|247=AQL"
        "|57=20260921090003000|40=123|196=25.50|64=99");
      REQUIRE(fixture.operation<FeedClient::RemoveOrderOperation>()->m_id ==
        order->m_id);
      fixture.require_empty();
    }
  }

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
      source.push("not a STAMP message");
    }
    SUBCASE("required_field") {
      source.push("\x01\x1e" "56=20260920090000000\x1c"
        "\x1e" "6=MBXMessage\x1e" "5=AssignCOP\x1e" "191=25.50"
        "\x1e" "57=20260920090000000");
    }
    SUBCASE("price") {
      source.push("\x01\x1e" "56=20260920090000000\x1c"
        "\x1e" "6=MBXMessage\x1e" "5=AssignCOP\x1e" "55=ABX"
        "\x1e" "191=invalid\x1e" "57=20260920090000000");
    }
    source.push("\x01\x1e" "56=20260920090000000\x1c"
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
    source.push("\x01\x1e" "56=20260920090000000\x1c"
      "\x1e" "6=StockStatus\x1e" "57=20260920090000000");
    source.push("\x01\x1e" "56=20260920090000000\x1c"
      "\x1e" "6=MBXMessage\x1e" "5=AssignCOP\x1e" "55=ABX"
      "\x1e" "191=25.50\x1e" "57=20260920090000000");
    source.push("\x01\x1e" "56=20260920090000000\x1c"
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
        "|572=BuySide|573=30|57=20260921090002000");
      auto quote = fixture.quote();
      REQUIRE(quote->m_bid == make_bid(Money(25.50), 1230));
      REQUIRE(quote->m_ask == make_ask(Money(25.50), 1200));
    }
    SUBCASE("sell_imbalance") {
      fixture.publish("|6=OpeningAuction|5=OddlotImbalance|55=ABX|247=TSE"
        "|572=SellSide|573=40|57=20260921090002000");
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
      fixture.publish("|6=OpeningAuction|5=OddlotImbalance|55=ABX|247=TSE"
        "|572=SellSide|573=40|57=20260921090002000");
      fixture.quote();
      SUBCASE("cop") {
        fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.60"
          "|57=20260921090003000");
      }
      SUBCASE("paired_volume") {
        fixture.publish("|6=OpeningAuction|5=PairedVolume|55=ABX|247=TSE"
          "|191=25.60|578=1200|57=20260921090003000");
      }
      SUBCASE("imbalance") {
        fixture.publish("|6=OpeningAuction|5=OddlotImbalance|55=ABX|247=TSE"
          "|191=25.60|572=SellSide|573=40|57=20260921090003000");
      }
      auto quote = fixture.quote();
      REQUIRE(quote->m_bid == make_bid(Money(25.60), 1200));
      REQUIRE(quote->m_ask == make_ask(Money(25.60), 1240));
    }
    SUBCASE("next_day") {
      fixture.m_time.set(time_from_string("2026-09-22 13:00:00"));
      fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.50"
        "|57=20260922090000000");
      auto quote = fixture.quote();
      REQUIRE(quote->m_bid.m_size == 0);
      REQUIRE(quote->m_ask.m_size == 0);
    }
    SUBCASE("source_session") {
      fixture.publish("|6=OpeningAuction|5=OddlotImbalance|55=ABX|247=TSE"
        "|572=SellSide|573=40|57=20260921090002000");
      auto quote = fixture.quote();
      REQUIRE(quote->m_bid.m_size == 1200);
      REQUIRE(quote->m_ask.m_size == 1240);
      fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.50"
        "|57=20260921090003000", 1);
      quote = fixture.quote();
      REQUIRE(quote->m_bid == make_bid(Money(25.50), 0));
      REQUIRE(quote->m_ask == make_ask(Money(25.50), 0));
      fixture.require_empty();
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
    auto fixture = Fixture(Venues::CSE, {
      TickerInfo(parse_ticker("ABX.TSX"), "", "", 100),
      TickerInfo(parse_ticker("CSE.CSE"), "", "", 100)});
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=CNQ|191=25.50"
      "|57=20260921090000000|654=700");
    fixture.require_empty();
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=CSE|247=CNQ|191=10"
      "|57=20260921090000000|654=700");
    auto quote = fixture.quote();
    REQUIRE(quote.get_index() == parse_ticker("CSE.CSE"));
    REQUIRE(quote->m_bid.m_size == 700);
    REQUIRE(quote->m_ask.m_size == 700);
    auto neo_fixture = Fixture(Venues::NEOE, {
      TickerInfo(parse_ticker("NEO.NEOE"), "", "", 100)});
    neo_fixture.publish("|6=MBXMessage|5=AssignCOP|55=NEO|247=AQL|191=12"
      "|57=20260921090000000|698=900|492=SellSide|493=200");
    quote = neo_fixture.quote();
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
    auto is_found = false;
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.50"
      "|57=20260921090000000");
    auto query = fixture.query();
    REQUIRE(query->m_query.get_index() == Scope(Countries::CA));
    REQUIRE(query->m_query.get_snapshot_limit() == SnapshotLimit::UNLIMITED);
    SUBCASE("found") {
      is_found = true;
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
    if(is_found) {
      REQUIRE(fixture.quote().get_index() == parse_ticker("ABX.TSX"));
    }
    fixture.require_empty();
    REQUIRE_FALSE(fixture.m_data_operations->try_pop().has_value());
  }

  TEST_CASE("publication_shutdown") {
    auto fixture = Fixture();
    fixture.publish("|6=MBXMessage|5=AssignCOP|55=ABX|247=TSE|191=25.50"
      "|57=20260921090000000");
    flush_pending_routines();
    auto operation = fixture.m_feed_operations->try_pop();
    REQUIRE(operation.has_value());
    REQUIRE(std::holds_alternative<FeedClient::PublishBboQuoteOperation>(
      **operation));
    REQUIRE_FALSE(fixture.m_client.is_finished());
    fixture.m_client.close();
    REQUIRE(fixture.m_client.is_finished());
    REQUIRE_FALSE(fixture.m_client.get_exception());
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

  TEST_CASE("quote_sessions") {
    auto fixture = Fixture({
      TickerInfo(parse_ticker("ABX.TSX"), "Barrick", "", 100),
      TickerInfo(parse_ticker("XYZ.TSXV"), "Venture", "", 100)});
    for(auto [timestamp, tsx, venture] : {
        std::tuple("2026-12-24 17:59:59", true, true),
        {"2026-12-24 18:00:00", false, true},
        {"2026-12-25 15:00:00", false, false},
        {"2026-12-28 14:29:59", false, false},
        {"2026-12-28 14:30:00", true, true},
        {"2027-03-15 13:29:59", false, false},
        {"2027-03-15 13:30:00", true, true}}) {
      fixture.m_time.set(time_from_string(timestamp));
      for(auto [symbol, venue, is_publishing] : {
          std::tuple("ABX", Venues::TSX, tsx),
          {"XYZ", Venues::TSXV, venture}}) {
        fixture.publish(std::string("|6=Quote|5=Quote|55=") + symbol +
          "|196=25.50|196.1=25.51|64=123|64.1=456");
        if(is_publishing) {
          REQUIRE(fixture.quote().get_index() == Ticker(symbol, venue));
        }
        fixture.require_empty();
      }
    }
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
