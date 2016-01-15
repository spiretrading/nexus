#include "Nexus/OrderTasksTests/SingleOrderTaskTester.hpp"
#include <Beam/TasksTests/TaskTestsUtilities.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::SignalHandling;
using namespace Beam::Tasks;
using namespace Beam::Tasks::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::OrderTasks;
using namespace Nexus::OrderTasks::Tests;
using namespace std;

void SingleOrderTaskTester::setUp() {
  m_client.Initialize();
  m_client->Open(Eval<void>());
  m_orderSink.Initialize();
  m_taskSink.Initialize();
  m_executionSink.Initialize();
  vector<const Order*> orders;
  m_orderSink->ConnectOrderExecutedSignal(m_executionSink->GetSlot<
    OrderExecutionMonitor::OrderExecutedSignal::slot_function_type>(),
    Store(orders));
}

void SingleOrderTaskTester::tearDown() {
  m_executionSink.Reset();
  m_taskSink.Reset();
  m_orderSink.Reset();
  m_client.Reset();
}

void SingleOrderTaskTester::TestEmptyOrder() {
  TestSingleOrderTaskFactory factory(Ref(*m_client), Ref(*m_orderSink),
    DirectoryEntry::GetRootAccount());
  factory.Set(TestSingleOrderTaskFactory::SECURITY,
    Security("TST", DefaultMarkets::NYSE(), DefaultCountries::US()));
  factory.Set<OrderType>(TestSingleOrderTaskFactory::ORDER_TYPE,
    OrderType::LIMIT);
  factory.Set<Side>(TestSingleOrderTaskFactory::SIDE, Side::BID);
  factory.Set(TestSingleOrderTaskFactory::DESTINATION, string("NYSE"));
  factory.Set<Quantity>(TestSingleOrderTaskFactory::QUANTITY, 0);
  factory.Set(TestSingleOrderTaskFactory::PRICE, Money::CENT);
  factory.Set(TestSingleOrderTaskFactory::TIME_IN_FORCE,
    TimeInForce(TimeInForce::Type::GTC));
  std::unique_ptr<Task> task = factory.Create();
  vector<Task::StateEntry> states;
  task->ConnectSignals(
    m_taskSink->GetSlot<Task::StateSignal::slot_function_type>(),
    Store(states));
  task->Execute();
  TestTaskActive(*m_taskSink, *task);
  TestTaskState(*m_taskSink, *task, Task::State::COMPLETE);
}

void SingleOrderTaskTester::TestRejectedOrder() {
  const Quantity QUANTITY = 100;
  TestSingleOrderTaskFactory factory(Ref(*m_client), Ref(*m_orderSink),
    DirectoryEntry::GetRootAccount());
  factory.Set(TestSingleOrderTaskFactory::SECURITY,
    Security("TST", DefaultMarkets::NYSE(), DefaultCountries::US()));
  factory.Set<OrderType>(TestSingleOrderTaskFactory::ORDER_TYPE,
    OrderType::LIMIT);
  factory.Set<Side>(TestSingleOrderTaskFactory::SIDE, Side::BID);
  factory.Set(TestSingleOrderTaskFactory::DESTINATION, string("NYSE"));
  factory.Set(TestSingleOrderTaskFactory::QUANTITY, QUANTITY);
  factory.Set(TestSingleOrderTaskFactory::PRICE, Money::ZERO);
  factory.Set(TestSingleOrderTaskFactory::TIME_IN_FORCE,
    TimeInForce(TimeInForce::Type::GTC));
  std::unique_ptr<Task> task = factory.Create();
  vector<Task::StateEntry> states;
  task->ConnectSignals(
    m_taskSink->GetSlot<Task::StateSignal::slot_function_type>(),
    Store(states));
  task->Execute();
  TestTaskActive(*m_taskSink, *task);

  // Grab the executed task and reject it.
  SignalSink::SignalEntry entry = m_executionSink->GetNextSignal(seconds(1));
  PrimitiveOrder* order = ExtractOrder(entry.m_parameters[0]);
  CPPUNIT_ASSERT(order->GetFields().m_quantity == QUANTITY);
  CPPUNIT_ASSERT(order->GetFields().m_side == Side::BID);
  CPPUNIT_ASSERT(order->GetFields().m_type == OrderType::LIMIT);
  SetOrderStatus(*order, OrderStatus::REJECTED,
    microsec_clock::universal_time());

  // The state should go to FAILED.
  TestTaskState(*m_taskSink, *task, Task::State::FAILED);
}

void SingleOrderTaskTester::TestFullOrder() {
  const Quantity QUANTITY = 100;
  TestSingleOrderTaskFactory factory(Ref(*m_client), Ref(*m_orderSink),
    DirectoryEntry::GetRootAccount());
  factory.Set(TestSingleOrderTaskFactory::SECURITY,
    Security("TST", DefaultMarkets::NYSE(), DefaultCountries::US()));
  factory.Set<OrderType>(TestSingleOrderTaskFactory::ORDER_TYPE,
    OrderType::LIMIT);
  factory.Set<Side>(TestSingleOrderTaskFactory::SIDE, Side::BID);
  factory.Set(TestSingleOrderTaskFactory::DESTINATION, string("NYSE"));
  factory.Set(TestSingleOrderTaskFactory::QUANTITY, QUANTITY);
  factory.Set(TestSingleOrderTaskFactory::PRICE, Money::ZERO);
  factory.Set(TestSingleOrderTaskFactory::TIME_IN_FORCE,
    TimeInForce(TimeInForce::Type::GTC));
  std::unique_ptr<Task> task = factory.Create();
  vector<Task::StateEntry> states;
  task->ConnectSignals(
    m_taskSink->GetSlot<Task::StateSignal::slot_function_type>(),
    Store(states));
  task->Execute();
  TestTaskActive(*m_taskSink, *task);

  // Grab the executed task.
  SignalSink::SignalEntry entry = m_executionSink->GetNextSignal(seconds(1));
  PrimitiveOrder* order = ExtractOrder(entry.m_parameters[0]);
  CPPUNIT_ASSERT(order->GetFields().m_quantity == QUANTITY);
  CPPUNIT_ASSERT(order->GetFields().m_side == Side::BID);
  CPPUNIT_ASSERT(order->GetFields().m_type == OrderType::LIMIT);
  SetOrderStatus(*order, OrderStatus::NEW, microsec_clock::universal_time());

  // Fill the executed task.
  FillOrder(*order, QUANTITY, microsec_clock::universal_time());
  TestTaskState(*m_taskSink, *task, Task::State::COMPLETE);
}

void SingleOrderTaskTester::TestCancelPriorToPendingOrder() {
  const Quantity QUANTITY = 100;
  TestSingleOrderTaskFactory factory(Ref(*m_client), Ref(*m_orderSink),
    DirectoryEntry::GetRootAccount());
  factory.Set(TestSingleOrderTaskFactory::SECURITY,
    Security("TST", DefaultMarkets::NYSE(), DefaultCountries::US()));
  factory.Set<OrderType>(TestSingleOrderTaskFactory::ORDER_TYPE,
    OrderType::LIMIT);
  factory.Set<Side>(TestSingleOrderTaskFactory::SIDE, Side::BID);
  factory.Set(TestSingleOrderTaskFactory::DESTINATION, string("NYSE"));
  factory.Set(TestSingleOrderTaskFactory::QUANTITY, QUANTITY);
  factory.Set(TestSingleOrderTaskFactory::PRICE, Money::ZERO);
  factory.Set(TestSingleOrderTaskFactory::TIME_IN_FORCE,
    TimeInForce(TimeInForce::Type::GTC));
  std::unique_ptr<Task> task = factory.Create();
  vector<Task::StateEntry> states;
  task->ConnectSignals(
    m_taskSink->GetSlot<Task::StateSignal::slot_function_type>(),
    Store(states));
  task->Execute();
  TestTaskActive(*m_taskSink, *task);

  // Grab the executed task.
  SignalSink::SignalEntry entry = m_executionSink->GetNextSignal(seconds(1));
  PrimitiveOrder* order = ExtractOrder(entry.m_parameters[0]);
  CPPUNIT_ASSERT(order->GetFields().m_quantity == QUANTITY);
  CPPUNIT_ASSERT(order->GetFields().m_side == Side::BID);
  CPPUNIT_ASSERT(order->GetFields().m_type == OrderType::LIMIT);

  // Cancel the SingleOrderSchema, it should go PENDING_CANCEL immediately to
  // reflect the cancel request.
  task->Cancel();
  TestTaskState(*m_taskSink, *task, Task::State::PENDING_CANCEL);

  // Now set the executed Order's state to NEW.
  SetOrderStatus(*order, OrderStatus::NEW, microsec_clock::universal_time());
  TestTaskState(*m_taskSink, *task, Task::State::CANCELED);
}

void SingleOrderTaskTester::TestCancelWithPreemptiveFill() {
  const Quantity QUANTITY = 100;
  TestSingleOrderTaskFactory factory(Ref(*m_client), Ref(*m_orderSink),
    DirectoryEntry::GetRootAccount());
  factory.Set(TestSingleOrderTaskFactory::SECURITY,
    Security("TST", DefaultMarkets::NYSE(), DefaultCountries::US()));
  factory.Set<OrderType>(TestSingleOrderTaskFactory::ORDER_TYPE,
    OrderType::LIMIT);
  factory.Set<Side>(TestSingleOrderTaskFactory::SIDE, Side::BID);
  factory.Set(TestSingleOrderTaskFactory::DESTINATION, string("NYSE"));
  factory.Set(TestSingleOrderTaskFactory::QUANTITY, QUANTITY);
  factory.Set(TestSingleOrderTaskFactory::PRICE, Money::ZERO);
  factory.Set(TestSingleOrderTaskFactory::TIME_IN_FORCE,
    TimeInForce(TimeInForce::Type::GTC));
  std::unique_ptr<Task> task = factory.Create();
  vector<Task::StateEntry> states;
  task->ConnectSignals(
    m_taskSink->GetSlot<Task::StateSignal::slot_function_type>(),
    Store(states));
  task->Execute();
  TestTaskActive(*m_taskSink, *task);

  // Grab the executed task.
  SignalSink::SignalEntry entry = m_executionSink->GetNextSignal(seconds(1));
  PrimitiveOrder* order = ExtractOrder(entry.m_parameters[0]);
  CPPUNIT_ASSERT(order->GetFields().m_quantity == QUANTITY);
  CPPUNIT_ASSERT(order->GetFields().m_side == Side::BID);
  CPPUNIT_ASSERT(order->GetFields().m_type == OrderType::LIMIT);
  SetOrderStatus(*order, OrderStatus::NEW, microsec_clock::universal_time());

  // Cancel the SingleOrderSchema, it should go PENDING_CANCEL immediately to
  // reflect the cancel request.
  m_client->SetAutoCancel(false);
  task->Cancel();
  TestTaskState(*m_taskSink, *task, Task::State::PENDING_CANCEL);

  // Now fill the executed task, the SingleOrderSchema should go COMPLETE to
  // reflect the pre-emptive fill.
  FillOrder(*order, QUANTITY, microsec_clock::universal_time());
  TestTaskState(*m_taskSink, *task, Task::State::COMPLETE);
}

void SingleOrderTaskTester::TestPreemptiveCancel() {
  const Quantity QUANTITY = 100;
  TestSingleOrderTaskFactory factory(Ref(*m_client), Ref(*m_orderSink),
    DirectoryEntry::GetRootAccount());
  factory.Set(TestSingleOrderTaskFactory::SECURITY,
    Security("TST", DefaultMarkets::NYSE(), DefaultCountries::US()));
  factory.Set<OrderType>(TestSingleOrderTaskFactory::ORDER_TYPE,
    OrderType::LIMIT);
  factory.Set<Side>(TestSingleOrderTaskFactory::SIDE, Side::BID);
  factory.Set(TestSingleOrderTaskFactory::DESTINATION, string("NYSE"));
  factory.Set(TestSingleOrderTaskFactory::QUANTITY, QUANTITY);
  factory.Set(TestSingleOrderTaskFactory::PRICE, Money::ZERO);
  factory.Set(TestSingleOrderTaskFactory::TIME_IN_FORCE,
    TimeInForce(TimeInForce::Type::GTC));
  std::unique_ptr<Task> task = factory.Create();
  vector<Task::StateEntry> states;
  task->ConnectSignals(
    m_taskSink->GetSlot<Task::StateSignal::slot_function_type>(),
    Store(states));
  task->Execute();
  TestTaskActive(*m_taskSink, *task);

  // Grab the executed task.
  SignalSink::SignalEntry entry = m_executionSink->GetNextSignal(seconds(1));
  PrimitiveOrder* order = ExtractOrder(entry.m_parameters[0]);
  CPPUNIT_ASSERT(order->GetFields().m_quantity == QUANTITY);
  CPPUNIT_ASSERT(order->GetFields().m_side == Side::BID);
  CPPUNIT_ASSERT(order->GetFields().m_type == OrderType::LIMIT);
  SetOrderStatus(*order, OrderStatus::NEW, microsec_clock::universal_time());

  // Cancel the executed task.
  m_client->Cancel(*order);

  // The SingleOrderSchema task should EXPIRE.
  TestTaskState(*m_taskSink, *task, Task::State::EXPIRED);
}

void SingleOrderTaskTester::TestUnfilledContinuation() {
  const Quantity QUANTITY = 100;
  TestSingleOrderTaskFactory factory(Ref(*m_client), Ref(*m_orderSink),
    DirectoryEntry::GetRootAccount());
  factory.Set(TestSingleOrderTaskFactory::SECURITY,
    Security("TST", DefaultMarkets::NYSE(), DefaultCountries::US()));
  factory.Set<OrderType>(TestSingleOrderTaskFactory::ORDER_TYPE,
    OrderType::LIMIT);
  factory.Set<Side>(TestSingleOrderTaskFactory::SIDE, Side::BID);
  factory.Set(TestSingleOrderTaskFactory::DESTINATION, string("NYSE"));
  factory.Set(TestSingleOrderTaskFactory::QUANTITY, QUANTITY);
  factory.Set(TestSingleOrderTaskFactory::PRICE, Money::ZERO);
  factory.Set(TestSingleOrderTaskFactory::TIME_IN_FORCE,
    TimeInForce(TimeInForce::Type::GTC));
  std::unique_ptr<Task> task = factory.Create();
  vector<Task::StateEntry> states;
  task->ConnectSignals(
    m_taskSink->GetSlot<Task::StateSignal::slot_function_type>(),
    Store(states));
  task->Execute();
  TestTaskActive(*m_taskSink, *task);

  // Grab the executed task.
  SignalSink::SignalEntry entry = m_executionSink->GetNextSignal(seconds(1));
  PrimitiveOrder* order = ExtractOrder(entry.m_parameters[0]);
  CPPUNIT_ASSERT(order->GetFields().m_quantity == QUANTITY);
  CPPUNIT_ASSERT(order->GetFields().m_side == Side::BID);
  CPPUNIT_ASSERT(order->GetFields().m_type == OrderType::LIMIT);
  SetOrderStatus(*order, OrderStatus::NEW, microsec_clock::universal_time());

  // Cancel the SingleOrderSchema and prepare it's continuation.
  task->Cancel();
  TestTaskState(*m_taskSink, *task, Task::State::PENDING_CANCEL);
  TestTaskState(*m_taskSink, *task, Task::State::CANCELED);
  factory.PrepareContinuation(*task);

  // Execute the continuation.
  std::unique_ptr<Task> continuationOrder = factory.Create();
  continuationOrder->ConnectSignals(
    m_taskSink->GetSlot<Task::StateSignal::slot_function_type>(),
    Store(states));
  continuationOrder->Execute();
  TestTaskActive(*m_taskSink, *continuationOrder);
  entry = m_executionSink->GetNextSignal(seconds(1));
  order = ExtractOrder(entry.m_parameters[0]);
  CPPUNIT_ASSERT(order->GetFields().m_quantity == QUANTITY);
  CPPUNIT_ASSERT(order->GetFields().m_side == Side::BID);
  CPPUNIT_ASSERT(order->GetFields().m_type == OrderType::LIMIT);
  SetOrderStatus(*order, OrderStatus::NEW, microsec_clock::universal_time());
  continuationOrder->Cancel();
  TestTaskState(*m_taskSink, *continuationOrder, Task::State::PENDING_CANCEL);
  SetOrderStatus(*order, OrderStatus::CANCELED,
    microsec_clock::universal_time());
  TestTaskState(*m_taskSink, *continuationOrder, Task::State::CANCELED);
}

void SingleOrderTaskTester::TestPartiallyFilledContinuation() {
  const Quantity QUANTITY = 200;
  const Quantity FILL_QUANTITY = 100;
  TestSingleOrderTaskFactory factory(Ref(*m_client), Ref(*m_orderSink),
    DirectoryEntry::GetRootAccount());
  factory.Set(TestSingleOrderTaskFactory::SECURITY,
    Security("TST", DefaultMarkets::NYSE(), DefaultCountries::US()));
  factory.Set<OrderType>(TestSingleOrderTaskFactory::ORDER_TYPE,
    OrderType::LIMIT);
  factory.Set<Side>(TestSingleOrderTaskFactory::SIDE, Side::BID);
  factory.Set(TestSingleOrderTaskFactory::DESTINATION, string("NYSE"));
  factory.Set(TestSingleOrderTaskFactory::QUANTITY, QUANTITY);
  factory.Set(TestSingleOrderTaskFactory::PRICE, Money::ZERO);
  factory.Set(TestSingleOrderTaskFactory::TIME_IN_FORCE,
    TimeInForce(TimeInForce::Type::GTC));
  std::unique_ptr<Task> task = factory.Create();
  vector<Task::StateEntry> states;
  task->ConnectSignals(
    m_taskSink->GetSlot<Task::StateSignal::slot_function_type>(),
    Store(states));
  task->Execute();
  TestTaskActive(*m_taskSink, *task);

  // Grab the executed task.
  SignalSink::SignalEntry entry = m_executionSink->GetNextSignal(seconds(1));
  PrimitiveOrder* order = ExtractOrder(entry.m_parameters[0]);
  CPPUNIT_ASSERT(order->GetFields().m_quantity == QUANTITY);
  CPPUNIT_ASSERT(order->GetFields().m_side == Side::BID);
  CPPUNIT_ASSERT(order->GetFields().m_type == OrderType::LIMIT);
  SetOrderStatus(*order, OrderStatus::NEW, microsec_clock::universal_time());

  // Partially fill the task.
  FillOrder(*order, FILL_QUANTITY, microsec_clock::universal_time());

  // Cancel the SingleOrderSchema and prepare it's continuation.
  task->Cancel();
  TestTaskState(*m_taskSink, *task, Task::State::PENDING_CANCEL);
  TestTaskState(*m_taskSink, *task, Task::State::CANCELED);
  factory.PrepareContinuation(*task);

  // Execute the continuation.
  std::unique_ptr<Task> continuationOrder = factory.Create();
  continuationOrder->ConnectSignals(
    m_taskSink->GetSlot<Task::StateSignal::slot_function_type>(),
    Store(states));
  continuationOrder->Execute();
  TestTaskActive(*m_taskSink, *continuationOrder);
  entry = m_executionSink->GetNextSignal(seconds(1));
  order = ExtractOrder(entry.m_parameters[0]);
  CPPUNIT_ASSERT(order->GetFields().m_quantity ==
    QUANTITY - FILL_QUANTITY);
  CPPUNIT_ASSERT(order->GetFields().m_side == Side::BID);
  CPPUNIT_ASSERT(order->GetFields().m_type == OrderType::LIMIT);
  SetOrderStatus(*order, OrderStatus::NEW, microsec_clock::universal_time());
  continuationOrder->Cancel();
  TestTaskState(*m_taskSink, *continuationOrder, Task::State::PENDING_CANCEL);
  SetOrderStatus(*order, OrderStatus::CANCELED,
    microsec_clock::universal_time());
  TestTaskState(*m_taskSink, *continuationOrder, Task::State::CANCELED);
}

void SingleOrderTaskTester::TestFilledContinuation() {
  const Quantity QUANTITY = 200;
  const Quantity FILL_QUANTITY = 200;
  TestSingleOrderTaskFactory factory(Ref(*m_client), Ref(*m_orderSink),
    DirectoryEntry::GetRootAccount());
  factory.Set(TestSingleOrderTaskFactory::SECURITY,
    Security("TST", DefaultMarkets::NYSE(), DefaultCountries::US()));
  factory.Set<OrderType>(TestSingleOrderTaskFactory::ORDER_TYPE,
    OrderType::LIMIT);
  factory.Set<Side>(TestSingleOrderTaskFactory::SIDE, Side::BID);
  factory.Set(TestSingleOrderTaskFactory::DESTINATION, string("NYSE"));
  factory.Set(TestSingleOrderTaskFactory::QUANTITY, QUANTITY);
  factory.Set(TestSingleOrderTaskFactory::PRICE, Money::ZERO);
  factory.Set(TestSingleOrderTaskFactory::TIME_IN_FORCE,
    TimeInForce(TimeInForce::Type::GTC));
  std::unique_ptr<Task> task = factory.Create();
  vector<Task::StateEntry> states;
  task->ConnectSignals(
    m_taskSink->GetSlot<Task::StateSignal::slot_function_type>(),
    Store(states));
  task->Execute();
  TestTaskActive(*m_taskSink, *task);

  // Grab the executed task.
  SignalSink::SignalEntry entry = m_executionSink->GetNextSignal(seconds(1));
  PrimitiveOrder* order = ExtractOrder(entry.m_parameters[0]);
  CPPUNIT_ASSERT(order->GetFields().m_quantity == QUANTITY);
  CPPUNIT_ASSERT(order->GetFields().m_side == Side::BID);
  CPPUNIT_ASSERT(order->GetFields().m_type == OrderType::LIMIT);
  SetOrderStatus(*order, OrderStatus::NEW, microsec_clock::universal_time());

  // Fill the task.
  FillOrder(*order, FILL_QUANTITY, microsec_clock::universal_time());
  TestTaskState(*m_taskSink, *task, Task::State::COMPLETE);

  // Prepare and execute the continuation, it should go COMPLETE immediately.
  factory.PrepareContinuation(*task);
  std::unique_ptr<Task> continuationOrder = factory.Create();
  continuationOrder->ConnectSignals(
    m_taskSink->GetSlot<Task::StateSignal::slot_function_type>(),
    Store(states));
  continuationOrder->Execute();
  TestTaskActive(*m_taskSink, *continuationOrder);
  TestTaskState(*m_taskSink, *continuationOrder, Task::State::COMPLETE);
}
