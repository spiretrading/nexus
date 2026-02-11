#include <Beam/ServiceLocator/NullAuthenticator.hpp>
#include <Beam/ServicesTests/ServiceClientFixture.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/ServiceMarketDataFeedClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

namespace {
  struct Fixture : ServiceClientFixture {
    using TestServiceMarketDataFeedClient = ServiceMarketDataFeedClient<
      std::string, TriggerTimer*,
      TestServiceProtocolClientBuilder::MessageProtocol, TriggerTimer>;
    TriggerTimer m_sampling_timer;
    std::unique_ptr<TestServiceMarketDataFeedClient> m_client;

    Fixture() {
      register_market_data_feed_messages(out(m_server.get_slots()));
      m_client = std::make_unique<TestServiceMarketDataFeedClient>(
        std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection), NullAuthenticator(), &m_sampling_timer,
        init());
    }
  };
}

TEST_SUITE("ServiceMarketDataFeedClient") {
  TEST_CASE("add_ticker_info") {
    auto fixture = Fixture();
    auto info = TickerInfo();
    info.m_ticker = parse_ticker("S32.ASX");
    info.m_name = "S32 Inc.";
    info.m_board_lot = 100;
    auto completion_token = Async<void>();
    fixture.on_message<SetTickerInfoMessage>(
      [&] (auto& client, const auto& received_info) {
        REQUIRE(received_info == info);
        completion_token.get_eval().set();
      });
    fixture.m_client->add(info);
    completion_token.get();
  }

  TEST_CASE("publish_order_imbalance") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("S32.ASX");
    auto imbalance1 = VenueOrderImbalance(OrderImbalance(ticker, Side::ASK,
      100, Money::ONE, time_from_string("2024-07-15 12:00:00")), ASX);
    auto imbalance2 = VenueOrderImbalance(OrderImbalance(ticker, Side::BID,
      200, 2 * Money::ONE, time_from_string("2024-07-15 12:00:01")), ASX);
    auto imbalance3 = VenueOrderImbalance(OrderImbalance(ticker, Side::ASK,
      300, 3 * Money::ONE, time_from_string("2024-07-15 12:00:02")), ASX);
    auto completion_token = Async<void>();
    fixture.on_message<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 3);
        auto received_imbalance1 = get<VenueOrderImbalance>(&messages[0]);
        REQUIRE(received_imbalance1);
        REQUIRE(*received_imbalance1 == imbalance1);
        auto received_imbalance2 = get<VenueOrderImbalance>(&messages[1]);
        REQUIRE(received_imbalance2);
        REQUIRE(*received_imbalance2 == imbalance2);
        auto received_imbalance3 = get<VenueOrderImbalance>(&messages[2]);
        REQUIRE(received_imbalance3);
        REQUIRE(*received_imbalance3 == imbalance3);
        completion_token.get_eval().set();
      });
    fixture.m_client->publish(imbalance1);
    fixture.m_client->publish(imbalance2);
    fixture.m_client->publish(imbalance3);
    fixture.m_sampling_timer.trigger();
    completion_token.get();
  }

  TEST_CASE("publish_bbo_quote") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("S32.ASX");
    auto bbo_quote = TickerBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-15 12:00:00")), ticker);
    auto completion_token = Async<void>();
    fixture.on_message<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<TickerBboQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(*received_quote == bbo_quote);
        completion_token.get_eval().set();
      });
    fixture.m_client->publish(bbo_quote);
    fixture.m_sampling_timer.trigger();
    completion_token.get();
  }

  TEST_CASE("publish_bbo_quote_sampling") {
    auto fixture = Fixture();
    auto ticker_a = parse_ticker("A.TSX");
    auto ticker_b = parse_ticker("B.TSX");
    auto ticker_c = parse_ticker("C.TSX");
    auto bbo_a1 = TickerBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-15 12:00:00")), ticker_a);
    auto bbo_b1 = TickerBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-15 12:00:01")), ticker_b);
    auto bbo_b2 = TickerBboQuote(
      BboQuote(make_bid(Money::CENT, 101), make_ask(2 * Money::CENT, 201),
        time_from_string("2024-07-15 12:00:02")), ticker_b);
    auto bbo_c1 = TickerBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-15 12:00:03")), ticker_c);
    auto bbo_c2 = TickerBboQuote(
      BboQuote(make_bid(Money::CENT, 101), make_ask(2 * Money::CENT, 201),
        time_from_string("2024-07-15 12:00:04")), ticker_c);
    auto bbo_c3 = TickerBboQuote(
      BboQuote(make_bid(Money::CENT, 102), make_ask(2 * Money::CENT, 202),
        time_from_string("2024-07-15 12:00:05")), ticker_c);
    auto completion_token = Async<void>();
    fixture.on_message<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 3);
        auto received_quotes =
          std::unordered_map<Ticker, TickerBboQuote>();
        for(auto& message : messages) {
          auto quote = get<TickerBboQuote>(&message);
          REQUIRE(quote);
          received_quotes.emplace(quote->get_index(), *quote);
        }
        REQUIRE(received_quotes.at(ticker_a) == bbo_a1);
        REQUIRE(received_quotes.at(ticker_b) == bbo_b2);
        REQUIRE(received_quotes.at(ticker_c) == bbo_c3);
        completion_token.get_eval().set();
      });
    fixture.m_client->publish(bbo_a1);
    fixture.m_client->publish(bbo_c1);
    fixture.m_client->publish(bbo_b1);
    fixture.m_client->publish(bbo_c2);
    fixture.m_client->publish(bbo_b2);
    fixture.m_client->publish(bbo_c3);
    fixture.m_sampling_timer.trigger();
    completion_token.get();
  }

  TEST_CASE("publish_book_quote") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TD.TSX");
    auto book_quote = TickerBookQuote(
      BookQuote("MP1", true, TSX, make_bid(10 * Money::ONE, 100),
        time_from_string("2024-07-15 12:00:00")), ticker);
    auto completion_token = Async<void>();
    fixture.on_message<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<TickerBookQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(*received_quote == book_quote);
        completion_token.get_eval().set();
      });
    fixture.m_client->publish(book_quote);
    fixture.m_sampling_timer.trigger();
    completion_token.get();
  }

  TEST_CASE("publish_book_quote_aggregation") {
    auto fixture = Fixture();
    auto ticker_a = parse_ticker("A.TSX");
    auto ticker_b = parse_ticker("B.TSX");
    auto ticker_c = parse_ticker("C.TSX");
    auto book_a1 = TickerBookQuote(
      BookQuote("MP1", true, TSX, make_bid(10 * Money::ONE, 100),
        time_from_string("2024-07-15 12:00:00")), ticker_a);
    auto book_b1 = TickerBookQuote(
      BookQuote("MP1", true, TSX, make_bid(20 * Money::ONE, 200),
        time_from_string("2024-07-15 12:00:01")), ticker_b);
    auto book_b2 = TickerBookQuote(
      BookQuote("MP1", true, TSX, make_bid(20 * Money::ONE, 250),
        time_from_string("2024-07-15 12:00:02")), ticker_b);
    auto book_c1 = TickerBookQuote(
      BookQuote("MP2", false, TSX, make_ask(parse_money("30.01"), 300),
        time_from_string("2024-07-15 12:00:03")), ticker_c);
    auto book_c2 = TickerBookQuote(
      BookQuote("MP2", false, TSX, make_ask(parse_money("30.02"), 350),
        time_from_string("2024-07-15 12:00:04")), ticker_c);
    auto completion_token = Async<void>();
    fixture.on_message<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        auto received_quotes = std::vector<TickerBookQuote>();
        for(auto& message : messages) {
          if(auto quote = get<TickerBookQuote>(&message)) {
            received_quotes.push_back(*quote);
          }
        }
        REQUIRE(received_quotes.size() == 4);
        REQUIRE(std::ranges::contains(received_quotes, book_a1));
        REQUIRE(std::ranges::contains(received_quotes, book_b2));
        REQUIRE(std::ranges::contains(received_quotes, book_c1));
        REQUIRE(std::ranges::contains(received_quotes, book_c2));
        completion_token.get_eval().set();
      });
    fixture.m_client->publish(book_a1);
    fixture.m_client->publish(book_b1);
    fixture.m_client->publish(book_b2);
    fixture.m_client->publish(book_c1);
    fixture.m_client->publish(book_c2);
    fixture.m_sampling_timer.trigger();
    completion_token.get();
  }

  TEST_CASE("order_add_and_remove") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TD.TSX");
    auto order_id = "1";
    auto bbo_quote = TickerBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-15 12:00:02")), ticker);
    auto completion_token = Async<void>();
    fixture.on_message<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<TickerBboQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(*received_quote == bbo_quote);
        completion_token.get_eval().set();
      });
    fixture.m_client->add_order(ticker, TSX, "MP1", true, order_id,
      Side::BID, Money::ONE, 100, time_from_string("2024-07-15 12:00:00"));
    fixture.m_client->remove_order(order_id,
      time_from_string("2024-07-15 12:00:01"));
    fixture.m_client->publish(bbo_quote);
    fixture.m_sampling_timer.trigger();
    completion_token.get();
  }

  TEST_CASE("order_aggregation") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TD.TSX");
    auto mpid = "MP1";
    auto price1 = Money::ONE;
    auto price2 = 2 * Money::ONE;
    auto timestamp1 = time_from_string("2024-07-15 12:00:00");
    auto timestamp2 = time_from_string("2024-07-15 12:00:01");
    auto completion_token = Async<void>();
    fixture.on_message<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<TickerBookQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(received_quote->get_index() == ticker);
        REQUIRE((*received_quote)->m_mpid == mpid);
        REQUIRE((*received_quote)->m_quote.m_price == price1);
        REQUIRE((*received_quote)->m_quote.m_side == Side::BID);
        REQUIRE((*received_quote)->m_quote.m_size == 300);
        REQUIRE((*received_quote)->m_timestamp == timestamp2);
        completion_token.get_eval().set();
      });
    fixture.m_client->add_order(
      ticker, TSX, mpid, true, "1", Side::BID, price1, 100, timestamp1);
    fixture.m_client->add_order(
      ticker, TSX, mpid, true, "2", Side::BID, price1, 200, timestamp2);
    fixture.m_sampling_timer.trigger();
    completion_token.get();
  }

  TEST_CASE("order_modify_size") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("S32.ASX");
    auto price = Money::ONE;
    auto mpid = "MP1";
    auto order_id = "1";
    auto timestamp1 = time_from_string("2024-07-15 12:00:00");
    auto timestamp2 = time_from_string("2024-07-15 12:00:01");
    auto completion_token = Async<void>();
    fixture.on_message<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<TickerBookQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(received_quote->get_index() == ticker);
        REQUIRE((*received_quote)->m_mpid == mpid);
        REQUIRE((*received_quote)->m_quote.m_price == price);
        REQUIRE((*received_quote)->m_quote.m_side == Side::BID);
        REQUIRE((*received_quote)->m_quote.m_size == 150);
        REQUIRE((*received_quote)->m_timestamp == timestamp2);
        completion_token.get_eval().set();
      });
    fixture.m_client->add_order(ticker, ASX, mpid, true, order_id,
      Side::BID, price, 100, timestamp1);
    fixture.m_client->modify_order_size(order_id, 150, timestamp2);
    fixture.m_sampling_timer.trigger();
    completion_token.get();
  }

  TEST_CASE("order_modify_price") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("S32.ASX");
    auto mpid = "MP1";
    auto order_id = "1";
    auto price1 = Money::ONE;
    auto price2 = 2 * Money::ONE;
    auto timestamp1 = time_from_string("2024-07-15 12:00:00");
    auto timestamp2 = time_from_string("2024-07-15 12:00:01");
    auto completion_token = Async<void>();
    fixture.on_message<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<TickerBookQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(received_quote->get_index() == ticker);
        REQUIRE((*received_quote)->m_mpid == mpid);
        REQUIRE((*received_quote)->m_quote.m_price == price2);
        REQUIRE((*received_quote)->m_quote.m_side == Side::BID);
        REQUIRE((*received_quote)->m_quote.m_size == 100);
        REQUIRE((*received_quote)->m_timestamp == timestamp2);
        completion_token.get_eval().set();
      });
    fixture.m_client->add_order(ticker, ASX, mpid, true, order_id,
      Side::BID, price1, 100, timestamp1);
    fixture.m_client->modify_order_price(order_id, price2, timestamp2);
    fixture.m_sampling_timer.trigger();
    completion_token.get();
  }

  TEST_CASE("order_invalid_operations") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TD.TSX");
    auto bbo_quote = TickerBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-15 12:00:01")), ticker);
    auto completion_token = Async<void>();
    fixture.on_message<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<TickerBboQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(*received_quote == bbo_quote);
        completion_token.get_eval().set();
      });
    fixture.m_client->modify_order_size(
      "1", 150, time_from_string("2024-07-15 12:00:00"));
    fixture.m_client->offset_order_size(
      "2", 50, time_from_string("2024-07-15 12:00:00"));
    fixture.m_client->modify_order_price(
      "3", Money::ONE, time_from_string("2024-07-15 12:00:00"));
    fixture.m_client->remove_order(
      "4", time_from_string("2024-07-15 12:00:00"));
    fixture.m_client->publish(bbo_quote);
    fixture.m_sampling_timer.trigger();
    completion_token.get();
  }

  TEST_CASE("order_complex_sequence") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TD.TSX");
    auto mpid = "MP1";
    auto price1 = Money::ONE;
    auto price2 = 2 * Money::ONE;
    auto timestamp1 = time_from_string("2024-07-15 12:00:00");
    auto timestamp2 = time_from_string("2024-07-15 12:00:01");
    auto timestamp3 = time_from_string("2024-07-15 12:00:02");
    auto timestamp4 = time_from_string("2024-07-15 12:00:03");
    auto timestamp5 = time_from_string("2024-07-15 12:00:04");
    auto completion_token = Async<void>();
    fixture.on_message<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 2);
        auto received_quotes = std::vector<TickerBookQuote>();
        for(auto& message : messages) {
          if(auto quote = get<TickerBookQuote>(&message)) {
            received_quotes.push_back(*quote);
          }
        }
        REQUIRE(received_quotes.size() == 2);
        auto quote1 = std::ranges::find_if(received_quotes,
          [&] (const auto& quote) {
            return quote->m_quote.m_price == price1;
          });
        REQUIRE(quote1 != received_quotes.end());
        REQUIRE(quote1->get_index() == ticker);
        REQUIRE((*quote1)->m_mpid == mpid);
        REQUIRE((*quote1)->m_quote.m_side == Side::BID);
        REQUIRE((*quote1)->m_quote.m_size == 125);
        REQUIRE((*quote1)->m_timestamp == timestamp4);
        auto quote2 = std::ranges::find_if(received_quotes,
          [&] (const auto& quote) {
            return quote->m_quote.m_price == price2;
          });
        REQUIRE(quote2 != received_quotes.end());
        REQUIRE(quote2->get_index() == ticker);
        REQUIRE((*quote2)->m_mpid == mpid);
        REQUIRE((*quote2)->m_quote.m_side == Side::BID);
        REQUIRE((*quote2)->m_quote.m_size == 75);
        REQUIRE((*quote2)->m_timestamp == timestamp5);
        completion_token.get_eval().set();
      });
    fixture.m_client->add_order(
      ticker, TSX, mpid, true, "A", Side::BID, price1, 100, timestamp1);
    fixture.m_client->add_order(
      ticker, TSX, mpid, true, "B", Side::BID, price1, 50, timestamp2);
    fixture.m_client->offset_order_size("A", 25, timestamp3);
    fixture.m_client->remove_order("B", timestamp4);
    fixture.m_client->add_order(
      ticker, TSX, mpid, true, "C", Side::BID, price2, 75, timestamp5);
    fixture.m_sampling_timer.trigger();
    completion_token.get();
  }

  TEST_CASE("publish_time_and_sale") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TD.TSX");
    auto time_and_sale1 = TickerTimeAndSale(
      TimeAndSale(time_from_string("2024-07-15 12:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""), ticker);
    auto time_and_sale2 = TickerTimeAndSale(
      TimeAndSale(time_from_string("2024-07-15 12:00:01"), 2 * Money::ONE, 200,
        TimeAndSale::Condition(), "TSX", "", ""), ticker);
    auto time_and_sale3 = TickerTimeAndSale(
      TimeAndSale(time_from_string("2024-07-15 12:00:02"), 3 * Money::ONE, 300,
        TimeAndSale::Condition(), "TSX", "", ""), ticker);
    auto completion_token = Async<void>();
    fixture.on_message<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 3);
        auto received_time_and_sale1 = get<TickerTimeAndSale>(&messages[0]);
        REQUIRE(received_time_and_sale1);
        REQUIRE(*received_time_and_sale1 == time_and_sale1);
        auto received_time_and_sale2 = get<TickerTimeAndSale>(&messages[1]);
        REQUIRE(received_time_and_sale2);
        REQUIRE(*received_time_and_sale2 == time_and_sale2);
        auto received_time_and_sale3 = get<TickerTimeAndSale>(&messages[2]);
        REQUIRE(received_time_and_sale3);
        REQUIRE(*received_time_and_sale3 == time_and_sale3);
        completion_token.get_eval().set();
      });
    fixture.m_client->publish(time_and_sale1);
    fixture.m_client->publish(time_and_sale2);
    fixture.m_client->publish(time_and_sale3);
    fixture.m_sampling_timer.trigger();
    completion_token.get();
  }
}
