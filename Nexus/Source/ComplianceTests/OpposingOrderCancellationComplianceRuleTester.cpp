#include "Nexus/ComplianceTests/OpposingOrderCancellationComplianceRuleTester.hpp"
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
using namespace Nexus::Compliance::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace std;

namespace {
  const auto TIMESTAMP = ptime{date{1984, May, 6}, seconds(10)};
  const time_duration DURATION = seconds(10);

  OrderFields BuildOrderFields(Side side) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{"TST", DefaultMarkets::TSX(), DefaultCountries::CA()},
      DefaultCurrencies::CAD(), side, DefaultDestinations::TSX(), 1000,
      Money::ONE);
    return fields;
  }
}

void OpposingOrderCancellationComplianceRuleTester::TestCancelWithoutFill() {
  FixedTimeClient timeClient{TIMESTAMP};
  OpposingOrderCancellationComplianceRule<FixedTimeClient*> rule{DURATION,
    &timeClient};
  {
    PrimitiveOrder order{{BuildOrderFields(Side::ASK), 1,
      timeClient.GetTime()}};
    rule.Add(order);
    CPPUNIT_ASSERT_NO_THROW(rule.Cancel(order));
  }
  {
    PrimitiveOrder order{{BuildOrderFields(Side::BID), 2,
      timeClient.GetTime()}};
    rule.Add(order);
    CPPUNIT_ASSERT_NO_THROW(rule.Cancel(order));
  }
}

void OpposingOrderCancellationComplianceRuleTester::
    TestInOrderCancelOpposingOrderWithoutFill() {
  FixedTimeClient timeClient{TIMESTAMP};
  OpposingOrderCancellationComplianceRule<FixedTimeClient*> rule{DURATION,
    &timeClient};
  PrimitiveOrder askOrder{{BuildOrderFields(Side::ASK), 1,
    timeClient.GetTime()}};
  rule.Add(askOrder);
  PrimitiveOrder bidOrder{{BuildOrderFields(Side::BID), 1,
    timeClient.GetTime()}};
  rule.Add(bidOrder);
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(bidOrder));
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(askOrder));
}

void OpposingOrderCancellationComplianceRuleTester::
    TestReverseOrderCancelOpposingOrderWithoutFill() {
  FixedTimeClient timeClient{TIMESTAMP};
  OpposingOrderCancellationComplianceRule<FixedTimeClient*> rule{DURATION,
    &timeClient};
  PrimitiveOrder askOrder{{BuildOrderFields(Side::ASK), 1,
    timeClient.GetTime()}};
  rule.Add(askOrder);
  PrimitiveOrder bidOrder{{BuildOrderFields(Side::BID), 1,
    timeClient.GetTime()}};
  rule.Add(bidOrder);
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(askOrder));
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(bidOrder));
}

void OpposingOrderCancellationComplianceRuleTester::
    TestInOrderCancelOpposingOrderWithFillInsidePeriod() {
  FixedTimeClient timeClient{TIMESTAMP};
  OpposingOrderCancellationComplianceRule<FixedTimeClient*> rule{DURATION,
    &timeClient};
  PrimitiveOrder askOrder{{BuildOrderFields(Side::ASK), 1,
    timeClient.GetTime()}};
  rule.Add(askOrder);
  PrimitiveOrder bidOrder{{BuildOrderFields(Side::BID), 1,
    timeClient.GetTime()}};
  rule.Add(bidOrder);
  SetOrderStatus(askOrder, OrderStatus::NEW, timeClient.GetTime());
  SetOrderStatus(bidOrder, OrderStatus::NEW, timeClient.GetTime());
  FillOrder(askOrder, 100, timeClient.GetTime());
  CPPUNIT_ASSERT_THROW(rule.Cancel(bidOrder), ComplianceCheckException);
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(askOrder));
  timeClient.SetTime(TIMESTAMP + DURATION + seconds(1));
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(bidOrder));
}

void OpposingOrderCancellationComplianceRuleTester::
    TestReverseOrderCancelOpposingOrderWithFillInsidePeriod() {
  FixedTimeClient timeClient{TIMESTAMP};
  OpposingOrderCancellationComplianceRule<FixedTimeClient*> rule{DURATION,
    &timeClient};
  PrimitiveOrder askOrder{{BuildOrderFields(Side::ASK), 1,
    timeClient.GetTime()}};
  rule.Add(askOrder);
  PrimitiveOrder bidOrder{{BuildOrderFields(Side::BID), 1,
    timeClient.GetTime()}};
  rule.Add(bidOrder);
  SetOrderStatus(askOrder, OrderStatus::NEW, timeClient.GetTime());
  SetOrderStatus(bidOrder, OrderStatus::NEW, timeClient.GetTime());
  FillOrder(askOrder, 100, timeClient.GetTime());
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(askOrder));
  CPPUNIT_ASSERT_THROW(rule.Cancel(bidOrder), ComplianceCheckException);
  timeClient.SetTime(TIMESTAMP + DURATION + seconds(1));
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(bidOrder));
}

void OpposingOrderCancellationComplianceRuleTester::
    TestInOrderCancelOpposingOrderWithFillOutsidePeriod() {
  FixedTimeClient timeClient{TIMESTAMP};
  OpposingOrderCancellationComplianceRule<FixedTimeClient*> rule{DURATION,
    &timeClient};
  PrimitiveOrder askOrder{{BuildOrderFields(Side::ASK), 1,
    timeClient.GetTime()}};
  rule.Add(askOrder);
  PrimitiveOrder bidOrder{{BuildOrderFields(Side::BID), 1,
    timeClient.GetTime()}};
  rule.Add(bidOrder);
  SetOrderStatus(askOrder, OrderStatus::NEW, timeClient.GetTime());
  SetOrderStatus(bidOrder, OrderStatus::NEW, timeClient.GetTime());
  FillOrder(askOrder, 100, timeClient.GetTime());
  timeClient.SetTime(TIMESTAMP + DURATION + seconds(1));
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(bidOrder));
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(askOrder));
}

void OpposingOrderCancellationComplianceRuleTester::
    TestReverseOrderCancelOpposingOrderWithFillOutsidePeriod() {
  FixedTimeClient timeClient{TIMESTAMP};
  OpposingOrderCancellationComplianceRule<FixedTimeClient*> rule{DURATION,
    &timeClient};
  PrimitiveOrder askOrder{{BuildOrderFields(Side::ASK), 1,
    timeClient.GetTime()}};
  rule.Add(askOrder);
  PrimitiveOrder bidOrder{{BuildOrderFields(Side::BID), 1,
    timeClient.GetTime()}};
  rule.Add(bidOrder);
  SetOrderStatus(askOrder, OrderStatus::NEW, timeClient.GetTime());
  SetOrderStatus(bidOrder, OrderStatus::NEW, timeClient.GetTime());
  FillOrder(askOrder, 100, timeClient.GetTime());
  timeClient.SetTime(TIMESTAMP + DURATION + seconds(1));
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(askOrder));
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(bidOrder));
}
