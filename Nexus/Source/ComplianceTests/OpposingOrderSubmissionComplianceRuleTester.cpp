#include "Nexus/ComplianceTests/OpposingOrderSubmissionComplianceRuleTester.hpp"
#include <Beam/TimeService/FixedTimeClient.hpp>
#include "Nexus/Compliance/OpposingOrderSubmissionComplianceRule.hpp"
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
  const auto RANGE = 2 * Money::CENT;

  OrderFields BuildLimitOrderFields(Side side, Money price) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{"TST", DefaultMarkets::TSX(), DefaultCountries::CA()},
      DefaultCurrencies::CAD(), side, DefaultDestinations::TSX(), 1000, price);
    return fields;
  }

  OrderFields BuildMarketOrderFields(Side side) {
    auto fields = OrderFields::BuildMarketOrder(
      DirectoryEntry::GetRootAccount(),
      Security{"TST", DefaultMarkets::TSX(), DefaultCountries::CA()},
      DefaultCurrencies::CAD(), side, DefaultDestinations::TSX(), 1000);
    return fields;
  }
}

void OpposingOrderSubmissionComplianceRuleTester::TestLimitAskCancel() {
  FixedTimeClient timeClient{TIMESTAMP};
  OpposingOrderSubmissionComplianceRule<FixedTimeClient*> rule{DURATION, RANGE,
    &timeClient};
  PrimitiveOrder order{{BuildLimitOrderFields(Side::ASK, Money::ONE), 1,
    timeClient.GetTime()}};
  {
    CPPUNIT_ASSERT_NO_THROW(rule.Submit(order));
    SetOrderStatus(order, OrderStatus::NEW, timeClient.GetTime());
    CPPUNIT_ASSERT_NO_THROW(rule.Cancel(order));
    SetOrderStatus(order, OrderStatus::PENDING_CANCEL, timeClient.GetTime());
    CancelOrder(order, timeClient.GetTime());
  }
  {
    PrimitiveOrder order{{BuildLimitOrderFields(Side::BID, Money::ONE), 2,
      timeClient.GetTime()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
  {
    PrimitiveOrder order{{BuildLimitOrderFields(Side::BID, 99 * Money::CENT), 3,
      timeClient.GetTime()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
  {
    PrimitiveOrder order{{BuildLimitOrderFields(Side::BID, 98 * Money::CENT), 4,
      timeClient.GetTime()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
  {
    PrimitiveOrder order{{BuildLimitOrderFields(Side::BID, 97 * Money::CENT), 5,
      timeClient.GetTime()}};
    CPPUNIT_ASSERT_NO_THROW(rule.Submit(order));
  }
  {
    PrimitiveOrder order{{BuildMarketOrderFields(Side::BID), 6,
      timeClient.GetTime()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
}

void OpposingOrderSubmissionComplianceRuleTester::TestLimitBidCancel() {
  FixedTimeClient timeClient{TIMESTAMP};
  OpposingOrderSubmissionComplianceRule<FixedTimeClient*> rule{DURATION, RANGE,
    &timeClient};
  PrimitiveOrder order{{BuildLimitOrderFields(Side::BID, Money::ONE), 1,
    timeClient.GetTime()}};
  {
    CPPUNIT_ASSERT_NO_THROW(rule.Submit(order));
    SetOrderStatus(order, OrderStatus::NEW, timeClient.GetTime());
    CPPUNIT_ASSERT_NO_THROW(rule.Cancel(order));
    SetOrderStatus(order, OrderStatus::PENDING_CANCEL, timeClient.GetTime());
    CancelOrder(order, timeClient.GetTime());
  }
  {
    PrimitiveOrder order{{BuildLimitOrderFields(Side::ASK, Money::ONE), 2,
      timeClient.GetTime()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
  {
    PrimitiveOrder order{{BuildLimitOrderFields(Side::ASK,
      Money::ONE + Money::CENT), 3, timeClient.GetTime()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
  {
    PrimitiveOrder order{{BuildLimitOrderFields(Side::ASK,
      Money::ONE + 2 * Money::CENT), 4, timeClient.GetTime()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
  {
    PrimitiveOrder order{{BuildLimitOrderFields(Side::ASK,
      Money::ONE + 3 * Money::CENT), 5, timeClient.GetTime()}};
    CPPUNIT_ASSERT_NO_THROW(rule.Submit(order));
  }
  {
    PrimitiveOrder order{{BuildMarketOrderFields(Side::ASK), 6,
      timeClient.GetTime()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
}
