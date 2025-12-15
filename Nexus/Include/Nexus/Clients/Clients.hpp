#ifndef NEXUS_CLIENTS_HPP
#define NEXUS_CLIENTS_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/IO/Connection.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/ChartingService/ChartingClient.hpp"
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/RiskService/RiskClient.hpp"

namespace Nexus {

  /** Checks if a type implements the Clients interface. */
  template<typename T>
  concept IsClients = Beam::IsConnection<T> && requires(T& clients) {
    { clients.get_service_locator_client() } ->
      std::convertible_to<Beam::ServiceLocatorClient>;
    { clients.get_administration_client() } ->
      std::convertible_to<AdministrationClient>;
    { clients.get_definitions_client() } ->
      std::convertible_to<DefinitionsClient>;
    { clients.get_market_data_client() } ->
      std::convertible_to<MarketDataClient>;
    { clients.get_charting_client() } -> std::convertible_to<ChartingClient>;
    { clients.get_compliance_client() } ->
      std::convertible_to<ComplianceClient>;
    { clients.get_order_execution_client() } ->
      std::convertible_to<OrderExecutionClient>;
    { clients.get_risk_client() } -> std::convertible_to<RiskClient>;
    { clients.get_time_client() } -> std::convertible_to<Beam::TimeClient>;
    { clients.make_timer(std::declval<boost::posix_time::time_duration>()) } ->
      std::convertible_to<Beam::Timer>;
    clients.close();
  };

  /** Consolidates the clients used for Nexus services. */
  class Clients {
    public:

      /** The service locator client interface. */
      using ServiceLocatorClient = Beam::ServiceLocatorClient;

      /** The administration client interface. */
      using AdministrationClient = Nexus::AdministrationClient;

      /** The definitions client interface. */
      using DefinitionsClient = Nexus::DefinitionsClient;

      /** The market data client interface. */
      using MarketDataClient = Nexus::MarketDataClient;

      /** The charting client interface. */
      using ChartingClient = Nexus::ChartingClient;

      /** The compliance client interface. */
      using ComplianceClient = Nexus::ComplianceClient;

      /** The order execution client interface. */
      using OrderExecutionClient = Nexus::OrderExecutionClient;

      /** The risk client interface. */
      using RiskClient = Nexus::RiskClient;

      /** The time client interface. */
      using TimeClient = Beam::TimeClient;

      /** The timer interface. */
      using Timer = Beam::Timer;

      /**
       * Constructs Clients of a specified type using emplacement.
       * @tparam T The type of clients to emplace.
       * @param args The arguments to pass to the emplaced clients.
       */
      template<IsClients T, typename... Args>
      explicit Clients(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs Clients by referencing existing clients.
       * @param clients The clients to reference.
       */
      template<Beam::DisableCopy<Clients> C> requires
        IsClients<Beam::dereference_t<C>>
      Clients(C&& clients);

      Clients(const Clients&) = default;
      Clients(Clients&&) = default;

      /** Returns the service locator client. */
      ServiceLocatorClient& get_service_locator_client();

      /** Returns the administration client. */
      AdministrationClient& get_administration_client();

      /** Returns the definitions client. */
      DefinitionsClient& get_definitions_client();

      /** Returns the market data client. */
      MarketDataClient& get_market_data_client();

      /** Returns the charting client. */
      ChartingClient& get_charting_client();

      /** Returns the compliance client. */
      ComplianceClient& get_compliance_client();

      /** Returns the order execution client. */
      OrderExecutionClient& get_order_execution_client();

      /** Returns the risk client. */
      RiskClient& get_risk_client();

      /** Returns the time client. */
      TimeClient& get_time_client();

      /** Creates a timer with the specified expiry. */
      std::unique_ptr<Timer> make_timer(
        boost::posix_time::time_duration expiry);

      /** Closes all clients. */
      void close();

    private:
      struct VirtualClients {
        virtual ~VirtualClients() = default;

        virtual ServiceLocatorClient& get_service_locator_client() = 0;
        virtual AdministrationClient& get_administration_client() = 0;
        virtual DefinitionsClient& get_definitions_client() = 0;
        virtual MarketDataClient& get_market_data_client() = 0;
        virtual ChartingClient& get_charting_client() = 0;
        virtual ComplianceClient& get_compliance_client() = 0;
        virtual OrderExecutionClient& get_order_execution_client() = 0;
        virtual RiskClient& get_risk_client() = 0;
        virtual TimeClient& get_time_client() = 0;
        virtual std::unique_ptr<Timer> make_timer(
          boost::posix_time::time_duration expiry) = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedClients final : VirtualClients {
        using Clients = C;
        Beam::local_ptr_t<Clients> m_clients;
        ServiceLocatorClient m_service_locator_client;
        AdministrationClient m_administration_client;
        DefinitionsClient m_definitions_client;
        MarketDataClient m_market_data_client;
        ChartingClient m_charting_client;
        ComplianceClient m_compliance_client;
        OrderExecutionClient m_order_execution_client;
        RiskClient m_risk_client;
        TimeClient m_time_client;

        template<typename... Args>
        WrappedClients(Args&&... args);

        ServiceLocatorClient& get_service_locator_client() override;
        AdministrationClient& get_administration_client() override;
        DefinitionsClient& get_definitions_client() override;
        MarketDataClient& get_market_data_client() override;
        ChartingClient& get_charting_client() override;
        ComplianceClient& get_compliance_client() override;
        OrderExecutionClient& get_order_execution_client() override;
        RiskClient& get_risk_client() override;
        TimeClient& get_time_client() override;
        std::unique_ptr<Timer> make_timer(
          boost::posix_time::time_duration expiry) override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualClients> m_clients;
  };

  template<IsClients T, typename... Args>
  Clients::Clients(std::in_place_type_t<T>, Args&&... args)
    : m_clients(Beam::make_virtual_ptr<WrappedClients<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<Clients> C> requires
    IsClients<Beam::dereference_t<C>>
  Clients::Clients(C&& clients)
    : m_clients(Beam::make_virtual_ptr<WrappedClients<std::remove_cvref_t<C>>>(
        std::forward<C>(clients))) {}

  inline Clients::ServiceLocatorClient& Clients::get_service_locator_client() {
    return m_clients->get_service_locator_client();
  }

  inline Clients::AdministrationClient& Clients::get_administration_client() {
    return m_clients->get_administration_client();
  }

  inline Clients::DefinitionsClient& Clients::get_definitions_client() {
    return m_clients->get_definitions_client();
  }

  inline Clients::MarketDataClient& Clients::get_market_data_client() {
    return m_clients->get_market_data_client();
  }

  inline Clients::ChartingClient& Clients::get_charting_client() {
    return m_clients->get_charting_client();
  }

  inline Clients::ComplianceClient& Clients::get_compliance_client() {
    return m_clients->get_compliance_client();
  }

  inline Clients::OrderExecutionClient& Clients::get_order_execution_client() {
    return m_clients->get_order_execution_client();
  }

  inline Clients::RiskClient& Clients::get_risk_client() {
    return m_clients->get_risk_client();
  }

  inline Clients::TimeClient& Clients::get_time_client() {
    return m_clients->get_time_client();
  }

  inline std::unique_ptr<Clients::Timer> Clients::make_timer(
      boost::posix_time::time_duration expiry) {
    return m_clients->make_timer(expiry);
  }

  inline void Clients::close() {
    m_clients->close();
  }

  template<typename C>
  template<typename... Args>
  Clients::WrappedClients<C>::WrappedClients(Args&&... args)
    : m_clients(std::forward<Args>(args)...),
      m_service_locator_client(&m_clients->get_service_locator_client()),
      m_administration_client(&m_clients->get_administration_client()),
      m_definitions_client(&m_clients->get_definitions_client()),
      m_market_data_client(&m_clients->get_market_data_client()),
      m_charting_client(&m_clients->get_charting_client()),
      m_compliance_client(&m_clients->get_compliance_client()),
      m_order_execution_client(&m_clients->get_order_execution_client()),
      m_risk_client(&m_clients->get_risk_client()),
      m_time_client(&m_clients->get_time_client()) {}

  template<typename C>
  Clients::ServiceLocatorClient&
      Clients::WrappedClients<C>::get_service_locator_client() {
    return m_service_locator_client;
  }

  template<typename C>
  Clients::AdministrationClient&
      Clients::WrappedClients<C>::get_administration_client() {
    return m_administration_client;
  }

  template<typename C>
  Clients::DefinitionsClient&
      Clients::WrappedClients<C>::get_definitions_client() {
    return m_definitions_client;
  }

  template<typename C>
  Clients::MarketDataClient&
      Clients::WrappedClients<C>::get_market_data_client() {
    return m_market_data_client;
  }

  template<typename C>
  Clients::ChartingClient& Clients::WrappedClients<C>::get_charting_client() {
    return m_charting_client;
  }

  template<typename C>
  Clients::ComplianceClient&
      Clients::WrappedClients<C>::get_compliance_client() {
    return m_compliance_client;
  }

  template<typename C>
  Clients::OrderExecutionClient&
      Clients::WrappedClients<C>::get_order_execution_client() {
    return m_order_execution_client;
  }

  template<typename C>
  Clients::RiskClient& Clients::WrappedClients<C>::get_risk_client() {
    return m_risk_client;
  }

  template<typename C>
  Clients::TimeClient& Clients::WrappedClients<C>::get_time_client() {
    return m_time_client;
  }

  template<typename C>
  std::unique_ptr<Clients::Timer> Clients::WrappedClients<C>::make_timer(
      boost::posix_time::time_duration expiry) {
    return std::make_unique<Timer>(m_clients->make_timer(expiry));
  }

  template<typename C>
  void Clients::WrappedClients<C>::close() {
    m_clients->close();
  }
}

#endif
