#include <doctest/doctest.h>
#include "Nexus/Compliance/BuyingPowerComplianceRule.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  using TestBuyingPowerComplianceRule =
    BuyingPowerComplianceRule<MarketDataClient>;
}

TEST_SUITE("BuyingPowerComplianceRule") {
  TEST_CASE("foo") {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("currency", USD);
    parameters.emplace_back("buying_power", Money::ZERO);
    parameters.emplace_back("region", Region::GLOBAL);
    auto exchange_rates = ExchangeRateTable();
    auto market_data_operations = make_test_market_data_operation_queue();
    auto client = TestMarketDataClient(market_data_operations);
    auto rule = BuyingPowerComplianceRule(parameters, exchange_rates, client);
  }
}
