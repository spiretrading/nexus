#ifndef NEXUS_PYTHON_SERVICE_CLIENTS_HPP
#define NEXUS_PYTHON_SERVICE_CLIENTS_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/ToPythonRegistryClient.hpp>
#include <Beam/Python/ToPythonServiceLocatorClient.hpp>
#include <Beam/Python/ToPythonTimeClient.hpp>
#include <Beam/Python/ToPythonTimer.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/Python/DllExport.hpp"
#include "Nexus/Python/ToPythonAdministrationClient.hpp"
#include "Nexus/Python/ToPythonChartingClient.hpp"
#include "Nexus/Python/ToPythonComplianceClient.hpp"
#include "Nexus/Python/ToPythonDefinitionsClient.hpp"
#include "Nexus/Python/ToPythonMarketDataClient.hpp"
#include "Nexus/Python/ToPythonOrderExecutionClient.hpp"
#include "Nexus/Python/ToPythonRiskClient.hpp"
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"

namespace Nexus {

  /**
   * Wraps a ServiceClients instance for use within Python.
   * @param <C> The type of ServiceClients to wrap.
   */
  template<typename C>
  class ToPythonServiceClients {
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

      /** The type of ServiceClients to wrap. */
      using Clients = C;

      /**
       * Constructs a ToPythonServiceClients.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonServiceClients, Args...>>
      ToPythonServiceClients(Args&&... args);

      ~ToPythonServiceClients();

      /** Returns the wrapped clients. */
      const Clients& GetClients() const;

      /** Returns the wrapped clients. */
      Clients& GetClients();

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
      boost::optional<Clients> m_clients;
      boost::optional<ServiceLocatorClient> m_serviceLocatorClient;
      boost::optional<RegistryClient> m_registryClient;
      boost::optional<AdministrationClient> m_administrationClient;
      boost::optional<DefinitionsClient> m_definitionsClient;
      boost::optional<MarketDataClient> m_marketDataClient;
      boost::optional<ChartingClient> m_chartingClient;
      boost::optional<ComplianceClient> m_complianceClient;
      boost::optional<OrderExecutionClient> m_orderExecutionClient;
      boost::optional<RiskClient> m_riskClient;
      boost::optional<TimeClient> m_timeClient;
      Beam::IO::OpenState m_openState;

      ToPythonServiceClients(const ToPythonServiceClients&) = delete;
      ToPythonServiceClients& operator =(
        const ToPythonServiceClients&) = delete;
  };

namespace Python {

  /** Returns the exported ServiceClientsBox. */
  NEXUS_EXPORT_DLL pybind11::class_<ServiceClientsBox>&
    GetExportedServiceClientsBox();

  /**
   * Exports the ApplicationServiceClients class.
   * @param module The module to export to.
   */
  void ExportApplicationServiceClients(pybind11::module& module);

  /**
   * Exports the ServiceClients classes.
   * @param module The module to export to.
   */
  void ExportServiceClients(pybind11::module& module);

  /**
   * Exports the TestEnvironment class.
   * @param module The module to export to.
   */
  void ExportTestEnvironment(pybind11::module& module);

  /**
   * Exports the TestServiceClients class.
   * @param module The module to export to.
   */
  void ExportTestServiceClients(pybind11::module& module);

  /**
   * Exports a ServiceClients class.
   * @param <Clients> The type of ServiceClients to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported ServiceClients.
   */
  template<typename Clients>
  auto ExportServiceClients(pybind11::module& module,
      const std::string& name) {
    auto clients = pybind11::class_<Clients, std::shared_ptr<Clients>>(module,
      name.c_str()).
      def("get_service_locator_client", &Clients::GetServiceLocatorClient,
        pybind11::return_value_policy::reference_internal).
      def("get_registry_client", &Clients::GetRegistryClient,
        pybind11::return_value_policy::reference_internal).
      def("get_administration_client", &Clients::GetAdministrationClient,
        pybind11::return_value_policy::reference_internal).
      def("get_definitions_client", &Clients::GetDefinitionsClient,
        pybind11::return_value_policy::reference_internal).
      def("get_market_data_client", &Clients::GetMarketDataClient,
        pybind11::return_value_policy::reference_internal).
      def("get_charting_client", &Clients::GetChartingClient,
        pybind11::return_value_policy::reference_internal).
      def("get_compliance_client", &Clients::GetComplianceClient,
        pybind11::return_value_policy::reference_internal).
      def("get_order_execution_client", &Clients::GetOrderExecutionClient,
        pybind11::return_value_policy::reference_internal).
      def("get_risk_client", &Clients::GetRiskClient,
        pybind11::return_value_policy::reference_internal).
      def("get_time_client", &Clients::GetTimeClient,
        pybind11::return_value_policy::reference_internal).
      def("make_timer",
        [] (Clients& serviceClients, boost::posix_time::time_duration expiry) {
          return std::shared_ptr(serviceClients.MakeTimer(expiry));
        }).
      def("close", &Clients::Close);
    if constexpr(!std::is_same_v<Clients, ServiceClientsBox>) {
      pybind11::implicitly_convertible<Clients, ServiceClientsBox>();
      GetExportedServiceClientsBox().def(
        pybind11::init<std::shared_ptr<Clients>>());
    }
    return clients;
  }
}

  template<typename Clients>
  ToPythonServiceClients(Clients&&) ->
    ToPythonServiceClients<std::decay_t<Clients>>;

  template<typename C>
  template<typename... Args, typename>
  ToPythonServiceClients<C>::ToPythonServiceClients(Args&&... args)
    : m_clients((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...),
      m_serviceLocatorClient(boost::in_place_init,
        std::in_place_type<Beam::ServiceLocator::ToPythonServiceLocatorClient<
          Beam::ServiceLocator::ServiceLocatorClientBox>>,
          &m_clients->GetServiceLocatorClient()),
      m_registryClient(boost::in_place_init,
        std::in_place_type<Beam::RegistryService::ToPythonRegistryClient<
          Beam::RegistryService::RegistryClientBox>>,
          &m_clients->GetRegistryClient()),
      m_administrationClient(boost::in_place_init,
        std::in_place_type<AdministrationService::ToPythonAdministrationClient<
          AdministrationService::AdministrationClientBox>>,
          &m_clients->GetAdministrationClient()),
      m_definitionsClient(boost::in_place_init,
        std::in_place_type<DefinitionsService::ToPythonDefinitionsClient<
          DefinitionsService::DefinitionsClientBox>>,
          &m_clients->GetDefinitionsClient()),
      m_marketDataClient(boost::in_place_init,
        std::in_place_type<MarketDataService::ToPythonMarketDataClient<
          MarketDataService::MarketDataClientBox>>,
          &m_clients->GetMarketDataClient()),
      m_chartingClient(boost::in_place_init,
        std::in_place_type<ChartingService::ToPythonChartingClient<
          ChartingService::ChartingClientBox>>,
        &m_clients->GetChartingClient()),
      m_complianceClient(boost::in_place_init,
        std::in_place_type<Compliance::ToPythonComplianceClient<
          Compliance::ComplianceClientBox>>, &m_clients->GetComplianceClient()),
      m_orderExecutionClient(boost::in_place_init,
        std::in_place_type<
          OrderExecutionService::ToPythonOrderExecutionClient<
            OrderExecutionService::OrderExecutionClientBox>>,
        &m_clients->GetOrderExecutionClient()),
      m_riskClient(boost::in_place_init,
        std::in_place_type<
          RiskService::ToPythonRiskClient<RiskService::RiskClientBox>>,
        &m_clients->GetRiskClient()),
      m_timeClient(Beam::TimeService::ToPythonTimeClient<
        Beam::TimeService::TimeClientBox>(&m_clients->GetTimeClient())) {}

  template<typename C>
  ToPythonServiceClients<C>::~ToPythonServiceClients() {
    auto release = Beam::Python::GilRelease();
    m_timeClient.reset();
    m_riskClient.reset();
    m_orderExecutionClient.reset();
    m_complianceClient.reset();
    m_chartingClient.reset();
    m_marketDataClient.reset();
    m_definitionsClient.reset();
    m_administrationClient.reset();
    m_registryClient.reset();
    m_serviceLocatorClient.reset();
    m_clients.reset();
    m_openState.Close();
  }

  template<typename C>
  const typename ToPythonServiceClients<C>::Clients&
      ToPythonServiceClients<C>::GetClients() const {
    return *m_clients;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::Clients&
      ToPythonServiceClients<C>::GetClients() {
    return *m_clients;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::ServiceLocatorClient&
      ToPythonServiceClients<C>::GetServiceLocatorClient() {
    return *m_serviceLocatorClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::RegistryClient&
      ToPythonServiceClients<C>::GetRegistryClient() {
    return *m_registryClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::AdministrationClient&
      ToPythonServiceClients<C>::GetAdministrationClient() {
    return *m_administrationClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::DefinitionsClient&
      ToPythonServiceClients<C>::GetDefinitionsClient() {
    return *m_definitionsClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::MarketDataClient&
      ToPythonServiceClients<C>::GetMarketDataClient() {
    return *m_marketDataClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::ChartingClient&
      ToPythonServiceClients<C>::GetChartingClient() {
    return *m_chartingClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::ComplianceClient&
      ToPythonServiceClients<C>::GetComplianceClient() {
    return *m_complianceClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::OrderExecutionClient&
      ToPythonServiceClients<C>::GetOrderExecutionClient() {
    return *m_orderExecutionClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::RiskClient&
      ToPythonServiceClients<C>::GetRiskClient() {
    return *m_riskClient;
  }

  template<typename C>
  typename ToPythonServiceClients<C>::TimeClient&
      ToPythonServiceClients<C>::GetTimeClient() {
    return *m_timeClient;
  }

  template<typename C>
  std::unique_ptr<typename ToPythonServiceClients<C>::Timer>
      ToPythonServiceClients<C>::MakeTimer(
      boost::posix_time::time_duration expiry) {
    auto release = Beam::Python::GilRelease();
    return std::make_unique<Timer>(std::in_place_type<
      Beam::Threading::ToPythonTimer<Beam::Threading::TimerBox>>,
      Beam::Threading::TimerBox(m_clients->MakeTimer(expiry)));
  }

  template<typename C>
  void ToPythonServiceClients<C>::Close() {
    auto release = Beam::Python::GilRelease();
    if(m_openState.SetClosing()) {
      return;
    }
    m_timeClient->Close();
    m_serviceLocatorClient->Close();
    m_riskClient->Close();
    m_orderExecutionClient->Close();
    m_complianceClient->Close();
    m_chartingClient->Close();
    m_marketDataClient->Close();
    m_definitionsClient->Close();
    m_administrationClient->Close();
    m_registryClient->Close();
    m_clients->Close();
    m_openState.Close();
  }
}

#endif
