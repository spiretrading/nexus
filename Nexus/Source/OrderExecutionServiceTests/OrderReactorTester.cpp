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
  Trigger::set_trigger(nullptr);
}
