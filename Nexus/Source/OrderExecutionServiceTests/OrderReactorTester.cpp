#include <future>
#include <Aspen/Aspen.hpp>
#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/OrderReactor.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionClient.hpp"

using namespace Aspen;
using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

namespace {
  struct Fixture {
    Beam::Queue<bool> m_commits;
    Trigger m_trigger;
    optional<Aspen::Shared<Aspen::Box<std::shared_ptr<Order>>>> m_order;
    int m_sequence;
    OrderId m_next_id;
    std::shared_ptr<Beam::Queue<
      std::shared_ptr<TestOrderExecutionClient::Operation>>> m_operations;
    TestOrderExecutionClient m_client;

    Fixture()
      : m_trigger([&] {
          m_commits.push(true);
        }),
        m_sequence(0),
        m_next_id(123),
        m_operations(std::make_shared<Beam::Queue<
          std::shared_ptr<TestOrderExecutionClient::Operation>>>()),
        m_client(m_operations) {
      Trigger::set_trigger(m_trigger);
    }

    void set(Aspen::Shared<Aspen::Box<std::shared_ptr<Order>>> order) {
      m_order.emplace(std::move(order));
    }

    void require_state(Aspen::State expected_state) {
      if(m_sequence != 0) {
        m_commits.pop();
      }
      REQUIRE(m_order->commit(m_sequence) == expected_state);
      ++m_sequence;
    }

    template<typename F>
    std::shared_ptr<PrimitiveOrder> require_submit(F&& expect) {
      auto submit = std::async(std::launch::async, [&] {
        auto operation = m_operations->pop();
        auto submit = std::get_if<TestOrderExecutionClient::SubmitOperation>(
          operation.get());
        REQUIRE(submit);
        std::forward<F>(expect)(submit->m_fields);
        auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
          submit->m_fields, m_next_id,
          time_from_string("2025-05-06 10:12:13:00")));
        ++m_next_id;
        submit->m_result.set(order);
        return order;
      });
      require_state(Aspen::State::EVALUATED);
      auto expected_order = submit.get();
      REQUIRE(m_order->eval() == expected_order);
      require_state(Aspen::State::NONE);
      return expected_order;
    }

    void require_cancel(PrimitiveOrder& order) {
      auto cancel_async = std::async(std::launch::async, [&] {
        auto operation = m_operations->pop();
        auto cancel = std::get_if<TestOrderExecutionClient::CancelOperation>(
          operation.get());
        REQUIRE(cancel);
        REQUIRE(cancel->m_id == order.get_info().m_id);
      });
      cancel_async.get();
      set_order_status(order, OrderStatus::PENDING_CANCEL);
      require_state(Aspen::State::NONE);
      cancel(order);
    }
  };
}

TEST_SUITE("OrderReactor") {
  TEST_CASE("empty_quantity") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 0, Money::ONE);
    auto quantity_reactor = Aspen::constant(Quantity(0));
    fixture.set(Aspen::Shared(make_limit_order_reactor(&fixture.m_client,
      Aspen::constant(fields.m_account), Aspen::constant(fields.m_ticker),
      Aspen::constant(fields.m_currency), Aspen::constant(fields.m_side),
      Aspen::constant(fields.m_destination), quantity_reactor,
      Aspen::constant(fields.m_price),
      Aspen::constant(fields.m_time_in_force))));
    fixture.require_state(Aspen::State::COMPLETE);
  }

  TEST_CASE("single_limit_order_submission") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    fixture.set(Aspen::Shared(make_limit_order_reactor(&fixture.m_client,
      Aspen::constant(fields.m_account), Aspen::constant(fields.m_ticker),
      Aspen::constant(fields.m_currency), Aspen::constant(fields.m_side),
      Aspen::constant(fields.m_destination), Aspen::constant(fields.m_quantity),
      Aspen::constant(fields.m_price),
      Aspen::constant(fields.m_time_in_force))));
    auto expected_order = fixture.require_submit([&] (const auto& fields) {
      REQUIRE(fields.m_ticker == ticker);
      REQUIRE(fields.m_quantity == 100);
      REQUIRE(fields.m_price == Money::ONE);
      REQUIRE(fields.m_side == Side::BID);
      REQUIRE(fields.m_currency == CAD);
      REQUIRE(fields.m_destination == "TSX");
    });
    accept(*expected_order);
    fixture.require_state(Aspen::State::NONE);
    fill(*expected_order, 100);
    fixture.require_state(Aspen::State::COMPLETE);
  }

  TEST_CASE("order_partial_fill_and_resubmit") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto quantity = Shared<Aspen::Queue<Quantity>>();
    quantity->push(100);
    fixture.set(Aspen::Shared(make_limit_order_reactor(&fixture.m_client,
      Aspen::constant(fields.m_account), Aspen::constant(fields.m_ticker),
      Aspen::constant(fields.m_currency), Aspen::constant(fields.m_side),
      Aspen::constant(fields.m_destination), quantity,
      Aspen::constant(fields.m_price),
      Aspen::constant(fields.m_time_in_force))));
    auto order1 = fixture.require_submit([&] (const auto& fields) {
      REQUIRE(fields.m_quantity == 100);
    });
    accept(*order1);
    fixture.require_state(Aspen::State::NONE);
    fill(*order1, 40);
    fixture.require_state(Aspen::State::NONE);
    quantity->set_complete(Quantity(70));
    fixture.require_state(Aspen::State::NONE);
    fixture.require_cancel(*order1);
    auto order2 = fixture.require_submit([&] (const auto& fields) {
      REQUIRE(fields.m_quantity == 30);
    });
    accept(*order2);
    fixture.require_state(Aspen::State::NONE);
    fill(*order2, 30);
    fixture.require_state(Aspen::State::COMPLETE);
  }

  TEST_CASE("order_rejection") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    fixture.set(Aspen::Shared(make_limit_order_reactor(&fixture.m_client,
      Aspen::constant(fields.m_account), Aspen::constant(fields.m_ticker),
      Aspen::constant(fields.m_currency), Aspen::constant(fields.m_side),
      Aspen::constant(fields.m_destination), Aspen::constant(fields.m_quantity),
      Aspen::constant(fields.m_price),
      Aspen::constant(fields.m_time_in_force))));
    auto order = fixture.require_submit([&] (const auto& fields) {});
    accept(*order);
    fixture.require_state(Aspen::State::NONE);
    reject(*order);
    fixture.require_state(Aspen::State::COMPLETE_EVALUATED);
    REQUIRE_THROWS(fixture.m_order->eval());
  }

  TEST_CASE("order_cancellation") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto quantity = Shared<Aspen::Queue<Quantity>>();
    quantity->push(100);
    fixture.set(Aspen::Shared(make_limit_order_reactor(&fixture.m_client,
      Aspen::constant(fields.m_account), Aspen::constant(fields.m_ticker),
      Aspen::constant(fields.m_currency), Aspen::constant(fields.m_side),
      Aspen::constant(fields.m_destination), quantity,
      Aspen::constant(fields.m_price),
      Aspen::constant(fields.m_time_in_force))));
    auto order = fixture.require_submit([&] (const auto& fields) {});
    accept(*order);
    fixture.require_state(Aspen::State::NONE);
    quantity->set_complete(Quantity(0));
    fixture.require_state(Aspen::State::NONE);
    fixture.require_cancel(*order);
    fixture.require_state(Aspen::State::COMPLETE);
  }

  TEST_CASE("fields_update_triggers_resubmission") {
    auto fixture = Fixture();
    auto ticker = Shared<Aspen::Queue<Ticker>>();
    ticker->push(parse_ticker("TST.TSX"));
    auto fields = make_limit_order_fields(
      parse_ticker("TST.TSX"), CAD, Side::BID, "TSX", 100, Money::ONE);
    fixture.set(Aspen::Shared(make_limit_order_reactor(&fixture.m_client,
      Aspen::constant(fields.m_account), ticker,
      Aspen::constant(fields.m_currency), Aspen::constant(fields.m_side),
      Aspen::constant(fields.m_destination), Aspen::constant(fields.m_quantity),
      Aspen::constant(fields.m_price),
      Aspen::constant(fields.m_time_in_force))));
    auto order1 = fixture.require_submit([&] (const auto& fields) {
      REQUIRE(fields.m_ticker == parse_ticker("TST.TSX"));
    });
    accept(*order1);
    fixture.require_state(Aspen::State::NONE);
    ticker->push(parse_ticker("TST2.TSX"));
    fixture.require_state(Aspen::State::NONE);
    fixture.require_cancel(*order1);
    auto order2 = fixture.require_submit([&] (const auto& fields) {
      REQUIRE(fields.m_ticker == parse_ticker("TST2.TSX"));
    });
    accept(*order2);
    fixture.require_state(Aspen::State::NONE);
    fill(*order2, 100);
    fixture.require_state(Aspen::State::COMPLETE);
  }
}
