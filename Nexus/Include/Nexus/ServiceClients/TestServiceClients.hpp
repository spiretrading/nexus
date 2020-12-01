#ifndef NEXUS_TEST_SERVICE_CLIENTS_HPP
#define NEXUS_TEST_SERVICE_CLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/RegistryService/RegistryClientBox.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Threading/TimerBox.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <Beam/TimeServiceTests/TestTimeClient.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include "Nexus/ChartingService/ChartingClientBox.hpp"
#include "Nexus/Compliance/ComplianceClientBox.hpp"
#include "Nexus/DefinitionsService/VirtualDefinitionsClient.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/RiskService/VirtualRiskClient.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"

namespace Nexus {

  /** Implements the ServiceClients interface for testing. */
  class TestServiceClients {
    public:
      using ServiceLocatorClient =
        Beam::ServiceLocator::ServiceLocatorClientBox;

      using RegistryClient = Beam::RegistryService::RegistryClientBox;

      using AdministrationClient =
        AdministrationService::AdministrationClientBox;

      using DefinitionsClient = DefinitionsService::VirtualDefinitionsClient;

      using MarketDataClient = MarketDataService::VirtualMarketDataClient;

      using ChartingClient = ChartingService::ChartingClientBox;

      using ComplianceClient = Compliance::ComplianceClientBox;

      using OrderExecutionClient =
        OrderExecutionService::VirtualOrderExecutionClient;

      using RiskClient = RiskService::VirtualRiskClient;

      using TimeClient = Beam::TimeService::TimeClientBox;

      using Timer = Beam::Threading::TimerBox;

      /**
       * Constructs a TestServiceClients.
       * @param environment The TestEnvironment to use.
       */
      TestServiceClients(Beam::Ref<TestEnvironment> environment);

      /**
       * Constructs a TestServiceClients.
       * @param username The username to login with.
       * @param password The password to use.
       * @param environment The TestEnvironment to use.
       */
      TestServiceClients(std::string username, std::string password,
        Beam::Ref<TestEnvironment> environment);

      ~TestServiceClients();

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
      TestEnvironment* m_environment;
      ServiceLocatorClient m_serviceLocatorClient;
      RegistryClient m_registryClient;
      std::unique_ptr<DefinitionsClient> m_definitionsClient;
      AdministrationClient m_administrationClient;
      std::unique_ptr<MarketDataClient> m_marketDataClient;
      ChartingClient m_chartingClient;
      ComplianceClient m_complianceClient;
      std::unique_ptr<OrderExecutionClient> m_orderExecutionClient;
      std::unique_ptr<RiskClient> m_riskClient;
      TimeClient m_timeClient;
      Beam::IO::OpenState m_openState;

      TestServiceClients(const TestServiceClients&) = delete;
      TestServiceClients& operator =(const TestServiceClients&) = delete;
  };

  inline TestServiceClients::TestServiceClients(
    Beam::Ref<TestEnvironment> environment)
    : TestServiceClients("root", "", Beam::Ref(environment)) {}

  inline TestServiceClients::TestServiceClients(std::string username,
    std::string password, Beam::Ref<TestEnvironment> environment)
    : m_environment(environment.Get()),
      m_serviceLocatorClient(
        m_environment->GetServiceLocatorEnvironment().MakeClient(
        std::move(username), std::move(password))),
      m_registryClient(m_environment->GetRegistryEnvironment().MakeClient(
        m_serviceLocatorClient)),
      m_definitionsClient(m_environment->GetDefinitionsEnvironment().MakeClient(
        m_serviceLocatorClient)),
      m_administrationClient(
        m_environment->GetAdministrationEnvironment().MakeClient(
          m_serviceLocatorClient)),
      m_marketDataClient(m_environment->GetMarketDataEnvironment().MakeClient(
        m_serviceLocatorClient)),
      m_chartingClient(m_environment->GetChartingEnvironment().MakeClient(
        m_serviceLocatorClient)),
      m_complianceClient(m_environment->GetComplianceEnvironment().MakeClient(
        m_serviceLocatorClient)),
      m_orderExecutionClient(
        m_environment->GetOrderExecutionEnvironment().MakeClient(
          m_serviceLocatorClient)),
      m_riskClient(m_environment->GetRiskEnvironment().MakeClient(
        m_serviceLocatorClient)),
      m_timeClient(std::make_unique<Beam::TimeService::Tests::TestTimeClient>(
        Beam::Ref(m_environment->GetTimeEnvironment()))) {}

  inline TestServiceClients::~TestServiceClients() {
    Close();
  }

  inline TestServiceClients::ServiceLocatorClient&
      TestServiceClients::GetServiceLocatorClient() {
    return m_serviceLocatorClient;
  }

  inline TestServiceClients::RegistryClient&
      TestServiceClients::GetRegistryClient() {
    return m_registryClient;
  }

  inline TestServiceClients::AdministrationClient&
      TestServiceClients::GetAdministrationClient() {
    return m_administrationClient;
  }

  inline TestServiceClients::DefinitionsClient&
      TestServiceClients::GetDefinitionsClient() {
    return *m_definitionsClient;
  }

  inline TestServiceClients::MarketDataClient&
      TestServiceClients::GetMarketDataClient() {
    return *m_marketDataClient;
  }

  inline TestServiceClients::ChartingClient&
      TestServiceClients::GetChartingClient() {
    return m_chartingClient;
  }

  inline TestServiceClients::ComplianceClient&
      TestServiceClients::GetComplianceClient() {
    return m_complianceClient;
  }

  inline TestServiceClients::OrderExecutionClient&
      TestServiceClients::GetOrderExecutionClient() {
    return *m_orderExecutionClient;
  }

  inline TestServiceClients::RiskClient& TestServiceClients::GetRiskClient() {
    return *m_riskClient;
  }

  inline TestServiceClients::TimeClient& TestServiceClients::GetTimeClient() {
    return m_timeClient;
  }

  inline std::unique_ptr<TestServiceClients::Timer>
      TestServiceClients::BuildTimer(boost::posix_time::time_duration expiry) {
    return std::make_unique<Timer>(
      std::make_unique<Beam::TimeService::Tests::TestTimer>(expiry,
      Beam::Ref(m_environment->GetTimeEnvironment())));
  }

  inline void TestServiceClients::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_timeClient.Close();
    m_riskClient->Close();
    m_orderExecutionClient->Close();
    m_complianceClient.Close();
    m_chartingClient.Close();
    m_marketDataClient->Close();
    m_administrationClient.Close();
    m_definitionsClient->Close();
    m_registryClient.Close();
    m_serviceLocatorClient.Close();
    m_openState.Close();
  }
}

#endif
