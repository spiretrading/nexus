#ifndef SPIRE_SERVICE_CLIENTS_HPP
#define SPIRE_SERVICE_CLIENTS_HPP
#include <memory>
#include <Beam/RegistryService/RegistryClientBox.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Threading/TimerBox.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include "Nexus/AdministrationService/AdministrationClientBox.hpp"
#include "Nexus/ChartingService/ChartingClientBox.hpp"
#include "Nexus/Compliance/ComplianceClientBox.hpp"
#include "Nexus/DefinitionsService/DefinitionsClientBox.hpp"
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClientBox.hpp"
#include "Nexus/RiskService/RiskClientBox.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /* Manages the set of clients connected to Spire services. */
  class SpireServiceClients {
    public:

      /**
       * Constructs a SpireServiceClients instance.
       * @param serviceLocatorClient The ServiceLocatorClient signed into the
       *        Spire services.
       */
      explicit SpireServiceClients(
        std::unique_ptr<Beam::ServiceLocator::ApplicationServiceLocatorClient>
          serviceLocatorClient);

      ~SpireServiceClients();

      /** Returns the ServiceLocatorClient. */
      Beam::ServiceLocator::ServiceLocatorClientBox& GetServiceLocatorClient();

      /** Returns the RegistryClient. */
      Beam::RegistryService::RegistryClientBox& GetRegistryClient();

      /** Returns the AdministrationClient. */
      Nexus::AdministrationService::AdministrationClientBox&
        GetAdministrationClient();

      /** Returns the DefinitionsClient. */
      Nexus::DefinitionsService::DefinitionsClientBox& GetDefinitionsClient();

      /** Returns the MarketDataClient. */
      Nexus::MarketDataService::MarketDataClientBox& GetMarketDataClient();

      /** Returns the ChartingClient. */
      Nexus::ChartingService::ChartingClientBox& GetChartingClient();

      /** Returns the ComplianceClient. */
      Nexus::Compliance::ComplianceClientBox& GetComplianceClient();

      /** Returns the OrderExecutionClient. */
      Nexus::OrderExecutionService::OrderExecutionClientBox&
        GetOrderExecutionClient();

      /** Returns the RiskClient. */
      Nexus::RiskService::RiskClientBox& GetRiskClient();

      /** Returns the TimeClient. */
      Beam::TimeService::TimeClientBox& GetTimeClient();

      std::unique_ptr<Beam::Threading::TimerBox> MakeTimer(
        boost::posix_time::time_duration expiry);

      /** Closes all the clients. */
      void Close();

    private:
      std::unique_ptr<Beam::ServiceLocator::ApplicationServiceLocatorClient>
        m_applicationServiceLocatorClient;
      Beam::ServiceLocator::ServiceLocatorClientBox m_serviceLocatorClient;
      Nexus::DefinitionsService::DefinitionsClientBox m_definitionsClient;
      Beam::RegistryService::RegistryClientBox m_registryClient;
      Nexus::AdministrationService::AdministrationClientBox
        m_administrationClient;
      Nexus::ChartingService::ChartingClientBox m_chartingClient;
      Nexus::Compliance::ComplianceClientBox m_complianceClient;
      Nexus::MarketDataService::MarketDataClientBox m_marketDataClient;
      Nexus::OrderExecutionService::OrderExecutionClientBox
        m_orderExecutionClient;
      Nexus::RiskService::RiskClientBox m_riskClient;
      Beam::TimeService::TimeClientBox m_timeClient;

      SpireServiceClients(const SpireServiceClients&) = delete;
      SpireServiceClients& operator =(const SpireServiceClients&) = delete;
  };
}

#endif
