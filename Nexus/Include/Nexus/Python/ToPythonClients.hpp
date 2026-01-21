#ifndef NEXUS_TO_PYTHON_CLIENTS_HPP
#define NEXUS_TO_PYTHON_CLIENTS_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Python/ToPythonServiceLocatorClient.hpp>
#include <Beam/Python/ToPythonTimeClient.hpp>
#include <Beam/Python/ToPythonTimer.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Clients/Clients.hpp"
#include "Nexus/Python/ToPythonAdministrationClient.hpp"
#include "Nexus/Python/ToPythonChartingClient.hpp"
#include "Nexus/Python/ToPythonComplianceClient.hpp"
#include "Nexus/Python/ToPythonDefinitionsClient.hpp"
#include "Nexus/Python/ToPythonMarketDataClient.hpp"
#include "Nexus/Python/ToPythonOrderExecutionClient.hpp"
#include "Nexus/Python/ToPythonRiskClient.hpp"

namespace Nexus {

  /**
   * Wraps an ChartingClient for use with Python.
   * @param <C> The type of ChartingClient to wrap.
   */
  template<IsClients C>
  class ToPythonClients {
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

      /** The type of clients to wrap. */
      using Clients = C;

      /**
       * Constructs a ToPythonClients.
       * @param args The arguments to forward to the Clients constructor.
       */
      template<typename... Args>
      ToPythonClients(Args&&... args);

      ~ToPythonClients();

      /** Returns a reference to the underlying clients. */
      Clients& get();

      /** Returns a reference to the underlying clients. */
      const Clients& get() const;

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
      boost::optional<Clients> m_clients;
      boost::optional<ServiceLocatorClient> m_service_locator_client;
      boost::optional<AdministrationClient> m_administration_client;
      boost::optional<DefinitionsClient> m_definitions_client;
      boost::optional<MarketDataClient> m_market_data_client;
      boost::optional<ChartingClient> m_charting_client;
      boost::optional<ComplianceClient> m_compliance_client;
      boost::optional<OrderExecutionClient> m_order_execution_client;
      boost::optional<RiskClient> m_risk_client;
      boost::optional<TimeClient> m_time_client;
      Beam::OpenState m_open_state;

      ToPythonClients(const ToPythonClients&) = delete;
      ToPythonClients& operator =(const ToPythonClients&) = delete;
  };

  template<typename Clients>
  ToPythonClients(Clients&&) -> ToPythonClients<std::remove_cvref_t<Clients>>;

  template<IsClients C>
  template<typename... Args>
  ToPythonClients<C>::ToPythonClients(Args&&... args)
    : m_clients((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...),
      m_service_locator_client(boost::in_place_init, std::in_place_type<
        Beam::Python::ToPythonServiceLocatorClient<ServiceLocatorClient>>,
        &m_clients->get_service_locator_client()),
      m_administration_client(boost::in_place_init,
        std::in_place_type<ToPythonAdministrationClient<AdministrationClient>>,
        &m_clients->get_administration_client()),
      m_definitions_client(boost::in_place_init,
        std::in_place_type<ToPythonDefinitionsClient<DefinitionsClient>>,
        &m_clients->get_definitions_client()),
      m_market_data_client(boost::in_place_init,
        std::in_place_type<ToPythonMarketDataClient<MarketDataClient>>,
        &m_clients->get_market_data_client()),
      m_charting_client(boost::in_place_init,
        std::in_place_type<ToPythonChartingClient<ChartingClient>>,
        &m_clients->get_charting_client()),
      m_compliance_client(boost::in_place_init,
        std::in_place_type<ToPythonComplianceClient<ComplianceClient>>,
        &m_clients->get_compliance_client()),
      m_order_execution_client(boost::in_place_init,
        std::in_place_type<ToPythonOrderExecutionClient<OrderExecutionClient>>,
        &m_clients->get_order_execution_client()),
      m_risk_client(boost::in_place_init,
        std::in_place_type<ToPythonRiskClient<RiskClient>>,
        &m_clients->get_risk_client()),
      m_time_client(boost::in_place_init,
        std::in_place_type<Beam::Python::ToPythonTimeClient<TimeClient>>,
        &m_clients->get_time_client()) {}

  template<IsClients C>
  ToPythonClients<C>::~ToPythonClients() {
    auto release = Beam::Python::GilRelease();
    m_time_client.reset();
    m_risk_client.reset();
    m_order_execution_client.reset();
    m_compliance_client.reset();
    m_charting_client.reset();
    m_market_data_client.reset();
    m_definitions_client.reset();
    m_administration_client.reset();
    m_service_locator_client.reset();
    m_clients.reset();
    m_open_state.close();
  }

  template<IsClients C>
  typename ToPythonClients<C>::ServiceLocatorClient&
      ToPythonClients<C>::get_service_locator_client() {
    return *m_service_locator_client;
  }

  template<IsClients C>
  typename ToPythonClients<C>::AdministrationClient&
      ToPythonClients<C>::get_administration_client() {
    return *m_administration_client;
  }

  template<IsClients C>
  typename ToPythonClients<C>::DefinitionsClient&
      ToPythonClients<C>::get_definitions_client() {
    return *m_definitions_client;
  }

  template<IsClients C>
  typename ToPythonClients<C>::MarketDataClient&
      ToPythonClients<C>::get_market_data_client() {
    return *m_market_data_client;
  }

  template<IsClients C>
  typename ToPythonClients<C>::ChartingClient&
      ToPythonClients<C>::get_charting_client() {
    return *m_charting_client;
  }

  template<IsClients C>
  typename ToPythonClients<C>::ComplianceClient&
      ToPythonClients<C>::get_compliance_client() {
    return *m_compliance_client;
  }

  template<IsClients C>
  typename ToPythonClients<C>::OrderExecutionClient&
      ToPythonClients<C>::get_order_execution_client() {
    return *m_order_execution_client;
  }

  template<IsClients C>
  typename ToPythonClients<C>::RiskClient&
      ToPythonClients<C>::get_risk_client() {
    return *m_risk_client;
  }

  template<IsClients C>
  typename ToPythonClients<C>::TimeClient&
      ToPythonClients<C>::get_time_client() {
    return *m_time_client;
  }

  template<IsClients C>
  std::unique_ptr<typename ToPythonClients<C>::Timer>
      ToPythonClients<C>::make_timer(boost::posix_time::time_duration expiry) {
    auto release = Beam::Python::GilRelease();
    if constexpr(std::is_same_v<
        decltype(m_clients->make_timer(expiry)), std::unique_ptr<Timer>>) {
      return m_clients->make_timer(expiry);
    } else {
      return std::make_unique<Timer>(m_clients->make_timer(expiry));
    }
  }

  template<IsClients C>
  void ToPythonClients<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_time_client->close();
    m_service_locator_client->close();
    m_risk_client->close();
    m_order_execution_client->close();
    m_compliance_client->close();
    m_charting_client->close();
    m_market_data_client->close();
    m_definitions_client->close();
    m_administration_client->close();
    m_clients->close();
    m_open_state.close();
  }
}

#endif
