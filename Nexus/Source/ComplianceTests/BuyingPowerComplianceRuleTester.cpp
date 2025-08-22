#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Compliance/BuyingPowerComplianceRule.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::Compliance;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::OrderExecutionService;

namespace {
  using TestBuyingPowerComplianceRule =
    BuyingPowerComplianceRule<MarketDataClient>;

  struct Fixture {
    ServiceLocatorTestEnvironment m_service_locator_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    MarketDataServiceTestEnvironment m_market_data_environment;

    Fixture()
      : m_administration_environment(
          make_administration_service_test_environment(
            m_service_locator_environment)),
        m_market_data_environment(make_market_data_service_test_environment(
          m_service_locator_environment, m_administration_environment)) {}
  };
}

TEST_SUITE("BuyingPowerComplianceRule") {
  TEST_CASE("submit_within_buying_power") {
    auto fixture = Fixture();
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("currency", USD);
    parameters.emplace_back("buying_power", 1000 * Money::ONE);
    auto exchange_rates = ExchangeRateTable();
    auto rule = BuyingPowerComplianceRule(parameters, exchange_rates,
      fixture.m_market_data_environment.get_registry_client());
    auto security = Security("TST", NYSE);
    fixture.m_market_data_environment.update_bbo(security, Money::ONE);
    auto fields = make_limit_order_fields(
      DirectoryEntry::MakeAccount(1, "test"), security, USD, Side::BID, "NYSE",
      10, 10 * Money::ONE);
    auto order_info =
      OrderInfo(fields, 1, time_from_string("2024-07-25 10:00:00"));
    auto order = std::make_shared<PrimitiveOrder>(order_info);
    REQUIRE_NOTHROW(rule.submit(order));
  }

  TEST_CASE("submit_exceeds_buying_power") {
    auto fixture = Fixture();
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("currency", USD);
    parameters.emplace_back("buying_power", 100 * Money::ONE);
    auto exchange_rates = ExchangeRateTable();
    auto rule = BuyingPowerComplianceRule(parameters, exchange_rates,
      fixture.m_market_data_environment.get_registry_client());
    auto security = Security("TST", NYSE);
    fixture.m_market_data_environment.update_bbo(security, Money::ONE);
    auto fields = make_limit_order_fields(
      DirectoryEntry::MakeAccount(1, "test"), security, USD, Side::BID, "NYSE",
      101, 2 * Money::ONE);
    auto order_info =
      OrderInfo(fields, 1, time_from_string("2024-07-25 10:00:00"));
    auto order = std::make_shared<PrimitiveOrder>(order_info);
    REQUIRE_THROWS_AS(rule.submit(order), ComplianceCheckException);
  }
}
