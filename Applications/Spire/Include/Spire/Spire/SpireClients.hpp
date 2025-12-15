#ifndef SPIRE_CLIENTS_HPP
#define SPIRE_CLIENTS_HPP
#include <memory>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include "Nexus/Clients/Clients.hpp"

namespace Spire {

  /* Manages the set of clients connected to Spire s. */
  class SpireClients {
    public:
      using ServiceLocatorClient = Beam::ServiceLocatorClient;
      using AdministrationClient = Nexus::AdministrationClient;
      using DefinitionsClient = Nexus::DefinitionsClient;
      using MarketDataClient = Nexus::MarketDataClient;
      using ChartingClient = Nexus::ChartingClient;
      using ComplianceClient = Nexus::ComplianceClient;
      using OrderExecutionClient = Nexus::OrderExecutionClient;
      using RiskClient = Nexus::RiskClient;
      using TimeClient = Beam::TimeClient;
      using Timer = Beam::Timer;

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
        m_application_service_locator_client;
      Beam::ServiceLocatorClient m_service_locator_client;
      Nexus::DefinitionsClient m_definitions_client;
      Nexus::AdministrationClient m_administration_client;
      Nexus::ChartingClient m_charting_client;
      Nexus::ComplianceClient m_compliance_client;
      Nexus::MarketDataClient m_market_data_client;
      Nexus::OrderExecutionClient m_order_execution_client;
      Nexus::RiskClient m_risk_client;
      Beam::TimeClient m_time_client;

      SpireClients(const SpireClients&) = delete;
      SpireClients& operator =(const SpireClients&) = delete;
  };
}

#endif
