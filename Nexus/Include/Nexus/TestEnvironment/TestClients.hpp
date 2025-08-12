#ifndef NEXUS_TEST_CLIENTS_HPP
#define NEXUS_TEST_CLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/RegistryService/RegistryClientBox.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Threading/TimerBox.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <Beam/TimeServiceTests/TestTimeClient.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/ChartingService/ChartingClient.hpp"
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/RiskService/RiskClient.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"

namespace Nexus {

  /** Implements the Clients interface for testing. */
  class TestClients {
    public:
      using ServiceLocatorClient =
        Beam::ServiceLocator::ServiceLocatorClientBox;

      using RegistryClient = Beam::RegistryService::RegistryClientBox;

      using AdministrationClient = AdministrationService::AdministrationClient;

      using DefinitionsClient = DefinitionsService::DefinitionsClient;

      using MarketDataClient = MarketDataService::MarketDataClient;

      using ChartingClient = ChartingService::ChartingClient;

      using ComplianceClient = Compliance::ComplianceClient;

      using OrderExecutionClient = OrderExecutionService::OrderExecutionClient;

      using RiskClient = RiskService::RiskClient;

      using TimeClient = Beam::TimeService::TimeClientBox;

      using Timer = Beam::Threading::TimerBox;

      /**
       * Constructs TestClients.
       * @param environment The TestEnvironment to use.
       */
      explicit TestClients(Beam::Ref<TestEnvironment> environment);

      /**
       * Constructs TestClients.
       * @param username The username to login with.
       * @param password The password to use.
       * @param environment The TestEnvironment to use.
       */
      TestClients(std::string username, std::string password,
        Beam::Ref<TestEnvironment> environment);

      ~TestClients();
      ServiceLocatorClient& get_service_locator_client();
      RegistryClient& get_registry_client();
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
      TestEnvironment* m_environment;
      ServiceLocatorClient m_service_locator_client;
      RegistryClient m_registry_client;
      DefinitionsClient m_definitions_client;
      AdministrationClient m_administration_client;
      MarketDataClient m_market_data_client;
      ChartingClient m_charting_client;
      ComplianceClient m_compliance_client;
      OrderExecutionClient m_order_execution_client;
      RiskClient m_risk_client;
      TimeClient m_time_client;
      Beam::IO::OpenState m_open_state;

      TestClients(const TestClients&) = delete;
      TestClients& operator =(const TestClients&) = delete;
  };

  inline TestClients::TestClients(Beam::Ref<TestEnvironment> environment)
    : TestClients("root", "", environment) {}

  inline TestClients::TestClients(std::string username, std::string password,
      Beam::Ref<TestEnvironment> environment)
    : m_environment(environment.Get()),
      m_service_locator_client(
        m_environment->get_service_locator_environment().MakeClient(
          std::move(username), std::move(password))),
      m_registry_client(m_environment->get_registry_environment().MakeClient(
        m_service_locator_client)),
      m_definitions_client(
        m_environment->get_definitions_environment().make_client(
          m_service_locator_client)),
      m_administration_client(
        m_environment->get_administration_environment().make_client(
          m_service_locator_client)),
      m_market_data_client(
        m_environment->get_market_data_environment().make_registry_client(
          m_service_locator_client)),
      m_charting_client(m_environment->get_charting_environment().make_client(
        m_service_locator_client)),
      m_compliance_client(
        m_environment->get_compliance_environment().make_client(
          m_service_locator_client)),
      m_order_execution_client(
        m_environment->get_order_execution_environment().make_client(
          m_service_locator_client)),
      m_risk_client(m_environment->get_risk_environment().make_client(
        m_service_locator_client)),
      m_time_client(std::make_unique<Beam::TimeService::Tests::TestTimeClient>(
        Beam::Ref(m_environment->get_time_environment()))) {}

  inline TestClients::~TestClients() {
    close();
  }

  inline TestClients::~TestClients() {
    close();
  }

  inline TestClients::ServiceLocatorClient&
      TestClients::get_service_locator_client() {
    return m_service_locator_client;
  }

  inline TestClients::RegistryClient& TestClients::get_registry_client() {
    return m_registry_client;
  }

  inline TestClients::AdministrationClient&
      TestClients::get_administration_client() {
    return m_administration_client;
  }

  inline TestClients::DefinitionsClient& TestClients::get_definitions_client() {
    return m_definitions_client;
  }

  inline TestClients::MarketDataClient& TestClients::get_market_data_client() {
    return m_market_data_client;
  }

  inline TestClients::ChartingClient& TestClients::get_charting_client() {
    return m_charting_client;
  }

  inline TestClients::ComplianceClient& TestClients::get_compliance_client() {
    return m_compliance_client;
  }

  inline TestClients::OrderExecutionClient&
      TestClients::get_order_execution_client() {
    return m_order_execution_client;
  }

  inline TestClients::RiskClient& TestClients::get_risk_client() {
    return m_risk_client;
  }

  inline TestClients::TimeClient& TestClients::get_time_client() {
    return m_time_client;
  }

  inline std::unique_ptr<TestClients::Timer>
      TestClients::make_timer(boost::posix_time::time_duration expiry) {
    return std::make_unique<Timer>(
      std::make_unique<Beam::TimeService::Tests::TestTimer>(
        expiry, Beam::Ref(m_environment->get_time_environment())));
  }

  inline void TestClients::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_time_client.Close();
    m_service_locator_client.Close();
    m_risk_client.close();
    m_order_execution_client.close();
    m_compliance_client.close();
    m_charting_client.close();
    m_market_data_client.close();
    m_administration_client.close();
    m_definitions_client.close();
    m_registry_client.Close();
    m_open_state.Close();
  }
}

#endif
