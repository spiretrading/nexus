#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
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
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  const auto TIMESTAMP = ptime(date(1984, May, 6), seconds(10));
  const auto DURATION = seconds(10);
  const auto RANGE = 2 * Money::CENT;

  auto BuildLimitOrderFields(Side side, Money price) {
    return OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security("TST", DefaultMarkets::TSX(), DefaultCountries::CA()),
      DefaultCurrencies::CAD(), side, DefaultDestinations::TSX(), 1000, price);
  }

  auto BuildMarketOrderFields(Side side) {
    return OrderFields::BuildMarketOrder(DirectoryEntry::GetRootAccount(),
      Security("TST", DefaultMarkets::TSX(), DefaultCountries::CA()),
      DefaultCurrencies::CAD(), side, DefaultDestinations::TSX(), 1000);
  }
}

TEST_SUITE("OpposingOrderSubmissionComplianceRule") {
  TEST_CASE("limit_ask_cancel") {
    auto timeClient = FixedTimeClient(TIMESTAMP);
    auto rule = OpposingOrderSubmissionComplianceRule(DURATION, RANGE,
      &timeClient);
    auto orderA = PrimitiveOrder({BuildLimitOrderFields(Side::ASK, Money::ONE),
      1, timeClient.GetTime()});
    {
      REQUIRE_NOTHROW(rule.Submit(orderA));
      SetOrderStatus(orderA, OrderStatus::NEW, timeClient.GetTime());
      REQUIRE_NOTHROW(rule.Cancel(orderA));
      SetOrderStatus(orderA, OrderStatus::PENDING_CANCEL, timeClient.GetTime());
      Cancel(orderA, timeClient.GetTime());
    }
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::BID, Money::ONE),
        2, timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::BID,
        99 * Money::CENT), 3, timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::BID,
        98 * Money::CENT), 4, timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::BID,
        97 * Money::CENT), 5, timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Submit(order));
    }
    {
      auto order = PrimitiveOrder({BuildMarketOrderFields(Side::BID), 6,
        timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    timeClient.SetTime(timeClient.GetTime() + DURATION + seconds(1));
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::BID, Money::ONE),
        7, timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Submit(order));
    }
  }

  TEST_CASE("limit_bid_cancel") {
    auto timeClient = FixedTimeClient(TIMESTAMP);
    auto rule = OpposingOrderSubmissionComplianceRule(DURATION, RANGE,
      &timeClient);
    auto order = PrimitiveOrder({BuildLimitOrderFields(Side::BID, Money::ONE),
      1, timeClient.GetTime()});
    {
      REQUIRE_NOTHROW(rule.Submit(order));
      SetOrderStatus(order, OrderStatus::NEW, timeClient.GetTime());
      REQUIRE_NOTHROW(rule.Cancel(order));
      SetOrderStatus(order, OrderStatus::PENDING_CANCEL, timeClient.GetTime());
      Cancel(order, timeClient.GetTime());
    }
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::ASK, Money::ONE),
        2, timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::ASK,
        Money::ONE + Money::CENT), 3, timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::ASK,
        Money::ONE + 2 * Money::CENT), 4, timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::ASK,
        Money::ONE + 3 * Money::CENT), 5, timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Submit(order));
    }
    {
      auto order = PrimitiveOrder({BuildMarketOrderFields(Side::ASK), 6,
        timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    timeClient.SetTime(timeClient.GetTime() + DURATION + seconds(1));
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::ASK,
        Money::ONE + 2 * Money::CENT), 7, timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Submit(order));
    }
  }

  TEST_CASE("better_price") {
    auto timeClient = FixedTimeClient(TIMESTAMP);
    auto rule = OpposingOrderSubmissionComplianceRule(DURATION, RANGE,
      &timeClient);
    auto orderA = PrimitiveOrder({BuildLimitOrderFields(Side::BID, Money::ONE),
      1, timeClient.GetTime()});
    {
      REQUIRE_NOTHROW(rule.Submit(orderA));
      SetOrderStatus(orderA, OrderStatus::NEW, timeClient.GetTime());
      REQUIRE_NOTHROW(rule.Cancel(orderA));
      SetOrderStatus(orderA, OrderStatus::PENDING_CANCEL, timeClient.GetTime());
      Cancel(orderA, timeClient.GetTime());
    }
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::ASK,
        Money::ONE + 3 * Money::CENT), 2, timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Submit(order));
    }
    auto orderB = PrimitiveOrder({BuildLimitOrderFields(Side::BID,
      Money::ONE + Money::CENT), 3, timeClient.GetTime()});
    {
      REQUIRE_NOTHROW(rule.Submit(orderB));
      SetOrderStatus(orderB, OrderStatus::NEW, timeClient.GetTime());
      REQUIRE_NOTHROW(rule.Cancel(orderB));
      SetOrderStatus(orderB, OrderStatus::PENDING_CANCEL, timeClient.GetTime());
      Cancel(orderB, timeClient.GetTime());
    }
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::ASK,
        Money::ONE + 3 * Money::CENT), 4, timeClient.GetTime()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    timeClient.SetTime(timeClient.GetTime() + DURATION + seconds(1));
    {
      auto order = PrimitiveOrder({BuildLimitOrderFields(Side::ASK,
        Money::ONE + 3 * Money::CENT), 5, timeClient.GetTime()});
      REQUIRE_NOTHROW(rule.Submit(order));
    }
  }
}
