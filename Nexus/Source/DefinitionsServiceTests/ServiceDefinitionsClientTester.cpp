#include <memory>
#include <vector>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServicesTests/ServiceClientFixture.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/DefinitionsService/ServiceDefinitionsClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCountries;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultDestinations;
using namespace Nexus::DefaultVenues;

namespace {
  struct Fixture : ServiceClientFixture {
    using TestDefinitionsClient =
      ServiceDefinitionsClient<TestServiceProtocolClientBuilder>;
    std::unique_ptr<TestDefinitionsClient> m_client;

    Fixture() {
      register_definitions_services(out(m_server.get_slots()));
      m_client = make_client<TestDefinitionsClient>();
    }
  };
}

TEST_SUITE("ServiceDefinitionsClient") {
  TEST_CASE("load_minimum_spire_client_version") {
    auto fixture = Fixture();
    auto version = std::string("v.1.2.3");
    fixture.on_request<LoadMinimumSpireClientVersionService>(
      [&] (auto& request) {
        request.set(version);
      });
    REQUIRE(fixture.m_client->load_minimum_spire_client_version() == version);
  }

  TEST_CASE("load_organization_name") {
    auto fixture = Fixture();
    auto name = std::string("Spire Trading Inc.");
    fixture.on_request<LoadOrganizationNameService>([&] (auto& request) {
      request.set(name);
    });
    REQUIRE(fixture.m_client->load_organization_name() == name);
  }

  TEST_CASE("load_country_database") {
    auto fixture = Fixture();
    fixture.on_request<LoadCountryDatabaseService>([&] (auto& request) {
      request.set(DEFAULT_COUNTRIES);
    });
    test_json_equality(
      fixture.m_client->load_country_database(), DEFAULT_COUNTRIES);
  }

  TEST_CASE("load_time_zone_database") {
    auto fixture = Fixture();
    fixture.on_request<LoadTimeZoneDatabaseService>([&] (auto& request) {
      request.set(Nexus::Details::get_base_time_zone_table());
    });
    auto time_zone_database =
      REQUIRE_NO_THROW(fixture.m_client->load_time_zone_database());
    REQUIRE(get_default_time_zone_database().region_list() ==
      time_zone_database.region_list());
  }

  TEST_CASE("load_currency_database") {
    auto fixture = Fixture();
    fixture.on_request<LoadCurrencyDatabaseService>([&] (auto& request) {
      request.set(DEFAULT_CURRENCIES);
    });
    test_json_equality(
      fixture.m_client->load_currency_database(), DEFAULT_CURRENCIES);
  }

  TEST_CASE("load_destination_database") {
    auto fixture = Fixture();
    fixture.on_request<LoadDestinationDatabaseService>([&] (auto& request) {
      request.set(DEFAULT_DESTINATIONS);
    });
    auto entries =
      fixture.m_client->load_destination_database().get_entries();
    auto expected_entries = DEFAULT_DESTINATIONS.get_entries();
    REQUIRE(entries.size() == expected_entries.size());
    for(auto& entry : entries) {
      REQUIRE(
        std::find(expected_entries.begin(), expected_entries.end(), entry) !=
          expected_entries.end());
    }
  }

  TEST_CASE("load_venue_database") {
    auto fixture = Fixture();
    fixture.on_request<LoadVenueDatabaseService>([&] (auto& request) {
      request.set(DEFAULT_VENUES);
    });
    test_json_equality(fixture.m_client->load_venue_database(), DEFAULT_VENUES);
  }

  TEST_CASE("load_exchange_rates") {
    auto fixture = Fixture();
    auto rates = std::vector<ExchangeRate>{
      ExchangeRate(CurrencyPair(AUD, CAD), rational<int>(1, 3))};
    fixture.on_request<LoadExchangeRatesService>([&] (auto& request) {
      request.set(rates);
    });
    test_json_equality(fixture.m_client->load_exchange_rates(), rates);
  }

  TEST_CASE("load_compliance_rule_schemas") {
    auto fixture = Fixture();
    auto schemas =
      std::vector<ComplianceRuleSchema>{ComplianceRuleSchema("rule1", {})};
    fixture.on_request<LoadComplianceRuleSchemasService>([&] (auto& request) {
      request.set(schemas);
    });
    test_json_equality(
      fixture.m_client->load_compliance_rule_schemas(), schemas);
  }

  TEST_CASE("load_trading_schedule") {
    auto fixture = Fixture();
    auto events = std::vector<TradingSchedule::Event>{
      {"O", ptime(date(2025, 7, 1), time_duration(1, 30, 0))}};
    auto rules = std::vector<TradingSchedule::Rule>{
      TradingSchedule::Rule({ASX}, {Tuesday}, {1}, {7}, {2025}, events)};
    auto schedule = TradingSchedule(rules);
    fixture.on_request<LoadTradingScheduleService>([&] (auto& request) {
      request.set(schedule);
    });
    test_json_equality(fixture.m_client->load_trading_schedule(), schedule);
  }
}
