#include <Aspen/Aspen.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/OrderReactor.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  const auto TEST_SECURITY = Security("TST", DefaultMarkets::TSX(),
    DefaultCountries::CA());
}

TEST_SUITE("OrderReactor") {
  TEST_CASE("empty_order_fields") {
    auto trigger = Trigger();
    Trigger::set_trigger(trigger);
    auto environment = TestEnvironment();
    environment.Open();
    auto serviceClients = TestServiceClients(Ref(environment));
    serviceClients.Open();
    auto reactor = MakeLimitOrderReactor(
      Ref(serviceClients.GetOrderExecutionClient()), Aspen::none<Security>(),
      Aspen::constant(Side::BID), Aspen::constant(100),
      Aspen::constant(Money::ONE));
    REQUIRE(reactor.commit(0) == Aspen::State::COMPLETE);
    Trigger::set_trigger(nullptr);
  }

  TEST_CASE("single_order_fields") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto environment = TestEnvironment();
    environment.Open();
    auto serviceClients = TestServiceClients(Ref(environment));
    serviceClients.Open();
    auto orderSubmissions = std::make_shared<Beam::Queue<const Order*>>();
    environment.MonitorOrderSubmissions(orderSubmissions);
    auto reactor = MakeLimitOrderReactor(
      Ref(serviceClients.GetOrderExecutionClient()),
      Aspen::constant(TEST_SECURITY), Aspen::constant(Side::BID),
      Aspen::constant(100), Aspen::constant(Money::ONE));
    REQUIRE(reactor.commit(0) == Aspen::State::EVALUATED);
    auto sentOrder = reactor.eval();
    REQUIRE(sentOrder->GetInfo().m_fields.m_security == TEST_SECURITY);
    REQUIRE(sentOrder->GetInfo().m_fields.m_side == Side::BID);
    REQUIRE(sentOrder->GetInfo().m_fields.m_quantity == 100);
    REQUIRE(sentOrder->GetInfo().m_fields.m_price == Money::ONE);
    auto receivedOrder = orderSubmissions->Top();
    orderSubmissions->Pop();
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(1) == Aspen::State::NONE);
    environment.AcceptOrder(*receivedOrder);
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(2) == Aspen::State::NONE);
    environment.FillOrder(*receivedOrder, 100);
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(3) == Aspen::State::COMPLETE);
    Trigger::set_trigger(nullptr);
  }

  TEST_CASE("order_fields_update") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto environment = TestEnvironment();
    environment.Open();
    auto serviceClients = TestServiceClients(Ref(environment));
    serviceClients.Open();
    auto orderSubmissions = std::make_shared<Beam::Queue<const Order*>>();
    environment.MonitorOrderSubmissions(orderSubmissions);
    auto price = Aspen::Shared<Aspen::Queue<Money>>();
    price->push(Money::CENT);
    auto reactor = MakeLimitOrderReactor(
      Ref(serviceClients.GetOrderExecutionClient()),
      Aspen::constant(TEST_SECURITY), Aspen::constant(Side::ASK),
      Aspen::constant(300), price);
    REQUIRE(reactor.commit(0) == Aspen::State::EVALUATED);
    auto sentOrder = reactor.eval();
    REQUIRE(sentOrder->GetInfo().m_fields.m_security == TEST_SECURITY);
    REQUIRE(sentOrder->GetInfo().m_fields.m_side == Side::ASK);
    REQUIRE(sentOrder->GetInfo().m_fields.m_quantity == 300);
    REQUIRE(sentOrder->GetInfo().m_fields.m_price == Money::CENT);
    auto receivedOrder = orderSubmissions->Top();
    orderSubmissions->Pop();
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(1) == Aspen::State::NONE);
    environment.AcceptOrder(*receivedOrder);
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(2) == Aspen::State::NONE);
    price->set_complete(2 * Money::ONE);
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(3) == Aspen::State::NONE);
    commits.Top();
    commits.Pop();
    environment.CancelOrder(*receivedOrder);
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(4) == Aspen::State::CONTINUE);
    REQUIRE(reactor.commit(5) == Aspen::State::EVALUATED);
    auto updatedSentOrder = reactor.eval();
    REQUIRE(updatedSentOrder->GetInfo().m_fields.m_security ==
      TEST_SECURITY);
    REQUIRE(updatedSentOrder->GetInfo().m_fields.m_side == Side::ASK);
    REQUIRE(updatedSentOrder->GetInfo().m_fields.m_quantity == 300);
    REQUIRE(updatedSentOrder->GetInfo().m_fields.m_price ==
      2 * Money::ONE);
    auto updatedReceivedOrder = orderSubmissions->Top();
    orderSubmissions->Pop();
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(6) == Aspen::State::NONE);
    environment.AcceptOrder(*updatedReceivedOrder);
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(7) == Aspen::State::NONE);
    environment.FillOrder(*updatedReceivedOrder, 300);
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(8) == Aspen::State::COMPLETE);
    Trigger::set_trigger(nullptr);
  }

  TEST_CASE("partial_fill_and_update") {
  }

  TEST_CASE("terminal_order_and_update") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto environment = TestEnvironment();
    environment.Open();
    auto serviceClients = TestServiceClients(Ref(environment));
    serviceClients.Open();
    auto orderSubmissions = std::make_shared<Beam::Queue<const Order*>>();
    environment.MonitorOrderSubmissions(orderSubmissions);
    auto price = Aspen::Shared<Aspen::Queue<Money>>();
    price->push(Money::CENT);
    auto reactor = MakeLimitOrderReactor(
      Ref(serviceClients.GetOrderExecutionClient()),
      Aspen::constant(TEST_SECURITY), Aspen::constant(Side::ASK),
      Aspen::constant(300), price);
    REQUIRE(reactor.commit(0) == Aspen::State::EVALUATED);
    auto sentOrder = reactor.eval();
    auto receivedOrder = orderSubmissions->Top();
    orderSubmissions->Pop();
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(1) == Aspen::State::NONE);
    environment.AcceptOrder(*receivedOrder);
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(2) == Aspen::State::NONE);
    environment.RejectOrder(*receivedOrder);
    commits.Top();
    commits.Pop();
    REQUIRE(reactor.commit(3) == Aspen::State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
    Trigger::set_trigger(nullptr);
  }

  TEST_CASE("delayed_quantity") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto environment = TestEnvironment();
    environment.Open();
    auto serviceClients = TestServiceClients(Ref(environment));
    serviceClients.Open();
    auto orderSubmissions = std::make_shared<Beam::Queue<const Order*>>();
    environment.MonitorOrderSubmissions(orderSubmissions);
    auto quantity = Aspen::Shared<Aspen::Queue<Quantity>>();
    auto reactor = MakeLimitOrderReactor(
      Ref(serviceClients.GetOrderExecutionClient()),
      Aspen::constant(TEST_SECURITY), Aspen::constant(Side::ASK),
      quantity, Aspen::constant(Money::ONE));
    REQUIRE(reactor.commit(0) == Aspen::State::NONE);
    quantity->push(1200);
    REQUIRE(reactor.commit(1) == Aspen::State::EVALUATED);
    auto sentOrder = reactor.eval();
    auto receivedOrder = orderSubmissions->Top();
    orderSubmissions->Pop();
    commits.Top();
    commits.Pop();
    REQUIRE(sentOrder->GetInfo().m_fields.m_quantity == 1200);
    Trigger::set_trigger(nullptr);
  }
}
