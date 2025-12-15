#ifndef NEXUS_SERVICE_DEFINITIONS_CLIENT_HPP
#define NEXUS_SERVICE_DEFINITIONS_CLIENT_HPP
#include <exception>
#include <functional>
#include <utility>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/DefinitionsService/DefinitionsServices.hpp"

namespace Nexus {

  /**
   * Implements a DefinitionsClient using Beam services.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ServiceDefinitionsClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::dereference_t<B>;

      /**
       * Constructs a DefinitionsClient.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit ServiceDefinitionsClient(BF&& client_builder);

      ~ServiceDefinitionsClient();

      std::string load_minimum_spire_client_version();
      std::string load_organization_name();
      CountryDatabase load_country_database();
      boost::local_time::tz_database load_time_zone_database();
      CurrencyDatabase load_currency_database();
      DestinationDatabase load_destination_database();
      VenueDatabase load_venue_database();
      std::vector<ExchangeRate> load_exchange_rates();
      std::vector<ComplianceRuleSchema> load_compliance_rule_schemas();
      TradingSchedule load_trading_schedule();
      void close();

    private:
      Beam::ServiceProtocolClientHandler<B> m_client_handler;
      Beam::OpenState m_open_state;

      ServiceDefinitionsClient(const ServiceDefinitionsClient&) = delete;
      ServiceDefinitionsClient& operator =(
        const ServiceDefinitionsClient&) = delete;
  };

  template<typename B>
  template<typename BF>
  ServiceDefinitionsClient<B>::ServiceDefinitionsClient(BF&& client_builder)
      try : m_client_handler(std::forward<BF>(client_builder)) {
    register_definitions_services(Beam::out(m_client_handler.get_slots()));
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Failed to connect to the definitions server."));
  }

  template<typename B>
  ServiceDefinitionsClient<B>::~ServiceDefinitionsClient() {
    close();
  }

  template<typename B>
  std::string ServiceDefinitionsClient<B>::load_minimum_spire_client_version() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        LoadMinimumSpireClientVersionService>();
    }, "Failed to load minimum Spire client version.");
  }

  template<typename B>
  std::string ServiceDefinitionsClient<B>::load_organization_name() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadOrganizationNameService>();
    }, "Failed to load organization name.");
  }

  template<typename B>
  CountryDatabase ServiceDefinitionsClient<B>::load_country_database() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadCountryDatabaseService>();
    }, "Failed to load country database.");
  }

  template<typename B>
  boost::local_time::tz_database
      ServiceDefinitionsClient<B>::load_time_zone_database() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      auto time_zones =
        client->template send_request<LoadTimeZoneDatabaseService>();
      auto database = boost::local_time::tz_database();
      auto stream = std::stringstream(time_zones);
      database.load_from_stream(stream);
      return database;
    }, "Failed to load timezone database.");
  }

  template<typename B>
  CurrencyDatabase ServiceDefinitionsClient<B>::load_currency_database() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadCurrencyDatabaseService>();
    }, "Failed to load currency database.");
  }

  template<typename B>
  DestinationDatabase ServiceDefinitionsClient<B>::load_destination_database() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadDestinationDatabaseService>();
    }, "Failed to load destination database.");
  }

  template<typename B>
  VenueDatabase ServiceDefinitionsClient<B>::load_venue_database() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadVenueDatabaseService>();
    }, "Failed to load venue database.");
  }

  template<typename B>
  std::vector<ExchangeRate> ServiceDefinitionsClient<B>::load_exchange_rates() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadExchangeRatesService>();
    }, "Failed to load exchange rates.");
  }

  template<typename B>
  std::vector<ComplianceRuleSchema>
      ServiceDefinitionsClient<B>::load_compliance_rule_schemas() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadComplianceRuleSchemasService>();
    }, "Failed to load compliance rule schemas.");
  }

  template<typename B>
  TradingSchedule ServiceDefinitionsClient<B>::load_trading_schedule() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadTradingScheduleService>();
    }, "Failed to load trading schedule.");
  }

  template<typename B>
  void ServiceDefinitionsClient<B>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_client_handler.close();
    m_open_state.close();
  }
}

#endif
