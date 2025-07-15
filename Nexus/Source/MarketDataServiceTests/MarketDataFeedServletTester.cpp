#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataFeedServlet.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;

namespace {
  struct TestRegistry {
    std::function<void (const SecurityInfo&)> m_add_slot;
    std::function<void (const VenueOrderImbalance&, int)>
      m_order_imbalance_slot;
    std::function<void (const SecurityBboQuote&, int)> m_bbo_quote_slot;
    std::function<void (const SecurityBookQuote&, int)> m_book_quote_slot;
    std::function<void (const SecurityTimeAndSale&, int)> m_time_and_sale_slot;
    std::function<void (int)> m_clear_slot;

    TestRegistry() {
      m_clear_slot = [] (auto) {};
    }

    void add(const SecurityInfo& info) {
      m_add_slot(info);
    }

    void publish(const VenueOrderImbalance& imbalance, int source_id) {
      m_order_imbalance_slot(imbalance, source_id);
    }

    void publish(const SecurityBboQuote& quote, int source_id) {
      m_bbo_quote_slot(quote, source_id);
    }

    void publish(const SecurityBookQuote& quote, int source_id) {
      m_book_quote_slot(quote, source_id);
    }

    void publish(const SecurityTimeAndSale& time_and_sale, int source_id) {
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
    std::shared_ptr<TestServerConnection> m_server_connection;
    optional<ServletContainer> m_container;
    std::unique_ptr<TestServiceProtocolClient> m_client;

    Fixture()
        : m_server_connection(std::make_shared<TestServerConnection>()) {
      m_container.emplace(Initialize(&m_registry),
        m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
      m_client = std::make_unique<TestServiceProtocolClient>(
        Initialize("test", *m_server_connection), Initialize());
      RegisterMarketDataFeedMessages(Store(m_client->GetSlots()));
    }
  };
}

TEST_SUITE("MarketDataFeedServlet") {
  TEST_CASE("set_security_info") {
    auto fixture = Fixture();
    auto info = SecurityInfo(Security("A", TSX), "SECURITY A", "", 100);
    auto completion_token = Async<void>();
    fixture.m_registry.m_add_slot = [&] (const auto& received_info) {
      REQUIRE(received_info == info);
      completion_token.GetEval().SetResult();
    };
    SendRecordMessage<SetSecurityInfoMessage>(*fixture.m_client, info);
    completion_token.Get();
  }

  TEST_CASE("send_bbo_quote") {
    auto fixture = Fixture();
    auto security = Security("A", TSX);
    auto bbo_quote = SecurityBboQuote(BboQuote(Quote(
      Money::CENT, 100, Side::BID), Quote(2 * Money::CENT, 200, Side::ASK),
      time_from_string("2024-07-14 12:00:00")), security);
    auto completion_token = Async<void>();
    fixture.m_registry.m_bbo_quote_slot =
      [&] (const auto& received_quote, auto source_id) {
        REQUIRE(received_quote == bbo_quote);
        completion_token.GetEval().SetResult();
      };
    SendRecordMessage<SendMarketDataFeedMessages>(
      *fixture.m_client, std::vector<MarketDataFeedMessage>({bbo_quote}));
    completion_token.Get();
  }

  TEST_CASE("send_book_quote") {
    auto fixture = Fixture();
    auto security = Security("A", TSX);
    auto book_quote = SecurityBookQuote(
      BookQuote("MP1", false, TSX, Quote(Money::CENT, 100, Side::BID),
      time_from_string("2024-07-14 12:00:00")), security);
    auto completion_token = Async<void>();
    fixture.m_registry.m_book_quote_slot =
      [&] (const auto& received_quote, auto source_id) {
        REQUIRE(received_quote == book_quote);
        completion_token.GetEval().SetResult();
      };
    SendRecordMessage<SendMarketDataFeedMessages>(
      *fixture.m_client, std::vector<MarketDataFeedMessage>({book_quote}));
    completion_token.Get();
  }

  TEST_CASE("send_order_imbalance") {
    auto fixture = Fixture();
    auto security = Security("A", TSX);
    auto order_imbalance = VenueOrderImbalance(OrderImbalance(security,
      Side::ASK, 100, Money::ONE, time_from_string("2024-07-14 12:00:00")),
      TSX);
    auto completion_token = Async<void>();
    fixture.m_registry.m_order_imbalance_slot =
      [&] (const auto& received_imbalance, auto source_id) {
        REQUIRE(received_imbalance == order_imbalance);
        completion_token.GetEval().SetResult();
      };
    SendRecordMessage<SendMarketDataFeedMessages>(
      *fixture.m_client, std::vector<MarketDataFeedMessage>({order_imbalance}));
    completion_token.Get();
  }

  TEST_CASE("send_time_and_sale") {
    auto fixture = Fixture();
    auto security = Security("A", TSX);
    auto time_and_sale = SecurityTimeAndSale(
      TimeAndSale(time_from_string("2024-07-14 12:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""), security);
    auto completion_token = Async<void>();
    fixture.m_registry.m_time_and_sale_slot =
      [&] (const auto& received_time_and_sale, auto source_id) {
        REQUIRE(received_time_and_sale == time_and_sale);
        completion_token.GetEval().SetResult();
      };
    SendRecordMessage<SendMarketDataFeedMessages>(
      *fixture.m_client, std::vector<MarketDataFeedMessage>({time_and_sale}));
    completion_token.Get();
  }

  TEST_CASE("send_multiple_messages") {
    auto fixture = Fixture();
    auto security = Security("A", TSX);
    auto bbo_quote = SecurityBboQuote(BboQuote(Quote(
      Money::CENT, 100, Side::BID), Quote(2 * Money::CENT, 200, Side::ASK),
      time_from_string("2024-07-14 12:00:00")), security);
    auto book_quote = SecurityBookQuote(
      BookQuote("MP1", false, TSX, Quote(Money::CENT, 100, Side::BID),
      time_from_string("2024-07-14 12:00:00")), security);
    auto order_imbalance = VenueOrderImbalance(OrderImbalance(security,
      Side::ASK, 100, Money::ONE, time_from_string("2024-07-14 12:00:00")),
      TSX);
    auto time_and_sale = SecurityTimeAndSale(
      TimeAndSale(time_from_string("2024-07-14 12:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""), security);
    auto bbo_quote_completion = Async<void>();
    auto book_quote_completion = Async<void>();
    auto order_imbalance_completion = Async<void>();
    auto time_and_sale_completion = Async<void>();
    fixture.m_registry.m_bbo_quote_slot =
      [&] (const auto& received_quote, auto source_id) {
        REQUIRE(received_quote == bbo_quote);
        bbo_quote_completion.GetEval().SetResult();
      };
    fixture.m_registry.m_book_quote_slot =
      [&] (const auto& received_quote, auto source_id) {
        REQUIRE(received_quote == book_quote);
        book_quote_completion.GetEval().SetResult();
      };
    fixture.m_registry.m_order_imbalance_slot =
      [&] (const auto& received_imbalance, auto source_id) {
        REQUIRE(received_imbalance == order_imbalance);
        order_imbalance_completion.GetEval().SetResult();
      };
    fixture.m_registry.m_time_and_sale_slot =
      [&] (const auto& received_time_and_sale, auto source_id) {
        REQUIRE(received_time_and_sale == time_and_sale);
        time_and_sale_completion.GetEval().SetResult();
      };
    auto messages = std::vector<MarketDataFeedMessage>{
      bbo_quote, book_quote, order_imbalance, time_and_sale};
    SendRecordMessage<SendMarketDataFeedMessages>(*fixture.m_client, messages);
    bbo_quote_completion.Get();
    book_quote_completion.Get();
    order_imbalance_completion.Get();
    time_and_sale_completion.Get();
  }

  TEST_CASE("client_closed") {
    auto fixture = Fixture();
    auto completion_token = Async<void>();
    fixture.m_registry.m_clear_slot = [&] (auto source_id) {
      completion_token.GetEval().SetResult();
    };
    fixture.m_client->Close();
    completion_token.Get();
  }
}
