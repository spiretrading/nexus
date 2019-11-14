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
  auto reactor = OrderReactor(Ref(serviceClients.GetOrderExecutionClient()),
    Aspen::none<OrderFields>());
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
  auto testFields = OrderFields::BuildLimitOrder(TEST_SECURITY, Side::BID, 100,
    Money::ONE);
  auto reactor = OrderReactor(Ref(serviceClients.GetOrderExecutionClient()),
    Aspen::constant(testFields));
  CPPUNIT_ASSERT(reactor.commit(0) == Aspen::State::NONE);
  auto order = orderSubmissions->Top();
  orderSubmissions->Pop();
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_security == testFields.m_security);
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_side == testFields.m_side);
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_quantity == testFields.m_quantity);
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_price == testFields.m_price);
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
  environment.FillOrder(*order, testFields.m_quantity);
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
  auto testFields = OrderFields::BuildLimitOrder(TEST_SECURITY, Side::BID, 100,
    Money::ONE);
  auto fieldsReactor = Aspen::Shared<Aspen::Queue<OrderFields>>();
  auto reactor = OrderReactor(Ref(serviceClients.GetOrderExecutionClient()),
    fieldsReactor);
  fieldsReactor->push(testFields);
  CPPUNIT_ASSERT(reactor.commit(0) == Aspen::State::NONE);
  auto order = orderSubmissions->Top();
  orderSubmissions->Pop();
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_security == testFields.m_security);
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_side == testFields.m_side);
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_quantity == testFields.m_quantity);
  CPPUNIT_ASSERT(order->GetInfo().m_fields.m_price == testFields.m_price);
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
  auto testFieldsUpdate = OrderFields::BuildLimitOrder(TEST_SECURITY, Side::BID,
    100, 2 * Money::ONE);
  fieldsReactor->set_complete(testFieldsUpdate);
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
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_security ==
    testFieldsUpdate.m_security);
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_side ==
    testFieldsUpdate.m_side);
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_quantity ==
    testFieldsUpdate.m_quantity);
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_price ==
    testFieldsUpdate.m_price);
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
  environment.FillOrder(*updatedOrder, testFieldsUpdate.m_quantity);
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
  auto testFields = OrderFields::BuildLimitOrder(TEST_SECURITY, Side::BID, 100,
    Money::ONE);
  auto fieldsReactor = Aspen::Shared<Aspen::Queue<OrderFields>>();
  auto reactor = OrderReactor(Ref(serviceClients.GetOrderExecutionClient()),
    fieldsReactor);
  fieldsReactor->push(testFields);
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
  auto testFieldsUpdate = OrderFields::BuildLimitOrder(TEST_SECURITY, Side::BID,
    100, 2 * Money::ONE);
  fieldsReactor->set_complete(testFieldsUpdate);
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(reactor.commit(3) == Aspen::State::EVALUATED);
  CPPUNIT_ASSERT(reactor.eval().m_executionReport.m_status ==
    OrderStatus::REJECTED);
  auto updatedOrder = orderSubmissions->Top();
  orderSubmissions->Pop();
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_security ==
    testFieldsUpdate.m_security);
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_side ==
    testFieldsUpdate.m_side);
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_quantity ==
    testFieldsUpdate.m_quantity);
  CPPUNIT_ASSERT(updatedOrder->GetInfo().m_fields.m_price ==
    testFieldsUpdate.m_price);
  Trigger::set_trigger(nullptr);
}
