#include <doctest/doctest.h>
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
using namespace Nexus::OrderExecutionService;

namespace {
  const auto START_TIME = ptime(date(1984, May, 6), seconds(10));
  const auto END_TIME = ptime(date(1984, May, 6), seconds(20));

  auto BuildOrderFields() {
    return OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security("TST", DefaultMarkets::TSX(), DefaultCountries::CA()),
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      Money::ONE);
  }
}

TEST_SUITE("TimeFilterComplianceRule") {
  TEST_CASE("add_during_compliance_period") {
    auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
    auto timeClient = FixedTimeClient();
    auto rule = TimeFilterComplianceRule(START_TIME.time_of_day(),
      END_TIME.time_of_day(), &timeClient, std::move(baseRule));
    {
      timeClient.SetTime(START_TIME);
      auto order = PrimitiveOrder({BuildOrderFields(), 1,
        timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Add(order));
    }
    {
      timeClient.SetTime(END_TIME);
      auto order = PrimitiveOrder({BuildOrderFields(), 2,
        timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Add(order));
    }
  }

  TEST_CASE("add_outside_compliance_period") {
    auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
    auto timeClient = FixedTimeClient();
    auto rule = TimeFilterComplianceRule(START_TIME.time_of_day(),
      END_TIME.time_of_day(), &timeClient, std::move(baseRule));
    {
      timeClient.SetTime(START_TIME - seconds(1));
      auto order = PrimitiveOrder({BuildOrderFields(), 1,
        timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Add(order));
    }
    {
      timeClient.SetTime(END_TIME + seconds(1));
      auto order = PrimitiveOrder({BuildOrderFields(), 2,
        timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Add(order));
    }
  }

  TEST_CASE("submit_during_compliance_period") {
    auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
    auto timeClient = FixedTimeClient();
    auto rule = TimeFilterComplianceRule(START_TIME.time_of_day(),
      END_TIME.time_of_day(), &timeClient, std::move(baseRule));
    {
      timeClient.SetTime(START_TIME);
      auto order = PrimitiveOrder({BuildOrderFields(), 1,
        timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    {
      timeClient.SetTime(END_TIME);
      auto order = PrimitiveOrder({BuildOrderFields(), 2,
        timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
  }

  TEST_CASE("submit_outside_compliance_period") {
    auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
    auto timeClient = FixedTimeClient();
    auto rule = TimeFilterComplianceRule(START_TIME.time_of_day(),
      END_TIME.time_of_day(), &timeClient, std::move(baseRule));
    {
      timeClient.SetTime(START_TIME - seconds(1));
      auto order = PrimitiveOrder({BuildOrderFields(), 1,
        timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Submit(order));
    }
    {
      timeClient.SetTime(END_TIME + seconds(1));
      auto order = PrimitiveOrder({BuildOrderFields(), 2,
        timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Submit(order));
    }
  }

  TEST_CASE("cancel_during_compliance_period") {
    auto baseRule = std::make_unique<RejectCancelsComplianceRule>();
    auto timeClient = FixedTimeClient();
    auto rule = TimeFilterComplianceRule(START_TIME.time_of_day(),
      END_TIME.time_of_day(), &timeClient, std::move(baseRule));
    {
      timeClient.SetTime(START_TIME);
      auto order = PrimitiveOrder({BuildOrderFields(), 1,
        timeClient.GetTime()});
      rule.Add(order);
      REQUIRE_THROWS_AS(rule.Cancel(order), ComplianceCheckException);
    }
    {
      timeClient.SetTime(END_TIME);
      auto order = PrimitiveOrder({BuildOrderFields(), 2,
        timeClient.GetTime()});
      rule.Add(order);
      REQUIRE_THROWS_AS(rule.Cancel(order), ComplianceCheckException);
    }
  }

  TEST_CASE("cancel_outside_compliance_period") {
    auto baseRule = std::make_unique<RejectCancelsComplianceRule>();
    auto timeClient = FixedTimeClient();
    auto rule = TimeFilterComplianceRule(START_TIME.time_of_day(),
      END_TIME.time_of_day(), &timeClient, std::move(baseRule));
    {
      timeClient.SetTime(START_TIME - seconds(1));
      auto order = PrimitiveOrder({BuildOrderFields(), 1,
        timeClient.GetTime()});
      rule.Add(order);
      REQUIRE_NOTHROW(rule.Cancel(order));
    }
    {
      timeClient.SetTime(END_TIME + seconds(1));
      auto order = PrimitiveOrder({BuildOrderFields(), 2,
        timeClient.GetTime()});
      rule.Add(order);
      REQUIRE_NOTHROW(rule.Cancel(order));
    }
  }

  TEST_CASE("start_time_follows_end_time") {
    auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
    auto timeClient = FixedTimeClient();
    auto rule = TimeFilterComplianceRule(END_TIME.time_of_day(),
      START_TIME.time_of_day(), &timeClient, std::move(baseRule));
    {
      timeClient.SetTime(START_TIME - seconds(1));
      auto order = PrimitiveOrder({BuildOrderFields(), 1,
        timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    {
      timeClient.SetTime(END_TIME + seconds(1));
      auto order = PrimitiveOrder({BuildOrderFields(), 2,
        timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    {
      timeClient.SetTime(END_TIME - seconds(1));
      auto order = PrimitiveOrder({BuildOrderFields(), 3,
        timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Submit(order));
    }
    {
      timeClient.SetTime(START_TIME + seconds(1));
      auto order = PrimitiveOrder({BuildOrderFields(), 4,
        timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Submit(order));
    }
  }
}
