#include <doctest/doctest.h>
#include "Nexus/Compliance/RejectSubmissionsComplianceRule.hpp"
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

  auto MakeOrderFields() {
    return OrderFields::MakeLimitOrder(DirectoryEntry::GetRootAccount(),
      Security("TST", DefaultMarkets::TSX(), DefaultCountries::CA()),
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      Money::ONE);
  }
}

TEST_SUITE("RejectSubmissionsComplianceRule") {
  TEST_CASE("add") {
    auto rule = RejectSubmissionsComplianceRule("message");
    auto order = PrimitiveOrder({MakeOrderFields(), 1, TIMESTAMP});
    REQUIRE_NOTHROW(rule.Add(order));
  }

  TEST_CASE("submit") {
    auto rule = RejectSubmissionsComplianceRule("message");
    auto order = PrimitiveOrder({MakeOrderFields(), 1, TIMESTAMP});
    REQUIRE_THROWS_WITH_AS(rule.Submit(order), "message",
      ComplianceCheckException);
  }

  TEST_CASE("cancel") {
    auto rule = RejectSubmissionsComplianceRule("message");
    auto order = PrimitiveOrder({MakeOrderFields(), 1, TIMESTAMP});
    rule.Add(order);
    REQUIRE_NOTHROW(rule.Cancel(order));
  }
}
