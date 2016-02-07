#ifndef NEXUS_SERVICECLIENTS_HPP
#define NEXUS_SERVICECLIENTS_HPP
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

namespace Nexus {
namespace ClientWebPortal {

  /*! \class ServiceClients
      \brief Manages the set of clients connected to Spire services.
   */
  class ServiceClients : private boost::noncopyable {
    public:

      //! Constructs a ServiceClients instance.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient logged into the
               Spire services.
        \param socketThreadPool The SocketThreadPool the clients will use.
        \param timerThreadPool The TimerThreadPool the clients will use.
      */
      ServiceClients(
        Beam::RefType<Beam::ServiceLocator::ApplicationServiceLocatorClient>
        serviceLocatorClient,
        Beam::RefType<Beam::Network::SocketThreadPool> socketThreadPool,
        Beam::RefType<Beam::Threading::TimerThreadPool> timerThreadPool);

      ~ServiceClients();

      //! Returns the ServiceLocatorClient.
      Beam::ServiceLocator::VirtualServiceLocatorClient&
        GetServiceLocatorClient() const;

      //! Returns the RegistryClient.
      Beam::RegistryService::VirtualRegistryClient& GetRegistryClient() const;

      //! Returns the AdministrationClient.
      AdministrationService::VirtualAdministrationClient&
        GetAdministrationClient() const;

      //! Returns the DefinitionsClient.
      DefinitionsService::VirtualDefinitionsClient&
        GetDefinitionsClient() const;

      //! Returns the MarketDataClient.
      MarketDataService::VirtualMarketDataClient& GetMarketDataClient() const;

      //! Returns the ChartingClient.
      ChartingService::VirtualChartingClient& GetChartingClient() const;

      //! Returns the ComplianceClient.
      Compliance::VirtualComplianceClient& GetComplianceClient() const;

      //! Returns the OrderExecutionClient.
      OrderExecutionService::VirtualOrderExecutionClient&
        GetOrderExecutionClient() const;

      //! Returns the RiskClient.
      RiskService::VirtualRiskClient& GetRiskClient() const;

      //! Returns the TimeClient.
      Beam::TimeService::VirtualTimeClient& GetTimeClient() const;

      //! Opens all the clients.
      void Open();

      //! Closes all the clients.
      void Close();

    private:
      Beam::ServiceLocator::ApplicationServiceLocatorClient*
        m_applicationServiceLocatorClient;
      std::unique_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        m_serviceLocatorClient;
      Beam::Network::SocketThreadPool* m_socketThreadPool;
      Beam::Threading::TimerThreadPool* m_timerThreadPool;
      std::unique_ptr<DefinitionsService::VirtualDefinitionsClient>
        m_definitionsClient;
      std::unique_ptr<Beam::RegistryService::VirtualRegistryClient>
        m_registryClient;
      std::unique_ptr<AdministrationService::VirtualAdministrationClient>
        m_administrationClient;
      std::unique_ptr<ChartingService::VirtualChartingClient> m_chartingClient;
      std::unique_ptr<Compliance::VirtualComplianceClient> m_complianceClient;
      std::unique_ptr<MarketDataService::VirtualMarketDataClient>
        m_marketDataClient;
      std::unique_ptr<OrderExecutionService::VirtualOrderExecutionClient>
        m_orderExecutionClient;
      std::unique_ptr<RiskService::VirtualRiskClient> m_riskClient;
      std::unique_ptr<Beam::TimeService::VirtualTimeClient> m_timeClient;
  };
}
}

#endif
