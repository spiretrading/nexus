#include "Nexus/ComplianceTests/ComplianceRuleSetTester.hpp"
#include "Nexus/Compliance/SymbolRestrictionComplianceRule.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::Compliance::Tests;
using namespace Nexus::OrderExecutionService;
using namespace std;

namespace {
  OrderFields BuildOrderFields(string symbol, MarketCode market) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{std::move(symbol), market, DefaultCountries::CA()},
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      Money::ONE);
    return fields;
  }
}

void ComplianceRuleSetTester::setUp() {
  m_serviceLocatorInstance.Initialize();
  m_serviceLocatorInstance->Open();
  auto serviceLocatorClient = m_serviceLocatorInstance->BuildClient();
  serviceLocatorClient->SetCredentials("root", "");
  serviceLocatorClient->Open();
  ServiceProtocolClientBuilder builder(
    [=] {
      return std::make_unique<ServiceProtocolClientBuilder::Channel>(("test"),
        Ref(*m_serverConnection));
    },
    [] {
      return std::make_unique<ServiceProtocolClientBuilder::Timer>();
    });
  m_complianceClient.Initialize(builder);
  m_complianceRuleSet.Initialize(&*m_complianceClient,
    std::move(serviceLocatorClient),
    [&] (const ComplianceRuleEntry& entry) {
      return std::make_unique<SymbolRestrictionComplianceRule>(
        vector<ComplianceParameter>());
    });
}

void ComplianceRuleSetTester::tearDown() {
  m_complianceRuleSet.Reset();
  m_complianceClient.Reset();
  m_serviceLocatorInstance.Reset();
}

void ComplianceRuleSetTester::TestSubmit() {
  OrderExecutionSession session;
  auto fields = BuildOrderFields("TST1", DefaultMarkets::TSX());
//  m_complianceRuleSet->Submit(session, 1, fields, false);
}
