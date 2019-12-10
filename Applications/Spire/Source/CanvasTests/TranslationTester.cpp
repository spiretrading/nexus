#include "Spire/CanvasTests/TranslationTester.hpp"
#include <Beam/Threading/TimerThreadPool.hpp>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "Spire/Canvas/ControlNodes/ChainNode.hpp"
#include "Spire/Canvas/ControlNodes/UntilNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/Tasks/Executor.hpp"
#include "Spire/Canvas/Tasks/Task.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::Tests;

namespace {
  struct Environment {
    TimerThreadPool m_timerThreadPool;
    TestEnvironment m_environment;
    std::unique_ptr<VirtualServiceClients> m_serviceClients;
    UserProfile m_userProfile;

    Environment()
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_serviceClients(
          [&] {
            m_environment.Open();
            return MakeVirtualServiceClients(
              std::make_unique<TestServiceClients>(Ref(m_environment)));
          }()),
        m_userProfile("", false, false, GetDefaultCountryDatabase(),
          GetDefaultTimeZoneDatabase(), GetDefaultCurrencyDatabase(), {},
          GetDefaultMarketDatabase(), GetDefaultDestinationDatabase(),
          EntitlementDatabase(), Ref(m_timerThreadPool),
          Ref(*m_serviceClients)) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
      m_serviceClients->Open();
    }
  };

  const auto TEST_SECURITY = ParseSecurity("TST.TSX");
}

void TranslationTester::TestTranslatingConstant() {
  auto environment = Environment();
  auto value = IntegerNode(100);
  auto executor = Executor();
  auto context = CanvasNodeTranslationContext(Ref(environment.m_userProfile),
    Ref(executor), DirectoryEntry());
  auto translation = Translate(context, value);
  auto result = translation.Extract<Aspen::Box<Quantity>>();
  CPPUNIT_ASSERT(result.commit(0) == Aspen::State::COMPLETE_EVALUATED);
  CPPUNIT_ASSERT(result.eval() == 100);
}

void TranslationTester::TestTranslatingChain() {
  auto environment = Environment();
  auto chain = std::unique_ptr<CanvasNode>(std::make_unique<ChainNode>());
  chain = chain->Replace("i0", std::make_unique<IntegerNode>(123));
  chain = chain->Replace("i1", std::make_unique<IntegerNode>(456));
  auto executor = Executor();
  auto context = CanvasNodeTranslationContext(Ref(environment.m_userProfile),
    Ref(executor), DirectoryEntry());
  auto translation = Translate(context, *chain);
  auto result = translation.Extract<Aspen::Box<Quantity>>();
  CPPUNIT_ASSERT(result.commit(0) == Aspen::State::CONTINUE_EVALUATED);
  CPPUNIT_ASSERT(result.eval() == 123);
  CPPUNIT_ASSERT(result.commit(1) == Aspen::State::COMPLETE_EVALUATED);
  CPPUNIT_ASSERT(result.eval() == 456);
}

void TranslationTester::TestTranslatingChainWithTailReference() {
  auto environment = Environment();
  auto chain = std::unique_ptr<CanvasNode>(std::make_unique<ChainNode>());
  chain = chain->Replace("i0", std::make_unique<IntegerNode>(123));
  chain = chain->Replace("i1", std::make_unique<ReferenceNode>("i0"));
  auto executor = Executor();
  auto context = CanvasNodeTranslationContext(Ref(environment.m_userProfile),
    Ref(executor), DirectoryEntry());
  auto translation = Translate(context, *chain);
  auto result = translation.Extract<Aspen::Box<Quantity>>();
  CPPUNIT_ASSERT(result.commit(0) == Aspen::State::CONTINUE_EVALUATED);
  CPPUNIT_ASSERT(result.eval() == 123);
  CPPUNIT_ASSERT(result.commit(1) == Aspen::State::COMPLETE_EVALUATED);
  CPPUNIT_ASSERT(result.eval() == 123);
}

void TranslationTester::TestTranslatingChainWithHeadReference() {
  auto environment = Environment();
  auto chain = std::unique_ptr<CanvasNode>(std::make_unique<ChainNode>());
  chain = chain->Replace("i0", std::make_unique<ReferenceNode>("i1"));
  chain = chain->Replace("i1", std::make_unique<IntegerNode>(123));
  chain = chain->Convert(IntegerType::GetInstance());
  auto executor = Executor();
  auto context = CanvasNodeTranslationContext(Ref(environment.m_userProfile),
    Ref(executor), DirectoryEntry());
  auto translation = Translate(context, *chain);
  auto result = translation.Extract<Aspen::Box<Quantity>>();
  CPPUNIT_ASSERT(result.commit(0) == Aspen::State::CONTINUE_EVALUATED);
  CPPUNIT_ASSERT(result.eval() == 123);
  CPPUNIT_ASSERT(result.commit(1) == Aspen::State::COMPLETE_EVALUATED);
  CPPUNIT_ASSERT(result.eval() == 123);
}

void TranslationTester::TestTranslatingOrder() {
  auto environment = Environment();
  auto orderNode = std::unique_ptr<CanvasNode>(
    std::make_unique<SingleOrderTaskNode>());
  orderNode = orderNode->Replace(SingleOrderTaskNode::SECURITY_PROPERTY,
    std::make_unique<SecurityNode>(TEST_SECURITY,
    environment.m_userProfile.GetMarketDatabase()));
  orderNode = orderNode->Replace(SingleOrderTaskNode::QUANTITY_PROPERTY,
    std::make_unique<IntegerNode>(100));
  orderNode = orderNode->Replace(SingleOrderTaskNode::SIDE_PROPERTY,
    std::make_unique<SideNode>(Side::BID));
  orderNode = orderNode->Replace(SingleOrderTaskNode::PRICE_PROPERTY,
    std::make_unique<MoneyNode>(Money::ONE));
  orderNode = orderNode->Replace(SingleOrderTaskNode::ORDER_TYPE_PROPERTY,
    std::make_unique<OrderTypeNode>(OrderType::LIMIT));
  auto executor = Executor();
  auto context = CanvasNodeTranslationContext(Ref(environment.m_userProfile),
    Ref(executor), DirectoryEntry());
  auto translation = Translate(context, *orderNode);
  auto result = translation.Extract<Aspen::Box<const Order*>>();
  CPPUNIT_ASSERT(result.commit(0) == Aspen::State::EVALUATED);
  auto order1 = result.eval();
  CPPUNIT_ASSERT(order1->GetInfo().m_fields.m_security == TEST_SECURITY);
  CPPUNIT_ASSERT(order1->GetInfo().m_fields.m_quantity == 100);
  CPPUNIT_ASSERT(order1->GetInfo().m_fields.m_side == Side::BID);
  CPPUNIT_ASSERT(order1->GetInfo().m_fields.m_price == Money::ONE);
  CPPUNIT_ASSERT(order1->GetInfo().m_fields.m_type == OrderType::LIMIT);
}

void TranslationTester::TestTranslatingOrderTask() {
  auto environment = Environment();
  auto orderNode = std::unique_ptr<CanvasNode>(
    std::make_unique<SingleOrderTaskNode>());
  orderNode = orderNode->Replace(SingleOrderTaskNode::SECURITY_PROPERTY,
    std::make_unique<SecurityNode>(TEST_SECURITY,
    environment.m_userProfile.GetMarketDatabase()));
  orderNode = orderNode->Replace(SingleOrderTaskNode::QUANTITY_PROPERTY,
    std::make_unique<IntegerNode>(100));
  orderNode = orderNode->Replace(SingleOrderTaskNode::SIDE_PROPERTY,
    std::make_unique<SideNode>(Side::BID));
  orderNode = orderNode->Replace(SingleOrderTaskNode::PRICE_PROPERTY,
    std::make_unique<MoneyNode>(Money::ONE));
  orderNode = orderNode->Replace(SingleOrderTaskNode::ORDER_TYPE_PROPERTY,
    std::make_unique<OrderTypeNode>(OrderType::LIMIT));
  auto task = std::make_shared<Task>(*orderNode, DirectoryEntry(),
    Ref(environment.m_userProfile));
  auto submittedOrders = std::make_shared<Queue<const Order*>>();
  task->GetContext().GetOrderPublisher().Monitor(submittedOrders);
  auto taskState = std::make_shared<Queue<Task::StateEntry>>();
  task->GetPublisher().Monitor(taskState);
  task->Execute();
  CPPUNIT_ASSERT(taskState->Top().m_state == Task::State::INITIALIZING);
  taskState->Pop();
  auto submittedOrder1 = submittedOrders->Top();
  submittedOrders->Pop();
  CPPUNIT_ASSERT(taskState->Top().m_state == Task::State::ACTIVE);
  taskState->Pop();
  CPPUNIT_ASSERT(submittedOrder1->GetInfo().m_fields.m_security ==
    TEST_SECURITY);
  CPPUNIT_ASSERT(submittedOrder1->GetInfo().m_fields.m_quantity == 100);
  CPPUNIT_ASSERT(submittedOrder1->GetInfo().m_fields.m_side == Side::BID);
  CPPUNIT_ASSERT(submittedOrder1->GetInfo().m_fields.m_price == Money::ONE);
  CPPUNIT_ASSERT(submittedOrder1->GetInfo().m_fields.m_type ==
    OrderType::LIMIT);
  auto receivedOrders = std::make_shared<Queue<const Order*>>();
  environment.m_environment.MonitorOrderSubmissions(receivedOrders);
  auto receivedOrder1 = receivedOrders->Top();
  receivedOrders->Pop();
  environment.m_environment.AcceptOrder(*receivedOrder1);
  auto executionReports = std::make_shared<Queue<ExecutionReport>>();
  submittedOrder1->GetPublisher().Monitor(executionReports);
  CPPUNIT_ASSERT(executionReports->Top().m_status == OrderStatus::PENDING_NEW);
  executionReports->Pop();
  CPPUNIT_ASSERT(executionReports->Top().m_status == OrderStatus::NEW);
  executionReports->Pop();
  task->Cancel();
  CPPUNIT_ASSERT(taskState->Top().m_state == Task::State::PENDING_CANCEL);
  taskState->Pop();
  CPPUNIT_ASSERT(executionReports->Top().m_status ==
    OrderStatus::PENDING_CANCEL);
  environment.m_environment.CancelOrder(*receivedOrder1);
  CPPUNIT_ASSERT(taskState->Top().m_state == Task::State::CANCELED);
  taskState->Pop();
}

void TranslationTester::TestTranslatingUntil() {
/*
  auto environment = Environment();
  auto untilNode = std::unique_ptr<CanvasNode>(std::make_unique<UntilNode>());
  orderNode = orderNode->Replace(SingleOrderTaskNode::SECURITY_PROPERTY,
    std::make_unique<SecurityNode>(TEST_SECURITY,
    environment.m_userProfile.GetMarketDatabase()));
  orderNode = orderNode->Replace(SingleOrderTaskNode::QUANTITY_PROPERTY,
    std::make_unique<IntegerNode>(100));
  orderNode = orderNode->Replace(SingleOrderTaskNode::SIDE_PROPERTY,
    std::make_unique<SideNode>(Side::BID));
  orderNode = orderNode->Replace(SingleOrderTaskNode::PRICE_PROPERTY,
    std::make_unique<MoneyNode>(Money::ONE));
  orderNode = orderNode->Replace(SingleOrderTaskNode::ORDER_TYPE_PROPERTY,
    std::make_unique<OrderTypeNode>(OrderType::LIMIT));
  auto executor = Executor();
  auto task = std::make_shared<Task>(*orderNode, DirectoryEntry(),
    Ref(environment.m_userProfile));
  auto orders = std::make_shared<Queue<const Order*>>();
  task->GetContext().GetOrderPublisher().Monitor(orders);
  task->Execute();
  auto order1 = orders->Top();
  orders->Pop();
  CPPUNIT_ASSERT(order1->GetInfo().m_fields.m_security == TEST_SECURITY);
  CPPUNIT_ASSERT(order1->GetInfo().m_fields.m_quantity == 100);
  CPPUNIT_ASSERT(order1->GetInfo().m_fields.m_side == Side::BID);
  CPPUNIT_ASSERT(order1->GetInfo().m_fields.m_price == Money::ONE);
  CPPUNIT_ASSERT(order1->GetInfo().m_fields.m_type == OrderType::LIMIT);
*/
}
