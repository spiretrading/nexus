#include <future>
#include <stdexcept>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataServiceTests/TestMarketDataFeedClient.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace Nexus::Venues;

namespace {
  struct Fixture {
    std::shared_ptr<TestMarketDataFeedClient::Queue> m_operations;
    TestMarketDataFeedClient m_client;
    std::future<void> m_result;

    Fixture()
      : m_operations(std::make_shared<TestMarketDataFeedClient::Queue>()),
        m_client(m_operations) {}

    ~Fixture() {
      m_client.close();
    }

    template<typename O>
    void require_operation(auto invoke, auto validate) {
      m_result = std::async(std::launch::async, [&] {
        invoke(m_client);
      });
      auto operation = m_operations->pop();
      auto actual = std::get_if<O>(&*operation);
      REQUIRE(actual);
      validate(*actual);
      actual->m_result.set();
      REQUIRE_NOTHROW(m_result.get());
      REQUIRE(!m_operations->try_pop());
    }
  };
}

TEST_SUITE("TestMarketDataFeedClient") {
  TEST_CASE("operations") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("BHP.ASX");
    auto timestamp = time_from_string("2026-09-14 10:00:00");
    auto price = parse_money("42.75");
    auto id = std::string("order-123");
    SUBCASE("add") {
      auto info = TickerInfo(ticker, "BHP Group", "Materials", 100);
      fixture.require_operation<TestMarketDataFeedClient::AddOperation>(
        [&] (auto& client) { client.add(info); },
        [&] (const auto& operation) { REQUIRE(operation.m_info == info); });
    }
    SUBCASE("publish_order_imbalance") {
      auto imbalance = VenueOrderImbalance(
        OrderImbalance(ticker, Side::ASK, 1200, price, timestamp), CXA);
      fixture.require_operation<
        TestMarketDataFeedClient::PublishOrderImbalanceOperation>(
          [&] (auto& client) { client.publish(imbalance); },
          [&] (const auto& operation) {
            REQUIRE(operation.m_imbalance == imbalance);
          });
    }
    SUBCASE("publish_bbo_quote") {
      auto quote = TickerBboQuote(BboQuote(make_bid(price, 200),
        make_ask(price + Money::CENT, 300), timestamp), ticker);
      fixture.require_operation<
        TestMarketDataFeedClient::PublishBboQuoteOperation>(
          [&] (auto& client) { client.publish(quote); },
          [&] (const auto& operation) { REQUIRE(operation.m_quote == quote); });
    }
    SUBCASE("publish_book_quote") {
      auto quote = TickerBookQuote(
        BookQuote("1234", true, CXA, make_bid(price, 400), timestamp), ticker);
      fixture.require_operation<
        TestMarketDataFeedClient::PublishBookQuoteOperation>(
          [&] (auto& client) { client.publish(quote); },
          [&] (const auto& operation) { REQUIRE(operation.m_quote == quote); });
    }
    SUBCASE("publish_time_and_sale") {
      auto sale = TickerTimeAndSale(TimeAndSale(timestamp, price, 500,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::CLOSE, "C"),
        "CXA", "1234", "5678"), ticker);
      fixture.require_operation<
        TestMarketDataFeedClient::PublishTimeAndSaleOperation>(
          [&] (auto& client) { client.publish(sale); },
          [&] (const auto& operation) {
            REQUIRE(operation.m_time_and_sale == sale);
          });
    }
    SUBCASE("publish_ticker_status") {
      auto status = IndexedTickerStatus(TickerStatus(
        CXA, "OPEN", TickerStatus::Flag::IS_CONTINUOUS, timestamp), ticker);
      fixture.require_operation<
        TestMarketDataFeedClient::PublishTickerStatusOperation>(
          [&] (auto& client) { client.publish(status); },
          [&] (const auto& operation) {
            REQUIRE(operation.m_status == status);
          });
    }
    SUBCASE("add_order") {
      fixture.require_operation<TestMarketDataFeedClient::AddOrderOperation>(
        [&] (auto& client) {
          client.add_order(
            ticker, CXA, "1234", true, id, Side::ASK, price, 600, timestamp);
        }, [&] (const auto& operation) {
          REQUIRE(operation.m_ticker == ticker);
          REQUIRE(operation.m_venue == CXA);
          REQUIRE(operation.m_mpid == "1234");
          REQUIRE(operation.m_is_primary_mpid);
          REQUIRE(operation.m_id == id);
          REQUIRE(operation.m_side == Side::ASK);
          REQUIRE(operation.m_price == price);
          REQUIRE(operation.m_size == 600);
          REQUIRE(operation.m_timestamp == timestamp);
        });
    }
    SUBCASE("modify_order_size") {
      fixture.require_operation<
        TestMarketDataFeedClient::ModifyOrderSizeOperation>(
          [&] (auto& client) {
            client.modify_order_size(id, 300, timestamp);
          }, [&] (const auto& operation) {
            REQUIRE(operation.m_id == id);
            REQUIRE(operation.m_size == 300);
            REQUIRE(operation.m_timestamp == timestamp);
          });
    }
    SUBCASE("offset_order_size") {
      fixture.require_operation<
        TestMarketDataFeedClient::OffsetOrderSizeOperation>(
          [&] (auto& client) {
            client.offset_order_size(id, -200, timestamp);
          }, [&] (const auto& operation) {
            REQUIRE(operation.m_id == id);
            REQUIRE(operation.m_delta == -200);
            REQUIRE(operation.m_timestamp == timestamp);
          });
    }
    SUBCASE("modify_order_price") {
      fixture.require_operation<
        TestMarketDataFeedClient::ModifyOrderPriceOperation>(
          [&] (auto& client) {
            client.modify_order_price(id, price, timestamp);
          }, [&] (const auto& operation) {
            REQUIRE(operation.m_id == id);
            REQUIRE(operation.m_price == price);
            REQUIRE(operation.m_timestamp == timestamp);
          });
    }
    SUBCASE("remove_order") {
      fixture.require_operation<
        TestMarketDataFeedClient::RemoveOrderOperation>(
          [&] (auto& client) { client.remove_order(id, timestamp); },
          [&] (const auto& operation) {
            REQUIRE(operation.m_id == id);
            REQUIRE(operation.m_timestamp == timestamp);
          });
    }
  }

  TEST_CASE("operation_failure") {
    auto fixture = Fixture();
    fixture.m_result = std::async(std::launch::async, [&] {
      fixture.m_client.add(TickerInfo());
    });
    auto operation = fixture.m_operations->pop();
    auto add = std::get_if<TestMarketDataFeedClient::AddOperation>(&*operation);
    REQUIRE(add);
    add->m_result.set(
      std::make_exception_ptr(std::runtime_error("Unable to add ticker.")));
    REQUIRE_THROWS_AS(fixture.m_result.get(), std::runtime_error);
  }

  TEST_CASE("close") {
    auto fixture = Fixture();
    SUBCASE("pending_operation") {
      fixture.m_result = std::async(std::launch::async, [&] {
        fixture.m_client.add(TickerInfo());
      });
      auto operation = fixture.m_operations->pop();
      REQUIRE(std::holds_alternative<TestMarketDataFeedClient::AddOperation>(
        *operation));
      fixture.m_client.close();
      REQUIRE_THROWS_AS(fixture.m_result.get(), EndOfFileException);
    }
    SUBCASE("new_operation") {
      fixture.m_client.close();
      REQUIRE_THROWS_AS(fixture.m_client.add(TickerInfo()), EndOfFileException);
    }
    fixture.m_client.close();
    REQUIRE(!fixture.m_operations->try_pop());
  }
}
