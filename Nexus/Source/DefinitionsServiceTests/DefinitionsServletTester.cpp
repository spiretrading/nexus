#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/optional/optional.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/DefinitionsService/DefinitionsServlet.hpp"

using namespace Beam;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCountries;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultDestinations;
using namespace Nexus::DefaultVenues;

namespace {
  using TestServletContainer = TestServiceProtocolServletContainer<
    MetaDefinitionsServlet, NativePointerPolicy>;

  struct Fixture {
    std::vector<ExchangeRate> m_exchange_rates;
    std::vector<ComplianceRuleSchema> m_compliance_rule_schemas;
    TradingSchedule m_trading_schedule;
    optional<TestServletContainer::Servlet> m_servlet;
    optional<TestServletContainer> m_container;
    optional<TestServiceProtocolClient> m_client;

    Fixture() {
      m_exchange_rates.push_back(
        ExchangeRate(CurrencyPair(AUD, CAD), rational<int>(1, 3)));
      m_compliance_rule_schemas.push_back(ComplianceRuleSchema("rule1", {}));
      auto events = std::vector<TradingSchedule::Event>();
      events.push_back(
        {"O", ptime(date(2025, 7, 1), time_duration(1, 30, 0, 0))});
      auto rules = std::vector<TradingSchedule::Rule>();
      rules.push_back(TradingSchedule::Rule(
        {ASX}, {Tuesday}, {1}, {7}, {2025}, events));
      m_trading_schedule = TradingSchedule(rules);
      m_servlet.emplace("1234", "Spire",
        Nexus::Details::get_base_time_zone_table(), DEFAULT_COUNTRIES,
        DEFAULT_CURRENCIES, DEFAULT_DESTINATIONS, DEFAULT_VENUES,
        m_exchange_rates, m_compliance_rule_schemas, m_trading_schedule);
      auto server_connection = std::make_shared<TestServerConnection>();
      m_container.emplace(&*m_servlet, server_connection,
        factory<std::unique_ptr<TriggerTimer>>());
      m_client.emplace(Initialize("test", *server_connection), Initialize());
      RegisterDefinitionsServices(Store(m_client->GetSlots()));
    }
  };
}

TEST_SUITE("DefinitionsServlet") {
  TEST_CASE_FIXTURE(Fixture, "services") {
    SUBCASE("load_minimum_spire_client_version") {
      auto result =
        m_client->SendRequest<LoadMinimumSpireClientVersionService>();
      REQUIRE(result == "1234");
    }
    SUBCASE("load_organization_name") {
      auto result = m_client->SendRequest<LoadOrganizationNameService>();
      REQUIRE(result == "Spire");
    }
    SUBCASE("load_country_database") {
      auto result = m_client->SendRequest<LoadCountryDatabaseService>();
      REQUIRE(result.from(CA) == DEFAULT_COUNTRIES.from(CA));
    }
    SUBCASE("load_time_zone_database") {
      auto result = m_client->SendRequest<LoadTimeZoneDatabaseService>();
      REQUIRE(result == Nexus::Details::get_base_time_zone_table());
    }
    SUBCASE("load_currency_database") {
      auto result = m_client->SendRequest<LoadCurrencyDatabaseService>();
      REQUIRE(result.from(CAD) == DEFAULT_CURRENCIES.from(CAD));
    }
    SUBCASE("load_destination_database") {
      auto result = m_client->SendRequest<LoadDestinationDatabaseService>();
      REQUIRE(result.from(CHIX) == DEFAULT_DESTINATIONS.from(CHIX));
    }
    SUBCASE("load_venue_database") {
      auto result = m_client->SendRequest<LoadVenueDatabaseService>();
      REQUIRE(result.from(ASX) == DEFAULT_VENUES.from(ASX));
    }
    SUBCASE("load_exchange_rates") {
      auto result = m_client->SendRequest<LoadExchangeRatesService>();
      REQUIRE(result == m_exchange_rates);
    }
    SUBCASE("load_compliance_rule_schemas") {
      auto result = m_client->SendRequest<LoadComplianceRuleSchemasService>();
      REQUIRE(result == m_compliance_rule_schemas);
    }
    SUBCASE("load_trading_schedule") {
      auto result = m_client->SendRequest<LoadTradingScheduleService>();
      auto e1 = result.find(date(2025, 7, 1), ASX);
      REQUIRE(e1 == m_trading_schedule.find(date(2025, 7, 1), ASX));
    }
  }
}
