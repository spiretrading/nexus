#include "Nexus/OrderExecutionServiceTests/OrderReactorTester.hpp"
#include <Aspen/Aspen.hpp>
#include "Nexus/OrderExecutionService/OrderReactor.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  auto TEST_SECURITY = Security("TST", DefaultMarkets::TSX(),
    DefaultCountries::CA());
}

void OrderReactorTester::TestEmptyOrderFields() {
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
  CPPUNIT_ASSERT(reactor.commit(0) == Aspen::State::COMPLETE);
  Trigger::set_trigger(nullptr);
}

void OrderReactorTester::TestSingleOrderFields() {
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
  CPPUNIT_ASSERT(reactor.commit(0) == Aspen::State::NONE);
  auto order = orderSubmissions->Top();
  orderSubmissions->Pop();
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_security == TEST_SECURITY);
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_side == Side::BID);
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_quantity == 100);
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_price == Money::ONE);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(1) == Aspen::State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status ==
    OrderStatus::PENDING_NEW);
  environment.AcceptOrder(*order);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(2) == Aspen::State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status == OrderStatus::NEW);
  environment.FillOrder(*order, 100);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(3) == Aspen::State::COMPLETE_EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status ==
    OrderStatus::FILLED);
  Trigger::set_trigger(nullptr);
}

void OrderReactorTester::TestOrderFieldsUpdate() {
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
  CPPUNIT_ASSERT(reactor.commit(0) == Aspen::State::NONE);
  auto order = orderSubmissions->Top();
  orderSubmissions->Pop();
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_security == TEST_SECURITY);
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_side == Side::ASK);
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_quantity == 300);
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_price == Money::CENT);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(1) == Aspen::State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status ==
    OrderStatus::PENDING_NEW);
  environment.AcceptOrder(*order);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(2) == Aspen::State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status == OrderStatus::NEW);
  price->set_complete(2 * Money::ONE);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(3) == Aspen::State::NONE);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(4) == Aspen::State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status ==
    OrderStatus::PENDING_CANCEL);
  environment.CancelOrder(*order);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(5) == Aspen::State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status ==
    OrderStatus::CANCELED);
  auto updatedOrder = orderSubmissions->Top();
  orderSubmissions->Pop();
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_security == TEST_SECURITY);
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_side == Side::ASK);
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_quantity == 300);
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_price == 2 * Money::ONE);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(6) == Aspen::State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status ==
    OrderStatus::PENDING_NEW);
  environment.AcceptOrder(*updatedOrder);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(7) == Aspen::State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status == OrderStatus::NEW);
  environment.FillOrder(*updatedOrder, 300);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(8) == Aspen::State::COMPLETE_EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status ==
    OrderStatus::FILLED);
  Trigger::set_trigger(nullptr);
}

void OrderReactorTester::TestPartialFillAndUpdate() {
}

void OrderReactorTester::TestTerminalOrderAndUpdate() {
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
  CPPUNIT_ASSERT(reactor.commit(0) == Aspen::State::NONE);
  auto order = orderSubmissions->Top();
  orderSubmissions->Pop();
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(1) == Aspen::State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status ==
    OrderStatus::PENDING_NEW);
  environment.AcceptOrder(*order);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(2) == Aspen::State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status == OrderStatus::NEW);
  environment.RejectOrder(*order);
  commits.Top();
  commits.Pop();
  price->set_complete(2 * Money::ONE);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(3) == Aspen::State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status ==
    OrderStatus::REJECTED);
  auto updatedOrder = orderSubmissions->Top();
  orderSubmissions->Pop();
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_security == TEST_SECURITY);
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_side == Side::ASK);
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_quantity == 300);
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_price == 2 * Money::ONE);
  Trigger::set_trigger(nullptr);
}
