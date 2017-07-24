#include "Nexus/ComplianceTests/ComplianceRuleSetTester.hpp"
#include <boost/functional/factory.hpp>
#include "Nexus/Compliance/SymbolRestrictionComplianceRule.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace boost;
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
  m_serviceLocatorEnvironment.emplace();
  m_serviceLocatorEnvironment->Open();
  auto serviceLocatorClient = m_serviceLocatorEnvironment->BuildClient();
  serviceLocatorClient->SetCredentials("root", "");
  serviceLocatorClient->Open();
  auto serverConnection = std::make_shared<TestServerConnection>();
  TestServiceProtocolClientBuilder builder{
    [=] {
      return std::make_unique<TestServiceProtocolClientBuilder::Channel>("test",
        Ref(*serverConnection));
    }, factory<unique_ptr<TestServiceProtocolClientBuilder::Timer>>()};
  m_complianceClient.emplace(builder);
  m_complianceRuleSet.emplace(&*m_complianceClient,
    std::move(serviceLocatorClient),
    [&] (auto& entry) {
      return std::make_unique<SymbolRestrictionComplianceRule>(
        vector<ComplianceParameter>());
    });
}

void ComplianceRuleSetTester::tearDown() {
  m_complianceRuleSet.reset();
  m_complianceClient.reset();
  m_serviceLocatorEnvironment.reset();
}

void ComplianceRuleSetTester::TestSubmit() {
  OrderExecutionSession session;
  auto fields = BuildOrderFields("TST1", DefaultMarkets::TSX());
//  m_complianceRuleSet->Submit(session, 1, fields, false);
}
