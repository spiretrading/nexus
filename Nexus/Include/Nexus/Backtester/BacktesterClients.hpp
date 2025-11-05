#ifndef NEXUS_BACKTESTER_CLIENTS_HPP
#define NEXUS_BACKTESTER_CLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/TimeServiceTests/TestTimer.hpp>
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterMarketDataClient.hpp"
#include "Nexus/Backtester/BacktesterTimeClient.hpp"
#include "Nexus/Backtester/BacktesterTimer.hpp"

namespace Nexus {

  /** Implements the Clients interface for the purpose of backtesting. */
  class BacktesterClients {
    public:
      using ServiceLocatorClient = Beam::ServiceLocatorClient;
      using AdministrationClient = Nexus::AdministrationClient;
      using DefinitionsClient = Nexus::DefinitionsClient;
      using MarketDataClient = Nexus::MarketDataClient;
      using ChartingClient = Nexus::ChartingClient;
      using ComplianceClient = Nexus::ComplianceClient;
      using OrderExecutionClient = Nexus::OrderExecutionClient;
      using RiskClient = Nexus::RiskClient;
      using TimeClient = Beam::TimeClient;
      using Timer = Beam::Timer;

      /**
       * Constructs BacktesterClients.
       * @param environment The BacktesterEnvironment to use.
       */
      explicit BacktesterClients(Beam::Ref<BacktesterEnvironment> environment);

      ~BacktesterClients();
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
      BacktesterEnvironment* m_environment;
      ServiceLocatorClient m_service_locator_client;
      DefinitionsClient m_definitions_client;
      AdministrationClient m_administration_client;
      MarketDataClient m_market_data_client;
      ChartingClient m_charting_client;
      ComplianceClient m_compliance_client;
      OrderExecutionClient m_order_execution_client;
      RiskClient m_risk_client;
      TimeClient m_time_client;
      Beam::OpenState m_open_state;

      BacktesterClients(const BacktesterClients&) = delete;
      BacktesterClients& operator =(const BacktesterClients&) = delete;
  };

  inline BacktesterClients::BacktesterClients(
    Beam::Ref<BacktesterEnvironment> environment)
    : m_environment(environment.get()),
      m_service_locator_client(
        m_environment->get_service_locator_environment().make_client()),
      m_definitions_client(
        m_environment->get_definitions_environment().make_client(
          Beam::Ref(m_service_locator_client))),
      m_administration_client(
        m_environment->get_administration_environment().make_client(
          Beam::Ref(m_service_locator_client))),
      m_market_data_client(std::in_place_type<BacktesterMarketDataClient>,
        Beam::Ref(m_environment->get_market_data_service()),
        m_environment->get_market_data_environment().make_registry_client(
          Beam::Ref(m_service_locator_client))),
      m_charting_client(m_environment->get_charting_environment().make_client(
        Beam::Ref(m_service_locator_client))),
      m_compliance_client(
        m_environment->get_compliance_environment().make_client(
          Beam::Ref(m_service_locator_client))),
      m_order_execution_client(
        m_environment->get_order_execution_environment().make_client(
          Beam::Ref(m_service_locator_client))),
      m_risk_client(m_environment->get_risk_environment().make_client(
        Beam::Ref(m_service_locator_client))),
      m_time_client(std::in_place_type<BacktesterTimeClient>,
        Beam::Ref(m_environment->get_event_handler())) {}

  inline BacktesterClients::~BacktesterClients() {
    close();
  }

  inline BacktesterClients::ServiceLocatorClient&
      BacktesterClients::get_service_locator_client() {
    return m_service_locator_client;
  }

  inline BacktesterClients::AdministrationClient&
      BacktesterClients::get_administration_client() {
    return m_administration_client;
  }

  inline BacktesterClients::DefinitionsClient&
      BacktesterClients::get_definitions_client() {
    return m_definitions_client;
  }

  inline BacktesterClients::MarketDataClient&
      BacktesterClients::get_market_data_client() {
    return m_market_data_client;
  }

  inline BacktesterClients::ChartingClient&
      BacktesterClients::get_charting_client() {
    return m_charting_client;
  }

  inline BacktesterClients::ComplianceClient&
      BacktesterClients::get_compliance_client() {
    return m_compliance_client;
  }

  inline BacktesterClients::OrderExecutionClient&
      BacktesterClients::get_order_execution_client() {
    return m_order_execution_client;
  }

  inline BacktesterClients::RiskClient&
      BacktesterClients::get_risk_client() {
    return m_risk_client;
  }

  inline BacktesterClients::TimeClient&
      BacktesterClients::get_time_client() {
    return m_time_client;
  }

  inline std::unique_ptr<BacktesterClients::Timer>
      BacktesterClients::make_timer(boost::posix_time::time_duration expiry) {
    return std::make_unique<Timer>(std::in_place_type<BacktesterTimer>, expiry,
      Beam::Ref(m_environment->get_event_handler()));
  }

  inline void BacktesterClients::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_time_client.close();
    m_risk_client.close();
    m_order_execution_client.close();
    m_compliance_client.close();
    m_charting_client.close();
    m_market_data_client.close();
    m_administration_client.close();
    m_definitions_client.close();
    m_service_locator_client.close();
    m_open_state.close();
  }
}

#endif
