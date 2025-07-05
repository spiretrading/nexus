#include <future>
#include <memory>
#include <vector>
#include <Beam/Queues/Queue.hpp>
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
#include "Nexus/DefinitionsServiceTests/TestDefinitionsClient.hpp"

using namespace Beam;
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
using namespace Nexus::DefinitionsService::Tests;

namespace {
  using OperationQueue =
    Queue<std::shared_ptr<TestDefinitionsClient::Operation>>;
  using TestServiceDefinitionsClient =
    ServiceDefinitionsClient<TestServiceProtocolClientBuilder>;

  template<typename O, typename F, typename R, typename E>
  void require_operation(TestServiceDefinitionsClient& client,
      OperationQueue& operations, F&& f, const R& expected, E&& e) {
    auto future = std::async(std::launch::async, [&] {
      return std::forward<F>(f)(client);
    });
    auto operation = operations.Pop();
    auto specific = std::get_if<O>(&*operation);
    REQUIRE(specific);
    specific->m_result.set(expected);
    std::forward<E>(e)(std::move(future).get());
  }

  template<typename O, typename F, typename R>
  void require_operation(TestServiceDefinitionsClient& client,
      OperationQueue& operations, F&& f, const R& expected) {
    require_operation<O>(client, operations, std::forward<F>(f), expected,
      [&] (const auto& result) {
        REQUIRE(result == expected);
      });
  }

  struct Fixture {
    std::shared_ptr<TestServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    std::shared_ptr<OperationQueue> m_operations;

    Fixture()
      : m_server_connection(std::make_shared<TestServerConnection>()),
        m_server(m_server_connection,
          factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot()),
        m_operations(std::make_shared<OperationQueue>()) {
      RegisterDefinitionsServices(Store(m_server.GetSlots()));
      LoadMinimumSpireClientVersionService::AddRequestSlot(
        Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_minimum_spire_client_version, this));
      LoadOrganizationNameService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_organization_name, this));
      LoadCountryDatabaseService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_country_database, this));
      LoadTimeZoneDatabaseService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_time_zone_database, this));
      LoadCurrencyDatabaseService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_currency_database, this));
      LoadDestinationDatabaseService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_destination_database, this));
      LoadVenueDatabaseService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_venue_database, this));
      LoadExchangeRatesService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_exchange_rates, this));
      LoadComplianceRuleSchemasService::AddRequestSlot(
        Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_compliance_rule_schemas, this));
      LoadTradingScheduleService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_trading_schedule, this));
    }

    template<typename T>
    using Request =
      RequestToken<TestServiceProtocolServer::ServiceProtocolClient, T>;

    std::unique_ptr<TestServiceDefinitionsClient> make_client() {
      auto builder = TestServiceProtocolClientBuilder([&] {
        return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection);
      }, factory<std::unique_ptr<
        TestServiceProtocolClientBuilder::Timer>>());
      return std::make_unique<TestServiceDefinitionsClient>(builder);
    }

    void on_load_minimum_spire_client_version(
        Request<LoadMinimumSpireClientVersionService>& request) {
      auto async = Beam::Routines::Async<std::string>();
      auto operation = std::make_shared<TestDefinitionsClient::Operation>(
        std::in_place_type<
          TestDefinitionsClient::LoadMinimumSpireClientVersionOperation>,
        async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(std::move(result));
    }

    void on_load_organization_name(
        Request<LoadOrganizationNameService>& request) {
      auto async = Beam::Routines::Async<std::string>();
      auto operation = std::make_shared<TestDefinitionsClient::Operation>(
        std::in_place_type<
          TestDefinitionsClient::LoadOrganizationNameOperation>,
        async.GetEval());
      m_operations->Push(operation);
      request.SetResult(std::move(async.Get()));
    }

    void on_load_country_database(
        Request<LoadCountryDatabaseService>& request) {
      auto async = Beam::Routines::Async<CountryDatabase>();
      auto operation = std::make_shared<TestDefinitionsClient::Operation>(
        std::in_place_type<TestDefinitionsClient::LoadCountryDatabaseOperation>,
        async.GetEval());
      m_operations->Push(operation);
      request.SetResult(std::move(async.Get()));
    }

    void on_load_time_zone_database(
        Request<LoadTimeZoneDatabaseService>& request) {
      auto async = Beam::Routines::Async<local_time::tz_database>();
      auto operation = std::make_shared<TestDefinitionsClient::Operation>(
        std::in_place_type<
          TestDefinitionsClient::LoadTimeZoneDatabaseOperation>,
        async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(Nexus::Details::get_base_time_zone_table());
    }

    void on_load_currency_database(
        Request<LoadCurrencyDatabaseService>& request) {
      auto async = Beam::Routines::Async<CurrencyDatabase>();
      auto operation = std::make_shared<TestDefinitionsClient::Operation>(
        std::in_place_type<
          TestDefinitionsClient::LoadCurrencyDatabaseOperation>,
        async.GetEval());
      m_operations->Push(operation);
      request.SetResult(std::move(async.Get()));
    }

    void on_load_destination_database(
        Request<LoadDestinationDatabaseService>& request) {
      auto async = Beam::Routines::Async<DestinationDatabase>();
      auto operation = std::make_shared<TestDefinitionsClient::Operation>(
        std::in_place_type<
          TestDefinitionsClient::LoadDestinationDatabaseOperation>,
        async.GetEval());
      m_operations->Push(operation);
      request.SetResult(std::move(async.Get()));
    }

    void on_load_venue_database(Request<LoadVenueDatabaseService>& request) {
      auto async = Beam::Routines::Async<VenueDatabase>();
      auto operation = std::make_shared<TestDefinitionsClient::Operation>(
        std::in_place_type<TestDefinitionsClient::LoadVenueDatabaseOperation>,
        async.GetEval());
      m_operations->Push(operation);
      request.SetResult(std::move(async.Get()));
    }

    void on_load_exchange_rates(Request<LoadExchangeRatesService>& request) {
      auto async = Beam::Routines::Async<std::vector<ExchangeRate>>();
      auto operation = std::make_shared<TestDefinitionsClient::Operation>(
        std::in_place_type<TestDefinitionsClient::LoadExchangeRatesOperation>,
        async.GetEval());
      m_operations->Push(operation);
      request.SetResult(std::move(async.Get()));
    }

    void on_load_compliance_rule_schemas(
        Request<LoadComplianceRuleSchemasService>& request) {
      auto async = Beam::Routines::Async<std::vector<ComplianceRuleSchema>>();
      auto operation = std::make_shared<TestDefinitionsClient::Operation>(
        std::in_place_type<
          TestDefinitionsClient::LoadComplianceRuleSchemasOperation>,
        async.GetEval());
      m_operations->Push(operation);
      request.SetResult(std::move(async.Get()));
    }

    void on_load_trading_schedule(
        Request<LoadTradingScheduleService>& request) {
      auto async = Beam::Routines::Async<TradingSchedule>();
      auto operation = std::make_shared<TestDefinitionsClient::Operation>(
        std::in_place_type<
          TestDefinitionsClient::LoadTradingScheduleOperation>,
        async.GetEval());
      m_operations->Push(operation);
      request.SetResult(std::move(async.Get()));
    }
  };
}

TEST_SUITE("ServiceDefinitionsClient") {
  TEST_CASE_FIXTURE(Fixture, "load_minimum_spire_client_version") {
    auto client = make_client();
    require_operation<
      TestDefinitionsClient::LoadMinimumSpireClientVersionOperation>(
        *client, *m_operations, [] (auto& client) {
          return client.load_minimum_spire_client_version();
        }, "v.1.2.3");
  }

  TEST_CASE_FIXTURE(Fixture, "load_organization_name") {
    auto client = make_client();
    require_operation<TestDefinitionsClient::LoadOrganizationNameOperation>(
      *client, *m_operations, [] (auto& client) {
        return client.load_organization_name();
      }, "Spire Trading");
  }

  TEST_CASE_FIXTURE(Fixture, "load_country_database") {
    auto client = make_client();
    auto countries = CountryDatabase();
    auto entry = DEFAULT_COUNTRIES.from(CA);
    countries.add(entry);
    require_operation<TestDefinitionsClient::LoadCountryDatabaseOperation>(
      *client, *m_operations, [] (auto& client) {
        return client.load_country_database();
      }, countries,
      [&] (const auto& result) {
        REQUIRE(result.from(CA) == entry);
      });
  }

  TEST_CASE_FIXTURE(Fixture, "load_time_zone_database") {
    auto client = make_client();
    auto time_zone_database = get_default_time_zone_database();
    require_operation<TestDefinitionsClient::LoadTimeZoneDatabaseOperation>(
      *client, *m_operations, [] (auto& client) {
        return client.load_time_zone_database();
      }, time_zone_database,
      [&] (const auto& result) {
        REQUIRE(result.region_list() == time_zone_database.region_list());
      });
  }

  TEST_CASE_FIXTURE(Fixture, "load_currency_database") {
    auto client = make_client();
    auto currencies = CurrencyDatabase();
    auto entry = DEFAULT_CURRENCIES.from(USD);
    currencies.add(entry);
    require_operation<TestDefinitionsClient::LoadCurrencyDatabaseOperation>(
      *client, *m_operations, [] (auto& client) {
        return client.load_currency_database();
      }, currencies,
      [&] (const auto& result) {
        REQUIRE(result.from(USD) == entry);
      });
  }

  TEST_CASE_FIXTURE(Fixture, "load_destination_database") {
    auto client = make_client();
    auto destinations = DestinationDatabase();
    auto entry = DEFAULT_DESTINATIONS.from(CHIX);
    destinations.add(entry);
    require_operation<TestDefinitionsClient::LoadDestinationDatabaseOperation>(
      *client, *m_operations, [] (auto& client) {
        return client.load_destination_database();
      }, destinations,
      [&] (const auto& result) {
        REQUIRE(result.from(CHIX) == entry);
      });
  }

  TEST_CASE_FIXTURE(Fixture, "load_venue_database") {
    auto client = make_client();
    auto venues = VenueDatabase();
    auto entry = DEFAULT_VENUES.from(ASX);
    venues.add(entry);
    require_operation<TestDefinitionsClient::LoadVenueDatabaseOperation>(
      *client, *m_operations, [] (auto& client) {
        return client.load_venue_database();
      }, venues,
      [&] (const auto& result) {
        REQUIRE(result.from(ASX) == entry);
      });
  }

  TEST_CASE_FIXTURE(Fixture, "load_exchange_rates") {
    auto client = make_client();
    auto rates = std::vector<ExchangeRate>{
      ExchangeRate(CurrencyPair(AUD, CAD), rational<int>(1, 3))};
    require_operation<TestDefinitionsClient::LoadExchangeRatesOperation>(
      *client, *m_operations,
      [] (auto& client) {
        return client.load_exchange_rates();
      }, rates);
  }

  TEST_CASE_FIXTURE(Fixture, "load_compliance_rule_schemas") {
    auto client = make_client();
    auto schemas =
      std::vector<ComplianceRuleSchema>{ComplianceRuleSchema("rule1", {})};
    require_operation<
      TestDefinitionsClient::LoadComplianceRuleSchemasOperation>(
        *client, *m_operations, [] (auto& client) {
          return client.load_compliance_rule_schemas();
        }, schemas);
  }

  TEST_CASE_FIXTURE(Fixture, "load_trading_schedule") {
    auto client = make_client();
    auto events = std::vector<TradingSchedule::Event>{
      {"O", ptime(date(2025, 7, 1), time_duration(1, 30, 0))}};
    auto rules = std::vector<TradingSchedule::Rule>{
      TradingSchedule::Rule({ASX}, {Tuesday}, {1}, {7}, {2025}, events)};
    auto schedule = TradingSchedule(rules);
    require_operation<TestDefinitionsClient::LoadTradingScheduleOperation>(
      *client, *m_operations, [] (auto& client) {
        return client.load_trading_schedule();
      }, schedule,
      [&] (const auto& result) {
        REQUIRE(result.find(date(2025, 7, 1), ASX) == events);
      });
  }
}
