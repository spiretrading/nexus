#ifndef NEXUS_TEST_SERVICE_CLIENTS_HPP
#define NEXUS_TEST_SERVICE_CLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/RegistryService/VirtualRegistryClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Threading/VirtualTimer.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <Beam/TimeServiceTests/TestTimeClient.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include "Nexus/ChartingService/VirtualChartingClient.hpp"
#include "Nexus/Compliance/VirtualComplianceClient.hpp"
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

      TestServiceClients(const TestServiceClients&) = delete;
      TestServiceClients& operator =(const TestServiceClients&) = delete;
      void Shutdown();
  };

  inline TestServiceClients::TestServiceClients(
    Beam::Ref<TestEnvironment> environment)
    : TestServiceClients("root", "", Beam::Ref(environment)) {}

  inline TestServiceClients::TestServiceClients(std::string username,
      std::string password, Beam::Ref<TestEnvironment> environment)
      : m_environment(environment.Get()),
        m_serviceLocatorClient(
          m_environment->GetServiceLocatorEnvironment().BuildClient(
          std::move(username), std::move(password))),
        m_registryClient(m_environment->GetRegistryEnvironment().BuildClient(
          Beam::Ref(*m_serviceLocatorClient))),
        m_definitionsClient(m_environment->GetDefinitionsEnvironment().
          BuildClient(Beam::Ref(*m_serviceLocatorClient))),
        m_administrationClient(m_environment->GetAdministrationEnvironment().
          BuildClient(Beam::Ref(*m_serviceLocatorClient))),
        m_marketDataClient(m_environment->GetMarketDataEnvironment().
          BuildClient(Beam::Ref(*m_serviceLocatorClient))),
        m_chartingClient(m_environment->GetChartingEnvironment().BuildClient(
          Beam::Ref(*m_serviceLocatorClient))),
        m_complianceClient(
          m_environment->GetComplianceEnvironment().BuildClient(
          Beam::Ref(*m_serviceLocatorClient))),
        m_orderExecutionClient(m_environment->GetOrderExecutionEnvironment().
          BuildClient(Beam::Ref(*m_serviceLocatorClient))),
        m_riskClient(m_environment->GetRiskEnvironment().BuildClient(
          Beam::Ref(*m_serviceLocatorClient))),
        m_timeClient(Beam::TimeService::MakeVirtualTimeClient(
          std::make_unique<Beam::TimeService::Tests::TestTimeClient>(
          Beam::Ref(m_environment->GetTimeEnvironment())))) {
    m_openState.SetOpen();
  }

  inline TestServiceClients::~TestServiceClients() {
    Close();
  }

  inline TestServiceClients::ServiceLocatorClient&
      TestServiceClients::GetServiceLocatorClient() {
    return *m_serviceLocatorClient;
  }

  inline TestServiceClients::RegistryClient&
      TestServiceClients::GetRegistryClient() {
    return *m_registryClient;
  }

  inline TestServiceClients::AdministrationClient&
      TestServiceClients::GetAdministrationClient() {
    return *m_administrationClient;
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
    return *m_chartingClient;
  }

  inline TestServiceClients::ComplianceClient&
      TestServiceClients::GetComplianceClient() {
    return *m_complianceClient;
  }

  inline TestServiceClients::OrderExecutionClient&
      TestServiceClients::GetOrderExecutionClient() {
    return *m_orderExecutionClient;
  }

  inline TestServiceClients::RiskClient& TestServiceClients::GetRiskClient() {
    return *m_riskClient;
  }

  inline TestServiceClients::TimeClient& TestServiceClients::GetTimeClient() {
    return *m_timeClient;
  }

  inline std::unique_ptr<TestServiceClients::Timer>
      TestServiceClients::BuildTimer(boost::posix_time::time_duration expiry) {
    return Beam::Threading::MakeVirtualTimer(
      std::make_unique<Beam::TimeService::Tests::TestTimer>(expiry,
      Beam::Ref(m_environment->GetTimeEnvironment())));
  }

  inline void TestServiceClients::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void TestServiceClients::Shutdown() {
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
