#ifndef NEXUS_APPLICATION_SERVICE_CLIENTS_HPP
#define NEXUS_APPLICATION_SERVICE_CLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/RegistryService/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/ChartingService/ApplicationDefinitions.hpp"
#include "Nexus/Compliance/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"
#include "Nexus/RiskService/ApplicationDefinitions.hpp"

namespace Nexus {

  /** Implements the ServiceClients interface using live application clients. */
  class ApplicationServiceClients {
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
       * Constructs an ApplicationServiceClients.
       * @param username The client's username.
       * @param password The client's password.
       * @param address The IpAddress to connect to.
       */
      ApplicationServiceClients(std::string username, std::string password,
        const Beam::Network::IpAddress& address);

      ~ApplicationServiceClients();

      ServiceLocatorClient& GetServiceLocatorClient();

      RegistryClient& GetRegistryClient();

      AdministrationClient& GetAdministrationClient();

      DefinitionsClient& GetDefinitionsClient();

      MarketDataClient& GetMarketDataClient();

      ChartingClient& GetChartingClient();

      ComplianceClient& GetComplianceClient();

      OrderExecutionClient& GetOrderExecutionClient();

      RiskClient& GetRiskClient();

      TimeClient& GetTimeClient();

      std::unique_ptr<Timer> BuildTimer(
        boost::posix_time::time_duration duration);

      void Close();

    private:
      Beam::ServiceLocator::ApplicationServiceLocatorClient
        m_serviceLocatorClient;
      Beam::RegistryService::ApplicationRegistryClient m_registryClient;
      AdministrationService::ApplicationAdministrationClient
        m_administrationClient;
      DefinitionsService::ApplicationDefinitionsClient m_definitionsClient;
      MarketDataService::ApplicationMarketDataClient m_marketDataClient;
      ChartingService::ApplicationChartingClient m_chartingClient;
      Compliance::ApplicationComplianceClient m_complianceClient;
      OrderExecutionService::ApplicationOrderExecutionClient
        m_orderExecutionClient;
      RiskService::ApplicationRiskClient m_riskClient;
      std::unique_ptr<TimeClient> m_timeClient;
      Beam::IO::OpenState m_openState;

      ApplicationServiceClients(const ApplicationServiceClients&) = delete;
      ApplicationServiceClients& operator =(
        const ApplicationServiceClients&) = delete;
  };

  inline ApplicationServiceClients::ApplicationServiceClients(
    std::string username, std::string password,
    const Beam::Network::IpAddress& address)
    : m_serviceLocatorClient(std::move(username), std::move(password), address),
      m_registryClient(Beam::Ref(*m_serviceLocatorClient)),
      m_administrationClient(Beam::Ref(*m_serviceLocatorClient)),
      m_definitionsClient(Beam::Ref(*m_serviceLocatorClient)),
      m_marketDataClient(Beam::Ref(*m_serviceLocatorClient)),
      m_chartingClient(Beam::Ref(*m_serviceLocatorClient)),
      m_complianceClient(Beam::Ref(*m_serviceLocatorClient)),
      m_orderExecutionClient(Beam::Ref(*m_serviceLocatorClient)),
      m_riskClient(Beam::Ref(*m_serviceLocatorClient)),
      m_timeClient(Beam::TimeService::MakeLiveNtpTimeClientFromServiceLocator(
        *m_serviceLocatorClient)) {}

  inline ApplicationServiceClients::~ApplicationServiceClients() {
    Close();
  }

  inline ApplicationServiceClients::ServiceLocatorClient&
      ApplicationServiceClients::GetServiceLocatorClient() {
    return *m_serviceLocatorClient;
  }

  inline ApplicationServiceClients::RegistryClient&
      ApplicationServiceClients::GetRegistryClient() {
    return *m_registryClient;
  }

  inline ApplicationServiceClients::AdministrationClient&
      ApplicationServiceClients::GetAdministrationClient() {
    return *m_administrationClient;
  }

  inline ApplicationServiceClients::DefinitionsClient&
      ApplicationServiceClients::GetDefinitionsClient() {
    return *m_definitionsClient;
  }

  inline ApplicationServiceClients::MarketDataClient&
      ApplicationServiceClients::GetMarketDataClient() {
    return *m_marketDataClient;
  }

  inline ApplicationServiceClients::ChartingClient&
      ApplicationServiceClients::GetChartingClient() {
    return *m_chartingClient;
  }

  inline ApplicationServiceClients::ComplianceClient&
      ApplicationServiceClients::GetComplianceClient() {
    return *m_complianceClient;
  }

  inline ApplicationServiceClients::OrderExecutionClient&
      ApplicationServiceClients::GetOrderExecutionClient() {
    return *m_orderExecutionClient;
  }

  inline ApplicationServiceClients::RiskClient&
      ApplicationServiceClients::GetRiskClient() {
    return *m_riskClient;
  }

  inline ApplicationServiceClients::TimeClient&
      ApplicationServiceClients::GetTimeClient() {
    return *m_timeClient;
  }

  inline std::unique_ptr<ApplicationServiceClients::Timer>
      ApplicationServiceClients::BuildTimer(
      boost::posix_time::time_duration duration) {
    return std::make_unique<Timer>(duration);
  }

  inline void ApplicationServiceClients::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_timeClient->Close();
    m_riskClient->Close();
    m_orderExecutionClient->Close();
    m_complianceClient->Close();
    m_chartingClient->Close();
    m_marketDataClient->Close();
    m_definitionsClient->Close();
    m_administrationClient->Close();
    m_registryClient->Close();
    m_serviceLocatorClient->Close();
    m_openState.Close();
  }
}

#endif
