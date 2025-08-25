#include <memory>
#include <Beam/Codecs/NullEncoder.hpp>
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include <Beam/ServiceLocator/NullAuthenticator.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/ServiceMarketDataFeedClient.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

namespace {
  struct Fixture {
    using TestServiceMarketDataFeedClient = ServiceMarketDataFeedClient<
      std::string, TriggerTimer*, MessageProtocol<
        std::unique_ptr<TestClientChannel>, BinarySender<SharedBuffer>,
        NullEncoder>, TriggerTimer>;
    std::shared_ptr<TestServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    TriggerTimer m_sampling_timer;
    std::unique_ptr<TestServiceMarketDataFeedClient> m_client;
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_handlers;

    Fixture()
        : m_server_connection(std::make_shared<TestServerConnection>()),
          m_server(m_server_connection,
            factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot()) {
      RegisterMarketDataFeedMessages(Store(m_server.GetSlots()));
      m_client = std::make_unique<TestServiceMarketDataFeedClient>(
        std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection), NullAuthenticator(), &m_sampling_timer,
        Initialize());
    }

    template<typename T, typename F>
    void handle(F&& handler) {
      using Slot = typename Beam::Services::Details::RecordMessageSlot<
        RecordMessage<
          T, TestServiceProtocolServer::ServiceProtocolClient>>::Slot;
      auto& stored_handler = m_handlers[typeid(T)];
      if(stored_handler) {
        *std::static_pointer_cast<Slot>(stored_handler) =
          std::forward<F>(handler);
      } else {
        auto shared_handler = std::make_shared<Slot>(std::forward<F>(handler));
        stored_handler = shared_handler;
        AddMessageSlot<T>(Store(m_server.GetSlots()),
          [handler = std::move(shared_handler)] (auto&&... args) {
            try {
              (*handler)(std::forward<decltype(args)>(args)...);
            } catch(...) {
              throw ServiceRequestException("Test failed.");
            }
          });
      }
    }
  };
}

TEST_SUITE("ServiceMarketDataFeedClient") {
  TEST_CASE("add_security_info") {
    auto fixture = Fixture();
    auto info = SecurityInfo(Security("GOOG", NASDAQ), "Google Inc.", "", 100);
    auto completion_token = Async<void>();
    fixture.handle<SetSecurityInfoMessage>(
      [&] (auto& client, const auto& received_info) {
        REQUIRE(received_info == info);
        completion_token.GetEval().SetResult();
      });
    fixture.m_client->add(info);
    completion_token.Get();
  }

  TEST_CASE("publish_order_imbalance") {
    auto fixture = Fixture();
    auto security = Security("GOOG", NASDAQ);
    auto imbalance1 = VenueOrderImbalance(OrderImbalance(security, Side::ASK,
      100, Money::ONE, time_from_string("2024-07-15 12:00:00")), NASDAQ);
    auto imbalance2 = VenueOrderImbalance(OrderImbalance(security, Side::BID,
      200, 2 * Money::ONE, time_from_string("2024-07-15 12:00:01")), NASDAQ);
    auto imbalance3 = VenueOrderImbalance(OrderImbalance(security, Side::ASK,
      300, 3 * Money::ONE, time_from_string("2024-07-15 12:00:02")), NASDAQ);
    auto completion_token = Async<void>();
    fixture.handle<SendMarketDataFeedMessages>(
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
        completion_token.GetEval().SetResult();
      });
    fixture.m_client->publish(imbalance1);
    fixture.m_client->publish(imbalance2);
    fixture.m_client->publish(imbalance3);
    fixture.m_sampling_timer.Trigger();
    completion_token.Get();
  }

  TEST_CASE("publish_bbo_quote") {
    auto fixture = Fixture();
    auto security = Security("GOOG", NASDAQ);
    auto bbo_quote = SecurityBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-15 12:00:00")), security);
    auto completion_token = Async<void>();
    fixture.handle<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<SecurityBboQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(*received_quote == bbo_quote);
        completion_token.GetEval().SetResult();
      });
    fixture.m_client->publish(bbo_quote);
    fixture.m_sampling_timer.Trigger();
    completion_token.Get();
  }

  TEST_CASE("publish_bbo_quote_sampling") {
    auto fixture = Fixture();
    auto security_a = Security("A", NASDAQ);
    auto security_b = Security("B", NASDAQ);
    auto security_c = Security("C", NASDAQ);
    auto bbo_a1 = SecurityBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-15 12:00:00")), security_a);
    auto bbo_b1 = SecurityBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-15 12:00:01")), security_b);
    auto bbo_b2 = SecurityBboQuote(
      BboQuote(make_bid(Money::CENT, 101), make_ask(2 * Money::CENT, 201),
        time_from_string("2024-07-15 12:00:02")), security_b);
    auto bbo_c1 = SecurityBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-15 12:00:03")), security_c);
    auto bbo_c2 = SecurityBboQuote(
      BboQuote(make_bid(Money::CENT, 101), make_ask(2 * Money::CENT, 201),
        time_from_string("2024-07-15 12:00:04")), security_c);
    auto bbo_c3 = SecurityBboQuote(
      BboQuote(make_bid(Money::CENT, 102), make_ask(2 * Money::CENT, 202),
        time_from_string("2024-07-15 12:00:05")), security_c);
    auto completion_token = Async<void>();
    fixture.handle<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 3);
        auto received_quotes =
          std::unordered_map<Security, SecurityBboQuote>();
        for(auto& message : messages) {
          auto quote = get<SecurityBboQuote>(&message);
          REQUIRE(quote);
          received_quotes.emplace(quote->GetIndex(), *quote);
        }
        REQUIRE(received_quotes.at(security_a) == bbo_a1);
        REQUIRE(received_quotes.at(security_b) == bbo_b2);
        REQUIRE(received_quotes.at(security_c) == bbo_c3);
        completion_token.GetEval().SetResult();
      });
    fixture.m_client->publish(bbo_a1);
    fixture.m_client->publish(bbo_c1);
    fixture.m_client->publish(bbo_b1);
    fixture.m_client->publish(bbo_c2);
    fixture.m_client->publish(bbo_b2);
    fixture.m_client->publish(bbo_c3);
    fixture.m_sampling_timer.Trigger();
    completion_token.Get();
  }

  TEST_CASE("publish_book_quote") {
    auto fixture = Fixture();
    auto security = Security("GOOG", NASDAQ);
    auto book_quote = SecurityBookQuote(
      BookQuote("MP1", true, NASDAQ, make_bid(10 * Money::ONE, 100),
        time_from_string("2024-07-15 12:00:00")), security);
    auto completion_token = Async<void>();
    fixture.handle<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<SecurityBookQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(*received_quote == book_quote);
        completion_token.GetEval().SetResult();
      });
    fixture.m_client->publish(book_quote);
    fixture.m_sampling_timer.Trigger();
    completion_token.Get();
  }

  TEST_CASE("publish_book_quote_aggregation") {
    auto fixture = Fixture();
    auto security_a = Security("A", NASDAQ);
    auto security_b = Security("B", NASDAQ);
    auto security_c = Security("C", NASDAQ);
    auto book_a1 = SecurityBookQuote(
      BookQuote("MP1", true, NASDAQ, make_bid(10 * Money::ONE, 100),
        time_from_string("2024-07-15 12:00:00")), security_a);
    auto book_b1 = SecurityBookQuote(
      BookQuote("MP1", true, NASDAQ, make_bid(20 * Money::ONE, 200),
        time_from_string("2024-07-15 12:00:01")), security_b);
    auto book_b2 = SecurityBookQuote(
      BookQuote("MP1", true, NASDAQ, make_bid(20 * Money::ONE, 250),
        time_from_string("2024-07-15 12:00:02")), security_b);
    auto book_c1 = SecurityBookQuote(
      BookQuote("MP2", false, NASDAQ, make_ask(parse_money("30.01"), 300),
        time_from_string("2024-07-15 12:00:03")), security_c);
    auto book_c2 = SecurityBookQuote(
      BookQuote("MP2", false, NASDAQ, make_ask(parse_money("30.02"), 350),
        time_from_string("2024-07-15 12:00:04")), security_c);
    auto completion_token = Async<void>();
    fixture.handle<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        auto received_quotes = std::vector<SecurityBookQuote>();
        for(auto& message : messages) {
          if(auto quote = get<SecurityBookQuote>(&message)) {
            received_quotes.push_back(*quote);
          }
        }
        REQUIRE(received_quotes.size() == 4);
        REQUIRE(std::find(received_quotes.begin(), received_quotes.end(),
          book_a1) != received_quotes.end());
        REQUIRE(std::find(received_quotes.begin(), received_quotes.end(),
          book_b2) != received_quotes.end());
        REQUIRE(std::find(received_quotes.begin(), received_quotes.end(),
          book_c1) != received_quotes.end());
        REQUIRE(std::find(received_quotes.begin(), received_quotes.end(),
          book_c2) != received_quotes.end());
        completion_token.GetEval().SetResult();
      });
    fixture.m_client->publish(book_a1);
    fixture.m_client->publish(book_b1);
    fixture.m_client->publish(book_b2);
    fixture.m_client->publish(book_c1);
    fixture.m_client->publish(book_c2);
    fixture.m_sampling_timer.Trigger();
    completion_token.Get();
  }

  TEST_CASE("order_add_and_remove") {
    auto fixture = Fixture();
    auto security = Security("GOOG", NASDAQ);
    auto order_id = "1";
    auto bbo_quote = SecurityBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-15 12:00:02")), security);
    auto completion_token = Async<void>();
    fixture.handle<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<SecurityBboQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(*received_quote == bbo_quote);
        completion_token.GetEval().SetResult();
      });
    fixture.m_client->add_order(security, NASDAQ, "MP1", true, order_id,
      Side::BID, Money::ONE, 100, time_from_string("2024-07-15 12:00:00"));
    fixture.m_client->remove_order(order_id,
      time_from_string("2024-07-15 12:00:01"));
    fixture.m_client->publish(bbo_quote);
    fixture.m_sampling_timer.Trigger();
    completion_token.Get();
  }

  TEST_CASE("order_aggregation") {
    auto fixture = Fixture();
    auto security = Security("GOOG", NASDAQ);
    auto price = Money::ONE;
    auto mpid = "MP1";
    auto timestamp1 = time_from_string("2024-07-15 12:00:00");
    auto timestamp2 = time_from_string("2024-07-15 12:00:01");
    auto completion_token = Async<void>();
    fixture.handle<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<SecurityBookQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(received_quote->GetIndex() == security);
        REQUIRE((*received_quote)->m_mpid == mpid);
        REQUIRE((*received_quote)->m_quote.m_price == price);
        REQUIRE((*received_quote)->m_quote.m_side == Side::BID);
        REQUIRE((*received_quote)->m_quote.m_size == 300);
        REQUIRE((*received_quote)->m_timestamp == timestamp2);
        completion_token.GetEval().SetResult();
      });
    fixture.m_client->add_order(
      security, NASDAQ, mpid, true, "1", Side::BID, price, 100, timestamp1);
    fixture.m_client->add_order(
      security, NASDAQ, mpid, true, "2", Side::BID, price, 200, timestamp2);
    fixture.m_sampling_timer.Trigger();
    completion_token.Get();
  }

  TEST_CASE("order_modify_size") {
    auto fixture = Fixture();
    auto security = Security("GOOG", NASDAQ);
    auto price = Money::ONE;
    auto mpid = "MP1";
    auto order_id = "1";
    auto timestamp1 = time_from_string("2024-07-15 12:00:00");
    auto timestamp2 = time_from_string("2024-07-15 12:00:01");
    auto completion_token = Async<void>();
    fixture.handle<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<SecurityBookQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(received_quote->GetIndex() == security);
        REQUIRE((*received_quote)->m_mpid == mpid);
        REQUIRE((*received_quote)->m_quote.m_price == price);
        REQUIRE((*received_quote)->m_quote.m_side == Side::BID);
        REQUIRE((*received_quote)->m_quote.m_size == 150);
        REQUIRE((*received_quote)->m_timestamp == timestamp2);
        completion_token.GetEval().SetResult();
      });
    fixture.m_client->add_order(security, NASDAQ, mpid, true, order_id,
      Side::BID, price, 100, timestamp1);
    fixture.m_client->modify_order_size(order_id, 150, timestamp2);
    fixture.m_sampling_timer.Trigger();
    completion_token.Get();
  }

  TEST_CASE("order_modify_price") {
    auto fixture = Fixture();
    auto security = Security("GOOG", NASDAQ);
    auto mpid = "MP1";
    auto order_id = "1";
    auto price1 = Money::ONE;
    auto price2 = 2 * Money::ONE;
    auto timestamp1 = time_from_string("2024-07-15 12:00:00");
    auto timestamp2 = time_from_string("2024-07-15 12:00:01");
    auto completion_token = Async<void>();
    fixture.handle<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<SecurityBookQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(received_quote->GetIndex() == security);
        REQUIRE((*received_quote)->m_mpid == mpid);
        REQUIRE((*received_quote)->m_quote.m_price == price2);
        REQUIRE((*received_quote)->m_quote.m_side == Side::BID);
        REQUIRE((*received_quote)->m_quote.m_size == 100);
        REQUIRE((*received_quote)->m_timestamp == timestamp2);
        completion_token.GetEval().SetResult();
      });
    fixture.m_client->add_order(security, NASDAQ, mpid, true, order_id,
      Side::BID, price1, 100, timestamp1);
    fixture.m_client->modify_order_price(order_id, price2, timestamp2);
    fixture.m_sampling_timer.Trigger();
    completion_token.Get();
  }

  TEST_CASE("order_invalid_operations") {
    auto fixture = Fixture();
    auto security = Security("GOOG", NASDAQ);
    auto bbo_quote = SecurityBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-15 12:00:01")), security);
    auto completion_token = Async<void>();
    fixture.handle<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 1);
        auto& message = messages.front();
        auto received_quote = get<SecurityBboQuote>(&message);
        REQUIRE(received_quote);
        REQUIRE(*received_quote == bbo_quote);
        completion_token.GetEval().SetResult();
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
    fixture.m_sampling_timer.Trigger();
    completion_token.Get();
  }

  TEST_CASE("order_complex_sequence") {
    auto fixture = Fixture();
    auto security = Security("GOOG", NASDAQ);
    auto mpid = "MP1";
    auto price1 = Money::ONE;
    auto price2 = 2 * Money::ONE;
    auto timestamp1 = time_from_string("2024-07-15 12:00:00");
    auto timestamp2 = time_from_string("2024-07-15 12:00:01");
    auto timestamp3 = time_from_string("2024-07-15 12:00:02");
    auto timestamp4 = time_from_string("2024-07-15 12:00:03");
    auto timestamp5 = time_from_string("2024-07-15 12:00:04");
    auto completion_token = Async<void>();
    fixture.handle<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 2);
        auto received_quotes = std::vector<SecurityBookQuote>();
        for(auto& message : messages) {
          if(auto quote = get<SecurityBookQuote>(&message)) {
            received_quotes.push_back(*quote);
          }
        }
        REQUIRE(received_quotes.size() == 2);
        auto quote1 =
          std::find_if(received_quotes.begin(), received_quotes.end(),
            [&] (const auto& quote) {
              return quote->m_quote.m_price == price1;
            });
        REQUIRE(quote1 != received_quotes.end());
        REQUIRE(quote1->GetIndex() == security);
        REQUIRE((*quote1)->m_mpid == mpid);
        REQUIRE((*quote1)->m_quote.m_side == Side::BID);
        REQUIRE((*quote1)->m_quote.m_size == 125);
        REQUIRE((*quote1)->m_timestamp == timestamp4);
        auto quote2 =
          std::find_if(received_quotes.begin(), received_quotes.end(),
            [&] (const auto& quote) {
              return quote->m_quote.m_price == price2;
            });
        REQUIRE(quote2 != received_quotes.end());
        REQUIRE(quote2->GetIndex() == security);
        REQUIRE((*quote2)->m_mpid == mpid);
        REQUIRE((*quote2)->m_quote.m_side == Side::BID);
        REQUIRE((*quote2)->m_quote.m_size == 75);
        REQUIRE((*quote2)->m_timestamp == timestamp5);
        completion_token.GetEval().SetResult();
      });
    fixture.m_client->add_order(
      security, NASDAQ, mpid, true, "A", Side::BID, price1, 100, timestamp1);
    fixture.m_client->add_order(
      security, NASDAQ, mpid, true, "B", Side::BID, price1, 50, timestamp2);
    fixture.m_client->offset_order_size("A", 25, timestamp3);
    fixture.m_client->remove_order("B", timestamp4);
    fixture.m_client->add_order(
      security, NASDAQ, mpid, true, "C", Side::BID, price2, 75, timestamp5);
    fixture.m_sampling_timer.Trigger();
    completion_token.Get();
  }

  TEST_CASE("publish_time_and_sale") {
    auto fixture = Fixture();
    auto security = Security("GOOG", NASDAQ);
    auto time_and_sale1 = SecurityTimeAndSale(
      TimeAndSale(time_from_string("2024-07-15 12:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""), security);
    auto time_and_sale2 = SecurityTimeAndSale(
      TimeAndSale(time_from_string("2024-07-15 12:00:01"), 2 * Money::ONE, 200,
        TimeAndSale::Condition(), "TSX", "", ""), security);
    auto time_and_sale3 = SecurityTimeAndSale(
      TimeAndSale(time_from_string("2024-07-15 12:00:02"), 3 * Money::ONE, 300,
        TimeAndSale::Condition(), "TSX", "", ""), security);
    auto completion_token = Async<void>();
    fixture.handle<SendMarketDataFeedMessages>(
      [&] (auto& client, const auto& messages) {
        REQUIRE(messages.size() == 3);
        auto received_time_and_sale1 = get<SecurityTimeAndSale>(&messages[0]);
        REQUIRE(received_time_and_sale1);
        REQUIRE(*received_time_and_sale1 == time_and_sale1);
        auto received_time_and_sale2 = get<SecurityTimeAndSale>(&messages[1]);
        REQUIRE(received_time_and_sale2);
        REQUIRE(*received_time_and_sale2 == time_and_sale2);
        auto received_time_and_sale3 = get<SecurityTimeAndSale>(&messages[2]);
        REQUIRE(received_time_and_sale3);
        REQUIRE(*received_time_and_sale3 == time_and_sale3);
        completion_token.GetEval().SetResult();
      });
    fixture.m_client->publish(time_and_sale1);
    fixture.m_client->publish(time_and_sale2);
    fixture.m_client->publish(time_and_sale3);
    fixture.m_sampling_timer.Trigger();
    completion_token.Get();
  }
}
