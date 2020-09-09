#ifndef NEXUS_BACKTESTER_SERVICE_CLIENTS_HPP
#define NEXUS_BACKTESTER_SERVICE_CLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterMarketDataClient.hpp"
#include "Nexus/Backtester/BacktesterTimeClient.hpp"
#include "Nexus/Backtester/BacktesterTimer.hpp"

namespace Nexus {

  /** Implements the ServiceClients interface for the purpose of backtesting. */
  class BacktesterServiceClients {
    public:
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      using RegistryClient = Beam::RegistryService::VirtualRegistryClient;

      using AdministrationClient =
        AdministrationService::VirtualAdministrationClient;

      using DefinitionsClient = DefinitionsService::VirtualDefinitionsClient;

      using MarketDataClient = MarketDataService::VirtualMarketDataClient;

      using ChartingClient = ChartingService::VirtualChartingClient;

      using ComplianceClient = Compliance::VirtualComplianceClient;

      using OrderExecutionClient =
        OrderExecutionService::VirtualOrderExecutionClient;

      using RiskClient = RiskService::VirtualRiskClient;

      using TimeClient = Beam::TimeService::VirtualTimeClient;

      using Timer = Beam::Threading::VirtualTimer;

      /**
       * Constructs a BacktesterServiceClients.
       * @param environment The BacktesterEnvironment to use.
       */
      BacktesterServiceClients(Beam::Ref<BacktesterEnvironment> environment);

      ~BacktesterServiceClients();

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
      BacktesterEnvironment* m_environment;
      std::unique_ptr<ServiceLocatorClient> m_serviceLocatorClient;
      std::unique_ptr<RegistryClient> m_registryClient;
      std::unique_ptr<DefinitionsClient> m_definitionsClient;
      std::unique_ptr<AdministrationClient> m_administrationClient;
      std::unique_ptr<MarketDataClient> m_marketDataClient;
      std::unique_ptr<ChartingClient> m_chartingClient;
      std::unique_ptr<ComplianceClient> m_complianceClient;
      std::unique_ptr<OrderExecutionClient> m_orderExecutionClient;
      std::unique_ptr<RiskClient> m_riskClient;
      std::unique_ptr<TimeClient> m_timeClient;
      Beam::IO::OpenState m_openState;

      BacktesterServiceClients(const BacktesterServiceClients&) = delete;
      BacktesterServiceClients& operator =(
        const BacktesterServiceClients&) = delete;
      void Shutdown();
  };

  inline BacktesterServiceClients::BacktesterServiceClients(
    Beam::Ref<BacktesterEnvironment> environment)
    : m_environment(environment.Get()),
      m_serviceLocatorClient(
        m_environment->GetServiceLocatorEnvironment().BuildClient()),
      m_registryClient(m_environment->GetRegistryEnvironment().BuildClient(
        Beam::Ref(*m_serviceLocatorClient))),
      m_definitionsClient(
        m_environment->GetDefinitionsEnvironment().BuildClient(
        Beam::Ref(*m_serviceLocatorClient))),
      m_administrationClient(
        m_environment->GetAdministrationEnvironment().BuildClient(
        Beam::Ref(*m_serviceLocatorClient))),
      m_marketDataClient(MarketDataService::MakeVirtualMarketDataClient(
        std::make_unique<BacktesterMarketDataClient>(
        Beam::Ref(m_environment->GetMarketDataService()),
        m_environment->GetMarketDataEnvironment().BuildClient(Beam::Ref(
        *m_serviceLocatorClient))))),
      m_chartingClient(m_environment->GetChartingEnvironment().BuildClient(
        Beam::Ref(*m_serviceLocatorClient))),
      m_complianceClient(m_environment->GetComplianceEnvironment().BuildClient(
        Beam::Ref(*m_serviceLocatorClient))),
      m_orderExecutionClient(
        m_environment->GetOrderExecutionEnvironment().BuildClient(
        Beam::Ref(*m_serviceLocatorClient))),
      m_riskClient(m_environment->GetRiskEnvironment().BuildClient(
        Beam::Ref(*m_serviceLocatorClient))),
      m_timeClient(Beam::TimeService::MakeVirtualTimeClient<
        BacktesterTimeClient>(Beam::Initialize(Beam::Ref(
        m_environment->GetEventHandler())))) {
    m_openState.SetOpen();
  }

  inline BacktesterServiceClients::~BacktesterServiceClients() {
    Close();
  }

  inline BacktesterServiceClients::ServiceLocatorClient&
      BacktesterServiceClients::GetServiceLocatorClient() {
    return *m_serviceLocatorClient;
  }

  inline BacktesterServiceClients::RegistryClient&
      BacktesterServiceClients::GetRegistryClient() {
    return *m_registryClient;
  }

  inline BacktesterServiceClients::AdministrationClient&
      BacktesterServiceClients::GetAdministrationClient() {
    return *m_administrationClient;
  }

  inline BacktesterServiceClients::DefinitionsClient&
      BacktesterServiceClients::GetDefinitionsClient() {
    return *m_definitionsClient;
  }

  inline BacktesterServiceClients::MarketDataClient&
      BacktesterServiceClients::GetMarketDataClient() {
    return *m_marketDataClient;
  }

  inline BacktesterServiceClients::ChartingClient&
      BacktesterServiceClients::GetChartingClient() {
    return *m_chartingClient;
  }

  inline BacktesterServiceClients::ComplianceClient&
      BacktesterServiceClients::GetComplianceClient() {
    return *m_complianceClient;
  }

  inline BacktesterServiceClients::OrderExecutionClient&
      BacktesterServiceClients::GetOrderExecutionClient() {
    return *m_orderExecutionClient;
  }

  inline BacktesterServiceClients::RiskClient&
      BacktesterServiceClients::GetRiskClient() {
    return *m_riskClient;
  }

  inline BacktesterServiceClients::TimeClient&
      BacktesterServiceClients::GetTimeClient() {
    return *m_timeClient;
  }

  inline std::unique_ptr<BacktesterServiceClients::Timer>
      BacktesterServiceClients::BuildTimer(
      boost::posix_time::time_duration expiry) {
    return Beam::Threading::MakeVirtualTimer(std::make_unique<BacktesterTimer>(
      expiry, Beam::Ref(m_environment->GetEventHandler())));
  }

  inline void BacktesterServiceClients::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void BacktesterServiceClients::Shutdown() {
    m_timeClient->Close();
    m_riskClient->Close();
    m_orderExecutionClient->Close();
    m_complianceClient->Close();
    m_chartingClient->Close();
    m_marketDataClient->Close();
    m_administrationClient->Close();
    m_definitionsClient->Close();
    m_registryClient->Close();
    m_serviceLocatorClient->Close();
    m_openState.SetClosed();
  }
}

#endif
