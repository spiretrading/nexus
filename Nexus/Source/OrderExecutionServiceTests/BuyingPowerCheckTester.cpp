#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionService/BuyingPowerCheck.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Tests;
using namespace Nexus::Venues;

namespace {
  auto make_test_order_fields() {
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto currency = CAD;
    auto side = Side::BID;
    auto destination = Destinations::TSX;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    return make_limit_order_fields(
      account, ticker, currency, side, destination, quantity, price);
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
    fixture.m_administration_environment.get_data_store().store(
      fields.m_account, risk_parameters);
    auto& feed_client = fixture.m_market_data_environment.get_feed_client();
    auto bbo = BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 100),
      time_from_string("2024-07-18 10:00:00"));
    feed_client.publish(TickerBboQuote(bbo, fields.m_ticker));
    auto order_info =
      OrderInfo(fields, 1, time_from_string("2024-07-18 10:01:00"));
    REQUIRE_NOTHROW(check->submit(order_info));
  }

  TEST_CASE("restore") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto& administration_client =
      fixture.m_administration_environment.get_client();
    auto& market_data_client =
      fixture.m_market_data_environment.get_registry_client();
    auto exchange_rates = ExchangeRateTable();
    auto check = make_buying_power_check(
      exchange_rates, administration_client, market_data_client);
    auto risk_parameters = RiskParameters();
    risk_parameters.m_currency = CAD;
    risk_parameters.m_buying_power = 100 * Money::ONE;
    fixture.m_administration_environment.get_data_store().store(
      account, risk_parameters);
    auto& feed_client = fixture.m_market_data_environment.get_feed_client();
    auto bbo = BboQuote(make_bid(Money::ONE, 100), make_ask(Money::ONE, 100),
      time_from_string("2024-07-18 10:00:00"));
    feed_client.publish(TickerBboQuote(bbo, ticker));
    auto snapshot = InventorySnapshot();
    snapshot.m_inventories.push_back(Inventory(
      Position(ticker, CAD, 100, 100 * Money::ONE), Money::ZERO, Money::ZERO,
      100, 1));
    check->restore(account, snapshot, {});
    auto buy_info = OrderInfo(make_limit_order_fields(
      account, ticker, CAD, Side::BID, Destinations::TSX, 100, Money::ONE), 1,
      time_from_string("2024-07-18 10:01:00"));
    REQUIRE_THROWS_AS(check->submit(buy_info), OrderSubmissionCheckException);
    auto sell_info = OrderInfo(make_limit_order_fields(
      account, ticker, CAD, Side::ASK, Destinations::TSX, 100, Money::ONE), 2,
      time_from_string("2024-07-18 10:02:00"));
    REQUIRE_NOTHROW(check->submit(sell_info));
  }
}
