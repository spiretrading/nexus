#include <future>
#include <Aspen/Aspen.hpp>
#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
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
using namespace Nexus::DefaultVenues;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  struct Fixture {
    Beam::Queue<bool> m_commits;
    Trigger m_trigger;
    optional<Aspen::Shared<Aspen::Box<std::shared_ptr<const Order>>>> m_order;
    int m_sequence;
    std::shared_ptr<Beam::Queue<
      std::shared_ptr<TestOrderExecutionClient::Operation>>> m_operations;
    TestOrderExecutionClient m_client;

    Fixture()
      : m_trigger([&] {
          m_commits.Push(true);
        }),
        m_sequence(0),
        m_operations(std::make_shared<Beam::Queue<
          std::shared_ptr<TestOrderExecutionClient::Operation>>>()),
        m_client(m_operations) {
      Trigger::set_trigger(m_trigger);
    }

    void set(Aspen::Shared<Aspen::Box<std::shared_ptr<const Order>>> order) {
      m_order.emplace(std::move(order));
    }

    void require_state(Aspen::State expected_state) {
      if(m_sequence != 0) {
        m_commits.Pop();
      }
      REQUIRE(m_order->commit(m_sequence) == expected_state);
      ++m_sequence;
    }
  };
}

TEST_SUITE("OrderReactor") {
  TEST_CASE("empty_quantity") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestOrderExecutionClient::Operation>>>();
    auto client = TestOrderExecutionClient(operations);
    auto security = Security("TST", TSX);
    auto fields =
      make_limit_order_fields(security, CAD, Side::BID, "TSX", 0, Money::ONE);
    auto quantity_reactor = Aspen::constant(Quantity(0));
    auto reactor = make_limit_order_reactor(&client,
      Aspen::constant(fields.m_account), Aspen::constant(fields.m_security),
      Aspen::constant(fields.m_currency), Aspen::constant(fields.m_side),
      Aspen::constant(fields.m_destination), quantity_reactor,
      Aspen::constant(fields.m_price), Aspen::constant(fields.m_time_in_force));
    auto state = reactor.commit(0);
    REQUIRE(Aspen::is_complete(state));
    REQUIRE(!operations->TryPop());
  }

  TEST_CASE("single_limit_order_submission") {
    auto fixture = Fixture();
    auto security = Security("TST", TSX);
    auto fields =
      make_limit_order_fields(security, CAD, Side::BID, "TSX", 100, Money::ONE);
    fixture.set(Aspen::Shared(make_limit_order_reactor(&fixture.m_client,
      Aspen::constant(fields.m_account), Aspen::constant(fields.m_security),
      Aspen::constant(fields.m_currency), Aspen::constant(fields.m_side),
      Aspen::constant(fields.m_destination), Aspen::constant(fields.m_quantity),
      Aspen::constant(fields.m_price),
      Aspen::constant(fields.m_time_in_force))));
    auto submit = std::async(std::launch::async, [&] {
      auto operation = fixture.m_operations->Pop();
      auto submit =
        std::get_if<TestOrderExecutionClient::SubmitOperation>(operation.get());
      REQUIRE(submit);
      REQUIRE(submit->m_fields.m_security == security);
      REQUIRE(submit->m_fields.m_quantity == 100);
      REQUIRE(submit->m_fields.m_price == Money::ONE);
      REQUIRE(submit->m_fields.m_side == Side::BID);
      REQUIRE(submit->m_fields.m_currency == CAD);
      REQUIRE(submit->m_fields.m_destination == "TSX");
      auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
        submit->m_fields, 123, time_from_string("2025-05-06 10:12:13:00")));
      submit->m_result.set(order);
      return order;
    });
    fixture.require_state(Aspen::State::EVALUATED);
    auto expected_order = submit.get();
    REQUIRE(fixture.m_order->eval() == expected_order);
    fixture.require_state(Aspen::State::NONE);
    accept(*expected_order);
    fixture.require_state(Aspen::State::NONE);
    fill(*expected_order, 100);
    fixture.require_state(Aspen::State::COMPLETE);
  }

  TEST_CASE("order_partial_fill_and_resubmit") {
    auto fixture = Fixture();
    auto security = Security("TST", TSX);
    auto fields =
      make_limit_order_fields(security, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto quantity = Shared<Aspen::Queue<Quantity>>();
    quantity->push(100);
    fixture.set(Aspen::Shared(make_limit_order_reactor(&fixture.m_client,
      Aspen::constant(fields.m_account), Aspen::constant(fields.m_security),
      Aspen::constant(fields.m_currency), Aspen::constant(fields.m_side),
      Aspen::constant(fields.m_destination), quantity,
      Aspen::constant(fields.m_price),
      Aspen::constant(fields.m_time_in_force))));
    auto submit1 = std::async(std::launch::async, [&] {
      auto operation = fixture.m_operations->Pop();
      auto submit =
        std::get_if<TestOrderExecutionClient::SubmitOperation>(operation.get());
      REQUIRE(submit);
      REQUIRE(submit->m_fields.m_quantity == 100);
      auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
        submit->m_fields, 123, time_from_string("2025-05-06 10:12:13:00")));
      submit->m_result.set(order);
      return order;
    });
    fixture.require_state(Aspen::State::EVALUATED);
    auto order1 = submit1.get();
    REQUIRE(fixture.m_order->eval() == order1);
    fixture.require_state(Aspen::State::NONE);
    accept(*order1);
    fixture.require_state(Aspen::State::NONE);
    fill(*order1, 40);
    fixture.require_state(Aspen::State::NONE);
    quantity->set_complete(70);
    auto cancel_async = std::async(std::launch::async, [&] {
      auto operation = fixture.m_operations->Pop();
      auto cancel =
        std::get_if<TestOrderExecutionClient::CancelOperation>(operation.get());
      REQUIRE(cancel);
      REQUIRE(cancel->m_id == 123);
    });
    fixture.require_state(Aspen::State::NONE);
    set_order_status(*order1, OrderStatus::PENDING_CANCEL);
    fixture.require_state(Aspen::State::NONE);
    cancel(*order1);
    auto submit2 = std::async(std::launch::async, [&] {
      auto operation = fixture.m_operations->Pop();
      auto submit =
        std::get_if<TestOrderExecutionClient::SubmitOperation>(operation.get());
      REQUIRE(submit);
      REQUIRE(submit->m_fields.m_quantity == 30);
      auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
        submit->m_fields, 124, time_from_string("2025-05-06 10:13:00:00")));
      submit->m_result.set(order);
      return order;
    });
    fixture.require_state(Aspen::State::EVALUATED);
    auto order2 = submit2.get();
    REQUIRE(fixture.m_order->eval() == order2);
    fixture.require_state(Aspen::State::NONE);
    accept(*order2);
    fixture.require_state(Aspen::State::NONE);
    fill(*order2, 30);
    fixture.require_state(Aspen::State::COMPLETE);
  }

  TEST_CASE("order_rejection") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.Push(true);
    });
    Trigger::set_trigger(trigger);
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestOrderExecutionClient::Operation>>>();
    auto client = TestOrderExecutionClient(operations);
    auto security = Security("TST", TSX);
    auto fields =
      make_limit_order_fields(security, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto reactor = make_limit_order_reactor(&client,
      Aspen::constant(fields.m_account), Aspen::constant(fields.m_security),
      Aspen::constant(fields.m_currency), Aspen::constant(fields.m_side),
      Aspen::constant(fields.m_destination), Aspen::constant(fields.m_quantity),
      Aspen::constant(fields.m_price), Aspen::constant(fields.m_time_in_force));
    auto submit = std::async(std::launch::async, [&] {
      auto operation = operations->Pop();
      auto submit =
        std::get_if<TestOrderExecutionClient::SubmitOperation>(operation.get());
      REQUIRE(submit);
      auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
        submit->m_fields, 123, time_from_string("2025-05-06 10:12:13:00")));
      submit->m_result.set(order);
      return order;
    });
    auto state = reactor.commit(0);
    REQUIRE(state == Aspen::State::EVALUATED);
    auto order = submit.get();
    REQUIRE(reactor.eval() == order);
    accept(*order);
    commits.Pop();
    reject(*order);
    commits.Pop();
    state = reactor.commit(1);
    REQUIRE(state == Aspen::State::COMPLETE);
  }

  TEST_CASE("order_cancellation") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.Push(true);
    });
    Trigger::set_trigger(trigger);
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestOrderExecutionClient::Operation>>>();
    auto client = TestOrderExecutionClient(operations);
    auto security = Security("TST", TSX);
    auto fields =
      make_limit_order_fields(security, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto quantity = Shared<Aspen::Queue<Quantity>>();
    quantity->push(100);
    auto reactor = make_limit_order_reactor(&client,
      Aspen::constant(fields.m_account), Aspen::constant(fields.m_security),
      Aspen::constant(fields.m_currency), Aspen::constant(fields.m_side),
      Aspen::constant(fields.m_destination), quantity,
      Aspen::constant(fields.m_price), Aspen::constant(fields.m_time_in_force));
    auto submit = std::async(std::launch::async, [&] {
      auto operation = operations->Pop();
      auto submit =
        std::get_if<TestOrderExecutionClient::SubmitOperation>(operation.get());
      REQUIRE(submit);
      auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
        submit->m_fields, 123, time_from_string("2025-05-06 10:12:13:00")));
      submit->m_result.set(order);
      return order;
    });
    auto state = reactor.commit(0);
    REQUIRE(state == Aspen::State::EVALUATED);
    auto order = submit.get();
    REQUIRE(reactor.eval() == order);
    accept(*order);
    commits.Pop();
    quantity->push(0);
    auto cancel_async = std::async(std::launch::async, [&] {
      auto operation = operations->Pop();
      auto cancel =
        std::get_if<TestOrderExecutionClient::CancelOperation>(operation.get());
      REQUIRE(cancel);
      REQUIRE(cancel->m_id == 123);
    });
    state = reactor.commit(1);
    REQUIRE(state == Aspen::State::NONE);
    cancel_async.get();
    cancel(*order);
    commits.Pop();
    state = reactor.commit(2);
    REQUIRE(state == Aspen::State::COMPLETE);
  }

  TEST_CASE("fields_update_triggers_resubmission") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.Push(true);
    });
    Trigger::set_trigger(trigger);
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestOrderExecutionClient::Operation>>>();
    auto client = TestOrderExecutionClient(operations);
    auto security = Shared<Aspen::Queue<Security>>();
    security->push(Security("TST", TSX));
    auto fields = make_limit_order_fields(
      Security("TST", TSX), CAD, Side::BID, "TSX", 100, Money::ONE);
    auto reactor = make_limit_order_reactor(&client,
      Aspen::constant(fields.m_account), security,
      Aspen::constant(fields.m_currency), Aspen::constant(fields.m_side),
      Aspen::constant(fields.m_destination), Aspen::constant(fields.m_quantity),
      Aspen::constant(fields.m_price), Aspen::constant(fields.m_time_in_force));
    auto submit1 = std::async(std::launch::async, [&] {
      auto operation = operations->Pop();
      auto submit =
        std::get_if<TestOrderExecutionClient::SubmitOperation>(operation.get());
      REQUIRE(submit);
      REQUIRE(submit->m_fields.m_security == Security("TST", TSX));
      auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
        submit->m_fields, 123, time_from_string("2025-05-06 10:12:13:00")));
      submit->m_result.set(order);
      return order;
    });
    auto state = reactor.commit(0);
    REQUIRE(state == Aspen::State::EVALUATED);
    auto order1 = submit1.get();
    REQUIRE(reactor.eval() == order1);
    accept(*order1);
    commits.Pop();
    security->push(Security("TST2", TSX));
    auto cancel_async = std::async(std::launch::async, [&] {
      auto operation = operations->Pop();
      auto cancel =
        std::get_if<TestOrderExecutionClient::CancelOperation>(operation.get());
      REQUIRE(cancel);
      REQUIRE(cancel->m_id == 123);
    });
    state = reactor.commit(1);
    REQUIRE(state == Aspen::State::NONE);
    cancel_async.get();
    cancel(*order1);
    commits.Pop();
    auto submit2 = std::async(std::launch::async, [&] {
      auto operation = operations->Pop();
      auto submit =
        std::get_if<TestOrderExecutionClient::SubmitOperation>(operation.get());
      REQUIRE(submit);
      REQUIRE(submit->m_fields.m_security == Security("TST2", TSX));
      auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
        submit->m_fields, 124, time_from_string("2025-05-06 10:13:00:00")));
      submit->m_result.set(order);
      return order;
    });
    state = reactor.commit(2);
    REQUIRE(state == Aspen::State::EVALUATED);
    auto order2 = submit2.get();
    REQUIRE(reactor.eval() == order2);
    accept(*order2);
    commits.Pop();
    fill(*order2, 100);
    commits.Pop();
    state = reactor.commit(3);
    REQUIRE(state == Aspen::State::COMPLETE);
  }
}
