#ifndef NEXUS_TO_PYTHON_SERVICE_CLIENTS_HPP
#define NEXUS_TO_PYTHON_SERVICE_CLIENTS_HPP
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/ToPythonServiceLocatorClient.hpp>
#include <Beam/Python/ToPythonTimeClient.hpp>
#include <Beam/Python/ToPythonTimer.hpp>
#include "Nexus/Python/ToPythonAdministrationClient.hpp"
#include "Nexus/Python/ToPythonComplianceClient.hpp"
#include "Nexus/Python/ToPythonDefinitionsClient.hpp"
#include "Nexus/Python/ToPythonMarketDataClient.hpp"
#include "Nexus/Python/ToPythonOrderExecutionClient.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"

namespace Nexus {

  /*! \class ToPythonServiceClients
      \brief Wraps a ServiceClients instance for use within Python.
      \tparam ClientType The type of ServiceClients to wrap.
   */
  template<typename ClientType>
  class ToPythonServiceClients : public VirtualServiceClients {
    public:

      //! The type of ServiceClients to wrap.
      using Client = ClientType;

      //! Constructs a ToPythonServiceClients instance.
      /*!
        \param client The ServiceClients to wrap.
      */
      ToPythonServiceClients(std::unique_ptr<Client> client);

      virtual ~ToPythonServiceClients() override;

      virtual ServiceLocatorClient& GetServiceLocatorClient() override final;

      virtual RegistryClient& GetRegistryClient() override final;

      virtual AdministrationClient& GetAdministrationClient() override final;

      virtual DefinitionsClient& GetDefinitionsClient() override final;

      virtual MarketDataClient& GetMarketDataClient() override final;

      virtual ChartingClient& GetChartingClient() override final;

      virtual ComplianceClient& GetComplianceClient() override final;

      virtual OrderExecutionClient& GetOrderExecutionClient() override final;

      virtual RiskClient& GetRiskClient() override final;

      virtual TimeClient& GetTimeClient() override final;

      virtual std::unique_ptr<Timer> BuildTimer(
        boost::posix_time::time_duration expiry);

      virtual void Open() override final;

      virtual void Close() override final;

    private:
      std::unique_ptr<Client> m_client;
      std::unique_ptr<ServiceLocatorClient> m_serviceLocatorClient;
      std::unique_ptr<AdministrationClient> m_administrationClient;
      std::unique_ptr<DefinitionsClient> m_definitionsClient;
      std::unique_ptr<MarketDataClient> m_marketDataClient;
      std::unique_ptr<ComplianceClient> m_complianceClient;
      std::unique_ptr<OrderExecutionClient> m_orderExecutionClient;
      std::unique_ptr<TimeClient> m_timeClient;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  //! Builds a ToPythonServiceClients instance.
  /*!
    \param client The ServiceClients instance to wrap.
  */
  template<typename Client>
  auto MakeToPythonServiceClients(std::unique_ptr<Client> client) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return std::make_shared<ToPythonServiceClients<Client>>(std::move(client));
  }

  template<typename ClientType>
  ToPythonServiceClients<ClientType>::ToPythonServiceClients(
      std::unique_ptr<Client> client)
      : m_client{std::move(client)} {}

  template<typename ClientType>
  ToPythonServiceClients<ClientType>::~ToPythonServiceClients() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    Close();
    m_timeClient.reset();
    m_orderExecutionClient.reset();
    m_complianceClient.reset();
    m_marketDataClient.reset();
    m_definitionsClient.reset();
    m_administrationClient.reset();
    m_serviceLocatorClient.reset();
    m_client.reset();
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::ServiceLocatorClient&
      ToPythonServiceClients<ClientType>::GetServiceLocatorClient() {
    if(m_openState.IsOpen()) {
      return *m_serviceLocatorClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::RegistryClient&
      ToPythonServiceClients<ClientType>::GetRegistryClient() {
    throw std::runtime_error{"Not implemented"};
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::AdministrationClient&
      ToPythonServiceClients<ClientType>::GetAdministrationClient() {
    if(m_openState.IsOpen()) {
      return *m_administrationClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::DefinitionsClient&
      ToPythonServiceClients<ClientType>::GetDefinitionsClient() {
    if(m_openState.IsOpen()) {
      return *m_definitionsClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::MarketDataClient&
      ToPythonServiceClients<ClientType>::GetMarketDataClient() {
    if(m_openState.IsOpen()) {
      return *m_marketDataClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::ChartingClient&
      ToPythonServiceClients<ClientType>::GetChartingClient() {
    throw std::runtime_error{"Not implemented"};
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::ComplianceClient&
      ToPythonServiceClients<ClientType>::GetComplianceClient() {
    if(m_openState.IsOpen()) {
      return *m_complianceClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::OrderExecutionClient&
      ToPythonServiceClients<ClientType>::GetOrderExecutionClient() {
    if(m_openState.IsOpen()) {
      return *m_orderExecutionClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::RiskClient&
      ToPythonServiceClients<ClientType>::GetRiskClient() {
    throw std::runtime_error{"Not implemented"};
  }

  template<typename ClientType>
  typename ToPythonServiceClients<ClientType>::TimeClient&
      ToPythonServiceClients<ClientType>::GetTimeClient() {
    if(m_openState.IsOpen()) {
      return *m_timeClient;
    }
    BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
  }

  template<typename ClientType>
  std::unique_ptr<typename ToPythonServiceClients<ClientType>::Timer>
      ToPythonServiceClients<ClientType>::BuildTimer(
      boost::posix_time::time_duration expiry) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return Beam::Threading::MakeVirtualTimer(
      Beam::Threading::MakeToPythonTimer(m_client->BuildTimer(expiry)));
  }

  template<typename ClientType>
  void ToPythonServiceClients<ClientType>::Open() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
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
      m_complianceClient = Compliance::MakeToPythonComplianceClient(
        Compliance::MakeVirtualComplianceClient(
        &m_client->GetComplianceClient()));
      m_complianceClient->Open();
      m_orderExecutionClient =
        OrderExecutionService::MakeToPythonOrderExecutionClient(
        OrderExecutionService::MakeVirtualOrderExecutionClient(
        &m_client->GetOrderExecutionClient()));
      m_orderExecutionClient->Open();
      m_timeClient = Beam::TimeService::MakeToPythonTimeClient(
        Beam::TimeService::MakeVirtualTimeClient(
        &m_client->GetTimeClient()));
      m_timeClient->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ClientType>
  void ToPythonServiceClients<ClientType>::Close() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ClientType>
  void ToPythonServiceClients<ClientType>::Shutdown() {
    if(m_timeClient != nullptr) {
      m_timeClient->Close();
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
