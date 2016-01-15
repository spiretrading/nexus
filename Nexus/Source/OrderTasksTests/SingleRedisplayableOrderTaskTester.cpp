#include "Nexus/OrderTasksTests/SingleRedisplayableOrderTaskTester.hpp"
#include <Beam/TasksTests/MockTask.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::OrderTasks;
using namespace Nexus::OrderTasks::Tests;
using namespace std;

namespace {
  static const Quantity BLOCK_SIZE = 100;
}

void SingleRedisplayableOrderTaskTester::setUp() {
  m_client.Initialize();
  m_client->Open(Eval<void>());
  m_orderSink.Initialize();
  m_executionSink.Initialize();
  vector<const Order*> orders;
  m_client->MonitorAllOrders().ConnectOrderExecutedSignal(
    m_executionSink->GetSlot<
    OrderExecutionMonitor::OrderExecutedSignal::slot_function_type>(),
    Store(orders));
}

void SingleRedisplayableOrderTaskTester::tearDown() {
  m_executionSink.Reset();
  m_orderSink.Reset();
  m_client.Reset();
}

void SingleRedisplayableOrderTaskTester::TestZeroSizedInitialOrder() {}

void SingleRedisplayableOrderTaskTester::TestNonEmptyInitialOrder() {}

void SingleRedisplayableOrderTaskTester::TestResizingFromEmptyOrder() {}

void SingleRedisplayableOrderTaskTester::TestResizingToExistingSize() {}

void SingleRedisplayableOrderTaskTester::TestIncreasingNonEmptyOrder() {}

void SingleRedisplayableOrderTaskTester::TestDecreasingToEmptyOrder() {}

void SingleRedisplayableOrderTaskTester::TestDecreasingToNonEmptyOrder() {}

void SingleRedisplayableOrderTaskTester::
    TestDecreasingFilledOrderToEmptyOrder() {}

void SingleRedisplayableOrderTaskTester::TestDecreasingFilledOrder() {}

void SingleRedisplayableOrderTaskTester::TestIncreasingFilledOrder() {}

void SingleRedisplayableOrderTaskTester::TestIncreasingPartiallyFilledOrder() {}

void SingleRedisplayableOrderTaskTester::TestExpiredOrder() {}

void SingleRedisplayableOrderTaskTester::TestPreemptiveExpiredOrder() {}

void SingleRedisplayableOrderTaskTester::TestCompletedOrder() {}

void SingleRedisplayableOrderTaskTester::TestPreemptiveCompletedOrder() {}

void SingleRedisplayableOrderTaskTester::TestFullOrder() {}

void SingleRedisplayableOrderTaskTester::TestUnfilledContinuation() {}

void SingleRedisplayableOrderTaskTester::TestPartiallyFilledContinuation() {}
