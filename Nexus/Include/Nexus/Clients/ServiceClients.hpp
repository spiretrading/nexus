#ifndef NEXUS_SERVICE_CLIENTS_HPP
#define NEXUS_SERVICE_CLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/RegistryService/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/ChartingService/ApplicationDefinitions.hpp"
#include "Nexus/Clients/Clients.hpp"
#include "Nexus/Compliance/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"
#include "Nexus/RiskService/ApplicationDefinitions.hpp"

namespace Nexus {

  /** Implements the Clients interface using the respective service client. */
  class ServiceClients {
    public:
      using ServiceLocatorClient =
        Beam::ServiceLocator::ApplicationServiceLocatorClient::Client;

      using RegistryClient =
        Beam::RegistryService::ApplicationRegistryClient::Client;

      using AdministrationClient =
        AdministrationService::ApplicationAdministrationClient::Client;

      using DefinitionsClient =
        DefinitionsService::ApplicationDefinitionsClient::Client;

      using MarketDataClient =
        MarketDataService::ApplicationMarketDataClient::Client;

      using ChartingClient = ChartingService::ApplicationChartingClient::Client;

      using ComplianceClient = Compliance::ApplicationComplianceClient::Client;

      using OrderExecutionClient =
        OrderExecutionService::ApplicationOrderExecutionClient::Client;

      using RiskClient = RiskService::ApplicationRiskClient::Client;

      using TimeClient = Beam::TimeService::LiveNtpTimeClient;

      using Timer = Beam::Threading::LiveTimer;

      /**
       * Constructs a ServiceClients.
       * @param username The client's username.
       * @param password The client's password.
       * @param address The IpAddress to connect to.
       */
      ServiceClients(std::string username, std::string password,
        const Beam::Network::IpAddress& address);

      ~ServiceClients();
      ServiceLocatorClient& get_service_locator_client();
      RegistryClient& get_registry_client();
      AdministrationClient& get_administration_client();
      DefinitionsClient& get_definitions_client();
      MarketDataClient& get_market_data_client();
      ChartingClient& get_charting_client();
      ComplianceClient& get_compliance_client();
      OrderExecutionClient& get_order_execution_client();
      RiskClient& get_risk_client();
      TimeClient& get_time_client();
      std::unique_ptr<Timer> make_timer(
        boost::posix_time::time_duration expiry);
      void close();

    private:
      Beam::ServiceLocator::ApplicationServiceLocatorClient
        m_service_locator_client;
      Beam::RegistryService::ApplicationRegistryClient m_registry_client;
      AdministrationService::ApplicationAdministrationClient
        m_administration_client;
      DefinitionsService::ApplicationDefinitionsClient m_definitions_client;
      MarketDataService::ApplicationMarketDataClient m_market_data_client;
      ChartingService::ApplicationChartingClient m_charting_client;
      Compliance::ApplicationComplianceClient m_compliance_client;
      OrderExecutionService::ApplicationOrderExecutionClient
        m_order_execution_client;
      RiskService::ApplicationRiskClient m_risk_client;
      std::unique_ptr<TimeClient> m_time_client;
      Beam::IO::OpenState m_open_state;

      ServiceClients(const ServiceClients&) = delete;
      ServiceClients& operator =(const ServiceClients&) = delete;
  };

  inline ServiceClients::ServiceClients(std::string username,
      std::string password, const Beam::Network::IpAddress& address)
    : m_service_locator_client(
        std::move(username), std::move(password), address),
      m_registry_client(m_service_locator_client.Get()),
      m_administration_client(m_service_locator_client.Get()),
      m_definitions_client(m_service_locator_client.Get()),
      m_market_data_client(m_service_locator_client.Get()),
      m_charting_client(m_service_locator_client.Get()),
      m_compliance_client(m_service_locator_client.Get()),
      m_order_execution_client(m_service_locator_client.Get()),
      m_risk_client(m_service_locator_client.Get()),
      m_time_client(Beam::TimeService::MakeLiveNtpTimeClientFromServiceLocator(
        *m_service_locator_client)) {}

  inline ServiceClients::~ServiceClients() {
    close();
  }

  inline ServiceClients::ServiceLocatorClient&
      ServiceClients::get_service_locator_client() {
    return *m_service_locator_client;
  }

  inline ServiceClients::RegistryClient&
      ServiceClients::get_registry_client() {
    return *m_registry_client;
  }

  inline ServiceClients::AdministrationClient&
      ServiceClients::get_administration_client() {
    return *m_administration_client;
  }

  inline ServiceClients::DefinitionsClient&
      ServiceClients::get_definitions_client() {
    return *m_definitions_client;
  }

  inline ServiceClients::MarketDataClient&
      ServiceClients::get_market_data_client() {
    return *m_market_data_client;
  }

  inline ServiceClients::ChartingClient&
      ServiceClients::get_charting_client() {
    return *m_charting_client;
  }

  inline ServiceClients::ComplianceClient&
      ServiceClients::get_compliance_client() {
    return *m_compliance_client;
  }

  inline ServiceClients::OrderExecutionClient&
      ServiceClients::get_order_execution_client() {
    return *m_order_execution_client;
  }

  inline ServiceClients::RiskClient&
      ServiceClients::get_risk_client() {
    return *m_risk_client;
  }

  inline ServiceClients::TimeClient&
      ServiceClients::get_time_client() {
    return *m_time_client;
  }

  inline std::unique_ptr<ServiceClients::Timer>
      ServiceClients::make_timer(boost::posix_time::time_duration expiry) {
    return std::make_unique<Timer>(expiry);
  }

  inline void ServiceClients::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_time_client->Close();
    m_service_locator_client->Close();
    m_risk_client->close();
    m_order_execution_client->close();
    m_compliance_client->close();
    m_charting_client->close();
    m_market_data_client->close();
    m_definitions_client->close();
    m_administration_client->close();
    m_registry_client->Close();
    m_open_state.Close();
  }
}

#endif
