#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/ExecutionReportPublisher.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  const auto TST = Security("TST", DefaultMarkets::TSX(),
    DefaultCountries::CA());
}

TEST_SUITE("ExecutionReportPublisher") {
  TEST_CASE("partial_snapshot") {
    auto initialOrder = PrimitiveOrder(OrderInfo(
      OrderFields::BuildLimitOrder(TST, Side::BID, 100, Money::ONE), 17,
      ptime(date(2020, 5, 12), time_duration(4, 12, 18))));
    auto orders = std::make_shared<Queue<const Order*>>();
    orders->Push(&initialOrder);
    auto publisher = ExecutionReportPublisher(orders);
    auto receiver = std::make_shared<Queue<ExecutionReportEntry>>();
    publisher.Monitor(receiver);
    auto reportA = receiver->Pop();
    REQUIRE(reportA.m_order == &initialOrder);
    REQUIRE(reportA.m_executionReport ==
      initialOrder.GetPublisher().GetSnapshot()->back());
    auto update = initialOrder.With([&] (auto status, const auto& reports) {
      auto update = ExecutionReport::BuildUpdatedReport(reports.back(),
        OrderStatus::NEW, reports.back().m_timestamp);
      initialOrder.Update(update);
      return update;
    });
    auto reportB = receiver->Pop();
    REQUIRE(reportB.m_order == &initialOrder);
    REQUIRE(reportB.m_executionReport == update);
  }
}
