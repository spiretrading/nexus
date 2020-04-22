#include <doctest/doctest.h>
#include "Nexus/Compliance/RejectCancelsComplianceRule.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::OrderExecutionService;

namespace {
  const auto TIMESTAMP = ptime(date(1984, May, 6), seconds(10));

  auto BuildOrderFields() {
    return OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security("TST", DefaultMarkets::TSX(), DefaultCountries::CA()),
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      Money::ONE);
  }
}

TEST_SUITE("RejectCancelsComplianceRule") {
  TEST_CASE("add") {
    auto rule = RejectCancelsComplianceRule("message");
    auto order = PrimitiveOrder({BuildOrderFields(), 1, TIMESTAMP});
    REQUIRE_NOTHROW(rule.Add(order));
  }

  TEST_CASE("submit") {
    auto rule = RejectCancelsComplianceRule("message");
    auto order = PrimitiveOrder({BuildOrderFields(), 1, TIMESTAMP});
    REQUIRE_NOTHROW(rule.Submit(order));
  }

  TEST_CASE("cancel") {
    auto rule = RejectCancelsComplianceRule("message");
    auto order = PrimitiveOrder({BuildOrderFields(), 1, TIMESTAMP});
    rule.Add(order);
    REQUIRE_THROWS_WITH_AS(rule.Cancel(order), "message",
      ComplianceCheckException);
  }
}
