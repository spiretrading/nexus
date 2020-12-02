#ifndef NEXUS_VIRTUAL_SERVICE_CLIENTS_HPP
#define NEXUS_VIRTUAL_SERVICE_CLIENTS_HPP
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
#include "Nexus/RiskService/VirtualRiskClient.hpp"

namespace Nexus {

  /** Provides a pure virtual interface to the set of ServiceClients. */
  class VirtualServiceClients {
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

      using RiskClient = RiskService::VirtualRiskClient;

      using TimeClient = Beam::TimeService::TimeClientBox;

      using Timer = Beam::Threading::TimerBox;

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

    protected:

      /** Constructs a VirtualServiceClients. */
      VirtualServiceClients() = default;

    private:
      VirtualServiceClients(const VirtualServiceClients&) = delete;
      VirtualServiceClients& operator =(const VirtualServiceClients&) = delete;
  };

  /**
   * Wraps ServiceClients providing it with a virtual interface.
   * @param <C> The type of ServiceClients to wrap.
   */
  template<typename C>
  class WrapperServiceClients : public VirtualServiceClients {
    public:

      /** The ServiceClients to wrap. */
      using Client = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a WrapperServiceClients.
       * @param client The ServiceClients to wrap.
       */
      template<typename CF>
      WrapperServiceClients(CF&& client);

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

    private:
      Beam::GetOptionalLocalPtr<C> m_client;
      ServiceLocatorClient m_serviceLocatorClient;
      RegistryClient m_registryClient;
      AdministrationClient m_administrationClient;
      DefinitionsClient m_definitionsClient;
      MarketDataClient m_marketDataClient;
      ChartingClient m_chartingClient;
      ComplianceClient m_complianceClient;
      OrderExecutionClient m_orderExecutionClient;
      std::unique_ptr<RiskClient> m_riskClient;
      TimeClient m_timeClient;
  };

  /**
   * Wraps ServiceClients into a VirtualServiceClients.
   * @param client The client to wrap.
   */
  template<typename ServiceClients>
  std::unique_ptr<VirtualServiceClients> MakeVirtualServiceClients(
      ServiceClients&& client) {
    return std::make_unique<WrapperServiceClients<
      std::remove_reference_t<ServiceClients>>>(
      std::forward<ServiceClients>(client));
  }

  template<typename C>
  template<typename CF>
  WrapperServiceClients<C>::WrapperServiceClients(CF&& client)
    : m_client(std::forward<CF>(client)),
      m_serviceLocatorClient(&m_client->GetServiceLocatorClient()),
      m_registryClient(&m_client->GetRegistryClient()),
      m_administrationClient(&m_client->GetAdministrationClient()),
      m_definitionsClient(&m_client->GetDefinitionsClient()),
      m_marketDataClient(&m_client->GetMarketDataClient()),
      m_chartingClient(&m_client->GetChartingClient()),
      m_complianceClient(&m_client->GetComplianceClient()),
      m_orderExecutionClient(&m_client->GetOrderExecutionClient()),
      m_riskClient(RiskService::MakeVirtualRiskClient(
        &m_client->GetRiskClient())),
      m_timeClient(&m_client->GetTimeClient()) {}

  template<typename C>
  typename WrapperServiceClients<C>::ServiceLocatorClient&
      WrapperServiceClients<C>::GetServiceLocatorClient() {
    return m_serviceLocatorClient;
  }

  template<typename C>
  typename WrapperServiceClients<C>::RegistryClient&
      WrapperServiceClients<C>::GetRegistryClient() {
    return m_registryClient;
  }

  template<typename C>
  typename WrapperServiceClients<C>::AdministrationClient&
      WrapperServiceClients<C>::GetAdministrationClient() {
    return m_administrationClient;
  }

  template<typename C>
  typename WrapperServiceClients<C>::DefinitionsClient&
      WrapperServiceClients<C>::GetDefinitionsClient() {
    return m_definitionsClient;
  }

  template<typename C>
  typename WrapperServiceClients<C>::MarketDataClient&
      WrapperServiceClients<C>::GetMarketDataClient() {
    return m_marketDataClient;
  }

  template<typename C>
  typename WrapperServiceClients<C>::ChartingClient&
      WrapperServiceClients<C>::GetChartingClient() {
    return m_chartingClient;
  }

  template<typename C>
  typename WrapperServiceClients<C>::ComplianceClient&
      WrapperServiceClients<C>::GetComplianceClient() {
    return m_complianceClient;
  }

  template<typename C>
  typename WrapperServiceClients<C>::OrderExecutionClient&
      WrapperServiceClients<C>::GetOrderExecutionClient() {
    return m_orderExecutionClient;
  }

  template<typename C>
  typename WrapperServiceClients<C>::RiskClient&
      WrapperServiceClients<C>::GetRiskClient() {
    return *m_riskClient;
  }

  template<typename C>
  typename WrapperServiceClients<C>::TimeClient&
      WrapperServiceClients<C>::GetTimeClient() {
    return m_timeClient;
  }

  template<typename C>
  std::unique_ptr<typename WrapperServiceClients<C>::Timer>
      WrapperServiceClients<C>::BuildTimer(
        boost::posix_time::time_duration expiry) {
    return std::make_unique<Beam::Threading::TimerBox>(
      m_client->BuildTimer(expiry));
  }

  template<typename C>
  void WrapperServiceClients<C>::Close() {
    m_client->Close();
  }
}

#endif
