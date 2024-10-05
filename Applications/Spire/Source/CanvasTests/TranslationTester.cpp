#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"
#include "Nexus/TelemetryServiceTests/TelemetryServiceTestEnvironment.hpp"
#include "Spire/Canvas/ControlNodes/ChainNode.hpp"
#include "Spire/Canvas/ControlNodes/SpawnNode.hpp"
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
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::TelemetryService;
using namespace Nexus::TelemetryService::Tests;
using namespace Spire;

namespace {
  struct Environment {
    TestEnvironment m_environment;
    ServiceClientsBox m_serviceClients;
    TelemetryServiceTestEnvironment m_telemetryEnvironment;
    TelemetryClientBox m_telemetryClient;
    UserProfile m_userProfile;

    Environment()
      : m_serviceClients(std::in_place_type<TestServiceClients>,
          Ref(m_environment)),
        m_telemetryEnvironment(m_serviceClients.GetServiceLocatorClient(),
          m_serviceClients.GetTimeClient(),
          m_serviceClients.GetAdministrationClient()),
        m_telemetryClient(m_telemetryEnvironment.MakeClient(
          m_serviceClients.GetServiceLocatorClient())),
        m_userProfile("", false, false, GetDefaultCountryDatabase(),
          GetDefaultTimeZoneDatabase(), GetDefaultCurrencyDatabase(), {},
          GetDefaultMarketDatabase(), GetDefaultDestinationDatabase(),
          EntitlementDatabase(), get_default_additional_tag_database(),
          TimeAndSalesProperties(), m_serviceClients, m_telemetryClient) {}
  };

  const auto TEST_SECURITY = ParseSecurity("TST.TSX");
}

TEST_SUITE("Translation") {
  TEST_CASE("translating_constant") {
    auto environment = Environment();
    auto value = IntegerNode(100);
    auto executor = Executor();
    auto context = CanvasNodeTranslationContext(Ref(environment.m_userProfile),
      Ref(executor), DirectoryEntry());
    auto translation = Translate(context, value);
    auto result = translation.Extract<Aspen::Box<Quantity>>();
    REQUIRE(result.commit(0) == Aspen::State::COMPLETE_EVALUATED);
    REQUIRE(result.eval() == 100);
  }

  TEST_CASE("translating_chain") {
    auto environment = Environment();
    auto chain = std::unique_ptr<CanvasNode>(std::make_unique<ChainNode>());
    chain = chain->Replace("i0", std::make_unique<IntegerNode>(123));
    chain = chain->Replace("i1", std::make_unique<IntegerNode>(456));
    auto executor = Executor();
    auto context = CanvasNodeTranslationContext(Ref(environment.m_userProfile),
      Ref(executor), DirectoryEntry());
    auto translation = Translate(context, *chain);
    auto result = translation.Extract<Aspen::Box<Quantity>>();
    REQUIRE(result.commit(0) == Aspen::State::CONTINUE_EVALUATED);
    REQUIRE(result.eval() == 123);
    REQUIRE(result.commit(1) == Aspen::State::COMPLETE_EVALUATED);
    REQUIRE(result.eval() == 456);
  }

  TEST_CASE("translating_chain_with_tail_reference") {
    auto environment = Environment();
    auto chain = std::unique_ptr<CanvasNode>(std::make_unique<ChainNode>());
    chain = chain->Replace("i0", std::make_unique<IntegerNode>(123));
    chain = chain->Replace("i1", std::make_unique<ReferenceNode>("i0"));
    auto executor = Executor();
    auto context = CanvasNodeTranslationContext(Ref(environment.m_userProfile),
      Ref(executor), DirectoryEntry());
    auto translation = Translate(context, *chain);
    auto result = translation.Extract<Aspen::Box<Quantity>>();
    REQUIRE(result.commit(0) == Aspen::State::CONTINUE_EVALUATED);
    REQUIRE(result.eval() == 123);
    REQUIRE(result.commit(1) == Aspen::State::COMPLETE_EVALUATED);
    REQUIRE(result.eval() == 123);
  }

  TEST_CASE("translating_chain_with_head_reference") {
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
    REQUIRE(result.commit(0) == Aspen::State::CONTINUE_EVALUATED);
    REQUIRE(result.eval() == 123);
    REQUIRE(result.commit(1) == Aspen::State::COMPLETE_EVALUATED);
    REQUIRE(result.eval() == 123);
  }

  TEST_CASE("translating_order") {
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
    REQUIRE(result.commit(0) == Aspen::State::EVALUATED);
    auto order1 = result.eval();
    REQUIRE(order1->GetInfo().m_fields.m_security == TEST_SECURITY);
    REQUIRE(order1->GetInfo().m_fields.m_quantity == 100);
    REQUIRE(order1->GetInfo().m_fields.m_side == Side::BID);
    REQUIRE(order1->GetInfo().m_fields.m_price == Money::ONE);
    REQUIRE(order1->GetInfo().m_fields.m_type == OrderType::LIMIT);
  }

  TEST_CASE("translating_order_task") {
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
    REQUIRE(taskState->Pop().m_state == Task::State::INITIALIZING);
    auto submittedOrder1 = submittedOrders->Pop();
    REQUIRE(taskState->Pop().m_state == Task::State::ACTIVE);
    REQUIRE(submittedOrder1->GetInfo().m_fields.m_security ==
      TEST_SECURITY);
    REQUIRE(submittedOrder1->GetInfo().m_fields.m_quantity == 100);
    REQUIRE(submittedOrder1->GetInfo().m_fields.m_side == Side::BID);
    REQUIRE(submittedOrder1->GetInfo().m_fields.m_price == Money::ONE);
    REQUIRE(submittedOrder1->GetInfo().m_fields.m_type ==
      OrderType::LIMIT);
    auto receivedOrders = std::make_shared<Queue<const Order*>>();
    environment.m_environment.MonitorOrderSubmissions(receivedOrders);
    auto receivedOrder1 = receivedOrders->Pop();
    environment.m_environment.Accept(*receivedOrder1);
    auto executionReports = std::make_shared<Queue<ExecutionReport>>();
    submittedOrder1->GetPublisher().Monitor(executionReports);
    REQUIRE(executionReports->Pop().m_status == OrderStatus::PENDING_NEW);
    REQUIRE(executionReports->Pop().m_status == OrderStatus::NEW);
    task->Cancel();
    REQUIRE(taskState->Pop().m_state == Task::State::PENDING_CANCEL);
    REQUIRE(executionReports->Pop().m_status == OrderStatus::PENDING_CANCEL);
    environment.m_environment.Cancel(*receivedOrder1);
    REQUIRE(taskState->Pop().m_state == Task::State::CANCELED);
  }

  TEST_CASE("translating_spawn") {
    auto environment = Environment();
    auto spawnNode = std::make_unique<SpawnNode>()->Replace(
      SpawnNode::TRIGGER_PROPERTY, std::make_unique<IntegerNode>())->Replace(
      SpawnNode::SERIES_PROPERTY, std::make_unique<IntegerNode>());
    auto task = std::make_shared<Task>(
      *spawnNode, DirectoryEntry(), Ref(environment.m_userProfile));
    auto taskState = std::make_shared<Queue<Task::StateEntry>>();
    task->GetPublisher().Monitor(taskState);
    task->Execute();
    REQUIRE(taskState->Pop().m_state == Task::State::INITIALIZING);
    REQUIRE(taskState->Pop().m_state == Task::State::ACTIVE);
    REQUIRE(taskState->Pop().m_state == Task::State::COMPLETE);
  }
}
