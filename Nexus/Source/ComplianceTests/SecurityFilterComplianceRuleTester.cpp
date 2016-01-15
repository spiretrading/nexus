#include "Nexus/ComplianceTests/SecurityFilterComplianceRuleTester.hpp"
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
using namespace Nexus::Compliance::Tests;
using namespace Nexus::OrderExecutionService;
using namespace std;

namespace {
  const auto TIMESTAMP = ptime{date{1984, May, 6}, seconds(10)};

  OrderFields BuildOrderFields(string symbol) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{std::move(symbol), DefaultMarkets::TSX(),
      DefaultCountries::CA()}, DefaultCurrencies::CAD(), Side::BID,
      DefaultDestinations::TSX(), 100, Money::ONE);
    return fields;
  }

  SecuritySet BuildSecuritySet() {
    SecuritySet securities;
    securities.Add(Security{"A", DefaultMarkets::TSX(),
      DefaultCountries::CA()});
    return securities;
  }
}

void SecurityFilterComplianceRuleTester::TestMatchingAdd() {
  auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
  SecurityFilterComplianceRule rule{BuildSecuritySet(), std::move(baseRule)};
  PrimitiveOrder order{{BuildOrderFields("A"), 1, TIMESTAMP}};
  CPPUNIT_ASSERT_NO_THROW(rule.Add(order));
}

void SecurityFilterComplianceRuleTester::TestUnmatchingAdd() {
  auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
  SecurityFilterComplianceRule rule{BuildSecuritySet(), std::move(baseRule)};
  PrimitiveOrder order{{BuildOrderFields("B"), 1, TIMESTAMP}};
  CPPUNIT_ASSERT_NO_THROW(rule.Add(order));
}

void SecurityFilterComplianceRuleTester::TestMatchingSubmit() {
  auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
  SecurityFilterComplianceRule rule{BuildSecuritySet(), std::move(baseRule)};
  PrimitiveOrder order{{BuildOrderFields("A"), 1, TIMESTAMP}};
  CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
}

void SecurityFilterComplianceRuleTester::TestUnmatchingSubmit() {
  auto baseRule = std::make_unique<RejectSubmissionsComplianceRule>();
  SecurityFilterComplianceRule rule{BuildSecuritySet(), std::move(baseRule)};
  PrimitiveOrder order{{BuildOrderFields("B"), 1, TIMESTAMP}};
  CPPUNIT_ASSERT_NO_THROW(rule.Submit(order));
}

void SecurityFilterComplianceRuleTester::TestMatchingCancel() {
  auto baseRule = std::make_unique<RejectCancelsComplianceRule>();
  SecurityFilterComplianceRule rule{BuildSecuritySet(), std::move(baseRule)};
  PrimitiveOrder order{{BuildOrderFields("A"), 1, TIMESTAMP}};
  CPPUNIT_ASSERT_THROW(rule.Cancel(order), ComplianceCheckException);
}

void SecurityFilterComplianceRuleTester::TestUnmatchingCancel() {
  auto baseRule = std::make_unique<RejectCancelsComplianceRule>();
  SecurityFilterComplianceRule rule{BuildSecuritySet(), std::move(baseRule)};
  PrimitiveOrder order{{BuildOrderFields("B"), 1, TIMESTAMP}};
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(order));
}
