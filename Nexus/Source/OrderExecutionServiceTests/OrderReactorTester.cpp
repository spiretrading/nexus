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
  const auto SECURITY_A = Security("TST", DefaultMarkets::TSX(),
    DefaultCountries::CA());
}

TEST_SUITE("OrderReactor") {
  TEST_CASE("empty_order_fields") {
    auto trigger = Trigger();
    Trigger::set_trigger(trigger);
    auto environment = TestEnvironment();
    auto serviceClients = TestServiceClients(Ref(environment));
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
    auto serviceClients = TestServiceClients(Ref(environment));
    auto orderSubmissions = std::make_shared<Beam::Queue<const Order*>>();
    environment.MonitorOrderSubmissions(orderSubmissions);
    auto reactor = MakeLimitOrderReactor(
      Ref(serviceClients.GetOrderExecutionClient()),
      Aspen::constant(SECURITY_A), Aspen::constant(Side::BID),
      Aspen::constant(100), Aspen::constant(Money::ONE));
    REQUIRE(reactor.commit(0) == Aspen::State::EVALUATED);
    auto sentOrder = reactor.eval();
    REQUIRE(sentOrder->GetInfo().m_fields.m_security == SECURITY_A);
    REQUIRE(sentOrder->GetInfo().m_fields.m_side == Side::BID);
    REQUIRE(sentOrder->GetInfo().m_fields.m_quantity == 100);
    REQUIRE(sentOrder->GetInfo().m_fields.m_price == Money::ONE);
    auto receivedOrder = orderSubmissions->Pop();
    commits.Pop();
    REQUIRE(reactor.commit(1) == Aspen::State::NONE);
    environment.Accept(*receivedOrder);
    commits.Pop();
    REQUIRE(reactor.commit(2) == Aspen::State::NONE);
    environment.Fill(*receivedOrder, 100);
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
    auto serviceClients = TestServiceClients(Ref(environment));
    auto orderSubmissions = std::make_shared<Beam::Queue<const Order*>>();
    environment.MonitorOrderSubmissions(orderSubmissions);
    auto price = Aspen::Shared<Aspen::Queue<Money>>();
    price->push(Money::CENT);
    auto reactor = MakeLimitOrderReactor(
      Ref(serviceClients.GetOrderExecutionClient()),
      Aspen::constant(SECURITY_A), Aspen::constant(Side::ASK),
      Aspen::constant(300), price);
    REQUIRE(reactor.commit(0) == Aspen::State::EVALUATED);
    auto sentOrder = reactor.eval();
    REQUIRE(sentOrder->GetInfo().m_fields.m_security == SECURITY_A);
    REQUIRE(sentOrder->GetInfo().m_fields.m_side == Side::ASK);
    REQUIRE(sentOrder->GetInfo().m_fields.m_quantity == 300);
    REQUIRE(sentOrder->GetInfo().m_fields.m_price == Money::CENT);
    auto receivedOrder = orderSubmissions->Pop();
    commits.Pop();
    REQUIRE(reactor.commit(1) == Aspen::State::NONE);
    environment.Accept(*receivedOrder);
    commits.Pop();
    REQUIRE(reactor.commit(2) == Aspen::State::NONE);
    price->set_complete(2 * Money::ONE);
    commits.Pop();
    REQUIRE(reactor.commit(3) == Aspen::State::NONE);
    commits.Pop();
    environment.Cancel(*receivedOrder);
    commits.Pop();
    REQUIRE(reactor.commit(4) == Aspen::State::CONTINUE);
    REQUIRE(reactor.commit(5) == Aspen::State::EVALUATED);
    auto updatedSentOrder = reactor.eval();
    REQUIRE(updatedSentOrder->GetInfo().m_fields.m_security ==
      SECURITY_A);
    REQUIRE(updatedSentOrder->GetInfo().m_fields.m_side == Side::ASK);
    REQUIRE(updatedSentOrder->GetInfo().m_fields.m_quantity == 300);
    REQUIRE(updatedSentOrder->GetInfo().m_fields.m_price ==
      2 * Money::ONE);
    auto updatedReceivedOrder = orderSubmissions->Pop();
    commits.Pop();
    REQUIRE(reactor.commit(6) == Aspen::State::NONE);
    environment.Accept(*updatedReceivedOrder);
    commits.Pop();
    REQUIRE(reactor.commit(7) == Aspen::State::NONE);
    environment.Fill(*updatedReceivedOrder, 300);
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
    auto serviceClients = TestServiceClients(Ref(environment));
    auto orderSubmissions = std::make_shared<Beam::Queue<const Order*>>();
    environment.MonitorOrderSubmissions(orderSubmissions);
    auto price = Aspen::Shared<Aspen::Queue<Money>>();
    price->push(Money::CENT);
    auto reactor = MakeLimitOrderReactor(
      Ref(serviceClients.GetOrderExecutionClient()),
      Aspen::constant(SECURITY_A), Aspen::constant(Side::ASK),
      Aspen::constant(300), price);
    REQUIRE(reactor.commit(0) == Aspen::State::EVALUATED);
    auto sentOrder = reactor.eval();
    auto receivedOrder = orderSubmissions->Pop();
    commits.Pop();
    REQUIRE(reactor.commit(1) == Aspen::State::NONE);
    environment.Accept(*receivedOrder);
    commits.Pop();
    REQUIRE(reactor.commit(2) == Aspen::State::NONE);
    environment.Reject(*receivedOrder);
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
    auto serviceClients = TestServiceClients(Ref(environment));
    auto orderSubmissions = std::make_shared<Beam::Queue<const Order*>>();
    environment.MonitorOrderSubmissions(orderSubmissions);
    auto quantity = Aspen::Shared<Aspen::Queue<Quantity>>();
    auto reactor = MakeLimitOrderReactor(
      Ref(serviceClients.GetOrderExecutionClient()),
      Aspen::constant(SECURITY_A), Aspen::constant(Side::ASK),
      quantity, Aspen::constant(Money::ONE));
    REQUIRE(reactor.commit(0) == Aspen::State::NONE);
    quantity->push(1200);
    REQUIRE(reactor.commit(1) == Aspen::State::EVALUATED);
    auto sentOrder = reactor.eval();
    auto receivedOrder = orderSubmissions->Pop();
    commits.Pop();
    REQUIRE(sentOrder->GetInfo().m_fields.m_quantity == 1200);
    Trigger::set_trigger(nullptr);
  }
}
