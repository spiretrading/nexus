#ifndef SPIRE_SERVICE_CLIENTS_HPP
#define SPIRE_SERVICE_CLIENTS_HPP
#include <memory>
#include <Beam/Network/Network.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/RegistryService/RegistryService.hpp>
#include <Beam/ServiceLocator/ServiceLocator.hpp>
#include <Beam/Threading/Threading.hpp>
#include <Beam/TimeService/TimeService.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/ChartingService/ChartingService.hpp"
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/DefinitionsService/DefinitionsService.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /* Manages the set of clients connected to Spire services. */
  class SpireServiceClients : private boost::noncopyable {
    public:

      //! Constructs a SpireServiceClients instance.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient logged into the
               Spire services.
        \param socketThreadPool The SocketThreadPool the clients will use.
        \param timerThreadPool The TimerThreadPool the clients will use.
      */
      SpireServiceClients(
        std::unique_ptr<Beam::ServiceLocator::ApplicationServiceLocatorClient>
        serviceLocatorClient,
        Beam::Ref<Beam::Network::SocketThreadPool> socketThreadPool,
        Beam::Ref<Beam::Threading::TimerThreadPool> timerThreadPool);

      ~SpireServiceClients();

      //! Returns the ServiceLocatorClient.
      Beam::ServiceLocator::VirtualServiceLocatorClient&
        GetServiceLocatorClient() const;

      //! Returns the RegistryClient.
      Beam::RegistryService::VirtualRegistryClient& GetRegistryClient() const;

      //! Returns the AdministrationClient.
      Nexus::AdministrationService::VirtualAdministrationClient&
        GetAdministrationClient() const;

      //! Returns the DefinitionsClient.
      Nexus::DefinitionsService::VirtualDefinitionsClient&
        GetDefinitionsClient() const;

      //! Returns the MarketDataClient.
      Nexus::MarketDataService::VirtualMarketDataClient&
        GetMarketDataClient() const;

      //! Returns the ChartingClient.
      Nexus::ChartingService::VirtualChartingClient& GetChartingClient() const;

      //! Returns the ComplianceClient.
      Nexus::Compliance::VirtualComplianceClient& GetComplianceClient() const;

      //! Returns the OrderExecutionClient.
      Nexus::OrderExecutionService::VirtualOrderExecutionClient&
        GetOrderExecutionClient() const;

      //! Returns the RiskClient.
      Nexus::RiskService::VirtualRiskClient& GetRiskClient() const;

      //! Returns the TimeClient.
      Beam::TimeService::VirtualTimeClient& GetTimeClient() const;

      std::unique_ptr<Beam::Threading::VirtualTimer> BuildTimer(
        boost::posix_time::time_duration expiry);

      //! Closes all the clients.
      void Close();

    private:
      std::unique_ptr<Beam::ServiceLocator::ApplicationServiceLocatorClient>
        m_applicationServiceLocatorClient;
      std::unique_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        m_serviceLocatorClient;
      Beam::Network::SocketThreadPool* m_socketThreadPool;
      Beam::Threading::TimerThreadPool* m_timerThreadPool;
      std::unique_ptr<Nexus::DefinitionsService::VirtualDefinitionsClient>
        m_definitionsClient;
      std::unique_ptr<Beam::RegistryService::VirtualRegistryClient>
        m_registryClient;
      std::unique_ptr<Nexus::AdministrationService::VirtualAdministrationClient>
        m_administrationClient;
      std::unique_ptr<Nexus::ChartingService::VirtualChartingClient>
        m_chartingClient;
      std::unique_ptr<Nexus::Compliance::VirtualComplianceClient>
        m_complianceClient;
      std::unique_ptr<Nexus::MarketDataService::VirtualMarketDataClient>
        m_marketDataClient;
      std::unique_ptr<Nexus::OrderExecutionService::VirtualOrderExecutionClient>
        m_orderExecutionClient;
      std::unique_ptr<Nexus::RiskService::VirtualRiskClient> m_riskClient;
      std::unique_ptr<Beam::TimeService::VirtualTimeClient> m_timeClient;
  };
}

#endif
