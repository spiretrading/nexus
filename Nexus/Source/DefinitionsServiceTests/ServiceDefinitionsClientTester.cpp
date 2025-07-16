#include <memory>
#include <vector>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/DefinitionsService/ServiceDefinitionsClient.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::DefaultCountries;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultDestinations;
using namespace Nexus::DefaultVenues;
using namespace Nexus::DefinitionsService;

namespace {
  struct Fixture {
    using TestServiceDefinitionsClient =
      ServiceDefinitionsClient<TestServiceProtocolClientBuilder>;
    std::shared_ptr<TestServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    std::unique_ptr<TestServiceDefinitionsClient> m_client;
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_handlers;

    Fixture()
        : m_server_connection(std::make_shared<TestServerConnection>()),
          m_server(m_server_connection,
            factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot()) {
      RegisterDefinitionsServices(Store(m_server.GetSlots()));
      auto builder = TestServiceProtocolClientBuilder([=] {
        return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection);
      }, factory<std::unique_ptr<
        TestServiceProtocolClientBuilder::Timer>>());
      m_client = std::make_unique<TestServiceDefinitionsClient>(builder);
    }

    template<typename T, typename F>
    void handle(F&& handler) {
      using Slot = typename Services::Details::GetSlotType<RequestToken<
        TestServiceProtocolServer::ServiceProtocolClient, T>>::type;
      auto& stored_handler = m_handlers[typeid(T)];
      if(stored_handler) {
        *std::static_pointer_cast<Slot>(stored_handler) =
          std::forward<F>(handler);
      } else {
        auto shared_handler = std::make_shared<Slot>(std::forward<F>(handler));
        stored_handler = shared_handler;
        T::AddRequestSlot(Store(m_server.GetSlots()),
          [handler = std::move(shared_handler)] (auto&&... args) {
            try {
              (*handler)(std::forward<decltype(args)>(args)...);
            } catch(...) {
              throw ServiceRequestException("Test failed.");
            }
          });
      }
    }
  };
}

#define REQUIRE_NO_THROW(expression) \
  [&] { \
    REQUIRE_NOTHROW(return (expression)); \
    throw 0; \
  }()

TEST_SUITE("ServiceDefinitionsClient") {
  TEST_CASE("load_minimum_spire_client_version") {
    auto fixture = Fixture();
    auto version = std::string("v.1.2.3");
    fixture.handle<LoadMinimumSpireClientVersionService>([&] (auto& request) {
      request.SetResult(version);
    });
    REQUIRE(fixture.m_client->load_minimum_spire_client_version() == version);
  }

  TEST_CASE("load_organization_name") {
    auto fixture = Fixture();
    auto name = std::string("Spire Trading Inc.");
    fixture.handle<LoadOrganizationNameService>([&] (auto& request) {
      request.SetResult(name);
    });
    REQUIRE(fixture.m_client->load_organization_name() == name);
  }

  TEST_CASE("load_country_database") {
    auto fixture = Fixture();
    fixture.handle<LoadCountryDatabaseService>([&] (auto& request) {
      request.SetResult(DEFAULT_COUNTRIES);
    });
    TestJsonEquality(
      fixture.m_client->load_country_database(), DEFAULT_COUNTRIES);
  }

  TEST_CASE("load_time_zone_database") {
    auto fixture = Fixture();
    fixture.handle<LoadTimeZoneDatabaseService>([&] (auto& request) {
      request.SetResult(Nexus::Details::get_base_time_zone_table());
    });
    auto time_zone_database =
      REQUIRE_NO_THROW(fixture.m_client->load_time_zone_database());
    REQUIRE(get_default_time_zone_database().region_list() ==
      time_zone_database.region_list());
  }

  TEST_CASE("load_currency_database") {
    auto fixture = Fixture();
    fixture.handle<LoadCurrencyDatabaseService>([&] (auto& request) {
      request.SetResult(DEFAULT_CURRENCIES);
    });
    TestJsonEquality(
      fixture.m_client->load_currency_database(), DEFAULT_CURRENCIES);
  }

  TEST_CASE("load_destination_database") {
    auto fixture = Fixture();
    fixture.handle<LoadDestinationDatabaseService>([&] (auto& request) {
      request.SetResult(DEFAULT_DESTINATIONS);
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
    fixture.handle<LoadVenueDatabaseService>([&] (auto& request) {
      request.SetResult(DEFAULT_VENUES);
    });
    TestJsonEquality(fixture.m_client->load_venue_database(), DEFAULT_VENUES);
  }

  TEST_CASE("load_exchange_rates") {
    auto fixture = Fixture();
    auto rates = std::vector<ExchangeRate>{
      ExchangeRate(CurrencyPair(AUD, CAD), rational<int>(1, 3))};
    fixture.handle<LoadExchangeRatesService>([&] (auto& request) {
      request.SetResult(rates);
    });
    TestJsonEquality(fixture.m_client->load_exchange_rates(), rates);
  }

  TEST_CASE("load_compliance_rule_schemas") {
    auto fixture = Fixture();
    auto schemas =
      std::vector<ComplianceRuleSchema>{ComplianceRuleSchema("rule1", {})};
    fixture.handle<LoadComplianceRuleSchemasService>([&] (auto& request) {
      request.SetResult(schemas);
    });
    TestJsonEquality(fixture.m_client->load_compliance_rule_schemas(), schemas);
  }

  TEST_CASE("load_trading_schedule") {
    auto fixture = Fixture();
    auto events = std::vector<TradingSchedule::Event>{
      {"O", ptime(date(2025, 7, 1), time_duration(1, 30, 0))}};
    auto rules = std::vector<TradingSchedule::Rule>{
      TradingSchedule::Rule({ASX}, {Tuesday}, {1}, {7}, {2025}, events)};
    auto schedule = TradingSchedule(rules);
    fixture.handle<LoadTradingScheduleService>([&] (auto& request) {
      request.SetResult(schedule);
    });
    TestJsonEquality(fixture.m_client->load_trading_schedule(), schedule);
  }
}
