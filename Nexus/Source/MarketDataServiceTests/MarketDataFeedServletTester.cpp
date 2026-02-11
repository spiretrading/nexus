#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/MarketDataFeedServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

namespace {
  struct TestRegistry {
    std::function<void (const TickerInfo&)> m_add_slot;
    std::function<void (const VenueOrderImbalance&, int)>
      m_order_imbalance_slot;
    std::function<void (const TickerBboQuote&, int)> m_bbo_quote_slot;
    std::function<void (const TickerBookQuote&, int)> m_book_quote_slot;
    std::function<void (const TickerTimeAndSale&, int)> m_time_and_sale_slot;
    std::function<void (int)> m_clear_slot;

    TestRegistry() {
      m_clear_slot = [] (auto) {};
    }

    void add(const TickerInfo& info) {
      m_add_slot(info);
    }

    void publish(const VenueOrderImbalance& imbalance, int source_id) {
      m_order_imbalance_slot(imbalance, source_id);
    }

    void publish(const TickerBboQuote& quote, int source_id) {
      m_bbo_quote_slot(quote, source_id);
    }

    void publish(const TickerBookQuote& quote, int source_id) {
      m_book_quote_slot(quote, source_id);
    }

    void publish(const TickerTimeAndSale& time_and_sale, int source_id) {
      m_time_and_sale_slot(time_and_sale, source_id);
    }

    void clear(int source_id) {
      m_clear_slot(source_id);
    }
  };

  struct Fixture {
    using ServletContainer = TestServiceProtocolServletContainer<
      MetaMarketDataFeedServlet<TestRegistry*>>;
    TestRegistry m_registry;
    std::shared_ptr<LocalServerConnection> m_server_connection;
    optional<ServletContainer> m_container;
    std::unique_ptr<TestServiceProtocolClient> m_client;

    Fixture()
        : m_server_connection(std::make_shared<LocalServerConnection>()) {
      m_container.emplace(init(&m_registry), m_server_connection,
        factory<std::unique_ptr<TriggerTimer>>());
      m_client = std::make_unique<TestServiceProtocolClient>(
        std::make_unique<LocalClientChannel>("test", *m_server_connection),
        init());
      register_market_data_feed_messages(out(m_client->get_slots()));
    }
  };
}

TEST_SUITE("MarketDataFeedServlet") {
  TEST_CASE("set_ticker_info") {
    auto fixture = Fixture();
    auto info = TickerInfo();
    info.m_ticker = parse_ticker("A.TSX");
    info.m_name = "SECURITY A";
    info.m_board_lot = 100;
    auto completion_token = Async<void>();
    fixture.m_registry.m_add_slot = [&] (const auto& received_info) {
      REQUIRE(received_info == info);
      completion_token.get_eval().set();
    };
    send_record_message<SetTickerInfoMessage>(*fixture.m_client, info);
    completion_token.get();
  }

  TEST_CASE("send_bbo_quote") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("A.TSX");
    auto bbo_quote = TickerBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-14 12:00:00")), ticker);
    auto completion_token = Async<void>();
    fixture.m_registry.m_bbo_quote_slot =
      [&] (const auto& received_quote, auto source_id) {
        REQUIRE(received_quote == bbo_quote);
        completion_token.get_eval().set();
      };
    send_record_message<SendMarketDataFeedMessages>(
      *fixture.m_client, std::vector<MarketDataFeedMessage>{bbo_quote});
    completion_token.get();
  }

  TEST_CASE("send_book_quote") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("A.TSX");
    auto book_quote = TickerBookQuote(
      BookQuote("MP1", false, TSX, make_bid(Money::CENT, 100),
        time_from_string("2024-07-14 12:00:00")), ticker);
    auto completion_token = Async<void>();
    fixture.m_registry.m_book_quote_slot =
      [&] (const auto& received_quote, auto source_id) {
        REQUIRE(received_quote == book_quote);
        completion_token.get_eval().set();
      };
    send_record_message<SendMarketDataFeedMessages>(
      *fixture.m_client, std::vector<MarketDataFeedMessage>{book_quote});
    completion_token.get();
  }

  TEST_CASE("send_order_imbalance") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("A.TSX");
    auto order_imbalance = VenueOrderImbalance(OrderImbalance(ticker,
      Side::ASK, 100, Money::ONE, time_from_string("2024-07-14 12:00:00")),
      TSX);
    auto completion_token = Async<void>();
    fixture.m_registry.m_order_imbalance_slot =
      [&] (const auto& received_imbalance, auto source_id) {
        REQUIRE(received_imbalance == order_imbalance);
        completion_token.get_eval().set();
      };
    send_record_message<SendMarketDataFeedMessages>(
      *fixture.m_client, std::vector<MarketDataFeedMessage>{order_imbalance});
    completion_token.get();
  }

  TEST_CASE("send_time_and_sale") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("A.TSX");
    auto time_and_sale = TickerTimeAndSale(
      TimeAndSale(time_from_string("2024-07-14 12:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""), ticker);
    auto completion_token = Async<void>();
    fixture.m_registry.m_time_and_sale_slot =
      [&] (const auto& received_time_and_sale, auto source_id) {
        REQUIRE(received_time_and_sale == time_and_sale);
        completion_token.get_eval().set();
      };
    send_record_message<SendMarketDataFeedMessages>(
      *fixture.m_client, std::vector<MarketDataFeedMessage>{time_and_sale});
    completion_token.get();
  }

  TEST_CASE("send_multiple_messages") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("A.TSX");
    auto bbo_quote = TickerBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-14 12:00:00")), ticker);
    auto book_quote = TickerBookQuote(
      BookQuote("MP1", false, TSX, make_bid(Money::CENT, 100),
        time_from_string("2024-07-14 12:00:00")), ticker);
    auto order_imbalance = VenueOrderImbalance(OrderImbalance(ticker,
      Side::ASK, 100, Money::ONE, time_from_string("2024-07-14 12:00:00")),
      TSX);
    auto time_and_sale = TickerTimeAndSale(
      TimeAndSale(time_from_string("2024-07-14 12:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""), ticker);
    auto bbo_quote_completion = Async<void>();
    auto book_quote_completion = Async<void>();
    auto order_imbalance_completion = Async<void>();
    auto time_and_sale_completion = Async<void>();
    fixture.m_registry.m_bbo_quote_slot =
      [&] (const auto& received_quote, auto source_id) {
        REQUIRE(received_quote == bbo_quote);
        bbo_quote_completion.get_eval().set();
      };
    fixture.m_registry.m_book_quote_slot =
      [&] (const auto& received_quote, auto source_id) {
        REQUIRE(received_quote == book_quote);
        book_quote_completion.get_eval().set();
      };
    fixture.m_registry.m_order_imbalance_slot =
      [&] (const auto& received_imbalance, auto source_id) {
        REQUIRE(received_imbalance == order_imbalance);
        order_imbalance_completion.get_eval().set();
      };
    fixture.m_registry.m_time_and_sale_slot =
      [&] (const auto& received_time_and_sale, auto source_id) {
        REQUIRE(received_time_and_sale == time_and_sale);
        time_and_sale_completion.get_eval().set();
      };
    auto messages = std::vector<MarketDataFeedMessage>{
      bbo_quote, book_quote, order_imbalance, time_and_sale};
    send_record_message<SendMarketDataFeedMessages>(
      *fixture.m_client, messages);
    bbo_quote_completion.get();
    book_quote_completion.get();
    order_imbalance_completion.get();
    time_and_sale_completion.get();
  }

  TEST_CASE("client_closed") {
    auto fixture = Fixture();
    auto completion_token = Async<void>();
    fixture.m_registry.m_clear_slot = [&] (auto source_id) {
      completion_token.get_eval().set();
    };
    fixture.m_client->close();
    completion_token.get();
  }
}
