#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <boost/functional/factory.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceRuleSet.hpp"
#include "Nexus/Compliance/SymbolRestrictionComplianceRule.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace boost;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::OrderExecutionService;

namespace {
  struct Fixture {
    using TestComplianceClient = ComplianceClient<
      TestServiceProtocolClientBuilder>;
    using TestComplianceRuleSet = ComplianceRuleSet<TestComplianceClient*,
      ServiceLocatorClientBox>;
    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    optional<TestServiceProtocolServer> m_server;
    optional<TestComplianceClient> m_complianceClient;
    optional<TestComplianceRuleSet> m_complianceRuleSet;

    Fixture() {
      auto serviceLocatorClient = m_serviceLocatorEnvironment.MakeClient();
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_complianceClient.emplace(TestServiceProtocolClientBuilder(
        std::bind_front(
          factory<std::unique_ptr<TestServiceProtocolClientBuilder::Channel>>(),
          "test", std::ref(*serverConnection)),
        factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>()));
      m_complianceRuleSet.emplace(&*m_complianceClient,
        std::move(serviceLocatorClient),
        std::bind(factory<std::unique_ptr<SymbolRestrictionComplianceRule>>(),
          std::vector<ComplianceParameter>()));
    }
  };

  auto MakeOrderFields(std::string symbol, MarketCode market) {
    auto fields = OrderFields::MakeLimitOrder(DirectoryEntry::GetRootAccount(),
      Security(std::move(symbol), market, DefaultCountries::CA()),
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      Money::ONE);
    return fields;
  }
}

TEST_SUITE("ComplianceRuleSet") {
  TEST_CASE("submit") {
    auto session = OrderExecutionSession();
    auto fields = MakeOrderFields("TST1", DefaultMarkets::TSX());
    //  m_complianceRuleSet->Submit(session, 1, fields, false);
  }
}
