#include <future>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/DefinitionsServiceTests/TestDefinitionsClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCountries;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultDestinations;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  template<typename O, typename F, typename R, typename E>
  void require_operation(F&& f, const R& expected, E&& e) {
    auto operations = std::make_shared<TestDefinitionsClient::Queue>();
    auto client = DefinitionsClient(
      std::in_place_type<TestDefinitionsClient>, operations);
    auto future = std::async(std::launch::async, [&] {
      return std::forward<F>(f)(client);
    });
    auto operation = operations->pop();
    auto specific = std::get_if<O>(&*operation);
    REQUIRE(specific);
    specific->m_result.set(expected);
    std::forward<E>(e)(std::move(future).get());
  }

  template<typename O, typename F, typename R>
  void require_operation(F&& f, const R& expected) {
    require_operation<O>(std::forward<F>(f), expected,
      [&] (const auto& result) {
        REQUIRE(result == expected);
      });
  }
}

TEST_SUITE("DefinitionsClient") {
  TEST_CASE("load_minimum_spire_client_version") {
    require_operation<
      TestDefinitionsClient::LoadMinimumSpireClientVersionOperation>(
        [] (auto& client) {
          return client.load_minimum_spire_client_version();
        }, "v1.2.3");
  }

  TEST_CASE("load_organization_name") {
    require_operation<TestDefinitionsClient::LoadOrganizationNameOperation>(
      [] (auto& client) {
        return client.load_organization_name();
      }, "Spire Trading");
  }

  TEST_CASE("load_country_database") {
    auto countries = CountryDatabase();
    auto entry = DEFAULT_COUNTRIES.from(CA);
    countries.add(entry);
    require_operation<TestDefinitionsClient::LoadCountryDatabaseOperation>(
      [] (auto& client) {
        return client.load_country_database();
      }, countries,
      [&] (const auto& result) {
        auto result_entry = result.from(CA);
        REQUIRE(result_entry == entry);
      });
  }

  TEST_CASE("load_time_zone_database") {
    require_operation<TestDefinitionsClient::LoadTimeZoneDatabaseOperation>(
      [] (auto& client) {
        return client.load_time_zone_database();
      }, get_default_time_zone_database(),
      [&] (const auto& result) {
        REQUIRE(result.region_list() ==
          get_default_time_zone_database().region_list());
      });
  }

  TEST_CASE("load_currency_database") {
    auto currencies = CurrencyDatabase();
    auto entry = DEFAULT_CURRENCIES.from(USD);
    currencies.add(entry);
    require_operation<TestDefinitionsClient::LoadCurrencyDatabaseOperation>(
      [] (auto& client) {
        return client.load_currency_database();
      }, currencies,
      [&] (const auto& result) {
        auto result_entry = result.from(USD);
        REQUIRE(result_entry == entry);
      });
  }

  TEST_CASE("load_destination_database") {
    auto destinations = DestinationDatabase();
    auto entry = DEFAULT_DESTINATIONS.from(CHIX);
    destinations.add(entry);
    require_operation<TestDefinitionsClient::LoadDestinationDatabaseOperation>(
      [] (auto& client) {
        return client.load_destination_database();
      }, destinations,
      [&] (const auto& result) {
        auto result_entry = result.from(CHIX);
        REQUIRE(result_entry == entry);
      });
  }

  TEST_CASE("load_venue_database") {
    auto venues = VenueDatabase();
    auto entry = DEFAULT_VENUES.from(ASX);
    venues.add(entry);
    require_operation<TestDefinitionsClient::LoadVenueDatabaseOperation>(
      [] (auto& client) {
        return client.load_venue_database();
      }, venues,
      [&] (const auto& result) {
        auto result_entry = result.from(ASX);
        REQUIRE(result_entry == entry);
      });
  }

  TEST_CASE("load_exchange_rates") {
    auto rates = std::vector<ExchangeRate>();
    rates.push_back(ExchangeRate(CurrencyPair(AUD, CAD), rational<int>(1, 3)));
    require_operation<TestDefinitionsClient::LoadExchangeRatesOperation>(
      [] (auto& client) {
        return client.load_exchange_rates();
      }, rates);
  }

  TEST_CASE("load_compliance_rule_schemas") {
    auto schemas = std::vector<ComplianceRuleSchema>();
    schemas.push_back(ComplianceRuleSchema("rule1", {}));
    require_operation<
      TestDefinitionsClient::LoadComplianceRuleSchemasOperation>(
        [] (auto& client) {
          return client.load_compliance_rule_schemas();
        }, schemas);
  }

  TEST_CASE("load_trading_schedule") {
    auto events = std::vector<TradingSchedule::Event>();
    events.push_back(
      {"O", ptime(date(2025, 7, 1), time_duration(1, 30, 0, 0))});
    auto rules = std::vector<TradingSchedule::Rule>();
    rules.push_back(TradingSchedule::Rule(
      {ASX}, {Tuesday}, {1}, {7}, {2025}, events));
    auto schedule = TradingSchedule(rules);
    require_operation<TestDefinitionsClient::LoadTradingScheduleOperation>(
      [] (auto& client) {
        return client.load_trading_schedule();
      }, schedule,
      [&] (const auto& result) {
        auto e1 = schedule.find(date(2025, 7, 1), ASX);
        REQUIRE(e1 == events);
      });
  }
}
