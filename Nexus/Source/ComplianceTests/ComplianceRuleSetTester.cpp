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
      std::unique_ptr<VirtualServiceLocatorClient>>;

    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    boost::optional<TestServiceProtocolServer> m_server;
    boost::optional<TestComplianceClient> m_complianceClient;
    boost::optional<TestComplianceRuleSet> m_complianceRuleSet;

    Fixture() {
      m_serviceLocatorEnvironment.Open();
      auto serviceLocatorClient = m_serviceLocatorEnvironment.BuildClient();
      serviceLocatorClient->SetCredentials("root", "");
      serviceLocatorClient->Open();
      auto serverConnection = std::make_shared<TestServerConnection>();
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
            "test", Ref(*serverConnection));
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_complianceClient.emplace(builder);
      m_complianceRuleSet.emplace(&*m_complianceClient,
        std::move(serviceLocatorClient),
        [] (const auto& entry) {
          return std::make_unique<SymbolRestrictionComplianceRule>(
            std::vector<ComplianceParameter>());
        });
    }
  };

  auto BuildOrderFields(std::string symbol, MarketCode market) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security(std::move(symbol), market, DefaultCountries::CA()),
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      Money::ONE);
    return fields;
  }
}

TEST_SUITE("ComplianceRuleSet") {
  TEST_CASE("submit") {
    auto session = OrderExecutionSession();
    auto fields = BuildOrderFields("TST1", DefaultMarkets::TSX());
    //  m_complianceRuleSet->Submit(session, 1, fields, false);
  }
}
