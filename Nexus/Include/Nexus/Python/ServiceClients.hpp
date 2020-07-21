#ifndef NEXUS_PYTHON_SERVICE_CLIENTS_HPP
#define NEXUS_PYTHON_SERVICE_CLIENTS_HPP
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/ToPythonServiceLocatorClient.hpp>
#include <Beam/Python/ToPythonTimeClient.hpp>
#include <Beam/Python/ToPythonTimer.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/Python/AdministrationClient.hpp"
#include "Nexus/Python/ComplianceClient.hpp"
#include "Nexus/Python/DefinitionsClient.hpp"
#include "Nexus/Python/MarketDataClient.hpp"
#include "Nexus/Python/OrderExecutionClient.hpp"
#include "Nexus/Python/RiskClient.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"

namespace Nexus {
namespace Python {

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
   * Exports the TestTimeClient class.
   * @param module The module to export to.
   */
  void ExportTestTimeClient(pybind11::module& module);

  /**
   * Exports the TestTimer class.
   * @param module The module to export to.
   */
  void ExportTestTimer(pybind11::module& module);

  /**
   * Exports the VirtualServiceClients class.
   * @param module The module to export to.
   */
  void ExportVirtualServiceClients(pybind11::module& module);
}

  /**
   * Wraps a ServiceClients instance for use within Python.
   * @param <C> The type of ServiceClients to wrap.
   */
  template<typename C>
  class ToPythonServiceClients : public VirtualServiceClients {
    public:

      /** The type of ServiceClients to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonServiceClients instance.
       * @param client The ServiceClients to wrap.
       */
      ToPythonServiceClients(std::unique_ptr<Client> client);

      ~ToPythonServiceClients() override;

      ServiceLocatorClient& GetServiceLocatorClient() override final;

      RegistryClient& GetRegistryClient() override final;

      AdministrationClient& GetAdministrationClient() override final;

      DefinitionsClient& GetDefinitionsClient() override final;

      MarketDataClient& GetMarketDataClient() override final;

      ChartingClient& GetChartingClient() override final;

      ComplianceClient& GetComplianceClient() override final;

      OrderExecutionClient& GetOrderExecutionClient() override final;

      RiskClient& GetRiskClient() override final;

      TimeClient& GetTimeClient() override final;

      std::unique_ptr<Timer> BuildTimer(
        boost::posix_time::time_duration expiry);

      void Open() override;

      void Close() override;

    private:
      std::unique_ptr<Client> m_client;
      std::unique_ptr<ServiceLocatorClient> m_serviceLocatorClient;
      std::unique_ptr<AdministrationClient> m_administrationClient;
      std::unique_ptr<DefinitionsClient> m_definitionsClient;
      std::unique_ptr<MarketDataClient> m_marketDataClient;
      std::unique_ptr<ComplianceClient> m_complianceClient;
      std::unique_ptr<OrderExecutionClient> m_orderExecutionClient;
      std::unique_ptr<RiskClient> m_riskClient;
      std::unique_ptr<TimeClient> m_timeClient;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  /**
   * Builds a ToPythonServiceClients instance.
   * @param client The ServiceClients instance to wrap.
   */
  template<typename Client>
  auto MakeToPythonServiceClients(std::unique_ptr<Client> client) {
    return std::make_shared<ToPythonServiceClients<Client>>(std::move(client));
  }

  template<typename C>
  ToPythonServiceClients<C>::ToPythonServiceClients(
    std::unique_ptr<Client> client)
    : m_client(std::move(client)) {}

  template<typename C>
  ToPythonServiceClients<C>::~ToPythonServiceClients() {
    Close();
    auto release = Beam::Python::GilRelease();
    m_timeClient.reset();
    m_riskClient.reset();
    m_orderExecutionClient.reset();
    m_complianceClient.reset();
    m_marketDataClient.reset();
    m_definitionsClient.reset();
    m_administrationClient.reset();
    m_serviceLocatorClient.reset();
    m_client.reset();
  }

  template<typename C>
  typename ToPythonServiceClients<C>::ServiceLocatorClient&
      ToPythonServiceClients<C>::GetServiceLocatorClient() {
    if(m_openState.IsOpen()) {
      return *m_serviceLocatorClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
  }

  template<typename C>
  typename ToPythonServiceClients<C>::RegistryClient&
      ToPythonServiceClients<C>::GetRegistryClient() {
    throw std::runtime_error{"Not implemented"};
  }

  template<typename C>
  typename ToPythonServiceClients<C>::AdministrationClient&
      ToPythonServiceClients<C>::GetAdministrationClient() {
    if(m_openState.IsOpen()) {
      return *m_administrationClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
  }

  template<typename C>
  typename ToPythonServiceClients<C>::DefinitionsClient&
      ToPythonServiceClients<C>::GetDefinitionsClient() {
    if(m_openState.IsOpen()) {
      return *m_definitionsClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
  }

  template<typename C>
  typename ToPythonServiceClients<C>::MarketDataClient&
      ToPythonServiceClients<C>::GetMarketDataClient() {
    if(m_openState.IsOpen()) {
      return *m_marketDataClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
  }

  template<typename C>
  typename ToPythonServiceClients<C>::ChartingClient&
      ToPythonServiceClients<C>::GetChartingClient() {
    throw std::runtime_error("Not implemented");
  }

  template<typename C>
  typename ToPythonServiceClients<C>::ComplianceClient&
      ToPythonServiceClients<C>::GetComplianceClient() {
    if(m_openState.IsOpen()) {
      return *m_complianceClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
  }

  template<typename C>
  typename ToPythonServiceClients<C>::OrderExecutionClient&
      ToPythonServiceClients<C>::GetOrderExecutionClient() {
    if(m_openState.IsOpen()) {
      return *m_orderExecutionClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
  }

  template<typename C>
  typename ToPythonServiceClients<C>::RiskClient&
      ToPythonServiceClients<C>::GetRiskClient() {
    if(m_openState.IsOpen()) {
      return *m_riskClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
  }

  template<typename C>
  typename ToPythonServiceClients<C>::TimeClient&
      ToPythonServiceClients<C>::GetTimeClient() {
    if(m_openState.IsOpen()) {
      return *m_timeClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
  }

  template<typename C>
  std::unique_ptr<typename ToPythonServiceClients<C>::Timer>
      ToPythonServiceClients<C>::BuildTimer(
      boost::posix_time::time_duration expiry) {
    auto release = Beam::Python::GilRelease();
    return Beam::Threading::MakeVirtualTimer(
      Beam::Threading::MakeToPythonTimer(m_client->BuildTimer(expiry)));
  }

  template<typename C>
  void ToPythonServiceClients<C>::Open() {
    auto release = Beam::Python::GilRelease();
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_client->Open();
      m_serviceLocatorClient =
        Beam::ServiceLocator::MakeToPythonServiceLocatorClient(
        Beam::ServiceLocator::MakeVirtualServiceLocatorClient(
        &m_client->GetServiceLocatorClient()));
      m_serviceLocatorClient->Open();
      m_administrationClient =
        AdministrationService::MakeToPythonAdministrationClient(
        AdministrationService::MakeVirtualAdministrationClient(
        &m_client->GetAdministrationClient()));
      m_administrationClient->Open();
      m_definitionsClient = DefinitionsService::MakeToPythonDefinitionsClient(
        DefinitionsService::MakeVirtualDefinitionsClient(
        &m_client->GetDefinitionsClient()));
      m_definitionsClient->Open();
      m_marketDataClient = MarketDataService::MakeToPythonMarketDataClient(
        MarketDataService::MakeVirtualMarketDataClient(
        &m_client->GetMarketDataClient()));
      m_marketDataClient->Open();
      // TODO: Implement the compliance client.
      m_orderExecutionClient =
        OrderExecutionService::MakeToPythonOrderExecutionClient(
        OrderExecutionService::MakeVirtualOrderExecutionClient(
        &m_client->GetOrderExecutionClient()));
      m_orderExecutionClient->Open();
      m_riskClient = RiskService::MakeToPythonRiskClient(
        RiskService::MakeVirtualRiskClient(&m_client->GetRiskClient()));
      m_riskClient->Open();
      m_timeClient = Beam::TimeService::MakeToPythonTimeClient(
        Beam::TimeService::MakeVirtualTimeClient(&m_client->GetTimeClient()));
      m_timeClient->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C>
  void ToPythonServiceClients<C>::Close() {
    auto release = Beam::Python::GilRelease();
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C>
  void ToPythonServiceClients<C>::Shutdown() {
    if(m_timeClient != nullptr) {
      m_timeClient->Close();
    }
    if(m_riskClient != nullptr) {
      m_riskClient->Close();
    }
    if(m_orderExecutionClient != nullptr) {
      m_orderExecutionClient->Close();
    }
    if(m_complianceClient != nullptr) {
      m_complianceClient->Close();
    }
    if(m_marketDataClient != nullptr) {
      m_marketDataClient->Close();
    }
    if(m_definitionsClient != nullptr) {
      m_definitionsClient->Close();
    }
    if(m_administrationClient != nullptr) {
      m_administrationClient->Close();
    }
    if(m_serviceLocatorClient != nullptr) {
      m_serviceLocatorClient->Close();
    }
    m_client->Close();
    m_openState.SetClosed();
  }
}

#endif
