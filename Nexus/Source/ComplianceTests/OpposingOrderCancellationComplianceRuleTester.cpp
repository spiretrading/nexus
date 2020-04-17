#include <doctest/doctest.h>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include "Nexus/Compliance/OpposingOrderCancellationComplianceRule.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  const auto TIMESTAMP = ptime(date(1984, May, 6), seconds(10));
  const auto DURATION = seconds(10);

  auto BuildOrderFields(Side side) {
    return OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security("TST", DefaultMarkets::TSX(), DefaultCountries::CA()),
      DefaultCurrencies::CAD(), side, DefaultDestinations::TSX(), 1000,
      Money::ONE);
  }
}

TEST_SUITE("OpposingOrderCancellationComplianceRule") {
  TEST_CASE("cancel_without_fill") {
    auto timeClient = FixedTimeClient(TIMESTAMP);
    auto rule = OpposingOrderCancellationComplianceRule(DURATION, &timeClient);
    {
      auto order = PrimitiveOrder{{BuildOrderFields(Side::ASK), 1,
        timeClient.GetTime()}};
      rule.Add(order);
      REQUIRE_NOTHROW(rule.Cancel(order));
    }
    {
      auto order = PrimitiveOrder({BuildOrderFields(Side::BID), 2,
        timeClient.GetTime()});
      rule.Add(order);
      REQUIRE_NOTHROW(rule.Cancel(order));
    }
  }

  TEST_CASE("in_order_cancel_opposing_order_without_fill") {
    auto timeClient = FixedTimeClient(TIMESTAMP);
    auto rule = OpposingOrderCancellationComplianceRule(DURATION, &timeClient);
    auto askOrder = PrimitiveOrder({BuildOrderFields(Side::ASK), 1,
      timeClient.GetTime()});
    rule.Add(askOrder);
    auto bidOrder = PrimitiveOrder({BuildOrderFields(Side::BID), 1,
      timeClient.GetTime()});
    rule.Add(bidOrder);
    REQUIRE_NOTHROW(rule.Cancel(bidOrder));
    REQUIRE_NOTHROW(rule.Cancel(askOrder));
  }

  TEST_CASE("reverse_order_cancel_opposing_order_without_fill") {
    auto timeClient = FixedTimeClient(TIMESTAMP);
    auto rule = OpposingOrderCancellationComplianceRule(DURATION, &timeClient);
    auto askOrder = PrimitiveOrder({BuildOrderFields(Side::ASK), 1,
      timeClient.GetTime()});
    rule.Add(askOrder);
    auto bidOrder = PrimitiveOrder({BuildOrderFields(Side::BID), 1,
      timeClient.GetTime()});
    rule.Add(bidOrder);
    REQUIRE_NOTHROW(rule.Cancel(askOrder));
    REQUIRE_NOTHROW(rule.Cancel(bidOrder));
  }

  TEST_CASE("in_order_cancel_opposing_order_with_fill_inside_period") {
    auto timeClient = FixedTimeClient(TIMESTAMP);
    auto rule = OpposingOrderCancellationComplianceRule(DURATION, &timeClient);
    auto askOrder = PrimitiveOrder({BuildOrderFields(Side::ASK), 1,
      timeClient.GetTime()});
    rule.Add(askOrder);
    auto bidOrder = PrimitiveOrder({BuildOrderFields(Side::BID), 1,
      timeClient.GetTime()});
    rule.Add(bidOrder);
    SetOrderStatus(askOrder, OrderStatus::NEW, timeClient.GetTime());
    SetOrderStatus(bidOrder, OrderStatus::NEW, timeClient.GetTime());
    FillOrder(askOrder, 100, timeClient.GetTime());
    REQUIRE_THROWS_AS(rule.Cancel(bidOrder), ComplianceCheckException);
    REQUIRE_NOTHROW(rule.Cancel(askOrder));
    timeClient.SetTime(TIMESTAMP + DURATION + seconds(1));
    REQUIRE_NOTHROW(rule.Cancel(bidOrder));
  }

  TEST_CASE("reverse_order_cancel_opposing_order_with_fill_inside_period") {
    auto timeClient = FixedTimeClient(TIMESTAMP);
    auto rule = OpposingOrderCancellationComplianceRule(DURATION, &timeClient);
    auto askOrder = PrimitiveOrder({BuildOrderFields(Side::ASK), 1,
      timeClient.GetTime()});
    rule.Add(askOrder);
    auto bidOrder = PrimitiveOrder({BuildOrderFields(Side::BID), 1,
      timeClient.GetTime()});
    rule.Add(bidOrder);
    SetOrderStatus(askOrder, OrderStatus::NEW, timeClient.GetTime());
    SetOrderStatus(bidOrder, OrderStatus::NEW, timeClient.GetTime());
    FillOrder(askOrder, 100, timeClient.GetTime());
    REQUIRE_NOTHROW(rule.Cancel(askOrder));
    REQUIRE_THROWS_AS(rule.Cancel(bidOrder), ComplianceCheckException);
    timeClient.SetTime(TIMESTAMP + DURATION + seconds(1));
    REQUIRE_NOTHROW(rule.Cancel(bidOrder));
  }

  TEST_CASE("in_order_cancel_opposing_order_with_fill_outside_period") {
    auto timeClient = FixedTimeClient(TIMESTAMP);
    auto rule = OpposingOrderCancellationComplianceRule(DURATION, &timeClient);
    auto askOrder = PrimitiveOrder({BuildOrderFields(Side::ASK), 1,
      timeClient.GetTime()});
    rule.Add(askOrder);
    auto bidOrder = PrimitiveOrder({BuildOrderFields(Side::BID), 1,
      timeClient.GetTime()});
    rule.Add(bidOrder);
    SetOrderStatus(askOrder, OrderStatus::NEW, timeClient.GetTime());
    SetOrderStatus(bidOrder, OrderStatus::NEW, timeClient.GetTime());
    FillOrder(askOrder, 100, timeClient.GetTime());
    timeClient.SetTime(TIMESTAMP + DURATION + seconds(1));
    REQUIRE_NOTHROW(rule.Cancel(bidOrder));
    REQUIRE_NOTHROW(rule.Cancel(askOrder));
  }

  TEST_CASE("reverse_order_cancel_opposing_order_with_fill_outside_period") {
    auto timeClient = FixedTimeClient(TIMESTAMP);
    auto rule = OpposingOrderCancellationComplianceRule(DURATION, &timeClient);
    auto askOrder = PrimitiveOrder({BuildOrderFields(Side::ASK), 1,
      timeClient.GetTime()});
    rule.Add(askOrder);
    auto bidOrder = PrimitiveOrder({BuildOrderFields(Side::BID), 1,
      timeClient.GetTime()});
    rule.Add(bidOrder);
    SetOrderStatus(askOrder, OrderStatus::NEW, timeClient.GetTime());
    SetOrderStatus(bidOrder, OrderStatus::NEW, timeClient.GetTime());
    FillOrder(askOrder, 100, timeClient.GetTime());
    timeClient.SetTime(TIMESTAMP + DURATION + seconds(1));
    REQUIRE_NOTHROW(rule.Cancel(askOrder));
    REQUIRE_NOTHROW(rule.Cancel(bidOrder));
  }
}
