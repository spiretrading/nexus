#include "Nexus/ComplianceTests/SymbolRestrictionComplianceRuleTester.hpp"
#include "Nexus/Compliance/SymbolRestrictionComplianceRule.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::Compliance::Tests;
using namespace Nexus::OrderExecutionService;
using namespace std;

namespace {
  OrderFields BuildOrderFields(string symbol, MarketCode market) {
    auto& marketEntry = GetDefaultMarketDatabase().FromCode(market);
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{std::move(symbol), market, marketEntry.m_countryCode},
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      Money::ONE);
    return fields;
  }
}

void SymbolRestrictionComplianceRuleTester::TestEmptyRestriction() {
  SymbolRestrictionComplianceRule rule{vector<ComplianceParameter>()};
  PrimitiveOrder order{{BuildOrderFields("TST1", DefaultMarkets::TSX()), 1,
    second_clock::universal_time()}};
  rule.Submit(order);
}

void SymbolRestrictionComplianceRuleTester::
    TestSingleSymbolSubmissionAndRestriction() {
  vector<ComplianceValue> symbols;
  symbols.push_back(Security{"TST1", DefaultMarkets::TSX(),
    DefaultCountries::CA()});
  vector<ComplianceParameter> parameters;
  parameters.emplace_back("symbols", symbols);
  SymbolRestrictionComplianceRule rule(parameters);
  {
    PrimitiveOrder order{{BuildOrderFields("TST1", DefaultMarkets::TSX()), 1,
      second_clock::universal_time()}};
    CPPUNIT_ASSERT_THROW(rule.Submit(order), ComplianceCheckException);
  }
  {
    PrimitiveOrder order{{BuildOrderFields("TST2", DefaultMarkets::TSX()), 2,
      second_clock::universal_time()}};
    CPPUNIT_ASSERT_NO_THROW(rule.Submit(order));
  }
  {
    PrimitiveOrder order{{BuildOrderFields("TST1", DefaultMarkets::ASX()), 3,
      second_clock::universal_time()}};
    CPPUNIT_ASSERT_NO_THROW(rule.Submit(order));
  }
}
