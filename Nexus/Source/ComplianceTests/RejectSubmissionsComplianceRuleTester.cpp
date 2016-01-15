#include "Nexus/ComplianceTests/RejectSubmissionsComplianceRuleTester.hpp"
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
using namespace Nexus::Compliance::Tests;
using namespace Nexus::OrderExecutionService;
using namespace std;

namespace {
  const auto TIMESTAMP = ptime{date{1984, May, 6}, seconds(10)};

  OrderFields BuildOrderFields() {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{"TST", DefaultMarkets::TSX(), DefaultCountries::CA()},
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      Money::ONE);
    return fields;
  }
}

void RejectSubmissionsComplianceRuleTester::TestAdd() {
  RejectSubmissionsComplianceRule rule{"message"};
  PrimitiveOrder order{{BuildOrderFields(), 1, TIMESTAMP}};
  CPPUNIT_ASSERT_NO_THROW(rule.Add(order));
}

void RejectSubmissionsComplianceRuleTester::TestSubmit() {
  RejectSubmissionsComplianceRule rule{"message"};
  PrimitiveOrder order{{BuildOrderFields(), 1, TIMESTAMP}};
  CPPUNIT_ASSERT_THROW_MESSAGE("message", rule.Submit(order),
    ComplianceCheckException);
}

void RejectSubmissionsComplianceRuleTester::TestCancel() {
  RejectSubmissionsComplianceRule rule{"message"};
  PrimitiveOrder order{{BuildOrderFields(), 1, TIMESTAMP}};
  rule.Add(order);
  CPPUNIT_ASSERT_NO_THROW(rule.Cancel(order));
}
