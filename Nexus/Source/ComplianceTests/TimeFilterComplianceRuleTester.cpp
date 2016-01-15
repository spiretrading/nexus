#include "Nexus/ComplianceTests/TimeFilterComplianceRuleTester.hpp"
#include <Beam/TimeService/FixedTimeClient.hpp>
#include "Nexus/Compliance/RejectCancelsComplianceRule.hpp"
#include "Nexus/Compliance/RejectSubmissionsComplianceRule.hpp"
#include "Nexus/Compliance/TimeFilterComplianceRule.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

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
using namespace std;

namespace {
  const auto START_TIME = ptime{date{1984, May, 6}, seconds(10)};
  const auto END_TIME = ptime{date{1984, May, 6}, seconds(20)};

  OrderFields BuildOrderFields() {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{"TST", DefaultMarkets::TSX(), DefaultCountries::CA()},
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      Money::ONE);
    return fields;
  }
}

void TimeFilterComplianceRuleTester::TestAddDuringCompliancePeriod() {
  auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
  FixedTimeClient timeClient;
  TimeFilterComplianceRule<FixedTimeClient*> rule{START_TIME.time_of_day(),
    END_TIME.time_of_day(), &timeClient, std::move(baseRule)};
  {
    timeClient.SetTime(START_TIME);
    PrimitiveOrder order{{BuildOrderFields(), 1, timeClient.GetTime()}};
    CPPUNIT_ASSERT_NO_THROW(rule.Add(order));
  }
  {
    timeClient.SetTime(END_TIME);
    PrimitiveOrder order{{BuildOrderFields(), 2, timeClient.GetTime()}};
    CPPUNIT_ASSERT_NO_THROW(rule.Add(order));
  }
}

void TimeFilterComplianceRuleTester::TestAddOutsideCompliancePeriod() {
  auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
  FixedTimeClient timeClient;
  TimeFilterComplianceRule<FixedTimeClient*> rule{START_TIME.time_of_day(),
    END_TIME.time_of_day(), &timeClient, std::move(baseRule)};
  {
    timeClient.SetTime(START_TIME - seconds(1));
    PrimitiveOrder order{{BuildOrderFields(), 1, timeClient.GetTime()}};
    CPPUNIT_ASSERT_NO_THROW(rule.Add(order));
  }
  {
    timeClient.SetTime(END_TIME + seconds(1));
    PrimitiveOrder order{{BuildOrderFields(), 2, timeClient.GetTime()}};
    CPPUNIT_ASSERT_NO_THROW(rule.Add(order));
  }
}

void TimeFilterComplianceRuleTester::TestSubmitDuringCompliancePeriod() {
  auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
  FixedTimeClient timeClient;
  TimeFilterComplianceRule<FixedTimeClient*> rule{START_TIME.time_of_day(),
    END_TIME.time_of_day(), &timeClient, std::move(baseRule)};
  {
    timeClient.SetTime(START_TIME);
    PrimitiveOrder order{{BuildOrderFields(), 1, timeClient.GetTime()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
  {
    timeClient.SetTime(END_TIME);
    PrimitiveOrder order{{BuildOrderFields(), 2, timeClient.GetTime()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
}

void TimeFilterComplianceRuleTester::TestSubmitOutsideCompliancePeriod() {
  auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
  FixedTimeClient timeClient;
  TimeFilterComplianceRule<FixedTimeClient*> rule{START_TIME.time_of_day(),
    END_TIME.time_of_day(), &timeClient, std::move(baseRule)};
  {
    timeClient.SetTime(START_TIME - seconds(1));
    PrimitiveOrder order{{BuildOrderFields(), 1, timeClient.GetTime()}};
    CPPUNIT_ASSERT_NO_THROW(rule.Submit(order));
  }
  {
    timeClient.SetTime(END_TIME + seconds(1));
    PrimitiveOrder order{{BuildOrderFields(), 2, timeClient.GetTime()}};
    CPPUNIT_ASSERT_NO_THROW(rule.Submit(order));
  }
}

void TimeFilterComplianceRuleTester::TestCancelDuringCompliancePeriod() {
  auto baseRule = std::make_unique<RejectCancelsComplianceRule>();
  FixedTimeClient timeClient;
  TimeFilterComplianceRule<FixedTimeClient*> rule{START_TIME.time_of_day(),
    END_TIME.time_of_day(), &timeClient, std::move(baseRule)};
  {
    timeClient.SetTime(START_TIME);
    PrimitiveOrder order{{BuildOrderFields(), 1, timeClient.GetTime()}};
    rule.Add(order);
    CPPUNIT_ASSERT_THROW(rule.Cancel(order), ComplianceCheckException);
  }
  {
    timeClient.SetTime(END_TIME);
    PrimitiveOrder order{{BuildOrderFields(), 2, timeClient.GetTime()}};
    rule.Add(order);
    CPPUNIT_ASSERT_THROW(rule.Cancel(order), ComplianceCheckException);
  }
}

void TimeFilterComplianceRuleTester::TestCancelOutsideCompliancePeriod() {
  auto baseRule = std::make_unique<RejectCancelsComplianceRule>();
  FixedTimeClient timeClient;
  TimeFilterComplianceRule<FixedTimeClient*> rule{START_TIME.time_of_day(),
    END_TIME.time_of_day(), &timeClient, std::move(baseRule)};
  {
    timeClient.SetTime(START_TIME - seconds(1));
    PrimitiveOrder order{{BuildOrderFields(), 1, timeClient.GetTime()}};
    rule.Add(order);
    CPPUNIT_ASSERT_NO_THROW(rule.Cancel(order));
  }
  {
    timeClient.SetTime(END_TIME + seconds(1));
    PrimitiveOrder order{{BuildOrderFields(), 2, timeClient.GetTime()}};
    rule.Add(order);
    CPPUNIT_ASSERT_NO_THROW(rule.Cancel(order));
  }
}

void TimeFilterComplianceRuleTester::TestStartTimeFollowsEndTime() {
  auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
  FixedTimeClient timeClient;
  TimeFilterComplianceRule<FixedTimeClient*> rule{END_TIME.time_of_day(),
    START_TIME.time_of_day(), &timeClient, std::move(baseRule)};
  {
    timeClient.SetTime(START_TIME - seconds(1));
    PrimitiveOrder order{{BuildOrderFields(), 1, timeClient.GetTime()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
  {
    timeClient.SetTime(END_TIME + seconds(1));
    PrimitiveOrder order{{BuildOrderFields(), 2, timeClient.GetTime()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
  {
    timeClient.SetTime(END_TIME - seconds(1));
    PrimitiveOrder order{{BuildOrderFields(), 3, timeClient.GetTime()}};
    CPPUNIT_ASSERT_NO_THROW(rule.Submit(order));
  }
  {
    timeClient.SetTime(START_TIME + seconds(1));
    PrimitiveOrder order{{BuildOrderFields(), 4, timeClient.GetTime()}};
    CPPUNIT_ASSERT_NO_THROW(rule.Submit(order));
  }
}
