#ifndef NEXUS_TO_PYTHON_CLIENTS_HPP
#define NEXUS_TO_PYTHON_CLIENTS_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/ToPythonRegistryClient.hpp>
#include <Beam/Python/ToPythonServiceLocatorClient.hpp>
#include <Beam/Python/ToPythonTimeClient.hpp>
#include <Beam/Python/ToPythonTimer.hpp>
#include <Beam/RegistryService/RegistryClientBox.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Threading/TimerBox.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <Beam/Utilities/TypeList.hpp>
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

      /** The type of clients to wrap. */
      using Clients = C;

      /**
       * Constructs a ToPythonClients.
       * @param args The arguments to forward to the Clients constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonClients, Args...>>
      ToPythonClients(Args&&... args);

      ~ToPythonClients();

      /** Returns the wrapped clients. */
      const Clients& get_clients() const;

      /** Returns the wrapped clients. */
      Clients& get_clients();

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
      boost::optional<Clients> m_clients;
      boost::optional<ServiceLocatorClient> m_service_locator_client;
      boost::optional<RegistryClient> m_registry_client;
      boost::optional<AdministrationClient> m_administration_client;
      boost::optional<DefinitionsClient> m_definitions_client;
      boost::optional<MarketDataClient> m_market_data_client;
      boost::optional<ChartingClient> m_charting_client;
      boost::optional<ComplianceClient> m_compliance_client;
      boost::optional<OrderExecutionClient> m_order_execution_client;
      boost::optional<RiskClient> m_risk_client;
      boost::optional<TimeClient> m_time_client;
      Beam::IO::OpenState m_open_state;

      ToPythonClients(const ToPythonClients&) = delete;
      ToPythonClients& operator =(const ToPythonClients&) = delete;
  };

  template<typename Clients>
  ToPythonClients(Clients&&) ->
    ToPythonClients<std::remove_reference_t<Clients>>;

  template<IsClients C>
  template<typename... Args, typename>
  ToPythonClients<C>::ToPythonClients(Args&&... args)
    : m_clients((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...),
      m_service_locator_client(boost::in_place_init, std::in_place_type<
        Beam::ServiceLocator::ToPythonServiceLocatorClient<
          Beam::ServiceLocator::ServiceLocatorClientBox>>,
        &m_clients->get_service_locator_client()),
      m_registry_client(boost::in_place_init,
        std::in_place_type<Beam::RegistryService::ToPythonRegistryClient<
          Beam::RegistryService::RegistryClientBox>>,
        &m_clients->get_registry_client()),
      m_administration_client(boost::in_place_init, std::in_place_type<
        AdministrationService::ToPythonAdministrationClient<
          AdministrationService::AdministrationClient>>,
        &m_clients->get_administration_client()),
      m_definitions_client(boost::in_place_init,
        std::in_place_type<DefinitionsService::ToPythonDefinitionsClient<
          DefinitionsService::DefinitionsClient>>,
        &m_clients->get_definitions_client()),
      m_market_data_client(boost::in_place_init,
        std::in_place_type<MarketDataService::ToPythonMarketDataClient<
          MarketDataService::MarketDataClient>>,
        &m_clients->get_market_data_client()),
      m_charting_client(boost::in_place_init,
        std::in_place_type<ChartingService::ToPythonChartingClient<
          ChartingService::ChartingClient>>,
        &m_clients->get_charting_client()),
      m_compliance_client(boost::in_place_init,
        std::in_place_type<Compliance::ToPythonComplianceClient<
          Compliance::ComplianceClient>>,
        &m_clients->get_compliance_client()),
      m_order_execution_client(boost::in_place_init, std::in_place_type<
        OrderExecutionService::ToPythonOrderExecutionClient<
          OrderExecutionService::OrderExecutionClient>>,
        &m_clients->get_order_execution_client()),
      m_risk_client(boost::in_place_init,
        std::in_place_type<RiskService::ToPythonRiskClient<
          RiskService::RiskClient>>, &m_clients->get_risk_client()),
      m_time_client(
        Beam::TimeService::ToPythonTimeClient<Beam::TimeService::TimeClientBox>(
          &m_clients->get_time_client())) {}

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
    m_registry_client.reset();
    m_service_locator_client.reset();
    m_clients.reset();
    m_open_state.Close();
  }

  template<IsClients C>
  const typename ToPythonClients<C>::Clients&
      ToPythonClients<C>::get_clients() const {
    return *m_clients;
  }

  template<IsClients C>
  typename ToPythonClients<C>::Clients& ToPythonClients<C>::get_clients() {
    return *m_clients;
  }

  template<IsClients C>
  typename ToPythonClients<C>::ServiceLocatorClient&
      ToPythonClients<C>::get_service_locator_client() {
    return *m_service_locator_client;
  }

  template<IsClients C>
  typename ToPythonClients<C>::RegistryClient&
      ToPythonClients<C>::get_registry_client() {
    return *m_registry_client;
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
    return std::make_unique<Timer>(std::in_place_type<
      Beam::Threading::ToPythonTimer<Beam::Threading::TimerBox>>,
      Beam::Threading::TimerBox(m_clients->make_timer(expiry)));
  }

  template<IsClients C>
  void ToPythonClients<C>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_time_client->Close();
    m_service_locator_client->Close();
    m_risk_client->close();
    m_order_execution_client->close();
    m_compliance_client->close();
    m_charting_client->close();
    m_market_data_client->close();
    m_definitions_client->close();
    m_administration_client->close();
    m_registry_client->Close();
    m_clients->close();
    m_open_state.Close();
  }
}

#endif
