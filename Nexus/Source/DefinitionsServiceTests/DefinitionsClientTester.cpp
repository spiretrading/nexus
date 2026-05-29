#include <future>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/StandardTimeZones.hpp"
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/DefinitionsServiceTests/TestDefinitionsClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Countries;
using namespace Nexus::Currencies;
using namespace Nexus::Destinations;
using namespace Nexus::Tests;
using namespace Nexus::Venues;

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
    auto entry = COUNTRIES.from(CA);
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
      }, TIME_ZONES,
      [&] (const auto& result) {
        REQUIRE(result.region_list() == TIME_ZONES.region_list());
      });
  }

  TEST_CASE("load_currency_database") {
    auto currencies = CurrencyDatabase();
    auto entry = CURRENCIES.from(USD);
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
    auto entry = DESTINATIONS.from(CHIX);
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
    auto entry = VENUES.from(ASX);
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

  TEST_CASE("load_definitions") {
    auto saved_countries = COUNTRIES;
    auto saved_time_zones = TIME_ZONES;
    auto saved_currencies = CURRENCIES;
    auto saved_destinations = DESTINATIONS;
    auto saved_venues = VENUES;
    auto test_country = CountryDatabase::Entry();
    test_country.m_code = CountryCode(999);
    test_country.m_name = "Testland";
    test_country.m_two_letter_code = "TL";
    test_country.m_three_letter_code = "TLD";
    auto test_countries = CountryDatabase();
    test_countries.add(test_country);
    auto test_currency = CurrencyDatabase::Entry();
    test_currency.m_id = CurrencyId(999);
    test_currency.m_code = "TST";
    test_currency.m_name = "Test";
    test_currency.m_sign = "T";
    auto test_currencies = CurrencyDatabase();
    test_currencies.add(test_currency);
    auto test_destination = DestinationDatabase::Entry();
    test_destination.m_id = "TEST_DEST";
    test_destination.m_description = "Test Destination";
    auto test_destinations = DestinationDatabase();
    test_destinations.add(test_destination);
    auto test_venue = VenueDatabase::Entry();
    test_venue.m_venue = Venue("XTST");
    test_venue.m_country_code = test_country.m_code;
    test_venue.m_currency = test_currency.m_id;
    test_venue.m_time_zone = "America/New_York";
    test_venue.m_description = "Test Exchange";
    test_venue.m_display_name = "TST";
    auto test_venues = VenueDatabase();
    test_venues.add(test_venue);
    auto test_time_zones = [&] {
      auto database = boost::local_time::tz_database();
      auto stream = std::stringstream(
        "\"Testland/Nowhere\",\"TST\",\"TST\",\"\",\"\","
        "\"+05:30:00\",\"+00:00:00\",\"\",\"\",\"\",\"+00:00:00\"");
      database.load_from_stream(stream);
      return database;
    }();
    auto operations = std::make_shared<TestDefinitionsClient::Queue>();
    auto client = DefinitionsClient(
      std::in_place_type<TestDefinitionsClient>, operations);
    auto future = std::async(std::launch::async, [&] {
      load_definitions(client);
    });
    for(auto i = 0; i < 5; ++i) {
      auto operation = operations->pop();
      std::visit([&] (auto& op) {
        using Operation = std::decay_t<decltype(op)>;
        if constexpr(std::is_same_v<Operation,
            TestDefinitionsClient::LoadCountryDatabaseOperation>) {
          op.m_result.set(test_countries);
        } else if constexpr(std::is_same_v<Operation,
            TestDefinitionsClient::LoadTimeZoneDatabaseOperation>) {
          op.m_result.set(test_time_zones);
        } else if constexpr(std::is_same_v<Operation,
            TestDefinitionsClient::LoadCurrencyDatabaseOperation>) {
          op.m_result.set(test_currencies);
        } else if constexpr(std::is_same_v<Operation,
            TestDefinitionsClient::LoadDestinationDatabaseOperation>) {
          op.m_result.set(test_destinations);
        } else if constexpr(std::is_same_v<Operation,
            TestDefinitionsClient::LoadVenueDatabaseOperation>) {
          op.m_result.set(test_venues);
        } else {
          FAIL("Unexpected operation.");
        }
      }, *operation);
    }
    future.get();
    REQUIRE(COUNTRIES.from(CountryCode(999)).m_name == "Testland");
    REQUIRE(TIME_ZONES.time_zone_from_region("Testland/Nowhere"));
    REQUIRE(CURRENCIES.from("TST").m_id == CurrencyId(999));
    REQUIRE(DESTINATIONS.from("TEST_DEST").m_id == "TEST_DEST");
    REQUIRE(VENUES.from("XTST").m_display_name == "TST");
    set_countries(saved_countries);
    set_time_zones(saved_time_zones);
    set_currencies(saved_currencies);
    set_destinations(saved_destinations);
    set_venues(saved_venues);
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
