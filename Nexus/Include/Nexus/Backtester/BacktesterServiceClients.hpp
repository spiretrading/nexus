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

      std::unique_ptr<Timer> MakeTimer(
        boost::posix_time::time_duration expiry);

      void Close();

    private:
      BacktesterEnvironment* m_environment;
      ServiceLocatorClient m_serviceLocatorClient;
      RegistryClient m_registryClient;
      DefinitionsClient m_definitionsClient;
      AdministrationClient m_administrationClient;
      MarketDataClient m_marketDataClient;
      ChartingClient m_chartingClient;
      ComplianceClient m_complianceClient;
      OrderExecutionClient m_orderExecutionClient;
      RiskClient m_riskClient;
      TimeClient m_timeClient;
      Beam::IO::OpenState m_openState;

      BacktesterServiceClients(const BacktesterServiceClients&) = delete;
      BacktesterServiceClients& operator =(
        const BacktesterServiceClients&) = delete;
  };

  inline BacktesterServiceClients::BacktesterServiceClients(
    Beam::Ref<BacktesterEnvironment> environment)
    : m_environment(environment.Get()),
      m_serviceLocatorClient(
        m_environment->GetServiceLocatorEnvironment().MakeClient()),
      m_registryClient(m_environment->GetRegistryEnvironment().MakeClient(
        m_serviceLocatorClient)),
      m_definitionsClient(
        m_environment->GetDefinitionsEnvironment().MakeClient(
          m_serviceLocatorClient)),
      m_administrationClient(
        m_environment->GetAdministrationEnvironment().MakeClient(
          m_serviceLocatorClient)),
      m_marketDataClient(std::make_unique<BacktesterMarketDataClient>(
        Beam::Ref(m_environment->GetMarketDataService()),
        m_environment->GetMarketDataEnvironment().MakeRegistryClient(
          m_serviceLocatorClient))),
      m_chartingClient(m_environment->GetChartingEnvironment().MakeClient(
        m_serviceLocatorClient)),
      m_complianceClient(m_environment->GetComplianceEnvironment().MakeClient(
        m_serviceLocatorClient)),
      m_orderExecutionClient(
        m_environment->GetOrderExecutionEnvironment().MakeClient(
          m_serviceLocatorClient)),
      m_riskClient(m_environment->GetRiskEnvironment().MakeClient(
        m_serviceLocatorClient)),
      m_timeClient(std::in_place_type<BacktesterTimeClient>,
        Beam::Ref(m_environment->GetEventHandler())) {}

  inline BacktesterServiceClients::~BacktesterServiceClients() {
    Close();
  }

  inline BacktesterServiceClients::ServiceLocatorClient&
      BacktesterServiceClients::GetServiceLocatorClient() {
    return m_serviceLocatorClient;
  }

  inline BacktesterServiceClients::RegistryClient&
      BacktesterServiceClients::GetRegistryClient() {
    return m_registryClient;
  }

  inline BacktesterServiceClients::AdministrationClient&
      BacktesterServiceClients::GetAdministrationClient() {
    return m_administrationClient;
  }

  inline BacktesterServiceClients::DefinitionsClient&
      BacktesterServiceClients::GetDefinitionsClient() {
    return m_definitionsClient;
  }

  inline BacktesterServiceClients::MarketDataClient&
      BacktesterServiceClients::GetMarketDataClient() {
    return m_marketDataClient;
  }

  inline BacktesterServiceClients::ChartingClient&
      BacktesterServiceClients::GetChartingClient() {
    return m_chartingClient;
  }

  inline BacktesterServiceClients::ComplianceClient&
      BacktesterServiceClients::GetComplianceClient() {
    return m_complianceClient;
  }

  inline BacktesterServiceClients::OrderExecutionClient&
      BacktesterServiceClients::GetOrderExecutionClient() {
    return m_orderExecutionClient;
  }

  inline BacktesterServiceClients::RiskClient&
      BacktesterServiceClients::GetRiskClient() {
    return m_riskClient;
  }

  inline BacktesterServiceClients::TimeClient&
      BacktesterServiceClients::GetTimeClient() {
    return m_timeClient;
  }

  inline std::unique_ptr<BacktesterServiceClients::Timer>
      BacktesterServiceClients::MakeTimer(
        boost::posix_time::time_duration expiry) {
    return std::make_unique<Timer>(std::in_place_type<BacktesterTimer>, expiry,
      Beam::Ref(m_environment->GetEventHandler()));
  }

  inline void BacktesterServiceClients::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_timeClient.Close();
    m_riskClient.Close();
    m_orderExecutionClient.Close();
    m_complianceClient.Close();
    m_chartingClient.Close();
    m_marketDataClient.Close();
    m_administrationClient.Close();
    m_definitionsClient.Close();
    m_registryClient.Close();
    m_serviceLocatorClient.Close();
    m_openState.Close();
  }
}

#endif
