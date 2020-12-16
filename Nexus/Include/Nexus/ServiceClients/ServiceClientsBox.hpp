#ifndef NEXUS_SERVICE_CLIENTS_BOX_HPP
#define NEXUS_SERVICE_CLIENTS_BOX_HPP
#include <memory>
#include <type_traits>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/RegistryService/RegistryClientBox.hpp>
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

namespace Nexus {

  /** Provides a generic interface over arbitrary ServiceClients. */
  class ServiceClientsBox {
    public:
      using ServiceLocatorClient =
        Beam::ServiceLocator::ServiceLocatorClientBox;

      using RegistryClient = Beam::RegistryService::RegistryClientBox;

      using AdministrationClient =
        AdministrationService::AdministrationClientBox;

      using DefinitionsClient = DefinitionsService::DefinitionsClientBox;

      using MarketDataClient = MarketDataService::MarketDataClientBox;

      using ChartingClient = ChartingService::ChartingClientBox;

      using ComplianceClient = Compliance::ComplianceClientBox;

      using OrderExecutionClient =
        OrderExecutionService::OrderExecutionClientBox;

      using RiskClient = RiskService::RiskClientBox;

      using TimeClient = Beam::TimeService::TimeClientBox;

      using Timer = Beam::Threading::TimerBox;

      /**
       * Constructs a ServiceClientsBox of a specified type using emplacement.
       * @param <T> The type of service clients to emplace.
       * @param args The arguments to pass to the emplaced service clients.
       */
      template<typename T, typename... Args>
      explicit ServiceClientsBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ServiceClientsBox by copying existing service clients.
       * @param clients The clients to copy.
       */
      template<typename ServiceClients>
      explicit ServiceClientsBox(ServiceClients clients);

      explicit ServiceClientsBox(ServiceClientsBox* clients);

      explicit ServiceClientsBox(
        const std::shared_ptr<ServiceClientsBox>& clients);

      explicit ServiceClientsBox(
        const std::unique_ptr<ServiceClientsBox>& clients);

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
        boost::posix_time::time_duration expiry);

      void Close();

    private:
      struct VirtualServiceClients {
        virtual ~VirtualServiceClients() = default;
        virtual ServiceLocatorClient& GetServiceLocatorClient() = 0;
        virtual RegistryClient& GetRegistryClient() = 0;
        virtual AdministrationClient& GetAdministrationClient() = 0;
        virtual DefinitionsClient& GetDefinitionsClient() = 0;
        virtual MarketDataClient& GetMarketDataClient() = 0;
        virtual ChartingClient& GetChartingClient() = 0;
        virtual ComplianceClient& GetComplianceClient() = 0;
        virtual OrderExecutionClient& GetOrderExecutionClient() = 0;
        virtual RiskClient& GetRiskClient() = 0;
        virtual TimeClient& GetTimeClient() = 0;
        virtual std::unique_ptr<Timer> BuildTimer(
          boost::posix_time::time_duration expiry) = 0;
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedServiceClients final : VirtualServiceClients {
        using Clients = C;
        Beam::GetOptionalLocalPtr<Clients> m_clients;
        ServiceLocatorClient m_serviceLocatorClient;
        RegistryClient m_registryClient;
        AdministrationClient m_administrationClient;
        DefinitionsClient m_definitionsClient;
        MarketDataClient m_marketDataClient;
        ChartingClient m_chartingClient;
        ComplianceClient m_complianceClient;
        OrderExecutionClient m_orderExecutionClient;
        RiskClient m_riskClient;
        TimeClient m_timeClient;

        template<typename... Args>
        WrappedServiceClients(Args&&... args);
        ServiceLocatorClient& GetServiceLocatorClient() override;
        RegistryClient& GetRegistryClient() override;
        AdministrationClient& GetAdministrationClient() override;
        DefinitionsClient& GetDefinitionsClient() override;
        MarketDataClient& GetMarketDataClient() override;
        ChartingClient& GetChartingClient() override;
        ComplianceClient& GetComplianceClient() override;
        OrderExecutionClient& GetOrderExecutionClient() override;
        RiskClient& GetRiskClient() override;
        TimeClient& GetTimeClient() override;
        std::unique_ptr<Timer> BuildTimer(
          boost::posix_time::time_duration expiry) override;
        void Close() override;
      };

      std::shared_ptr<VirtualServiceClients> m_clients;
  };

  template<typename T, typename... Args>
  ServiceClientsBox::ServiceClientsBox(std::in_place_type_t<T>, Args&&... args)
    : m_clients(std::make_shared<WrappedServiceClients<T>>(
        std::forward<Args>(args)...)) {}

  template<typename ServiceClients>
  ServiceClientsBox::ServiceClientsBox(ServiceClients clients)
    : ServiceClientsBox(std::in_place_type<ServiceClients>,
        std::move(clients)) {}

  inline ServiceClientsBox::ServiceClientsBox(ServiceClientsBox* clients)
    : ServiceClientsBox(*clients) {}

  inline ServiceClientsBox::ServiceClientsBox(
    const std::shared_ptr<ServiceClientsBox>& clients)
    : ServiceClientsBox(*clients) {}

  inline ServiceClientsBox::ServiceClientsBox(
    const std::unique_ptr<ServiceClientsBox>& clients)
    : ServiceClientsBox(*clients) {}

  inline ServiceClientsBox::ServiceLocatorClient&
      ServiceClientsBox::GetServiceLocatorClient() {
    return m_clients->GetServiceLocatorClient();
  }

  inline ServiceClientsBox::RegistryClient&
      ServiceClientsBox::GetRegistryClient() {
    return m_clients->GetRegistryClient();
  }

  inline ServiceClientsBox::AdministrationClient&
      ServiceClientsBox::GetAdministrationClient() {
    return m_clients->GetAdministrationClient();
  }

  inline ServiceClientsBox::DefinitionsClient&
      ServiceClientsBox::GetDefinitionsClient() {
    return m_clients->GetDefinitionsClient();
  }

  inline ServiceClientsBox::MarketDataClient&
      ServiceClientsBox::GetMarketDataClient() {
    return m_clients->GetMarketDataClient();
  }

  inline ServiceClientsBox::ChartingClient&
      ServiceClientsBox::GetChartingClient() {
    return m_clients->GetChartingClient();
  }

  inline ServiceClientsBox::ComplianceClient&
      ServiceClientsBox::GetComplianceClient() {
    return m_clients->GetComplianceClient();
  }

  inline ServiceClientsBox::OrderExecutionClient&
      ServiceClientsBox::GetOrderExecutionClient() {
    return m_clients->GetOrderExecutionClient();
  }

  inline ServiceClientsBox::RiskClient& ServiceClientsBox::GetRiskClient() {
    return m_clients->GetRiskClient();
  }

  inline ServiceClientsBox::TimeClient& ServiceClientsBox::GetTimeClient() {
    return m_clients->GetTimeClient();
  }

  inline std::unique_ptr<ServiceClientsBox::Timer>
      ServiceClientsBox::BuildTimer(boost::posix_time::time_duration expiry) {
    return m_clients->BuildTimer(expiry);
  }

  inline void ServiceClientsBox::Close() {
    m_clients->Close();
  }

  template<typename C>
  template<typename... Args>
  ServiceClientsBox::WrappedServiceClients<C>::WrappedServiceClients(
    Args&&... args)
    : m_clients(std::forward<Args>(args)...),
      m_serviceLocatorClient(&m_clients->GetServiceLocatorClient()),
      m_registryClient(&m_clients->GetRegistryClient()),
      m_administrationClient(&m_clients->GetAdministrationClient()),
      m_definitionsClient(&m_clients->GetDefinitionsClient()),
      m_marketDataClient(&m_clients->GetMarketDataClient()),
      m_chartingClient(&m_clients->GetChartingClient()),
      m_complianceClient(&m_clients->GetComplianceClient()),
      m_orderExecutionClient(&m_clients->GetOrderExecutionClient()),
      m_riskClient(&m_clients->GetRiskClient()),
      m_timeClient(&m_clients->GetTimeClient()) {}

  template<typename C>
  ServiceClientsBox::ServiceLocatorClient&
      ServiceClientsBox::WrappedServiceClients<C>::GetServiceLocatorClient() {
    return m_serviceLocatorClient;
  }

  template<typename C>
  ServiceClientsBox::RegistryClient&
      ServiceClientsBox::WrappedServiceClients<C>::GetRegistryClient() {
    return m_registryClient;
  }

  template<typename C>
  ServiceClientsBox::AdministrationClient&
      ServiceClientsBox::WrappedServiceClients<C>::GetAdministrationClient() {
    return m_administrationClient;
  }

  template<typename C>
  ServiceClientsBox::DefinitionsClient&
      ServiceClientsBox::WrappedServiceClients<C>::GetDefinitionsClient() {
    return m_definitionsClient;
  }

  template<typename C>
  ServiceClientsBox::MarketDataClient&
      ServiceClientsBox::WrappedServiceClients<C>::GetMarketDataClient() {
    return m_marketDataClient;
  }

  template<typename C>
  ServiceClientsBox::ChartingClient&
      ServiceClientsBox::WrappedServiceClients<C>::GetChartingClient() {
    return m_chartingClient;
  }

  template<typename C>
  ServiceClientsBox::ComplianceClient&
      ServiceClientsBox::WrappedServiceClients<C>::GetComplianceClient() {
    return m_complianceClient;
  }

  template<typename C>
  ServiceClientsBox::OrderExecutionClient&
      ServiceClientsBox::WrappedServiceClients<C>::GetOrderExecutionClient() {
    return m_orderExecutionClient;
  }

  template<typename C>
  ServiceClientsBox::RiskClient&
      ServiceClientsBox::WrappedServiceClients<C>::GetRiskClient() {
    return m_riskClient;
  }

  template<typename C>
  ServiceClientsBox::TimeClient&
      ServiceClientsBox::WrappedServiceClients<C>::GetTimeClient() {
    return m_timeClient;
  }

  template<typename C>
  std::unique_ptr<ServiceClientsBox::Timer>
      ServiceClientsBox::WrappedServiceClients<C>::BuildTimer(
        boost::posix_time::time_duration expiry) {
    return std::make_unique<Beam::Threading::TimerBox>(
      m_clients->BuildTimer(expiry));
  }

  template<typename C>
  void ServiceClientsBox::WrappedServiceClients<C>::Close() {
    m_clients->Close();
  }
}

#endif
