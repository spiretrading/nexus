#ifndef SPIRE_CLIENTS_HPP
#define SPIRE_CLIENTS_HPP
#include <memory>
#include <Beam/RegistryService/RegistryClientBox.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Threading/TimerBox.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include "Nexus/Clients/Clients.hpp"

namespace Spire {

  /* Manages the set of clients connected to Spire s. */
  class SpireClients {
    public:
      using ServiceLocatorClient =
        Beam::ServiceLocatorClientBox;
      using RegistryClient = Beam::RegistryService::RegistryClientBox;
      using AdministrationClient = Nexus::AdministrationClient;
      using DefinitionsClient = Nexus::DefinitionsClient;
      using MarketDataClient = Nexus::MarketDataClient;
      using ChartingClient = Nexus::ChartingClient;
      using ComplianceClient = Nexus::ComplianceClient;
      using OrderExecutionClient = Nexus::OrderExecutionClient;
      using RiskClient = Nexus::RiskClient;
      using TimeClient = Beam::TimeService::TimeClientBox;
      using Timer = Beam::TimerBox;

      /**
       * Constructs a SpireClients instance.
       * @param serviceLocatorClient The ServiceLocatorClient logged into the
       *        Spire services.
       */
      explicit SpireClients(
        std::unique_ptr<Beam::ApplicationServiceLocatorClient>
          serviceLocatorClient);

      ~SpireClients();
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
      std::unique_ptr<Beam::ApplicationServiceLocatorClient>
        m_applicationServiceLocatorClient;
      Beam::ServiceLocatorClientBox m_serviceLocatorClient;
      Nexus::DefinitionsClient m_definitionsClient;
      Beam::RegistryService::RegistryClientBox m_registryClient;
      Nexus::AdministrationClient m_administrationClient;
      Nexus::ChartingClient m_chartingClient;
      Nexus::ComplianceClient m_complianceClient;
      Nexus::MarketDataClient m_marketDataClient;
      Nexus::OrderExecutionClient m_orderExecutionClient;
      Nexus::RiskClient m_riskClient;
      Beam::TimeService::TimeClientBox m_timeClient;

      SpireClients(const SpireClients&) = delete;
      SpireClients& operator =(const SpireClients&) = delete;
  };
}

#endif
