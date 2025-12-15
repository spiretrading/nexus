#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionService/BuyingPowerCheck.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  auto make_test_order_fields() {
    auto account = DirectoryEntry::make_account(123, "test");
    auto security = Security("TST", TSX);
    auto currency = CAD;
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    return make_limit_order_fields(
      account, security, currency, side, destination, quantity, price);
  }

  struct Fixture {
    ServiceLocatorTestEnvironment m_service_locator_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    MarketDataServiceTestEnvironment m_market_data_environment;

    Fixture()
      : m_administration_environment(
          make_administration_service_test_environment(
            m_service_locator_environment)),
        m_market_data_environment(
          make_market_data_service_test_environment(
            m_service_locator_environment, m_administration_environment)) {}
  };
}

TEST_SUITE("BuyingPowerCheck") {
  TEST_CASE("submit_within_buying_power") {
    auto fixture = Fixture();
    auto fields = make_test_order_fields();
    auto& administration_client =
      fixture.m_administration_environment.get_client();
    auto& market_data_client =
      fixture.m_market_data_environment.get_registry_client();
    auto exchange_rates = ExchangeRateTable();
    exchange_rates.add(
      ExchangeRate(CurrencyPair(CAD, USD), rational<int>(3, 4)));
    auto check = make_buying_power_check(
      exchange_rates, administration_client, market_data_client);
    auto risk_parameters = RiskParameters();
    risk_parameters.m_currency = CAD;
    risk_parameters.m_buying_power = 1000 * Money::ONE;
    administration_client.store(fields.m_account, risk_parameters);
    auto& feed_client = fixture.m_market_data_environment.get_feed_client();
    auto bbo = BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 100),
      time_from_string("2024-07-18 10:00:00"));
    feed_client.publish(SecurityBboQuote(bbo, fields.m_security));
    auto order_info =
      OrderInfo(fields, 1, time_from_string("2024-07-18 10:01:00"));
    REQUIRE_NOTHROW(check->submit(order_info));
  }
}
