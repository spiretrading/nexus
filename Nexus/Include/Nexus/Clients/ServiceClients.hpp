#ifndef NEXUS_SERVICE_CLIENTS_HPP
#define NEXUS_SERVICE_CLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/ChartingService/ApplicationDefinitions.hpp"
#include "Nexus/Clients/Clients.hpp"
#include "Nexus/Compliance/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"
#include "Nexus/RiskService/ApplicationDefinitions.hpp"

namespace Nexus {

  /** Implements the Clients interface using the respective service client. */
  class ServiceClients {
    public:
      using ServiceLocatorClient = Beam::ApplicationServiceLocatorClient;
      using AdministrationClient = ApplicationAdministrationClient;
      using DefinitionsClient = ApplicationDefinitionsClient;
      using MarketDataClient = ApplicationMarketDataClient;
      using ChartingClient = ApplicationChartingClient;
      using ComplianceClient = ApplicationComplianceClient;
      using OrderExecutionClient = ApplicationOrderExecutionClient;
      using RiskClient = ApplicationRiskClient;
      using TimeClient = Beam::LiveNtpTimeClient;
      using Timer = Beam::LiveTimer;

      /**
       * Constructs a ServiceClients.
       * @param username The client's username.
       * @param password The client's password.
       * @param address The IpAddress to connect to.
       */
      ServiceClients(const std::string& username, const std::string& password,
        const Beam::IpAddress& address);

      ~ServiceClients();

      ServiceLocatorClient& get_service_locator_client();
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
      Beam::ApplicationServiceLocatorClient m_service_locator_client;
      ApplicationAdministrationClient m_administration_client;
      ApplicationDefinitionsClient m_definitions_client;
      ApplicationMarketDataClient m_market_data_client;
      ApplicationChartingClient m_charting_client;
      ApplicationComplianceClient m_compliance_client;
      ApplicationOrderExecutionClient m_order_execution_client;
      ApplicationRiskClient m_risk_client;
      std::unique_ptr<TimeClient> m_time_client;
      Beam::OpenState m_open_state;

      ServiceClients(const ServiceClients&) = delete;
      ServiceClients& operator =(const ServiceClients&) = delete;
  };

  inline ServiceClients::ServiceClients(const std::string& username,
      const std::string& password, const Beam::IpAddress& address)
    : m_service_locator_client(username, password, address),
      m_administration_client(Beam::Ref(m_service_locator_client)),
      m_definitions_client(Beam::Ref(m_service_locator_client)),
      m_market_data_client(Beam::Ref(m_service_locator_client)),
      m_charting_client(Beam::Ref(m_service_locator_client)),
      m_compliance_client(Beam::Ref(m_service_locator_client)),
      m_order_execution_client(Beam::Ref(m_service_locator_client)),
      m_risk_client(Beam::Ref(m_service_locator_client)),
      m_time_client(
        Beam::make_live_ntp_time_client(m_service_locator_client)) {}

  inline ServiceClients::~ServiceClients() {
    close();
  }

  inline ServiceClients::ServiceLocatorClient&
      ServiceClients::get_service_locator_client() {
    return m_service_locator_client;
  }

  inline ServiceClients::AdministrationClient&
      ServiceClients::get_administration_client() {
    return m_administration_client;
  }

  inline ServiceClients::DefinitionsClient&
      ServiceClients::get_definitions_client() {
    return m_definitions_client;
  }

  inline ServiceClients::MarketDataClient&
      ServiceClients::get_market_data_client() {
    return m_market_data_client;
  }

  inline ServiceClients::ChartingClient& ServiceClients::get_charting_client() {
    return m_charting_client;
  }

  inline ServiceClients::ComplianceClient&
      ServiceClients::get_compliance_client() {
    return m_compliance_client;
  }

  inline ServiceClients::OrderExecutionClient&
      ServiceClients::get_order_execution_client() {
    return m_order_execution_client;
  }

  inline ServiceClients::RiskClient& ServiceClients::get_risk_client() {
    return m_risk_client;
  }

  inline ServiceClients::TimeClient& ServiceClients::get_time_client() {
    return *m_time_client;
  }

  inline std::unique_ptr<ServiceClients::Timer>
      ServiceClients::make_timer(boost::posix_time::time_duration expiry) {
    return std::make_unique<Timer>(expiry);
  }

  inline void ServiceClients::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_time_client->close();
    m_service_locator_client.close();
    m_risk_client.close();
    m_order_execution_client.close();
    m_compliance_client.close();
    m_charting_client.close();
    m_market_data_client.close();
    m_definitions_client.close();
    m_administration_client.close();
    m_open_state.close();
  }
}

#endif
