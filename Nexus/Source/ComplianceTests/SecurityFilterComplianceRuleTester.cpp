#include <doctest/doctest.h>
#include "Nexus/Compliance/RejectCancelsComplianceRule.hpp"
#include "Nexus/Compliance/RejectSubmissionsComplianceRule.hpp"
#include "Nexus/Compliance/SecurityFilterComplianceRule.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
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

  auto MakeOrderFields(std::string symbol) {
    return OrderFields::MakeLimitOrder(DirectoryEntry::GetRootAccount(),
      Security(std::move(symbol), DefaultMarkets::TSX(),
      DefaultCountries::CA()), DefaultCurrencies::CAD(), Side::BID,
      DefaultDestinations::TSX(), 100, Money::ONE);
  }

  auto MakeRegion() {
    return Security("A", DefaultMarkets::TSX(), DefaultCountries::CA());
  }
}

TEST_SUITE("SecurityFilterComplianceRule") {
  TEST_CASE("matching_add") {
    auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
    auto rule = SecurityFilterComplianceRule(MakeRegion(), std::move(baseRule));
    auto order = PrimitiveOrder({MakeOrderFields("A"), 1, TIMESTAMP});
    REQUIRE_NOTHROW(rule.Add(order));
  }

  TEST_CASE("unmatching_add") {
    auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
    auto rule = SecurityFilterComplianceRule(MakeRegion(), std::move(baseRule));
    auto order = PrimitiveOrder({MakeOrderFields("B"), 1, TIMESTAMP});
    REQUIRE_NOTHROW(rule.Add(order));
  }

  TEST_CASE("matching_submit") {
    auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
    auto rule = SecurityFilterComplianceRule(MakeRegion(), std::move(baseRule));
    auto order = PrimitiveOrder({MakeOrderFields("A"), 1, TIMESTAMP});
    REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
  }

  TEST_CASE("unmatching_submit") {
    auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
    auto rule = SecurityFilterComplianceRule(MakeRegion(), std::move(baseRule));
    auto order = PrimitiveOrder({MakeOrderFields("B"), 1, TIMESTAMP});
    REQUIRE_NOTHROW(rule.Submit(order));
  }

  TEST_CASE("matching_cancel") {
    auto baseRule = std::make_unique<RejectCancelsComplianceRule>();
    auto rule = SecurityFilterComplianceRule(MakeRegion(), std::move(baseRule));
    auto order = PrimitiveOrder({MakeOrderFields("A"), 1, TIMESTAMP});
    REQUIRE_THROWS_AS(rule.Cancel(order), ComplianceCheckException);
  }

  TEST_CASE("unmatching_cancel") {
    auto baseRule = std::make_unique<RejectCancelsComplianceRule>();
    auto rule = SecurityFilterComplianceRule(MakeRegion(), std::move(baseRule));
    auto order = PrimitiveOrder({MakeOrderFields("B"), 1, TIMESTAMP});
    REQUIRE_NOTHROW(rule.Cancel(order));
  }
}
